#include "AnycubicWorkbenchPanel.hpp"

#include <algorithm>
#include <wx/button.h>
#include <wx/choicdlg.h>
#include <wx/dcbuffer.h>
#include <wx/gauge.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

namespace Slic3r::GUI {

namespace {

wxString utf8(const char* text)
{
    return wxString::FromUTF8(text);
}

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
    case AnycubicWorkbenchState::completed:     return utf8(u8"Conclu\u00edda");
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
    header->Add(make_heading(this, utf8(u8"Bancada de trabalho")), 1, wxALIGN_CENTER_VERTICAL | wxALL, 16);
    header->Add(new wxStaticText(this, wxID_ANY, "Anycubic"), 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 16);
    root->Add(header, 0, wxEXPAND);

    auto* content = new wxBoxSizer(wxHORIZONTAL);
    auto* printers = new wxStaticBoxSizer(wxVERTICAL, this, utf8(u8"Impressoras na rede"));
    auto* selected_printer = new wxPanel(this);
    selected_printer->SetBackgroundColour(wxColour(232, 247, 245));
    auto* printer_sizer = new wxBoxSizer(wxVERTICAL);
    auto* printer_name = make_heading(selected_printer, "Anycubic Kobra 4 Combo");
    printer_name->SetFont(printer_name->GetFont().Scale(0.85));
    printer_sizer->Add(printer_name, 0, wxALL, 10);
    printer_sizer->Add(new wxStaticText(selected_printer, wxID_ANY, utf8(u8"Imprimindo")), 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);
    printer_sizer->Add(new wxStaticText(selected_printer, wxID_ANY, "SIM-K4C-0001"), 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);
    selected_printer->SetSizer(printer_sizer);
    printers->Add(selected_printer, 0, wxEXPAND | wxALL, 8);
    printers->Add(new wxStaticText(this, wxID_ANY, utf8(u8"Modo simulado — nenhuma conexão ativa")), 0, wxALL, 10);
    content->Add(printers, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 16);

    auto* center = new wxBoxSizer(wxVERTICAL);
    auto* job_box = new wxStaticBoxSizer(wxVERTICAL, this, utf8(u8"Tarefa de impressão"));
    m_job = new wxStaticText(this, wxID_ANY, wxEmptyString);
    m_layers = new wxStaticText(this, wxID_ANY, wxEmptyString);
    m_objects = new wxStaticText(this, wxID_ANY, wxEmptyString);
    m_progress = new wxGauge(this, wxID_ANY, 100);
    job_box->Add(m_job, 0, wxALL, 12);
    job_box->Add(m_layers, 0, wxLEFT | wxRIGHT | wxBOTTOM, 12);
    job_box->Add(m_objects, 0, wxLEFT | wxRIGHT | wxBOTTOM, 12);
    job_box->Add(m_progress, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);
    center->Add(job_box, 0, wxEXPAND | wxBOTTOM, 12);

    auto* camera_box = new wxStaticBoxSizer(wxVERTICAL, this, utf8(u8"Câmera"));
    m_plate_preview = new wxPanel(this, wxID_ANY, wxDefaultPosition, wxSize(320, 190));
    m_plate_preview->SetBackgroundColour(wxColour(42, 44, 48));
    m_plate_preview->Bind(wxEVT_PAINT, [this](wxPaintEvent&) {
        wxAutoBufferedPaintDC dc(m_plate_preview);
        dc.SetBackground(wxBrush(wxColour(42, 44, 48)));
        dc.Clear();
        const wxSize size = m_plate_preview->GetClientSize();
        const wxRect plate(30, 18, std::max(120, size.GetWidth() - 60), std::max(100, size.GetHeight() - 36));
        dc.SetBrush(wxBrush(wxColour(205, 207, 210)));
        dc.SetPen(*wxTRANSPARENT_PEN);
        dc.DrawRoundedRectangle(plate, 8);
        const wxColour colors[] = {wxColour(92, 154, 220), wxColour(224, 151, 75), wxColour(104, 184, 133)};
        const wxPoint centers[] = {{plate.x + plate.width / 3, plate.y + plate.height / 3},
                                   {plate.x + plate.width * 2 / 3, plate.y + plate.height / 3},
                                   {plate.x + plate.width / 2, plate.y + plate.height * 2 / 3}};
        for (int i = 0; i < 3; ++i) {
            dc.SetBrush(wxBrush(colors[i]));
            dc.DrawCircle(centers[i], 25);
            dc.SetTextForeground(*wxWHITE);
            dc.DrawText(wxString::Format("ID %d", i), centers[i].x - 16, centers[i].y - 8);
        }
    });
    camera_box->Add(m_plate_preview, 1, wxEXPAND | wxALL, 8);
    camera_box->Add(new wxStaticText(this, wxID_ANY, utf8(u8"Pré-visualização simulada da placa")), 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);
    center->Add(camera_box, 1, wxEXPAND);
    content->Add(center, 2, wxEXPAND | wxRIGHT | wxBOTTOM, 16);

