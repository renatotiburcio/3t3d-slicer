#pragma once

#include "slic3r/Utils/AnycubicWorkbenchState.hpp"

#include <wx/panel.h>
#include <wx/timer.h>

class wxButton;
class wxGauge;
class wxPanel;
class wxStaticText;

namespace Slic3r::GUI {

class AnycubicWorkbenchPanel final : public wxPanel
{
public:
    explicit AnycubicWorkbenchPanel(wxWindow* parent);

private:
    void sync_project_objects();
    void refresh_snapshot();
    void on_timer(wxTimerEvent& event);
    void on_pause_resume(wxCommandEvent& event);
    void on_stop(wxCommandEvent& event);
    void on_skip_objects(wxCommandEvent& event);

    AnycubicKobra4Simulator m_simulator;
    wxTimer                 m_timer;
    wxStaticText*           m_state{nullptr};
    wxStaticText*           m_job{nullptr};
    wxStaticText*           m_layers{nullptr};
    wxStaticText*           m_objects{nullptr};
    wxStaticText*           m_temperatures{nullptr};
    wxStaticText*           m_device{nullptr};
    wxStaticText*           m_materials{nullptr};
    wxGauge*                m_progress{nullptr};
    wxButton*               m_pause_resume{nullptr};
    wxButton*               m_stop{nullptr};
    wxButton*               m_skip_objects{nullptr};
    wxPanel*                m_plate_preview{nullptr};
    int                     m_selected_object{-1};
};

} // namespace Slic3r::GUI
