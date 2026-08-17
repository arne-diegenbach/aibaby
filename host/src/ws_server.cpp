#include "host/ws_server.h"

#include <arpa/inet.h>
#include <errno.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <poll.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#include <algorithm>
#include <cstdio>
#include <fstream>
#include <sstream>

namespace aibaby_host {
namespace {

// --- SHA-1, needed only for the WebSocket handshake ------------------------

void sha1(const uint8_t* data, size_t len, uint8_t out[20]) {
  uint32_t h[5] = {0x67452301u, 0xEFCDAB89u, 0x98BADCFEu, 0x10325476u, 0xC3D2E1F0u};

  std::vector<uint8_t> msg(data, data + len);
  msg.push_back(0x80);
  while (msg.size() % 64 != 56) msg.push_back(0x00);
  const uint64_t bits = uint64_t(len) * 8;
  for (int i = 7; i >= 0; --i) msg.push_back(uint8_t((bits >> (i * 8)) & 0xFF));

  auto rotl = [](uint32_t v, int k) { return (v << k) | (v >> (32 - k)); };

  for (size_t chunk = 0; chunk < msg.size(); chunk += 64) {
    uint32_t w[80];
    for (int i = 0; i < 16; ++i) {
      w[i] = (uint32_t(msg[chunk + i * 4]) << 24) |
             (uint32_t(msg[chunk + i * 4 + 1]) << 16) |
             (uint32_t(msg[chunk + i * 4 + 2]) << 8) |
             uint32_t(msg[chunk + i * 4 + 3]);
    }
    for (int i = 16; i < 80; ++i) {
      w[i] = rotl(w[i - 3] ^ w[i - 8] ^ w[i - 14] ^ w[i - 16], 1);
    }

    uint32_t a = h[0], b = h[1], c = h[2], d = h[3], e = h[4];
    for (int i = 0; i < 80; ++i) {
      uint32_t f, k;
      if (i < 20)      { f = (b & c) | (~b & d);          k = 0x5A827999u; }
      else if (i < 40) { f = b ^ c ^ d;                   k = 0x6ED9EBA1u; }
      else if (i < 60) { f = (b & c) | (b & d) | (c & d); k = 0x8F1BBCDCu; }
      else             { f = b ^ c ^ d;                   k = 0xCA62C1D6u; }
      const uint32_t tmp = rotl(a, 5) + f + e + k + w[i];
      e = d; d = c; c = rotl(b, 30); b = a; a = tmp;
    }
    h[0] += a; h[1] += b; h[2] += c; h[3] += d; h[4] += e;
  }

  for (int i = 0; i < 5; ++i) {
    out[i * 4]     = uint8_t((h[i] >> 24) & 0xFF);
    out[i * 4 + 1] = uint8_t((h[i] >> 16) & 0xFF);
    out[i * 4 + 2] = uint8_t((h[i] >> 8) & 0xFF);
    out[i * 4 + 3] = uint8_t(h[i] & 0xFF);
  }
}

std::string base64(const uint8_t* data, size_t len) {
  static const char* kAlphabet =
      "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
  std::string out;
  out.reserve(((len + 2) / 3) * 4);
  for (size_t i = 0; i < len; i += 3) {
    const uint32_t a = data[i];
    const uint32_t b = (i + 1 < len) ? data[i + 1] : 0;
    const uint32_t c = (i + 2 < len) ? data[i + 2] : 0;
    const uint32_t triple = (a << 16) | (b << 8) | c;
    out.push_back(kAlphabet[(triple >> 18) & 0x3F]);
    out.push_back(kAlphabet[(triple >> 12) & 0x3F]);
    out.push_back(i + 1 < len ? kAlphabet[(triple >> 6) & 0x3F] : '=');
    out.push_back(i + 2 < len ? kAlphabet[triple & 0x3F] : '=');
  }
  return out;
}

std::string lower(std::string s) {
  std::transform(s.begin(), s.end(), s.begin(),
                 [](unsigned char c) { return char(::tolower(c)); });
  return s;
}

// Reads a header value from a raw HTTP request, case-insensitively.
std::string header_value(const std::string& request, const std::string& name) {
  const std::string haystack = lower(request);
  const std::string needle = lower(name) + ":";
  size_t pos = haystack.find("\r\n" + needle);
  if (pos == std::string::npos) return "";
  pos += 2 + needle.size();
  size_t end = request.find("\r\n", pos);
  if (end == std::string::npos) return "";
  std::string value = request.substr(pos, end - pos);
  size_t b = value.find_first_not_of(" \t");
  if (b == std::string::npos) return "";
  size_t e = value.find_last_not_of(" \t");
  return value.substr(b, e - b + 1);
}

const char* content_type_for(const std::string& path) {
  auto ends_with = [&path](const char* suffix) {
    const size_t n = strlen(suffix);
    return path.size() >= n && path.compare(path.size() - n, n, suffix) == 0;
  };
  if (ends_with(".html")) return "text/html; charset=utf-8";
  if (ends_with(".js")) return "application/javascript; charset=utf-8";
  if (ends_with(".css")) return "text/css; charset=utf-8";
  if (ends_with(".json")) return "application/json";
  if (ends_with(".svg")) return "image/svg+xml";
  return "application/octet-stream";
}

void set_nonblocking(int fd) {
  const int flags = fcntl(fd, F_GETFL, 0);
  fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

}  // namespace

WsServer::~WsServer() { stop(); }

bool WsServer::start(uint16_t port, const std::string& web_root, std::string& error) {
  web_root_ = web_root;

  listen_fd_ = ::socket(AF_INET, SOCK_STREAM, 0);
  if (listen_fd_ < 0) {
    error = std::string("socket() failed: ") + strerror(errno);
    return false;
  }

  int one = 1;
  ::setsockopt(listen_fd_, SOL_SOCKET, SO_REUSEADDR, &one, sizeof(one));

  sockaddr_in addr{};
  addr.sin_family = AF_INET;
  addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);  // localhost only
  addr.sin_port = htons(port);

