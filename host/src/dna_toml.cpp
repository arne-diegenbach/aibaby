#include "host/dna_toml.h"

#include <cstdlib>
#include <cstring>
#include <fstream>
#include <map>
#include <sstream>

#include "aibaby/dna.h"

namespace aibaby_host {
namespace {

// --- Minimal TOML subset ---------------------------------------------------
// Supports: comments, [table], [[array of tables]], and key = value where
// value is a number, quoted string, bool, or single-line numeric array.
// That is the whole genome format; anything more is scope we do not need.

struct Value {
  enum class Kind { kNumber, kString, kArray };
  Kind kind = Kind::kNumber;
  double number = 0.0;
  std::string text;
  std::vector<double> array;
};

struct Table {
  std::string name;
  std::map<std::string, Value> kv;
};

std::string trim(const std::string& s) {
  size_t b = s.find_first_not_of(" \t\r\n");
  if (b == std::string::npos) return "";
  size_t e = s.find_last_not_of(" \t\r\n");
  return s.substr(b, e - b + 1);
}

// Strips a trailing comment, respecting quotes so a '#' inside a string
// survives.
std::string strip_comment(const std::string& line) {
  bool in_quotes = false;
  for (size_t i = 0; i < line.size(); ++i) {
    if (line[i] == '"') in_quotes = !in_quotes;
    else if (line[i] == '#' && !in_quotes) return line.substr(0, i);
  }
  return line;
}

bool parse_value(const std::string& raw, Value& out, std::string& error) {
  const std::string s = trim(raw);
  if (s.empty()) { error = "empty value"; return false; }

  if (s.front() == '"') {
    size_t end = s.find('"', 1);
    if (end == std::string::npos) { error = "unterminated string"; return false; }
    out.kind = Value::Kind::kString;
    out.text = s.substr(1, end - 1);
    return true;
  }

  if (s.front() == '[') {
    size_t end = s.rfind(']');
    if (end == std::string::npos) { error = "unterminated array"; return false; }
    out.kind = Value::Kind::kArray;
    std::stringstream ss(s.substr(1, end - 1));
    std::string item;
    while (std::getline(ss, item, ',')) {
      const std::string t = trim(item);
      if (t.empty()) continue;
      out.array.push_back(std::strtod(t.c_str(), nullptr));
    }
    return true;
  }

  out.kind = Value::Kind::kNumber;
  if (s == "true") { out.number = 1.0; return true; }
  if (s == "false") { out.number = 0.0; return true; }

  const char* begin = s.c_str();
  char* end = nullptr;
  if (s.size() > 2 && s[0] == '0' && (s[1] == 'x' || s[1] == 'X')) {
    out.number = double(std::strtoull(begin + 2, &end, 16));
  } else {
    out.number = std::strtod(begin, &end);
  }
  if (end == begin) { error = "not a number: '" + s + "'"; return false; }
  return true;
}

bool parse_toml(const std::string& path, std::vector<Table>& tables, std::string& error) {
  std::ifstream file(path);
  if (!file) { error = "cannot open " + path; return false; }

  tables.clear();
  tables.push_back(Table{"", {}});  // top level

  std::string line;
  int line_no = 0;
  while (std::getline(file, line)) {
    ++line_no;
    const std::string s = trim(strip_comment(line));
    if (s.empty()) continue;

    auto fail = [&](const std::string& msg) {
      error = path + ":" + std::to_string(line_no) + ": " + msg;
      return false;
    };

    if (s.size() > 3 && s[0] == '[' && s[1] == '[') {
      size_t end = s.find("]]");
      if (end == std::string::npos) return fail("unterminated [[table]]");
      tables.push_back(Table{trim(s.substr(2, end - 2)), {}});
      continue;
    }
    if (s.front() == '[') {
      size_t end = s.find(']');
      if (end == std::string::npos) return fail("unterminated [table]");
      tables.push_back(Table{trim(s.substr(1, end - 1)), {}});
      continue;
    }

    size_t eq = s.find('=');
    if (eq == std::string::npos) return fail("expected 'key = value'");
    const std::string key = trim(s.substr(0, eq));
    Value value;
    std::string verr;
    if (!parse_value(s.substr(eq + 1), value, verr)) return fail(verr);
    tables.back().kv[key] = value;
  }
  return true;
}

// --- TOML -> binary --------------------------------------------------------

class Reader {
 public:
  Reader(const Table& table, const std::string& context)
      : table_(table), context_(context) {}

