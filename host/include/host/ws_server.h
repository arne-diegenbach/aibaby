// Dependency-free HTTP + WebSocket server for the telemetry link.
//
// Serves the static panel and upgrades /ws to a WebSocket on the same port,
// so the whole UI is one `http://localhost:<port>` away with nothing to
// install. Single-threaded and non-blocking: it is polled from the same loop
// that ticks the brain, so telemetry can never race the simulation.

#ifndef AIBABY_HOST_WS_SERVER_H
#define AIBABY_HOST_WS_SERVER_H

#include <cstdint>
#include <functional>
#include <string>
#include <vector>

namespace aibaby_host {

class WsServer {
 public:
  ~WsServer();

  bool start(uint16_t port, const std::string& web_root, std::string& error);
  void stop();

  // Accepts connections, drains reads, flushes writes. Never blocks.
  void poll();

  // Broadcasts to every upgraded client. Slow clients are dropped rather than
  // allowed to stall the simulation loop.
  void broadcast_text(const std::string& payload);
  void broadcast_binary(const void* data, size_t size);

  bool has_clients() const;

  // Text frames are commands (JSON); binary frames are sensory data. Keeping
  // them apart means the mic stream never has to be parsed to find out it is
  // not a button press.
  std::function<void(const std::string&)> on_message;
  std::function<void(const uint8_t*, size_t)> on_binary;

 private:
  struct Client {
    int fd = -1;
    bool websocket = false;
    std::string in;
    std::string out;
    bool close_after_write = false;
  };

  void accept_pending();
  void handle_readable(Client& c);
  void process_http(Client& c);
  void process_ws_frames(Client& c);
  void serve_file(Client& c, const std::string& path);
  void queue_frame(Client& c, uint8_t opcode, const std::string& payload);
  void drop(size_t index);

  int listen_fd_ = -1;
  std::string web_root_;
  std::vector<Client> clients_;

  // A client whose outbound buffer passes this is not keeping up. Telemetry
  // is lossy by nature, so we disconnect rather than grow without bound.
  static constexpr size_t kMaxOutBytes = 4u << 20;
};

}  // namespace aibaby_host

#endif  // AIBABY_HOST_WS_SERVER_H
