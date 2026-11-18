#include "AnycubicWorkbenchPanel.hpp"

#include <algorithm>
#include <wx/button.h>
#include <wx/checklst.h>
#include <wx/dcbuffer.h>
#include <wx/dialog.h>
#include <wx/gauge.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/stattext.h>

namespace Slic3r::GUI {
namespace {

const wxColour kBackground(245, 247, 250);
const wxColour kSurface(255, 255, 255);
const wxColour kInk(33, 30, 31);
const wxColour kCyan(29, 189, 223);
const wxColour kGreen(26, 168, 112);
const wxColour kMagenta(209, 31, 119);
const wxColour kSlate(42, 44, 48);

wxString u8(const char* text) { return wxString::FromUTF8(text); }

wxStaticText* heading(wxWindow* parent, const wxString& text, int size = 12)
{
    auto* label = new wxStaticText(parent, wxID_ANY, text);
    wxFont font = label->GetFont();
    font.SetWeight(wxFONTWEIGHT_BOLD);
    font.SetPointSize(size);
    label->SetFont(font);
    label->SetForegroundColour(kInk);
    label->SetBackgroundColour(kSurface);
    return label;
}

wxStaticText* body_text(wxWindow* parent, const wxString& text, const wxColour& background = kSurface)
{
    auto* label = new wxStaticText(parent, wxID_ANY, text);
    label->SetBackgroundColour(background);
    label->SetForegroundColour(kInk);
    return label;
}

struct WorkbenchCard
{
    wxPanel*     panel;
    wxBoxSizer*  body;
};

WorkbenchCard card(wxWindow* parent, const wxString& title, const wxColour& accent)
{
    auto* panel = new wxPanel(parent);
    panel->SetBackgroundColour(kSurface);
    auto* root = new wxBoxSizer(wxVERTICAL);
    auto* header = new wxPanel(panel);
    header->SetBackgroundColour(kSurface);
    auto* header_sizer = new wxBoxSizer(wxVERTICAL);
    auto* label = new wxStaticText(header, wxID_ANY, title);
    wxFont font = label->GetFont();
    font.SetWeight(wxFONTWEIGHT_BOLD);
    font.SetPointSize(11);
    label->SetFont(font);
    label->SetForegroundColour(kInk);
    label->SetBackgroundColour(kSurface);
    header_sizer->Add(label, 0, wxLEFT | wxRIGHT | wxTOP, 12);
    auto* stripe = new wxPanel(header, wxID_ANY, wxDefaultPosition, wxSize(-1, 3));
    stripe->SetBackgroundColour(accent);
    stripe->SetMinSize(wxSize(-1, 3));
    header_sizer->Add(stripe, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 8);
    header->SetSizer(header_sizer);
    root->Add(header, 0, wxEXPAND);
    auto* body = new wxBoxSizer(wxVERTICAL);
    root->Add(body, 1, wxEXPAND | wxALL, 8);
    panel->SetSizer(root);
    return {panel, body};
}

wxString state_text(AnycubicWorkbenchState state)
{
    switch (state) {
    case AnycubicWorkbenchState::disconnected: return "Desconectada";
    case AnycubicWorkbenchState::connecting:    return "Conectando";
    case AnycubicWorkbenchState::idle:          return "Pronta";
    case AnycubicWorkbenchState::preparing:     return "Preparando";
    case AnycubicWorkbenchState::printing:      return "Imprimindo";
    case AnycubicWorkbenchState::paused:        return "Pausada";
    case AnycubicWorkbenchState::completed:     return u8(u8"Conclu\u00edda");
    case AnycubicWorkbenchState::error:         return "Erro";
    }
    return "Desconhecido";
}

} // namespace

AnycubicWorkbenchPanel::AnycubicWorkbenchPanel(wxWindow* parent)
    : wxPanel(parent, wxID_ANY), m_timer(this)
{
    SetBackgroundColour(kBackground);
    m_simulator.set_state(AnycubicWorkbenchState::printing);
    m_simulator.set_progress(12, 479);
    m_simulator.set_temperatures(220.0, 220.0, 60.0, 60.0);

    auto* root = new wxBoxSizer(wxVERTICAL);
    auto* header = new wxPanel(this);
    header->SetBackgroundColour(kInk);
    auto* header_sizer = new wxBoxSizer(wxHORIZONTAL);
    auto* title = heading(header, u8(u8"Bancada de trabalho"), 15);
    title->SetForegroundColour(*wxWHITE);
    title->SetBackgroundColour(kInk);
    header_sizer->Add(title, 1, wxALIGN_CENTER_VERTICAL | wxLEFT, 18);
    auto* brand = new wxStaticText(header, wxID_ANY, "3T3D  |  Anycubic");
    brand->SetForegroundColour(wxColour(220, 245, 242));
    brand->SetFont(brand->GetFont().Bold());
    header_sizer->Add(brand, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 18);
    header->SetSizer(header_sizer);
    root->Add(header, 0, wxEXPAND);

    auto* content = new wxBoxSizer(wxHORIZONTAL);
    auto printers = card(this, u8(u8"Impressoras na rede"), kCyan);
    auto* selected = new wxPanel(printers.panel);
    selected->SetBackgroundColour(wxColour(232, 248, 245));
    auto* selected_sizer = new wxBoxSizer(wxVERTICAL);
    auto* printer_title = heading(selected, "Anycubic Kobra 4 Combo", 10);
    printer_title->SetBackgroundColour(wxColour(232, 248, 245));
    selected_sizer->Add(printer_title, 0, wxALL, 10);
    auto* online = new wxStaticText(selected, wxID_ANY, u8("\u25cf  Imprimindo"));
    online->SetBackgroundColour(wxColour(232, 248, 245));
    online->SetForegroundColour(kGreen);
    selected_sizer->Add(online, 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);
    selected_sizer->Add(body_text(selected, "SIM-K4C-0001", wxColour(232, 248, 245)), 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);
    selected->SetSizer(selected_sizer);
    printers.body->Add(selected, 0, wxEXPAND | wxALL, 8);
    printers.body->Add(body_text(printers.panel, u8(u8"Modo simulado \u2014 nenhuma conex\u00e3o ativa"), kSurface), 0, wxALL, 10);
    content->Add(printers.panel, 0, wxEXPAND | wxALL, 14);

    auto* center = new wxBoxSizer(wxVERTICAL);
    auto job = card(this, u8(u8"Tarefa de impress\u00e3o"), kCyan);
    m_job = body_text(job.panel, wxEmptyString);
    m_layers = body_text(job.panel, wxEmptyString);
    m_objects = body_text(job.panel, wxEmptyString);
    m_progress = new wxGauge(job.panel, wxID_ANY, 100);
    m_progress->SetForegroundColour(kCyan);
    job.body->Add(m_job, 0, wxALL, 12);
    job.body->Add(m_layers, 0, wxLEFT | wxRIGHT | wxBOTTOM, 12);
    job.body->Add(m_objects, 0, wxLEFT | wxRIGHT | wxBOTTOM, 12);
    job.body->Add(m_progress, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);
    center->Add(job.panel, 0, wxEXPAND | wxTOP | wxBOTTOM, 14);

    auto camera = card(this, u8(u8"C\u00e2mera / placa de impress\u00e3o"), kGreen);
    m_plate_preview = new wxPanel(camera.panel, wxID_ANY, wxDefaultPosition, wxSize(320, 190));
    m_plate_preview->SetBackgroundColour(kSlate);
    m_plate_preview->Bind(wxEVT_PAINT, [this](wxPaintEvent&) {
        wxAutoBufferedPaintDC dc(m_plate_preview);
        dc.SetBackground(wxBrush(kSlate));
        dc.Clear();
        const wxSize size = m_plate_preview->GetClientSize();
        const wxRect plate(30, 22, std::max(140, size.x - 60), std::max(110, size.y - 44));
        dc.SetBrush(wxBrush(wxColour(226, 229, 233)));
        dc.SetPen(wxPen(wxColour(155, 160, 166), 1));
        dc.DrawRoundedRectangle(plate, 10);
        const wxColour colors[] = {kCyan, kMagenta, kGreen};
        const wxPoint points[] = {{plate.x + plate.width / 3, plate.y + plate.height / 3},
                                  {plate.x + plate.width * 2 / 3, plate.y + plate.height / 3},
                                  {plate.x + plate.width / 2, plate.y + plate.height * 2 / 3}};
        for (std::size_t i = 0; i < 3; ++i) {
            const bool skipped = std::find(m_simulator.snapshot().skipped_object_ids.begin(),
                                           m_simulator.snapshot().skipped_object_ids.end(),
                                           static_cast<std::uint32_t>(i)) != m_simulator.snapshot().skipped_object_ids.end();
            dc.SetBrush(wxBrush(skipped ? wxColour(170, 174, 180) : colors[i]));
            dc.SetPen(wxPen(*wxWHITE, 2));
            dc.DrawCircle(points[i], 25);
            dc.SetTextForeground(*wxWHITE);
            dc.DrawText(wxString::Format("ID %zu", i), points[i].x - 16, points[i].y - 8);
        }
    });
    camera.body->Add(m_plate_preview, 1, wxEXPAND | wxALL, 8);
    camera.body->Add(body_text(camera.panel, u8(u8"Pr\u00e9-visualiza\u00e7\u00e3o simulada")), 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);
    camera.body->Add(body_text(camera.panel, u8(u8"Ciano, magenta e verde identificam os objetos da tarefa.")), 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);
    center->Add(camera.panel, 1, wxEXPAND | wxBOTTOM, 14);
    content->Add(center, 2, wxEXPAND | wxRIGHT, 2);

    auto* right = new wxBoxSizer(wxVERTICAL);
    auto control = card(this, "Controle", kCyan);
    m_state = body_text(control.panel, wxEmptyString);
    m_temperatures = body_text(control.panel, wxEmptyString);
    control.body->Add(m_state, 0, wxALL, 12);
    control.body->Add(m_temperatures, 0, wxLEFT | wxRIGHT | wxBOTTOM, 12);
    auto* actions = new wxBoxSizer(wxHORIZONTAL);
    m_pause_resume = new wxButton(control.panel, wxID_ANY, u8(u8"Pausar"));
    m_stop = new wxButton(control.panel, wxID_ANY, u8(u8"Parar simula\u00e7\u00e3o"));
    m_skip_objects = new wxButton(control.panel, wxID_ANY, u8(u8"Ignorar objetos..."));
    m_pause_resume->SetBackgroundColour(kCyan);
    m_pause_resume->SetForegroundColour(*wxWHITE);
    m_skip_objects->SetBackgroundColour(kMagenta);
    m_skip_objects->SetForegroundColour(*wxWHITE);
    m_pause_resume->SetMinSize(wxSize(96, 32));
    m_stop->SetMinSize(wxSize(136, 32));
    m_skip_objects->SetMinSize(wxSize(150, 32));
    m_stop->SetBackgroundColour(wxColour(236, 238, 241));
    m_stop->SetForegroundColour(kInk);
    actions->Add(m_pause_resume, 0, wxRIGHT, 8);
    actions->Add(m_stop, 0, wxRIGHT, 8);
    actions->Add(m_skip_objects, 0);
    control.body->Add(actions, 0, wxLEFT | wxRIGHT | wxBOTTOM, 12);
    right->Add(control.panel, 0, wxEXPAND | wxTOP | wxBOTTOM, 14);

    auto materials = card(this, u8(u8"Gest\u00e3o de materiais"), kMagenta);
    m_materials = body_text(materials.panel, u8(u8"ACE: 4 posi\u00e7\u00f5es simuladas\nPLA 1   PLA 2   PLA 3   PLA 4"));
    materials.body->Add(m_materials, 0, wxALL, 12);
    right->Add(materials.panel, 0, wxEXPAND | wxBOTTOM, 14);
    auto device = card(this, u8(u8"Informa\u00e7\u00f5es do dispositivo"), kGreen);
    m_device = body_text(device.panel, wxEmptyString);
    device.body->Add(m_device, 0, wxALL, 12);
    right->Add(device.panel, 0, wxEXPAND | wxBOTTOM, 14);
    content->Add(right, 1, wxEXPAND | wxRIGHT, 14);
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
    m_state->SetLabel(wxString("Estado: ") + state_text(snapshot.state));
    m_state->SetForegroundColour(snapshot.state == AnycubicWorkbenchState::printing ? kGreen : kInk);
    m_job->SetLabel(wxString::FromUTF8(snapshot.job_name));
    m_layers->SetLabel(wxString::Format("Camadas: %u/%u   %.0f%%", snapshot.current_layer,
                                        snapshot.total_layers, snapshot.progress_percent));
    wxString object_summary = u8(u8"Objetos na placa: ");
    object_summary += wxString::Format("%zu", snapshot.print_objects.size());
    for (std::size_t i = 0; i < snapshot.print_objects.size(); ++i)
        object_summary += wxString::Format(u8(u8"\nID %zu  \u2014  %s"), i, wxString::FromUTF8(snapshot.print_objects[i]));
    m_objects->SetLabel(object_summary);
    m_progress->SetValue(static_cast<int>(snapshot.progress_percent));
    m_temperatures->SetLabel(wxString::Format(u8(u8"Bico: %.0f/%.0f \u00b0C   Mesa: %.0f/%.0f \u00b0C"),
                                               snapshot.nozzle_temperature, snapshot.nozzle_target_temperature,
                                               snapshot.bed_temperature, snapshot.bed_target_temperature));
    m_device->SetLabel(wxString::Format(u8(u8"%s\nN\u00famero de s\u00e9rie: %s\nHost: simulador local"),
                                         wxString::FromUTF8(snapshot.display_name), wxString::FromUTF8(snapshot.serial_number)));
    m_pause_resume->SetLabel(snapshot.state == AnycubicWorkbenchState::paused ? u8(u8"Retomar") : u8(u8"Pausar"));
    m_pause_resume->Enable(snapshot.state == AnycubicWorkbenchState::printing || snapshot.state == AnycubicWorkbenchState::paused);
    m_stop->Enable(snapshot.state == AnycubicWorkbenchState::printing || snapshot.state == AnycubicWorkbenchState::paused);
    m_skip_objects->Enable(snapshot.state == AnycubicWorkbenchState::printing && !snapshot.print_objects.empty());
    Layout();
}

void AnycubicWorkbenchPanel::on_timer(wxTimerEvent&) { m_simulator.tick(1); refresh_snapshot(); }

void AnycubicWorkbenchPanel::on_pause_resume(wxCommandEvent&)
{
    const auto state = m_simulator.snapshot().state;
    m_simulator.set_state(state == AnycubicWorkbenchState::paused ? AnycubicWorkbenchState::printing : AnycubicWorkbenchState::paused);
    refresh_snapshot();
}

void AnycubicWorkbenchPanel::on_stop(wxCommandEvent&) { m_simulator.set_state(AnycubicWorkbenchState::idle); refresh_snapshot(); }

void AnycubicWorkbenchPanel::on_skip_objects(wxCommandEvent&)
{
    const auto& snapshot = m_simulator.snapshot();
    wxDialog dialog(this, wxID_ANY, u8(u8"Ignorar objetos"), wxDefaultPosition, wxSize(760, 520), wxDEFAULT_DIALOG_STYLE | wxRESIZE_BORDER);
    dialog.SetBackgroundColour(kBackground);
    auto* root = new wxBoxSizer(wxVERTICAL);
    root->Add(heading(&dialog, u8(u8"Selecione os objetos que ser\u00e3o ignorados"), 13), 0, wxALL, 18);
    root->Add(new wxStaticText(&dialog, wxID_ANY, u8(u8"Marque ou desmarque objetos. O ID exibido na placa corresponde ao ID da lista.")), 0, wxLEFT | wxRIGHT | wxBOTTOM, 18);
    auto* body = new wxBoxSizer(wxHORIZONTAL);
    auto* preview = new wxPanel(&dialog, wxID_ANY, wxDefaultPosition, wxSize(390, 300));
    preview->SetBackgroundColour(kSlate);
    preview->Bind(wxEVT_PAINT, [&snapshot, preview](wxPaintEvent&) {
        wxAutoBufferedPaintDC dc(preview); dc.SetBackground(wxBrush(kSlate)); dc.Clear();
        const wxSize size = preview->GetClientSize();
        const wxRect plate(28, 28, std::max(180, size.x - 56), std::max(160, size.y - 56));
        dc.SetBrush(wxBrush(wxColour(226, 229, 233))); dc.SetPen(wxPen(wxColour(155, 160, 166), 1)); dc.DrawRoundedRectangle(plate, 10);
        const wxColour colors[] = {kCyan, kMagenta, kGreen};
        const wxPoint points[] = {{plate.x + plate.width / 3, plate.y + plate.height / 3}, {plate.x + plate.width * 2 / 3, plate.y + plate.height / 3}, {plate.x + plate.width / 2, plate.y + plate.height * 2 / 3}};
        for (std::size_t i = 0; i < snapshot.print_objects.size() && i < 3; ++i) {
            const bool skipped = std::find(snapshot.skipped_object_ids.begin(), snapshot.skipped_object_ids.end(), static_cast<std::uint32_t>(i)) != snapshot.skipped_object_ids.end();
            dc.SetBrush(wxBrush(skipped ? wxColour(170, 174, 180) : colors[i])); dc.SetPen(wxPen(*wxWHITE, 2)); dc.DrawCircle(points[i], 38);
            dc.SetTextForeground(*wxWHITE); dc.DrawText(wxString::Format("ID %zu", i), points[i].x - 18, points[i].y - 9);
        }
        dc.SetTextForeground(wxColour(45, 45, 48)); dc.DrawText(u8(u8"Placa de impress\u00e3o"), 42, 38);
    });
    body->Add(preview, 1, wxEXPAND | wxRIGHT, 18);
    auto* list = new wxCheckListBox(&dialog, wxID_ANY);
    list->SetBackgroundColour(kSurface);
    list->SetForegroundColour(kInk);
    for (std::size_t i = 0; i < snapshot.print_objects.size(); ++i) {
        const bool skipped = std::find(snapshot.skipped_object_ids.begin(), snapshot.skipped_object_ids.end(), static_cast<std::uint32_t>(i)) != snapshot.skipped_object_ids.end();
        list->Append(wxString::Format(u8(u8"ID %zu  \u2014  %s%s"), i, wxString::FromUTF8(snapshot.print_objects[i]), skipped ? u8(u8" (j\u00e1 ignorado)") : wxString()));
        list->Check(static_cast<unsigned int>(i), skipped);
    }
    body->Add(list, 1, wxEXPAND); root->Add(body, 1, wxEXPAND | wxLEFT | wxRIGHT, 18);
    root->Add(new wxStaticText(&dialog, wxID_ANY, u8(u8"Objetos marcados ser\u00e3o pulados pelo simulador e ficar\u00e3o identificados na placa.")), 0, wxLEFT | wxRIGHT | wxTOP, 18);
    auto* buttons = dialog.CreateStdDialogButtonSizer(wxOK | wxCANCEL);
    root->Add(buttons, 0, wxEXPAND | wxALL, 18);
    dialog.SetSizerAndFit(root); dialog.SetMinSize(wxSize(680, 460));
    if (dialog.ShowModal() != wxID_OK) return;
    std::vector<std::uint32_t> ids;
    for (unsigned int i = 0; i < list->GetCount(); ++i) if (list->IsChecked(i)) ids.push_back(static_cast<std::uint32_t>(i));
    if (ids.empty()) return;
    if (wxMessageBox(u8(u8"Os objetos selecionados ser\u00e3o ignorados somente no simulador. Deseja continuar?"), u8(u8"Confirmar a\u00e7\u00e3o"), wxYES_NO | wxICON_WARNING, this) != wxYES) return;
    m_simulator.skip_objects(ids); refresh_snapshot(); m_plate_preview->Refresh();
}

} // namespace Slic3r::GUI