  if (::bind(listen_fd_, reinterpret_cast<sockaddr*>(&addr), sizeof(addr)) < 0) {
    error = "bind() to port " + std::to_string(port) + " failed: " + strerror(errno);
    ::close(listen_fd_);
    listen_fd_ = -1;
    return false;
  }
  if (::listen(listen_fd_, 8) < 0) {
    error = std::string("listen() failed: ") + strerror(errno);
    ::close(listen_fd_);
    listen_fd_ = -1;
    return false;
  }

  set_nonblocking(listen_fd_);
  return true;
}

void WsServer::stop() {
  for (Client& c : clients_) {
    if (c.fd >= 0) ::close(c.fd);
  }
  clients_.clear();
  if (listen_fd_ >= 0) {
    ::close(listen_fd_);
    listen_fd_ = -1;
  }
}

bool WsServer::has_clients() const {
  for (const Client& c : clients_) {
    if (c.websocket) return true;
  }
  return false;
}

void WsServer::drop(size_t index) {
  if (clients_[index].fd >= 0) ::close(clients_[index].fd);
  clients_.erase(clients_.begin() + long(index));
}

void WsServer::accept_pending() {
  for (;;) {
    const int fd = ::accept(listen_fd_, nullptr, nullptr);
    if (fd < 0) return;  // EAGAIN: nothing more to accept
    set_nonblocking(fd);
    int one = 1;
    // Telemetry frames are small and latency matters more than packing.
    ::setsockopt(fd, IPPROTO_TCP, TCP_NODELAY, &one, sizeof(one));
    Client c;
    c.fd = fd;
    clients_.push_back(std::move(c));
  }
}

void WsServer::poll() {
  if (listen_fd_ < 0) return;

  std::vector<pollfd> fds;
  fds.reserve(clients_.size() + 1);
  fds.push_back(pollfd{listen_fd_, POLLIN, 0});
  for (const Client& c : clients_) {
    short events = POLLIN;
    if (!c.out.empty()) events |= POLLOUT;
    fds.push_back(pollfd{c.fd, events, 0});
  }

  if (::poll(fds.data(), nfds_t(fds.size()), 0) <= 0) return;

  if (fds[0].revents & POLLIN) accept_pending();

  // Iterate backwards so dropping a client does not disturb pending indices.
  for (size_t i = clients_.size(); i-- > 0;) {
    // clients_ may have grown in accept_pending(); those have no pollfd yet.
    if (i + 1 >= fds.size()) continue;
    Client& c = clients_[i];
    const short revents = fds[i + 1].revents;

    if (revents & (POLLERR | POLLHUP | POLLNVAL)) {
      drop(i);
      continue;
    }

    if (revents & POLLOUT) {
      const ssize_t n = ::send(c.fd, c.out.data(), c.out.size(), MSG_NOSIGNAL);
      if (n > 0) c.out.erase(0, size_t(n));
      else if (n < 0 && errno != EAGAIN && errno != EWOULDBLOCK) {
        drop(i);
        continue;
      }
      if (c.out.empty() && c.close_after_write) {
        drop(i);
        continue;
      }
    }

    if (revents & POLLIN) {
      handle_readable(c);
      if (c.fd < 0) {
        drop(i);
        continue;
      }
    }

    if (c.out.size() > kMaxOutBytes) drop(i);
  }
}

void WsServer::handle_readable(Client& c) {
  char buffer[8192];
  for (;;) {
    const ssize_t n = ::recv(c.fd, buffer, sizeof(buffer), 0);
    if (n > 0) {
      c.in.append(buffer, size_t(n));
      continue;
    }
    if (n == 0) {  // peer closed
      ::close(c.fd);
      c.fd = -1;
      return;
    }
    if (errno == EAGAIN || errno == EWOULDBLOCK) break;
    ::close(c.fd);
    c.fd = -1;
    return;
  }

  if (c.websocket) process_ws_frames(c);
  else process_http(c);
}

void WsServer::process_http(Client& c) {
  const size_t end = c.in.find("\r\n\r\n");
  if (end == std::string::npos) return;  // headers still arriving

  const std::string request = c.in.substr(0, end + 4);
  c.in.erase(0, end + 4);

  const std::string key = header_value(request, "Sec-WebSocket-Key");
  const std::string upgrade = lower(header_value(request, "Upgrade"));

  if (upgrade == "websocket" && !key.empty()) {
    static const char* kGuid = "258EAFA5-E914-47DA-95CA-C5AB0DC85B11";
    const std::string combined = key + kGuid;
    uint8_t digest[20];
    sha1(reinterpret_cast<const uint8_t*>(combined.data()), combined.size(), digest);

    std::ostringstream response;
    response << "HTTP/1.1 101 Switching Protocols\r\n"
             << "Upgrade: websocket\r\n"
             << "Connection: Upgrade\r\n"
             << "Sec-WebSocket-Accept: " << base64(digest, 20) << "\r\n\r\n";
    c.out += response.str();
    c.websocket = true;
    return;
  }

  // Plain HTTP: pull the path out of the request line.
  std::string path = "/";
  const size_t sp1 = request.find(' ');
  const size_t sp2 = request.find(' ', sp1 + 1);
  if (sp1 != std::string::npos && sp2 != std::string::npos) {
    path = request.substr(sp1 + 1, sp2 - sp1 - 1);
  }
  const size_t query = path.find('?');
  if (query != std::string::npos) path.resize(query);
  if (path == "/") path = "/index.html";
  serve_file(c, path);
}

void WsServer::serve_file(Client& c, const std::string& path) {
  c.close_after_write = true;

  // Reject traversal outright rather than trying to normalise it.
  if (path.find("..") != std::string::npos) {
    c.out += "HTTP/1.1 403 Forbidden\r\nContent-Length: 0\r\nConnection: close\r\n\r\n";
    return;
  }

  const std::string full = web_root_ + path;
  std::ifstream file(full, std::ios::binary);
  if (!file) {
    const std::string body = "not found: " + path;
    std::ostringstream out;
    out << "HTTP/1.1 404 Not Found\r\nContent-Type: text/plain\r\nContent-Length: "
        << body.size() << "\r\nConnection: close\r\n\r\n" << body;
    c.out += out.str();
    return;
  }

  std::ostringstream contents;
  contents << file.rdbuf();
  const std::string body = contents.str();

  std::ostringstream out;
  out << "HTTP/1.1 200 OK\r\nContent-Type: " << content_type_for(path)
      << "\r\nContent-Length: " << body.size()
      << "\r\nCache-Control: no-store\r\nConnection: close\r\n\r\n" << body;
  c.out += out.str();
}

void WsServer::process_ws_frames(Client& c) {
  for (;;) {
    if (c.in.size() < 2) return;

    const uint8_t b0 = uint8_t(c.in[0]);
    const uint8_t b1 = uint8_t(c.in[1]);
    const uint8_t opcode = b0 & 0x0F;
    const bool masked = (b1 & 0x80) != 0;
    uint64_t length = b1 & 0x7F;
    size_t offset = 2;

    if (length == 126) {
      if (c.in.size() < offset + 2) return;
      length = (uint64_t(uint8_t(c.in[offset])) << 8) | uint8_t(c.in[offset + 1]);
      offset += 2;
    } else if (length == 127) {
      if (c.in.size() < offset + 8) return;
      length = 0;
      for (int i = 0; i < 8; ++i) {
        length = (length << 8) | uint8_t(c.in[offset + size_t(i)]);
      }
      offset += 8;
    }

    uint8_t mask[4] = {0, 0, 0, 0};
    if (masked) {
      if (c.in.size() < offset + 4) return;
      for (int i = 0; i < 4; ++i) mask[i] = uint8_t(c.in[offset + size_t(i)]);
      offset += 4;
    }

    if (c.in.size() < offset + length) return;  // frame still arriving

    std::string payload = c.in.substr(offset, size_t(length));
    if (masked) {
      for (size_t i = 0; i < payload.size(); ++i) {
        payload[i] = char(uint8_t(payload[i]) ^ mask[i % 4]);
      }
    }
    c.in.erase(0, offset + size_t(length));

    switch (opcode) {
      case 0x1:  // text
        if (on_message) on_message(payload);
        break;
      case 0x2:  // binary
        if (on_binary && !payload.empty()) {
          on_binary(reinterpret_cast<const uint8_t*>(payload.data()), payload.size());
        }
        break;
      case 0x8:  // close
        queue_frame(c, 0x8, "");
        c.close_after_write = true;
        return;
      case 0x9:  // ping
        queue_frame(c, 0xA, payload);
        break;
      default:
        break;  // continuation frames: browsers do not send them for our sizes
    }
  }
}

void WsServer::queue_frame(Client& c, uint8_t opcode, const std::string& payload) {
  std::string frame;
  frame.push_back(char(0x80 | opcode));  // FIN

  const size_t n = payload.size();
  if (n < 126) {
    frame.push_back(char(n));
  } else if (n <= 0xFFFF) {
    frame.push_back(char(126));
    frame.push_back(char((n >> 8) & 0xFF));
    frame.push_back(char(n & 0xFF));
  } else {
    frame.push_back(char(127));
    for (int i = 7; i >= 0; --i) frame.push_back(char((n >> (i * 8)) & 0xFF));
  }
  frame += payload;  // server-to-client frames are never masked
  c.out += frame;
}

void WsServer::broadcast_text(const std::string& payload) {
  for (Client& c : clients_) {
    if (c.websocket) queue_frame(c, 0x1, payload);
  }
}

void WsServer::broadcast_binary(const void* data, size_t size) {
  const std::string payload(static_cast<const char*>(data), size);
  for (Client& c : clients_) {
    if (c.websocket) queue_frame(c, 0x2, payload);
  }
}

}  // namespace aibaby_host
