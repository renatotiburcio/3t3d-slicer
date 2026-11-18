#include "AnycubicWorkbenchState.hpp"

#include <algorithm>
#include <utility>

namespace Slic3r {

AnycubicKobra4Simulator::AnycubicKobra4Simulator()
{
    reset();
}

void AnycubicKobra4Simulator::reset()
{
    m_snapshot = AnycubicWorkbenchSnapshot{};
    m_snapshot.serial_number = "SIM-K4C-0001";
    m_snapshot.host = "simulator://anycubic-kobra-4-combo";
    m_snapshot.job_name = "1117-1304-Body(01)_PLA_0.12_15h1m28s";
    m_snapshot.print_objects = {"Peca principal", "Suporte frontal", "Tampa superior"};
    m_snapshot.camera_available = true;
    m_snapshot.ace_available = true;
    m_snapshot.ace_slot_count = 4;
}

void AnycubicKobra4Simulator::set_state(AnycubicWorkbenchState state)
{
    m_snapshot.state = state;
    if (state == AnycubicWorkbenchState::error)
        m_snapshot.error_message = "Estado de erro simulado";
    else
        m_snapshot.error_message.clear();
}

void AnycubicKobra4Simulator::set_progress(std::uint32_t current_layer, std::uint32_t total_layers)
{
    m_snapshot.current_layer = current_layer;
    m_snapshot.total_layers = total_layers;
    if (total_layers == 0) {
        m_snapshot.progress_percent = 0.0;
        return;
    }

    m_snapshot.progress_percent = std::min(
        100.0,
        100.0 * static_cast<double>(current_layer) / static_cast<double>(total_layers));
}

void AnycubicKobra4Simulator::set_temperatures(double nozzle, double nozzle_target,
                                               double bed, double bed_target)
{
    m_snapshot.nozzle_temperature = nozzle;
    m_snapshot.nozzle_target_temperature = nozzle_target;
    m_snapshot.bed_temperature = bed;
    m_snapshot.bed_target_temperature = bed_target;
}

void AnycubicKobra4Simulator::set_print_objects(std::vector<std::string> object_names)
{
    if (object_names.empty())
        return;

    m_snapshot.print_objects = std::move(object_names);
    m_snapshot.skipped_object_ids.erase(
        std::remove_if(m_snapshot.skipped_object_ids.begin(), m_snapshot.skipped_object_ids.end(),
                       [this](std::uint32_t id) { return id >= m_snapshot.print_objects.size(); }),
        m_snapshot.skipped_object_ids.end());
}

void AnycubicKobra4Simulator::set_skipped_objects(const std::vector<std::uint32_t>& object_ids)
{
    m_snapshot.skipped_object_ids.clear();
    for (std::uint32_t object_id : object_ids) {
        if (object_id < m_snapshot.print_objects.size() &&
            std::find(m_snapshot.skipped_object_ids.begin(), m_snapshot.skipped_object_ids.end(), object_id) ==
                m_snapshot.skipped_object_ids.end())
            m_snapshot.skipped_object_ids.push_back(object_id);
    }
}

void AnycubicKobra4Simulator::skip_objects(const std::vector<std::uint32_t>& object_ids)
{
    for (std::uint32_t object_id : object_ids) {
        if (object_id >= m_snapshot.print_objects.size())
            continue;
        if (std::find(m_snapshot.skipped_object_ids.begin(), m_snapshot.skipped_object_ids.end(), object_id) ==
            m_snapshot.skipped_object_ids.end())
            m_snapshot.skipped_object_ids.push_back(object_id);
    }
}

void AnycubicKobra4Simulator::tick(std::uint32_t elapsed_seconds)
{
    if (m_snapshot.state != AnycubicWorkbenchState::printing || m_snapshot.total_layers == 0)
        return;

    // A deterministic one-layer-per-minute progression is sufficient for UI
    // previews and deliberately does not represent a real printer estimate.
    const std::uint32_t layers = elapsed_seconds / 60;
    set_progress(std::min(m_snapshot.total_layers, m_snapshot.current_layer + layers),
                 m_snapshot.total_layers);
    if (m_snapshot.current_layer >= m_snapshot.total_layers)
        set_state(AnycubicWorkbenchState::completed);
}

const char* anycubic_workbench_state_name(AnycubicWorkbenchState state)
{
    switch (state) {
    case AnycubicWorkbenchState::disconnected: return "disconnected";
    case AnycubicWorkbenchState::connecting:    return "connecting";
    case AnycubicWorkbenchState::idle:          return "idle";
    case AnycubicWorkbenchState::preparing:     return "preparing";
    case AnycubicWorkbenchState::printing:      return "printing";
    case AnycubicWorkbenchState::paused:        return "paused";
    case AnycubicWorkbenchState::completed:     return "completed";
    case AnycubicWorkbenchState::error:         return "error";
    }
    return "unknown";
}

} // namespace Slic3r
