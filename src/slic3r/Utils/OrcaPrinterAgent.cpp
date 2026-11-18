#include "OrcaPrinterAgent.hpp"
#include "IPrinterAgent.hpp"
#include "NetworkAgentFactory.hpp"
#include <string>
#include <utility>
#include <memory>
#include <mutex>
#include "bambu_networking.hpp"

namespace Slic3r { class ICloudServiceAgent; }

namespace Slic3r {

const std::string OrcaPrinterAgent_VERSION = "0.0.1";

OrcaPrinterAgent::OrcaPrinterAgent(std::string log_dir) : log_dir(std::move(log_dir))
{
}

OrcaPrinterAgent::~OrcaPrinterAgent() = default;

void OrcaPrinterAgent::set_cloud_agent(std::shared_ptr<ICloudServiceAgent> cloud)
{
    std::lock_guard<std::mutex> lock(state_mutex);
    m_cloud_agent = cloud;
}

// ============================================================================
// Communication
// ============================================================================
// This compatibility agent is intentionally not a transport implementation.
// Returning success here makes the UI report a connected/printed state even
// though no bytes were sent.  Keep unsupported operations explicit until a
// real protocol adapter is installed.

int OrcaPrinterAgent::send_message(std::string dev_id, std::string json_str, int qos, int flag)
{
    (void)dev_id; (void)json_str; (void)qos; (void)flag;
    return ORCA_NETWORK_ERR_CMD_NOT_SUPPORTED;
}

int OrcaPrinterAgent::connect_printer(const PrinterConnectionParams& params)
{
    (void)params;
    return ORCA_NETWORK_ERR_CMD_NOT_SUPPORTED;
}

int OrcaPrinterAgent::disconnect_printer()
{
    // Disconnect is idempotent even when no transport is installed.
    return BAMBU_NETWORK_SUCCESS;
}

int OrcaPrinterAgent::send_message_to_printer(std::string dev_id, std::string json_str, int qos, int flag)
{
    (void)dev_id; (void)json_str; (void)qos; (void)flag;
    return ORCA_NETWORK_ERR_CMD_NOT_SUPPORTED;
}

// ============================================================================
// Discovery
// ============================================================================

bool OrcaPrinterAgent::start_discovery(bool start, bool sending)
{
    (void)start; (void)sending;
    return false;
}

int OrcaPrinterAgent::bind_detect(std::string dev_ip, std::string sec_link, detectResult& detect)
{
    (void)dev_ip; (void)sec_link; (void)detect;
    return ORCA_NETWORK_ERR_CMD_NOT_SUPPORTED;
}

int OrcaPrinterAgent::set_server_callback(OnServerErrFn fn)
{
    std::lock_guard<std::mutex> lock(state_mutex);
    on_server_err_fn = fn;
    return BAMBU_NETWORK_SUCCESS;
}

// ============================================================================
// Machine Selection
// ============================================================================

std::string OrcaPrinterAgent::get_user_selected_machine()
{
    std::lock_guard<std::mutex> lock(state_mutex);
    return selected_machine;
}

int OrcaPrinterAgent::set_user_selected_machine(std::string dev_id)
{
    std::lock_guard<std::mutex> lock(state_mutex);
    selected_machine = dev_id;
    return BAMBU_NETWORK_SUCCESS;
}

// ============================================================================
// Agent Information
// ============================================================================
AgentInfo OrcaPrinterAgent::get_agent_info_static()
{
    return AgentInfo{ORCA_PRINTER_AGENT_ID, "Orca", OrcaPrinterAgent_VERSION, "Orca Printer Communication Protocol Agent"};
}

// ============================================================================
// Print Job Operations
// ============================================================================

int OrcaPrinterAgent::start_print(PrintParams params, OnUpdateStatusFn update_fn, WasCancelledFn cancel_fn, OnWaitFn wait_fn)
{
    (void)params; (void)update_fn; (void)cancel_fn; (void)wait_fn;
    return ORCA_NETWORK_ERR_CMD_NOT_SUPPORTED;
}

int OrcaPrinterAgent::start_local_print_with_record(PrintParams      params,
                                                    OnUpdateStatusFn update_fn,
                                                    WasCancelledFn   cancel_fn,
                                                    OnWaitFn         wait_fn)
{
    (void)params; (void)update_fn; (void)cancel_fn; (void)wait_fn;
    return ORCA_NETWORK_ERR_CMD_NOT_SUPPORTED;
}

int OrcaPrinterAgent::start_send_gcode_to_sdcard(PrintParams params, OnUpdateStatusFn update_fn, WasCancelledFn cancel_fn, OnWaitFn wait_fn)
{
    (void)params; (void)update_fn; (void)cancel_fn; (void)wait_fn;
    return ORCA_NETWORK_ERR_CMD_NOT_SUPPORTED;
}

int OrcaPrinterAgent::start_local_print(PrintParams params, OnUpdateStatusFn update_fn, WasCancelledFn cancel_fn)
{
    (void)params; (void)update_fn; (void)cancel_fn;
    return ORCA_NETWORK_ERR_CMD_NOT_SUPPORTED;
}

int OrcaPrinterAgent::start_sdcard_print(PrintParams params, OnUpdateStatusFn update_fn, WasCancelledFn cancel_fn)
{
    (void)params; (void)update_fn; (void)cancel_fn;
    return ORCA_NETWORK_ERR_CMD_NOT_SUPPORTED;
}

// ============================================================================
// Callback Registration
// ============================================================================

int OrcaPrinterAgent::set_on_ssdp_msg_fn(OnMsgArrivedFn fn)
{
    std::lock_guard<std::mutex> lock(state_mutex);
    on_ssdp_msg_fn = fn;
    return BAMBU_NETWORK_SUCCESS;
}

int OrcaPrinterAgent::set_on_printer_connected_fn(OnPrinterConnectedFn fn)
{
    std::lock_guard<std::mutex> lock(state_mutex);
    on_printer_connected_fn = fn;
    return BAMBU_NETWORK_SUCCESS;
}

int OrcaPrinterAgent::set_on_subscribe_failure_fn(GetSubscribeFailureFn fn)
{
    std::lock_guard<std::mutex> lock(state_mutex);
    on_subscribe_failure_fn = fn;
    return BAMBU_NETWORK_SUCCESS;
}

int OrcaPrinterAgent::set_on_message_fn(OnMessageFn fn)
{
    std::lock_guard<std::mutex> lock(state_mutex);
    on_message_fn = fn;
    return BAMBU_NETWORK_SUCCESS;
}

int OrcaPrinterAgent::set_on_user_message_fn(OnMessageFn fn)
{
    std::lock_guard<std::mutex> lock(state_mutex);
    on_user_message_fn = fn;
    return BAMBU_NETWORK_SUCCESS;
}

int OrcaPrinterAgent::set_on_local_connect_fn(OnLocalConnectedFn fn)
{
    std::lock_guard<std::mutex> lock(state_mutex);
    on_local_connect_fn = fn;
    return BAMBU_NETWORK_SUCCESS;
}

int OrcaPrinterAgent::set_on_local_message_fn(OnMessageFn fn)
{
    std::lock_guard<std::mutex> lock(state_mutex);
    on_local_message_fn = fn;
    return BAMBU_NETWORK_SUCCESS;
}

int OrcaPrinterAgent::set_queue_on_main_fn(QueueOnMainFn fn)
{
    std::lock_guard<std::mutex> lock(state_mutex);
    queue_on_main_fn = fn;
    return BAMBU_NETWORK_SUCCESS;
}

} // namespace Slic3r