    auto* right = new wxBoxSizer(wxVERTICAL);
    auto* control_box = new wxStaticBoxSizer(wxVERTICAL, this, utf8(u8"Controle"));
    m_state = new wxStaticText(this, wxID_ANY, wxEmptyString);
    m_temperatures = new wxStaticText(this, wxID_ANY, wxEmptyString);
    control_box->Add(m_state, 0, wxALL, 12);
    control_box->Add(m_temperatures, 0, wxLEFT | wxRIGHT | wxBOTTOM, 12);
    auto* controls = new wxBoxSizer(wxHORIZONTAL);
    m_pause_resume = new wxButton(this, wxID_ANY, utf8(u8"Pausar"));
    m_stop = new wxButton(this, wxID_ANY, utf8(u8"Parar simulação"));
    m_skip_objects = new wxButton(this, wxID_ANY, utf8(u8"Ignorar objetos..."));
    controls->Add(m_pause_resume, 0, wxRIGHT, 8);
    controls->Add(m_stop, 0, wxRIGHT, 8);
    controls->Add(m_skip_objects, 0);
    control_box->Add(controls, 0, wxLEFT | wxRIGHT | wxBOTTOM, 12);
    right->Add(control_box, 0, wxEXPAND | wxBOTTOM, 12);

    auto* materials_box = new wxStaticBoxSizer(wxVERTICAL, this, utf8(u8"Gestão de materiais"));
    m_materials = new wxStaticText(this, wxID_ANY, utf8(u8"ACE: 4 posições simuladas\nPLA 1   PLA 2   PLA 3   PLA 4"));
    materials_box->Add(m_materials, 0, wxALL, 12);
    right->Add(materials_box, 0, wxEXPAND | wxBOTTOM, 12);

    auto* device_box = new wxStaticBoxSizer(wxVERTICAL, this, utf8(u8"Informações do dispositivo"));
    m_device = new wxStaticText(this, wxID_ANY, wxEmptyString);
    device_box->Add(m_device, 0, wxALL, 12);
    right->Add(device_box, 0, wxEXPAND);
    content->Add(right, 1, wxEXPAND | wxRIGHT | wxBOTTOM, 16);
    root->Add(content, 1, wxEXPAND);

    SetSizer(root);
    Bind(wxEVT_TIMER, &AnycubicWorkbenchPanel::on_timer, this);
    m_pause_resume->Bind(wxEVT_BUTTON, &AnycubicWorkbenchPanel::on_pause_resume, this);
    m_stop->Bind(wxEVT_BUTTON, &AnycubicWorkbenchPanel::on_stop, this);
    m_skip_objects->Bind(wxEVT_BUTTON, &AnycubicWorkbenchPanel::on_skip_objects, this);
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
    wxString object_summary = utf8(u8"Objetos na placa: ");
    object_summary += wxString::Format("%zu", snapshot.print_objects.size());
    for (std::size_t i = 0; i < snapshot.print_objects.size(); ++i)
        object_summary += wxString::Format("\nID %zu  —  %s", i, wxString::FromUTF8(snapshot.print_objects[i]));
    m_objects->SetLabel(object_summary);
    m_progress->SetValue(static_cast<int>(snapshot.progress_percent));
    m_temperatures->SetLabel(wxString::Format(utf8(u8"Bico: %.0f/%.0f °C   Mesa: %.0f/%.0f °C"),
                                               snapshot.nozzle_temperature, snapshot.nozzle_target_temperature,
                                               snapshot.bed_temperature, snapshot.bed_target_temperature));
    m_device->SetLabel(wxString::Format(utf8(u8"%s\nNúmero de série: %s\nHost: simulador local"),
                                         wxString::FromUTF8(snapshot.display_name),
                                         wxString::FromUTF8(snapshot.serial_number)));
    m_pause_resume->SetLabel(snapshot.state == AnycubicWorkbenchState::paused ? utf8(u8"Retomar") : utf8(u8"Pausar"));
    m_pause_resume->Enable(snapshot.state == AnycubicWorkbenchState::printing ||
                           snapshot.state == AnycubicWorkbenchState::paused);
    m_stop->Enable(snapshot.state == AnycubicWorkbenchState::printing ||
                   snapshot.state == AnycubicWorkbenchState::paused);
    m_skip_objects->Enable(snapshot.state == AnycubicWorkbenchState::printing &&
                           !snapshot.print_objects.empty());
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

void AnycubicWorkbenchPanel::on_skip_objects(wxCommandEvent&)
{
    const auto& snapshot = m_simulator.snapshot();
    wxArrayString choices;
    for (std::size_t i = 0; i < snapshot.print_objects.size(); ++i) {
        const bool skipped = std::find(snapshot.skipped_object_ids.begin(), snapshot.skipped_object_ids.end(),
                                       static_cast<std::uint32_t>(i)) != snapshot.skipped_object_ids.end();
        choices.Add(wxString::Format("ID %zu — %s%s", i, wxString::FromUTF8(snapshot.print_objects[i]),
                                     skipped ? utf8(u8" (já ignorado)") : wxString()));
    }

    wxMultiChoiceDialog dialog(this, utf8(u8"Selecione as peças que não devem mais ser impressas nesta tarefa:"),
                               utf8(u8"Ignorar objetos"), choices);
    if (dialog.ShowModal() != wxID_OK || dialog.GetSelections().empty())
        return;

    if (wxMessageBox(utf8(u8"A peça selecionada será ignorada somente no simulador. Deseja continuar?"),
                     utf8(u8"Confirmar ação"), wxYES_NO | wxICON_WARNING, this) != wxYES)
        return;

    std::vector<std::uint32_t> object_ids;
    for (int selection : dialog.GetSelections())
        object_ids.push_back(static_cast<std::uint32_t>(selection));
    m_simulator.skip_objects(object_ids);
    refresh_snapshot();
    m_plate_preview->Refresh();
}

} // namespace Slic3r::GUI
