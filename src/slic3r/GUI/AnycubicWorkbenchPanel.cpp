#include "AnycubicWorkbenchPanel.hpp"

#include <wx/button.h>
#include <wx/gauge.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

namespace Slic3r::GUI {

namespace {

wxStaticText* make_heading(wxWindow* parent, const wxString& text)
{
    auto* label = new wxStaticText(parent, wxID_ANY, text);
    wxFont font = label->GetFont();
    font.SetWeight(wxFONTWEIGHT_BOLD);
    font.SetPointSize(font.GetPointSize() + 2);
    label->SetFont(font);
    return label;
}

wxString state_label(AnycubicWorkbenchState state)
{
    switch (state) {
    case AnycubicWorkbenchState::disconnected: return "Desconectada";
    case AnycubicWorkbenchState::connecting:    return "Conectando";
    case AnycubicWorkbenchState::idle:          return "Pronta";
    case AnycubicWorkbenchState::preparing:     return "Preparando";
    case AnycubicWorkbenchState::printing:      return "Imprimindo";
    case AnycubicWorkbenchState::paused:        return "Pausada";
    case AnycubicWorkbenchState::completed:     return "Concluída";
    case AnycubicWorkbenchState::error:         return "Erro";
    }
    return "Desconhecido";
}

} // namespace

AnycubicWorkbenchPanel::AnycubicWorkbenchPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY), m_timer(this)
{
    SetBackgroundColour(wxColour(245, 247, 248));

    m_simulator.set_state(AnycubicWorkbenchState::printing);
    m_simulator.set_progress(12, 479);
    m_simulator.set_temperatures(220.0, 220.0, 60.0, 60.0);

    auto* root = new wxBoxSizer(wxVERTICAL);
    auto* header = new wxBoxSizer(wxHORIZONTAL);
    header->Add(make_heading(this, "Bancada de trabalho"), 1, wxALIGN_CENTER_VERTICAL | wxALL, 16);
    header->Add(new wxStaticText(this, wxID_ANY, "Anycubic"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 16);
    root->Add(header, 0, wxEXPAND);

    auto* content = new wxBoxSizer(wxHORIZONTAL);
    auto* left = new wxBoxSizer(wxVERTICAL);
    auto* job_box = new wxStaticBoxSizer(wxVERTICAL, this, "Tarefa de impressão");
    m_job = new wxStaticText(this, wxID_ANY, wxEmptyString);
    m_layers = new wxStaticText(this, wxID_ANY, wxEmptyString);
    m_progress = new wxGauge(this, wxID_ANY, 100);
    job_box->Add(m_job, 0, wxALL, 12);
    job_box->Add(m_layers, 0, wxLEFT | wxRIGHT | wxBOTTOM, 12);
    job_box->Add(m_progress, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);
    left->Add(job_box, 0, wxEXPAND | wxBOTTOM, 12);

    auto* camera_box = new wxStaticBoxSizer(wxVERTICAL, this, "Câmera");
    camera_box->Add(new wxStaticText(this, wxID_ANY, "Pré-visualização indisponível no modo simulado."), 1,
                    wxEXPAND | wxALL, 18);
    left->Add(camera_box, 1, wxEXPAND);
    content->Add(left, 2, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 16);

    auto* right = new wxBoxSizer(wxVERTICAL);
    auto* control_box = new wxStaticBoxSizer(wxVERTICAL, this, "Controle");
    m_state = new wxStaticText(this, wxID_ANY, wxEmptyString);
    m_temperatures = new wxStaticText(this, wxID_ANY, wxEmptyString);
    control_box->Add(m_state, 0, wxALL, 12);
    control_box->Add(m_temperatures, 0, wxLEFT | wxRIGHT | wxBOTTOM, 12);
    auto* controls = new wxBoxSizer(wxHORIZONTAL);
    m_pause_resume = new wxButton(this, wxID_ANY, "Pausar");
    m_stop = new wxButton(this, wxID_ANY, "Parar simulação");
    controls->Add(m_pause_resume, 0, wxRIGHT, 8);
    controls->Add(m_stop, 0);
    control_box->Add(controls, 0, wxLEFT | wxRIGHT | wxBOTTOM, 12);
    right->Add(control_box, 0, wxEXPAND | wxBOTTOM, 12);

    auto* materials_box = new wxStaticBoxSizer(wxVERTICAL, this, "Gestão de materiais");
    m_materials = new wxStaticText(this, wxID_ANY, "ACE: 4 posições simuladas\nPLA 1   PLA 2   PLA 3   PLA 4");
    materials_box->Add(m_materials, 0, wxALL, 12);
    right->Add(materials_box, 0, wxEXPAND | wxBOTTOM, 12);

    auto* device_box = new wxStaticBoxSizer(wxVERTICAL, this, "Informações do dispositivo");
    m_device = new wxStaticText(this, wxID_ANY, wxEmptyString);
    device_box->Add(m_device, 0, wxALL, 12);
    right->Add(device_box, 0, wxEXPAND);
    content->Add(right, 1, wxEXPAND | wxRIGHT | wxBOTTOM, 16);
    root->Add(content, 1, wxEXPAND);

    SetSizer(root);
    Bind(wxEVT_TIMER, &AnycubicWorkbenchPanel::on_timer, this);
    m_pause_resume->Bind(wxEVT_BUTTON, &AnycubicWorkbenchPanel::on_pause_resume, this);
    m_stop->Bind(wxEVT_BUTTON, &AnycubicWorkbenchPanel::on_stop, this);
    refresh_snapshot();
    m_timer.Start(1000);
}

void AnycubicWorkbenchPanel::refresh_snapshot()
{
    const auto& snapshot = m_simulator.snapshot();
    m_state->SetLabel(wxString("Estado: ") + state_label(snapshot.state));
    m_job->SetLabel(wxString::FromUTF8(snapshot.job_name));
    m_layers->SetLabel(wxString::Format("Camadas: %u/%u   %.0f%%", snapshot.current_layer,
                                        snapshot.total_layers, snapshot.progress_percent));
    m_progress->SetValue(static_cast<int>(snapshot.progress_percent));
    m_temperatures->SetLabel(wxString::Format("Bico: %.0f/%.0f °C   Mesa: %.0f/%.0f °C",
                                               snapshot.nozzle_temperature, snapshot.nozzle_target_temperature,
                                               snapshot.bed_temperature, snapshot.bed_target_temperature));
    m_device->SetLabel(wxString::Format("%s\nNúmero de série: %s\nHost: simulador local",
                                         wxString::FromUTF8(snapshot.display_name),
                                         wxString::FromUTF8(snapshot.serial_number)));
    m_pause_resume->SetLabel(snapshot.state == AnycubicWorkbenchState::paused ? "Retomar" : "Pausar");
    m_pause_resume->Enable(snapshot.state == AnycubicWorkbenchState::printing ||
                           snapshot.state == AnycubicWorkbenchState::paused);
    m_stop->Enable(snapshot.state == AnycubicWorkbenchState::printing ||
                   snapshot.state == AnycubicWorkbenchState::paused);
    Layout();
}

void AnycubicWorkbenchPanel::on_timer(wxTimerEvent&)
{
    m_simulator.tick(1);
    refresh_snapshot();
}

void AnycubicWorkbenchPanel::on_pause_resume(wxCommandEvent&)
{
    const auto state = m_simulator.snapshot().state;
    m_simulator.set_state(state == AnycubicWorkbenchState::paused ?
                          AnycubicWorkbenchState::printing : AnycubicWorkbenchState::paused);
    refresh_snapshot();
}

void AnycubicWorkbenchPanel::on_stop(wxCommandEvent&)
{
    m_simulator.set_state(AnycubicWorkbenchState::idle);
    refresh_snapshot();
}

} // namespace Slic3r::GUI