  // Every genome field is required. Silent defaults would let two DNA files
  // that look different produce the same brain, or worse, the reverse — and
  // reproducibility (G1) is the one property we cannot debug our way back to.
  double number(const char* key) {
    auto it = table_.kv.find(key);
    if (it == table_.kv.end()) { miss(key); return 0.0; }
    return it->second.number;
  }

  std::string text(const char* key) {
    auto it = table_.kv.find(key);
    if (it == table_.kv.end() || it->second.kind != Value::Kind::kString) {
      miss(key);
      return "";
    }
    return it->second.text;
  }

  void vec3(const char* key, float* out) {
    auto it = table_.kv.find(key);
    if (it == table_.kv.end() || it->second.array.size() != 3) {
      errors_.push_back(context_ + ": '" + key + "' must be an array of 3 numbers");
      return;
    }
    for (int i = 0; i < 3; ++i) out[i] = float(it->second.array[size_t(i)]);
  }

  const std::vector<std::string>& errors() const { return errors_; }

 private:
  void miss(const char* key) {
    errors_.push_back(context_ + ": missing required key '" + std::string(key) + "'");
  }

  const Table& table_;
  std::string context_;
  std::vector<std::string> errors_;
};

void append(std::vector<uint8_t>& out, const void* data, size_t size) {
  const uint8_t* p = static_cast<const uint8_t*>(data);
  out.insert(out.end(), p, p + size);
}

// Roles are spelled out in the genome rather than inferred from the module
// name, so that renaming a module is cosmetic and rewiring it to the outside
// world is not.
bool parse_role(const std::string& text, uint32_t& out) {
  static const struct { const char* name; aibaby::ModuleRole role; } kRoles[] = {
      {"association", aibaby::ModuleRole::kAssociation},
      {"auditory", aibaby::ModuleRole::kAuditory},
      {"vision", aibaby::ModuleRole::kVision},
      {"somato", aibaby::ModuleRole::kSomato},
      {"vocal", aibaby::ModuleRole::kVocal},
      {"expression", aibaby::ModuleRole::kExpression},
      {"visual_cortex", aibaby::ModuleRole::kVisualCortex},
      {"visual_form", aibaby::ModuleRole::kVisualForm},
      {"hippocampus", aibaby::ModuleRole::kHippocampus},
      {"interneuron", aibaby::ModuleRole::kInterneuron},
      {"context", aibaby::ModuleRole::kContext},
  };
  for (const auto& entry : kRoles) {
    if (text == entry.name) {
      out = uint32_t(entry.role);
      return true;
    }
  }
  return false;
}

bool parse_projection_source(const std::string& text, uint32_t& out) {
  if (text == "either") { out = uint32_t(aibaby::ProjectionSource::kEither); return true; }
  if (text == "excitatory") {
    out = uint32_t(aibaby::ProjectionSource::kExcitatory);
    return true;
  }
  if (text == "inhibitory") {
    out = uint32_t(aibaby::ProjectionSource::kInhibitory);
    return true;
  }
  return false;
}

bool parse_projection_kind(const std::string& text, uint32_t& out) {
  if (text == "random") { out = uint32_t(aibaby::ProjectionKind::kRandom); return true; }
  if (text == "gabor") { out = uint32_t(aibaby::ProjectionKind::kGabor); return true; }
  if (text == "curvature") { out = uint32_t(aibaby::ProjectionKind::kCurvature); return true; }
  if (text == "topographic") { out = uint32_t(aibaby::ProjectionKind::kTopographic); return true; }
  return false;
}

}  // namespace

bool compile_dna_toml(const std::string& path, std::vector<uint8_t>& out,
                      std::string& error) {
  std::vector<Table> tables;
  if (!parse_toml(path, tables, error)) return false;

  std::vector<std::string> errors;
  auto collect = [&errors](const Reader& r) {
    for (const auto& e : r.errors()) errors.push_back(e);
  };

  const Table* root = nullptr;
  const Table* sim = nullptr;
  const Table* stdp = nullptr;
  const Table* homeo = nullptr;
  const Table* growth = nullptr;
  const Table* consolidation = nullptr;
  const Table* drives = nullptr;
  const Table* audio = nullptr;
  const Table* vision = nullptr;
  const Table* touch = nullptr;
  const Table* vocal = nullptr;
  const Table* curiosity = nullptr;
  const Table* exploration = nullptr;
  const Table* normalisation = nullptr;
  const Table* neuromod = nullptr;
  const Table* interneuron = nullptr;
  std::vector<const Table*> modules;
  std::vector<const Table*> projections;

  for (const Table& t : tables) {
    if (t.name.empty()) root = &t;
    else if (t.name == "sim") sim = &t;
    else if (t.name == "stdp") stdp = &t;
    else if (t.name == "homeostasis") homeo = &t;
    else if (t.name == "growth") growth = &t;
    else if (t.name == "consolidation") consolidation = &t;
    else if (t.name == "drives") drives = &t;
    else if (t.name == "audio") audio = &t;
    else if (t.name == "vision") vision = &t;
    else if (t.name == "touch") touch = &t;
    else if (t.name == "vocal") vocal = &t;
    else if (t.name == "curiosity") curiosity = &t;
    else if (t.name == "exploration") exploration = &t;
    else if (t.name == "normalisation") normalisation = &t;
    else if (t.name == "neuromod") neuromod = &t;
    else if (t.name == "interneuron") interneuron = &t;
    else if (t.name == "module") modules.push_back(&t);
    else if (t.name == "projection") projections.push_back(&t);
    else errors.push_back("unknown section [" + t.name + "]");
  }

  if (!sim) errors.push_back("missing [sim] section");
  if (!stdp) errors.push_back("missing [stdp] section");
  if (!homeo) errors.push_back("missing [homeostasis] section");
  if (!growth) errors.push_back("missing [growth] section");
  if (!consolidation) errors.push_back("missing [consolidation] section");
  if (!drives) errors.push_back("missing [drives] section");
  if (!audio) errors.push_back("missing [audio] section");
  if (!vision) errors.push_back("missing [vision] section");
  if (!touch) errors.push_back("missing [touch] section");
  if (!vocal) errors.push_back("missing [vocal] section");
  if (!curiosity) errors.push_back("missing [curiosity] section");
  if (!exploration) errors.push_back("missing [exploration] section");
  if (!normalisation) errors.push_back("missing [normalisation] section");
  if (!neuromod) errors.push_back("missing [neuromod] section");
  if (!interneuron) errors.push_back("missing [interneuron] section");
  if (modules.empty()) errors.push_back("genome defines no [[module]]");
  if (modules.size() > aibaby::kMaxModules) errors.push_back("too many modules");
  if (projections.size() > aibaby::kMaxProjections) errors.push_back("too many projections");

  if (!errors.empty()) {
    error = errors.front();
    return false;
  }

  aibaby::DnaHeader header{};
  header.magic = aibaby::kDnaMagic;
  header.version = aibaby::kDnaVersion;
  header.module_count = uint32_t(modules.size());
  header.projection_count = uint32_t(projections.size());

  {
    Reader r(*root, "root");
    header.seed = uint64_t(r.number("seed"));
    collect(r);
  }
  {
    Reader r(*sim, "[sim]");
    header.sim.dt_ms = float(r.number("dt_ms"));
    header.sim.max_delay_ticks = uint32_t(r.number("max_delay_ticks"));
    header.sim.conduction_velocity = float(r.number("conduction_velocity"));
    header.sim.plasticity_interval_ticks =
        uint32_t(r.number("plasticity_interval_ticks"));
    collect(r);
  }
  {
    Reader r(*stdp, "[stdp]");
    header.stdp.a_plus = float(r.number("a_plus"));
    header.stdp.a_minus = float(r.number("a_minus"));
    header.stdp.tau_plus_ms = float(r.number("tau_plus_ms"));
    header.stdp.tau_minus_ms = float(r.number("tau_minus_ms"));
    header.stdp.tau_elig_ms = float(r.number("tau_elig_ms"));
    header.stdp.eta = float(r.number("eta"));
    header.stdp.reward_clip = float(r.number("reward_clip"));
    header.stdp.elig_baseline_tau_ms = float(r.number("elig_baseline_tau_ms"));
    header.stdp.burst_baseline_tau_ms = float(r.number("burst_baseline_tau_ms"));
    header.stdp.elig_pre_centre = float(r.number("elig_pre_centre"));
    collect(r);
  }
  {
    Reader r(*homeo, "[homeostasis]");
    header.homeo.ip_rate = float(r.number("ip_rate"));
    header.homeo.threshold_min = float(r.number("threshold_min"));
    header.homeo.threshold_max = float(r.number("threshold_max"));
    header.homeo.w_max = float(r.number("w_max"));
    header.homeo.scaling_rate = float(r.number("scaling_rate"));
    header.homeo.scaling_band = float(r.number("scaling_band"));
    header.homeo.inhib_plastic = uint32_t(r.number("inhib_plastic"));
    header.homeo.interval_ticks = uint32_t(r.number("interval_ticks"));
    collect(r);
  }
  {
    Reader r(*growth, "[growth]");
    header.growth.enabled = uint32_t(r.number("enabled"));
    header.growth.plateau_window_ticks = uint32_t(r.number("plateau_window_ticks"));
    header.growth.epsilon = float(r.number("epsilon"));
    header.growth.insert_k = uint32_t(r.number("insert_k"));
    header.growth.saturation_rate_hz = float(r.number("saturation_rate_hz"));
    header.growth.saturation_weight = float(r.number("saturation_weight"));
    header.growth.require_saturation = uint32_t(r.number("require_saturation"));
    header.growth.error_floor = float(r.number("error_floor"));
    header.growth.patience = uint32_t(r.number("patience"));
    header.growth.new_weight = float(r.number("new_weight"));
    header.growth.refractory_ticks = uint32_t(r.number("refractory_ticks"));
    collect(r);
  }
  {
    Reader r(*consolidation, "[consolidation]");
    header.consolidate.enabled = uint32_t(r.number("enabled"));
    header.consolidate.traffic_tau_ms = float(r.number("traffic_tau_ms"));
    header.consolidate.traffic_half = float(r.number("traffic_half"));
    header.consolidate.delay_floor_frac = float(r.number("delay_floor_frac"));
    header.consolidate.eta_floor_frac = float(r.number("eta_floor_frac"));
    header.consolidate.interval_ticks = uint32_t(r.number("interval_ticks"));
    header.consolidate.downscale = float(r.number("downscale"));
    header.consolidate.downscale_floor = float(r.number("downscale_floor"));
    header.consolidate.prune_weight = float(r.number("prune_weight"));
    header.consolidate.prune_traffic = float(r.number("prune_traffic"));
    header.consolidate.prune_compete = float(r.number("prune_compete"));
    header.consolidate.prune_compete_min_in = uint32_t(r.number("prune_compete_min_in"));
    header.consolidate.replay_episodes = uint32_t(r.number("replay_episodes"));
    header.consolidate.replay_ticks = uint32_t(r.number("replay_ticks"));
    header.consolidate.replay_threshold = float(r.number("replay_threshold"));
    collect(r);
  }
  {
    Reader r(*drives, "[drives]");
    header.drives.hunger_rate = float(r.number("hunger_rate"));
    header.drives.comfort_decay = float(r.number("comfort_decay"));
    header.drives.fatigue_rate = float(r.number("fatigue_rate"));
    header.drives.curiosity_weight = float(r.number("curiosity_weight"));
    header.drives.w_external = float(r.number("w_external"));
    header.drives.w_hunger = float(r.number("w_hunger"));
    header.drives.w_comfort = float(r.number("w_comfort"));
    header.drives.reward_baseline_tau_ms = float(r.number("reward_baseline_tau_ms"));
    header.drives.fatigue_recovery = float(r.number("fatigue_recovery"));
    header.drives.sleep_threshold = float(r.number("sleep_threshold"));
    header.drives.wake_threshold = float(r.number("wake_threshold"));
    collect(r);
  }
  {
    Reader r(*audio, "[audio]");
    header.audio.sample_rate = uint32_t(r.number("sample_rate"));
    header.audio.window = uint32_t(r.number("window"));
    header.audio.hop = uint32_t(r.number("hop"));
    header.audio.mel_channels = uint32_t(r.number("mel_channels"));
    header.audio.mel_low_hz = float(r.number("mel_low_hz"));
    header.audio.mel_high_hz = float(r.number("mel_high_hz"));
    header.audio.floor_db = float(r.number("floor_db"));
    header.audio.self_gain = float(r.number("self_gain"));
    header.audio.gain = float(r.number("gain"));
    collect(r);
  }
  {
    Reader r(*vision, "[vision]");
    header.vision.frame_size = uint32_t(r.number("frame_size"));
    header.vision.fovea_size = uint32_t(r.number("fovea_size"));
    header.vision.fovea_grid = uint32_t(r.number("fovea_grid"));
    header.vision.ring_grid = uint32_t(r.number("ring_grid"));
    header.vision.center_sigma = float(r.number("center_sigma"));
    header.vision.surround_sigma = float(r.number("surround_sigma"));
    header.vision.contrast_gain = float(r.number("contrast_gain"));
    header.vision.contrast_floor = float(r.number("contrast_floor"));
    header.vision.gaze_contrast_floor = float(r.number("gaze_contrast_floor"));
    header.vision.radial_bins = uint32_t(r.number("radial_bins"));
    header.vision.gain = float(r.number("gain"));
    header.vision.frame_hz = float(r.number("frame_hz"));
    header.vision.latency_ms = float(r.number("latency_ms"));
    header.vision.gaze_rate_hz = float(r.number("gaze_rate_hz"));
    header.vision.gaze_gain = float(r.number("gaze_gain"));
    header.vision.gaze_peak_frac = float(r.number("gaze_peak_frac"));
    header.vision.gaze_peak_radius = float(r.number("gaze_peak_radius"));
    collect(r);
  }
  {
    Reader r(*touch, "[touch]");
    header.touch.gain = float(r.number("gain"));
    header.touch.duration_ms = float(r.number("duration_ms"));
    collect(r);
  }
  {
    Reader r(*vocal, "[vocal]");
    header.vocal.f0_min = float(r.number("f0_min"));
    header.vocal.f0_max = float(r.number("f0_max"));
    header.vocal.f1_min = float(r.number("f1_min"));
    header.vocal.f1_max = float(r.number("f1_max"));
    header.vocal.f2_min = float(r.number("f2_min"));
    header.vocal.f2_max = float(r.number("f2_max"));
    header.vocal.f3_min = float(r.number("f3_min"));
    header.vocal.f3_max = float(r.number("f3_max"));
    header.vocal.bw_min = float(r.number("bw_min"));
    header.vocal.bw_max = float(r.number("bw_max"));
    header.vocal.voicing_threshold = float(r.number("voicing_threshold"));
    header.vocal.smoothing_ms = float(r.number("smoothing_ms"));
    header.vocal.gate_smoothing_ms = float(r.number("gate_smoothing_ms"));
    header.vocal.rate_norm_hz = float(r.number("rate_norm_hz"));
    header.vocal.dictionary_units = uint32_t(r.number("dictionary_units"));
    header.vocal.dictionary_dwell_ms = float(r.number("dictionary_dwell_ms"));
    header.vocal.dictionary_temp = float(r.number("dictionary_temp"));
    collect(r);
  }
  {
    Reader r(*curiosity, "[curiosity]");
    header.curiosity.learn_rate = float(r.number("learn_rate"));
    header.curiosity.fast_tau_ms = float(r.number("fast_tau_ms"));
    header.curiosity.slow_tau_ms = float(r.number("slow_tau_ms"));
    header.curiosity.gain = float(r.number("gain"));
    header.curiosity.predict_gain = float(r.number("predict_gain"));
    header.curiosity.persistence_base = uint32_t(r.number("persistence_base"));
    collect(r);
  }
  {
    Reader r(*exploration, "[exploration]");
    header.exploration.fast_tau_ms = float(r.number("fast_tau_ms"));
    header.exploration.slow_tau_ms = float(r.number("slow_tau_ms"));
    header.exploration.sensitivity = float(r.number("sensitivity"));
    header.exploration.floor = float(r.number("floor"));
    header.exploration.ceiling = float(r.number("ceiling"));
    header.exploration.drive_compensation = float(r.number("drive_compensation"));
    header.exploration.perturb_tau_ms = float(r.number("perturb_tau_ms"));
    header.exploration.perturb_rate = float(r.number("perturb_rate"));
    header.exploration.perturb_max = float(r.number("perturb_max"));
    header.exploration.meta_window = float(r.number("meta_window"));
    header.exploration.meta_floor = float(r.number("meta_floor"));
    header.exploration.meta_ref = float(r.number("meta_ref"));
    header.exploration.meta_commit = float(r.number("meta_commit"));
    header.exploration.meta_flow = float(r.number("meta_flow"));
    header.exploration.meta_ratio = float(r.number("meta_ratio"));
    header.exploration.enabled = uint32_t(r.number("enabled"));
    collect(r);
  }

  {
    Reader r(*normalisation, "[normalisation]");
    header.normalisation.floor = float(r.number("floor"));
    header.normalisation.ceiling = float(r.number("ceiling"));
    header.normalisation.enabled = uint32_t(r.number("enabled"));
    collect(r);
  }  {
    Reader r(*neuromod, "[neuromod]");
    header.neuromod.enabled = uint32_t(r.number("enabled"));
    header.neuromod.pad = 0;
  }
  {
    Reader r(*interneuron, "[interneuron]");
    header.interneuron.tau_ms = float(r.number("tau_ms"));
    header.interneuron.pad = 0;
  }


  std::vector<aibaby::DnaModule> module_blobs;
  std::map<std::string, uint32_t> module_index;

  for (size_t i = 0; i < modules.size(); ++i) {
    Reader r(*modules[i], "[[module]] #" + std::to_string(i));
    aibaby::DnaModule m{};

    const std::string name = r.text("name");
    if (name.size() >= aibaby::kMaxNameLen) {
      errors.push_back("module name too long: " + name);
    } else {
      std::memcpy(m.name, name.c_str(), name.size() + 1);
    }
    if (module_index.count(name)) errors.push_back("duplicate module name: " + name);
    module_index[name] = uint32_t(i);

    const std::string role = r.text("role");
    if (!role.empty() && !parse_role(role, m.role)) {
      errors.push_back("unknown module role '" + role + "' on module '" + name +
                       "' (association|auditory|vision|somato|vocal|expression|visual_cortex)");
    }

    m.neurons = uint32_t(r.number("neurons"));
    m.n_max = uint32_t(r.number("n_max"));
    m.max_out_degree = uint32_t(r.number("max_out_degree"));
    r.vec3("extent", m.extent);
    m.conn_radius = float(r.number("conn_radius"));
    m.conn_density = float(r.number("conn_density"));
    m.chain_weight = float(r.number("chain_weight"));
    m.chain_density = float(r.number("chain_density"));
    m.chain_group = uint32_t(r.number("chain_group"));
    m.chain_delay_ms = float(r.number("chain_delay_ms"));
    m.threshold = float(r.number("threshold"));
    m.v_rest = float(r.number("v_rest"));
    m.leak_tau_ms = float(r.number("leak_tau_ms"));
    m.refractory_ms = float(r.number("refractory_ms"));
    m.target_rate_hz = float(r.number("target_rate_hz"));
    m.inhib_fraction = float(r.number("inhib_fraction"));
    m.inhib_gain = float(r.number("inhib_gain"));
    m.weight_init = float(r.number("weight_init"));
    m.noise_amp = float(r.number("noise_amp"));
    m.ip_wake_scale = float(r.number("ip_wake_scale"));
    m.ip_sleep_scale = float(r.number("ip_sleep_scale"));
    m.syn_wake_scale = float(r.number("syn_wake_scale"));
    m.syn_sleep_scale = float(r.number("syn_sleep_scale"));
    m.explore_scale = float(r.number("explore_scale"));
    m.norm_gain = float(r.number("norm_gain"));
    m.eta_scale = float(r.number("eta_scale"));
    m.nm_external = float(r.number("nm_external"));
    m.nm_hunger = float(r.number("nm_hunger"));
    m.nm_comfort = float(r.number("nm_comfort"));
    m.nm_curiosity = float(r.number("nm_curiosity"));
    m.ffi_source = int32_t(r.number("ffi_source"));
    m.ffi_gain = float(r.number("ffi_gain"));
    m.rebound_source = int32_t(r.number("rebound_source"));
    m.rebound_gain = float(r.number("rebound_gain"));
    m.rebound_mean_tau_ms = float(r.number("rebound_mean_tau_ms"));
    m.ffi_apical = uint32_t(r.number("ffi_apical"));
    m.ffi_learn = float(r.number("ffi_learn"));
    m.apical_tau_ms = float(r.number("apical_tau_ms"));
    m.apical_threshold = float(r.number("apical_threshold"));
    m.apical_gain = float(r.number("apical_gain"));
    m.apical_plateau_ms = float(r.number("apical_plateau_ms"));
    m.theta_hz = float(r.number("theta_hz"));
    m.theta_amp = float(r.number("theta_amp"));
    m.gamma_hz = float(r.number("gamma_hz"));
    m.gamma_amp = float(r.number("gamma_amp"));
    m.gamma_theta_coupling = float(r.number("gamma_theta_coupling"));
    m.critical_tau_ms = float(r.number("critical_tau_ms"));
    m.critical_floor = float(r.number("critical_floor"));
    m.plateau_gate = float(r.number("plateau_gate"));
    m.lateral_gain = float(r.number("lateral_gain"));
    m.lateral_sigma = float(r.number("lateral_sigma"));
    m.lateral_fields = uint32_t(r.number("lateral_fields"));
    m.burst_ms = float(r.number("burst_ms"));
    m.burst_refrac_scale = float(r.number("burst_refrac_scale"));
    m.elig_tau_scale = float(r.number("elig_tau_scale"));

    collect(r);
    module_blobs.push_back(m);
  }

  std::vector<aibaby::DnaProjection> projection_blobs;
  for (size_t i = 0; i < projections.size(); ++i) {
    Reader r(*projections[i], "[[projection]] #" + std::to_string(i));
    aibaby::DnaProjection p{};

    const std::string src = r.text("src");
    const std::string dst = r.text("dst");
    auto si = module_index.find(src);
    auto di = module_index.find(dst);
    if (si == module_index.end()) errors.push_back("projection src unknown: " + src);
    else p.src = si->second;
    if (di == module_index.end()) errors.push_back("projection dst unknown: " + dst);
    else p.dst = di->second;

    p.density = float(r.number("density"));
    p.weight = float(r.number("weight"));
    p.delay_ms = float(r.number("delay_ms"));
    p.delay_jitter_ms = float(r.number("delay_jitter_ms"));
    p.hebb = float(r.number("hebb"));
    p.apical = uint32_t(r.number("apical"));
    p.exuberance = float(r.number("exuberance"));
    p.birth_weight = float(r.number("birth_weight"));
    p.stp_use = float(r.number("stp_use"));
    p.stp_recover_ms = float(r.number("stp_recover_ms"));
    p.stp_facil_ms = float(r.number("stp_facil_ms"));
    p.burst_learn = float(r.number("burst_learn"));

    // Spelled out on every projection rather than defaulted to "random", for
    // the reason the whole file has no defaults: a genome that does not say how
    // a tract is wired reads the same as one that does, and they build
    // different brains.
    const std::string kind = r.text("kind");
    if (!kind.empty() && !parse_projection_kind(kind, p.kind)) {
      errors.push_back("unknown projection kind '" + kind + "' on " + src + "->" + dst +
                       " (random|gabor|curvature|topographic)");
    }

    // Same reasoning as `kind`: spelled out on every projection, never
    // defaulted. A tract that draws only from inhibitory cells and one that
    // draws from both build very different brains, and silence must not be
    // readable as either.
    const std::string source = r.text("source");
    if (!source.empty() && !parse_projection_source(source, p.source)) {
      errors.push_back("unknown projection source '" + source + "' on " + src + "->" +
                       dst + " (either|excitatory|inhibitory)");
    }

    // The receptive-field parameters are required exactly when they mean
    // something. Asking every random projection to carry four numbers it will
    // never read would make the genome harder to read, not more explicit.
    if (p.kind == uint32_t(aibaby::ProjectionKind::kGabor)) {
      p.rf_sigma = float(r.number("rf_sigma"));
      p.rf_lambda = float(r.number("rf_lambda"));
      p.rf_aspect = float(r.number("rf_aspect"));
      p.rf_floor = float(r.number("rf_floor"));
      p.rf_magnification = float(r.number("rf_magnification"));
    }
    if (p.kind == uint32_t(aibaby::ProjectionKind::kCurvature)) {
      p.rf_sigma = float(r.number("rf_sigma"));
      p.rf_floor = float(r.number("rf_floor"));
      p.rf_magnification = float(r.number("rf_magnification"));
      p.rf_radius_min = float(r.number("rf_radius_min"));
      p.rf_radius_max = float(r.number("rf_radius_max"));
      p.rf_tangent_sigma = float(r.number("rf_tangent_sigma"));
    }
    // DNA v43. Same rule as the two above: required exactly when they mean
    // something. `topo_dst_lo > topo_dst_hi` is legal and reverses the map,
    // which is how one wave drives F1 up while F2 comes down.
    if (p.kind == uint32_t(aibaby::ProjectionKind::kTopographic)) {
      p.topo_src_lo = float(r.number("topo_src_lo"));
      p.topo_src_hi = float(r.number("topo_src_hi"));
      p.topo_dst_lo = float(r.number("topo_dst_lo"));
      p.topo_dst_hi = float(r.number("topo_dst_hi"));
      p.topo_sigma = float(r.number("topo_sigma"));
    }

    collect(r);
    projection_blobs.push_back(p);
  }

  if (!errors.empty()) {
    std::string joined;
    for (size_t i = 0; i < errors.size() && i < 8; ++i) joined += "\n  " + errors[i];
    if (errors.size() > 8) {
      joined += "\n  ... and " + std::to_string(errors.size() - 8) + " more";
    }
    error = "genome has " + std::to_string(errors.size()) + " problem(s):" + joined;
    return false;
  }

  out.clear();
  out.reserve(sizeof(header) +
              module_blobs.size() * sizeof(aibaby::DnaModule) +
              projection_blobs.size() * sizeof(aibaby::DnaProjection));
  append(out, &header, sizeof(header));
  for (const auto& m : module_blobs) append(out, &m, sizeof(m));
  for (const auto& p : projection_blobs) append(out, &p, sizeof(p));
  return true;
}

}  // namespace aibaby_host
