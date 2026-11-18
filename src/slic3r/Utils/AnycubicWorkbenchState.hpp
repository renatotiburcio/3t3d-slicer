#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace Slic3r {

// Vendor-neutral states used by the future Bancada de trabalho UI. The real
// Anycubic adapter will translate its LAN/MQTT states into this vocabulary.
enum class AnycubicWorkbenchState : std::uint8_t {
    disconnected,
    connecting,
    idle,
    preparing,
    printing,
    paused,
    completed,
    error,
};

struct AnycubicWorkbenchSnapshot {
    std::string model = "Anycubic Kobra 4 Combo";
    std::string display_name = "Anycubic Kobra 4 Combo";
    std::string serial_number;
    std::string host;
    AnycubicWorkbenchState state = AnycubicWorkbenchState::disconnected;
    std::string job_name;
    std::uint32_t current_layer = 0;
    std::uint32_t total_layers = 0;
    double progress_percent = 0.0;
    double nozzle_temperature = 0.0;
    double nozzle_target_temperature = 0.0;
    double bed_temperature = 0.0;
    double bed_target_temperature = 0.0;
    std::uint8_t fan_percent = 0;
    bool camera_available = false;
    bool ace_available = false;
    std::uint8_t ace_slot_count = 0;
    std::string error_message;
    std::vector<std::string> print_objects;
    std::vector<std::uint32_t> skipped_object_ids;
};

// Deterministic app-side simulator. It never opens a socket and is intended
// for UI development, screenshots and state-transition tests until a real
// Anycubic LAN adapter is validated with a free printer.
class AnycubicKobra4Simulator {
public:
    AnycubicKobra4Simulator();

    const AnycubicWorkbenchSnapshot& snapshot() const { return m_snapshot; }

    void reset();
    void set_state(AnycubicWorkbenchState state);
    void set_progress(std::uint32_t current_layer, std::uint32_t total_layers);
    void set_temperatures(double nozzle, double nozzle_target, double bed, double bed_target);
    void set_print_objects(std::vector<std::string> object_names);
    void set_skipped_objects(const std::vector<std::uint32_t>& object_ids);
    void skip_objects(const std::vector<std::uint32_t>& object_ids);
    void tick(std::uint32_t elapsed_seconds);

private:
    AnycubicWorkbenchSnapshot m_snapshot;
};

const char* anycubic_workbench_state_name(AnycubicWorkbenchState state);

} // namespace Slic3r
