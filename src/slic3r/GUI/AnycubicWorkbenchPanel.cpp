#include "AnycubicWorkbenchPanel.hpp"
#include "GUI_App.hpp"
#include "GLCanvas3D.hpp"
#include "Plater.hpp"

#include <algorithm>
#include <cmath>
#include <limits>
#include <utility>
#include <wx/button.h>
#include <wx/checklst.h>
#include <wx/dcbuffer.h>
#include <wx/dialog.h>
#include <wx/gauge.h>
#include <wx/msgdlg.h>
#include <wx/sizer.h>
#include <wx/statbmp.h>
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
constexpr int kCardRadius = 12;
constexpr int kCardGap = 12;
constexpr int kCardPadding = 10;

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

std::vector<wxPoint> object_points(const wxRect& plate, std::size_t count)
{
    const wxPoint fallback[] = {{plate.x + plate.width / 3, plate.y + plate.height / 3},
                                {plate.x + plate.width * 2 / 3, plate.y + plate.height / 3},
                                {plate.x + plate.width / 2, plate.y + plate.height * 2 / 3}};
    std::vector<wxPoint> points;
    points.reserve(count);
    if (wxGetApp().plater() == nullptr || wxGetApp().plater()->model().objects.empty()) {
        for (std::size_t i = 0; i < count; ++i)
            points.push_back(fallback[i % 3]);
        return points;
    }

    const auto& objects = wxGetApp().plater()->model().objects;
    const std::size_t available = std::min(count, objects.size());
    double min_x = std::numeric_limits<double>::max();
    double max_x = std::numeric_limits<double>::lowest();
    double min_y = std::numeric_limits<double>::max();
    double max_y = std::numeric_limits<double>::lowest();
    for (std::size_t i = 0; i < available; ++i) {
        if (objects[i] == nullptr || objects[i]->instances.empty())
            continue;
        const Vec3d offset = objects[i]->instances.front()->get_offset();
        min_x = std::min(min_x, offset.x()); max_x = std::max(max_x, offset.x());
        min_y = std::min(min_y, offset.y()); max_y = std::max(max_y, offset.y());
    }
    if (min_x == std::numeric_limits<double>::max()) {
        for (std::size_t i = 0; i < count; ++i)
            points.push_back(fallback[i % 3]);
        return points;
    }

    const double range_x = std::max(1.0, max_x - min_x);
    const double range_y = std::max(1.0, max_y - min_y);
    const double padding_x = plate.width * 0.16;
    const double padding_y = plate.height * 0.16;
    for (std::size_t i = 0; i < count; ++i) {
        if (i >= available || objects[i] == nullptr || objects[i]->instances.empty()) {
            points.push_back(fallback[i % 3]);
            continue;
        }
        const Vec3d offset = objects[i]->instances.front()->get_offset();
        const double nx = (offset.x() - min_x) / range_x;
        const double ny = (offset.y() - min_y) / range_y;
        points.emplace_back(static_cast<int>(plate.x + padding_x + nx * (plate.width - 2 * padding_x)),
                            static_cast<int>(plate.y + padding_y + (1.0 - ny) * (plate.height - 2 * padding_y)));
    }
    return points;
}

std::vector<wxRect> object_shapes(const wxRect& plate, std::size_t count, int base_radius)
{
    const auto points = object_points(plate, count);
    std::vector<wxRect> shapes;
    shapes.reserve(count);
    const int fallback_size = base_radius * 2;
    if (wxGetApp().plater() == nullptr || wxGetApp().plater()->model().objects.empty()) {
        for (const wxPoint& point : points)
            shapes.emplace_back(point.x - base_radius, point.y - base_radius, fallback_size, fallback_size);
        return shapes;
    }

    const auto& objects = wxGetApp().plater()->model().objects;
    double largest_span = 1.0;
    for (std::size_t i = 0; i < std::min(count, objects.size()); ++i) {
        if (objects[i] == nullptr || objects[i]->volumes.empty())
            continue;
        const Vec3d size = objects[i]->raw_mesh_bounding_box().size();
        largest_span = std::max(largest_span, std::max(std::abs(size.x()), std::abs(size.y())));
    }
    for (std::size_t i = 0; i < points.size(); ++i) {
        int width = fallback_size;
        int height = fallback_size;
        if (i < objects.size() && objects[i] != nullptr && !objects[i]->volumes.empty()) {
            const Vec3d size = objects[i]->raw_mesh_bounding_box().size();
            width = std::clamp(static_cast<int>(base_radius * 2.4 * std::abs(size.x()) / largest_span), 28, base_radius * 3);
            height = std::clamp(static_cast<int>(base_radius * 2.4 * std::abs(size.y()) / largest_span), 28, base_radius * 3);
        }
        shapes.emplace_back(points[i].x - width / 2, points[i].y - height / 2, width, height);
    }
    return shapes;
}

std::vector<double> object_rotations(std::size_t count)
{
    std::vector<double> rotations(count, 0.0);
    if (wxGetApp().plater() == nullptr)
        return rotations;
    const auto& objects = wxGetApp().plater()->model().objects;
    for (std::size_t i = 0; i < std::min(count, objects.size()); ++i) {
        if (objects[i] != nullptr && !objects[i]->instances.empty())
            rotations[i] = objects[i]->instances.front()->get_rotation().z();
    }
    return rotations;
}

wxString short_object_name(const std::string& name)
{
    wxString label = wxString::FromUTF8(name);
    if (label.Length() > 14)
        label = label.Left(13) + "...";
    return label;
}

wxBitmap load_printer_cover(const wxSize& requested_size)
{
    const std::string path = Slic3r::resources_dir() + "/profiles/Anycubic/Anycubic Kobra 4 Combo_cover.png";
    wxImage image(wxString::FromUTF8(path), wxBITMAP_TYPE_PNG);
    if (!image.IsOk())
        return wxBitmap();
    const wxSize size = image.GetSize();
    const int width = std::max(1, requested_size.x);
    const int height = std::max(1, requested_size.y);
    const double scale = std::min(static_cast<double>(width) / size.x, static_cast<double>(height) / size.y);
    image.Rescale(std::max(1, static_cast<int>(size.x * scale)),
                  std::max(1, static_cast<int>(size.y * scale)), wxIMAGE_QUALITY_HIGH);
    return wxBitmap(image);
}

struct WorkbenchCard
{
    wxPanel*     panel;
    wxPanel*     surface;
    wxBoxSizer*  body;
};

WorkbenchCard card(wxWindow* parent, const wxString& title, const wxColour& accent)
{
    auto* panel = new wxPanel(parent);
    panel->SetBackgroundColour(kBackground);
    auto* surface = new wxPanel(panel);
    surface->SetBackgroundColour(kSurface);
    surface->SetBackgroundStyle(wxBG_STYLE_PAINT);
    surface->Bind(wxEVT_PAINT, [surface](wxPaintEvent&) {
        wxAutoBufferedPaintDC dc(surface);
        dc.SetBackground(wxBrush(kSurface));
        dc.Clear();
        wxRect bounds = surface->GetClientRect();
        bounds.Deflate(1);
        dc.SetBrush(wxBrush(kSurface));
        dc.SetPen(wxPen(wxColour(226, 229, 234), 1));
        dc.DrawRoundedRectangle(bounds, kCardRadius);
    });
    auto* outer = new wxBoxSizer(wxVERTICAL);
    outer->Add(surface, 1, wxEXPAND | wxALL, 2);
    panel->SetSizer(outer);
    auto* root = new wxBoxSizer(wxVERTICAL);
    auto* header = new wxPanel(surface);
    header->SetBackgroundColour(kSurface);
    auto* header_sizer = new wxBoxSizer(wxVERTICAL);
    auto* label = new wxStaticText(header, wxID_ANY, title);
    wxFont font = label->GetFont();
    font.SetWeight(wxFONTWEIGHT_BOLD);
    font.SetPointSize(11);
    label->SetFont(font);
    label->SetForegroundColour(kInk);
    label->SetBackgroundColour(kSurface);
    header_sizer->Add(label, 0, wxLEFT | wxRIGHT | wxTOP, kCardPadding + 2);
    auto* stripe = new wxPanel(header, wxID_ANY, wxDefaultPosition, wxSize(-1, 3));
    stripe->SetBackgroundColour(accent);
    stripe->SetMinSize(wxSize(-1, 3));
    header_sizer->Add(stripe, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, kCardPadding);
    header->SetSizer(header_sizer);
    root->Add(header, 0, wxEXPAND);
    auto* body = new wxBoxSizer(wxVERTICAL);
    root->Add(body, 1, wxEXPAND | wxALL, kCardPadding);
    surface->SetSizer(root);
    return {panel, surface, body};
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
    auto* selected = new wxPanel(printers.surface);
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
    printers.body->Add(body_text(printers.surface, u8(u8"Modo simulado \u2014 nenhuma conex\u00e3o ativa"), kSurface), 0, wxALL, 10);
    content->Add(printers.panel, 0, wxEXPAND | wxALL, kCardGap);

    auto* center = new wxBoxSizer(wxVERTICAL);
    auto job = card(this, u8(u8"Tarefa de impress\u00e3o"), kCyan);
    m_job = body_text(job.surface, wxEmptyString);
    m_layers = body_text(job.surface, wxEmptyString);
    m_objects = body_text(job.surface, wxEmptyString);
    m_progress = new wxGauge(job.surface, wxID_ANY, 100);
    m_progress->SetForegroundColour(kCyan);
    auto* task_row = new wxBoxSizer(wxHORIZONTAL);
    m_task_thumbnail = new wxPanel(job.surface, wxID_ANY, wxDefaultPosition, wxSize(150, 112));
    m_task_thumbnail->SetBackgroundColour(wxColour(248, 249, 251));
    m_task_thumbnail->Bind(wxEVT_PAINT, [this](wxPaintEvent&) {
        wxAutoBufferedPaintDC dc(m_task_thumbnail);
        dc.SetBackground(wxBrush(wxColour(248, 249, 251))); dc.Clear();
        const wxSize size = m_task_thumbnail->GetClientSize();
        const wxRect plate(12, 12, std::max(80, size.x - 24), std::max(60, size.y - 24));
        dc.SetBrush(wxBrush(wxColour(231, 234, 238))); dc.SetPen(wxPen(wxColour(205, 209, 215), 1));
        dc.DrawRoundedRectangle(plate, 8);
        const wxColour colors[] = {kCyan, kMagenta, kGreen};
        const auto points = object_points(plate, 3);
        const auto shapes = object_shapes(plate, 3, 15);
        for (std::size_t i = 0; i < shapes.size() && i < 3; ++i) {
            dc.SetBrush(wxBrush(colors[i])); dc.SetPen(wxPen(*wxWHITE, 1));
            dc.DrawRoundedRectangle(shapes[i], 4);
            dc.SetTextForeground(*wxWHITE);
            dc.DrawText(wxString::Format("%zu", i + 1), points[i].x - 4, points[i].y - 7);
        }
    });
    task_row->Add(m_task_thumbnail, 0, wxEXPAND | wxRIGHT, 14);
    auto* task_details = new wxBoxSizer(wxVERTICAL);
    task_details->Add(m_job, 0, wxBOTTOM, 10);
    task_details->Add(m_layers, 0, wxBOTTOM, 6);
    task_details->Add(m_objects, 0, wxBOTTOM, 6);
    task_row->Add(task_details, 1, wxEXPAND);
    job.body->Add(task_row, 0, wxEXPAND | wxALL, 12);
    job.body->Add(m_progress, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 12);
    center->Add(job.panel, 0, wxEXPAND | wxTOP | wxBOTTOM, kCardGap);

    auto camera = card(this, u8(u8"C\u00e2mera / placa de impress\u00e3o"), kGreen);
    m_plate_preview = new wxPanel(camera.surface, wxID_ANY, wxDefaultPosition, wxSize(320, 190));
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
        const auto points = object_points(plate, 3);
        const auto shapes = object_shapes(plate, 3, 25);
        const auto rotations = object_rotations(3);
        for (std::size_t i = 0; i < 3; ++i) {
            const bool skipped = std::find(m_simulator.snapshot().skipped_object_ids.begin(),
                                           m_simulator.snapshot().skipped_object_ids.end(),
                                           static_cast<std::uint32_t>(i)) != m_simulator.snapshot().skipped_object_ids.end();
            dc.SetBrush(wxBrush(skipped ? wxColour(170, 174, 180) : colors[i]));
            dc.SetPen(wxPen(*wxWHITE, 2));
            dc.DrawRoundedRectangle(shapes[i], 8);
            dc.SetTextForeground(*wxWHITE);
            dc.DrawText(wxString::Format("ID %zu", i), points[i].x - 16, points[i].y - 8);
            if (i < m_simulator.snapshot().print_objects.size()) {
                dc.SetTextForeground(kInk);
                dc.DrawText(short_object_name(m_simulator.snapshot().print_objects[i]), shapes[i].x,
                            shapes[i].GetBottom() + 4);
            }
            const int arrow_length = std::max(8, std::min(shapes[i].GetWidth(), shapes[i].GetHeight()) / 3);
            const wxPoint tip(points[i].x + static_cast<int>(std::cos(rotations[i]) * arrow_length),
                              points[i].y - static_cast<int>(std::sin(rotations[i]) * arrow_length));
            dc.SetPen(wxPen(*wxWHITE, 2));
            dc.DrawLine(points[i], tip);
            if (skipped)
                dc.DrawText("IGN", shapes[i].GetRight() - 24, shapes[i].GetTop() + 3);
            if (m_selected_object == static_cast<int>(i) ||
                std::find(m_selected_objects.begin(), m_selected_objects.end(), static_cast<int>(i)) != m_selected_objects.end()) {
                dc.SetBrush(*wxTRANSPARENT_BRUSH);
                dc.SetPen(wxPen(kInk, 3));
                wxRect selected_shape = shapes[i];
                selected_shape.Inflate(4);
                dc.DrawRoundedRectangle(selected_shape, 10);
            }
        }
    });
    m_plate_preview->Bind(wxEVT_LEFT_DOWN, [this](wxMouseEvent& event) {
        const wxSize size = m_plate_preview->GetClientSize();
        const wxRect plate(30, 22, std::max(140, size.x - 60), std::max(110, size.y - 44));
        const auto shapes = object_shapes(plate, 3, 25);
        m_selected_object = -1;
        for (std::size_t i = 0; i < 3; ++i) {
            wxRect hit_shape = shapes[i];
            hit_shape.Inflate(8);
            if (hit_shape.Contains(event.GetPosition())) {
                m_selected_object = static_cast<int>(i);
                m_selected_objects = {m_selected_object};
                if (wxGetApp().plater() != nullptr &&
                    static_cast<std::size_t>(m_selected_object) < wxGetApp().plater()->model().objects.size()) {
                    auto* canvas = wxGetApp().plater()->get_view3D_canvas3D();
                    if (canvas != nullptr) {
                        auto& selection = canvas->get_selection();
                        selection.clear();
                        selection.add_object(static_cast<unsigned int>(m_selected_object));
                        canvas->set_as_dirty();
                    }
                }
                break;
            }
        }
        m_plate_preview->Refresh();
    });
    m_plate_preview->Bind(wxEVT_MOTION, [this](wxMouseEvent& event) {
        const wxSize size = m_plate_preview->GetClientSize();
        const wxRect plate(30, 22, std::max(140, size.x - 60), std::max(110, size.y - 44));
        const auto shapes = object_shapes(plate, m_simulator.snapshot().print_objects.size(), 25);
        wxString tooltip;
        for (std::size_t i = 0; i < shapes.size() && i < m_simulator.snapshot().print_objects.size() && i < 3; ++i) {
            wxRect hit_shape = shapes[i];
            hit_shape.Inflate(8);
            if (!hit_shape.Contains(event.GetPosition()))
                continue;
            const bool skipped = std::find(m_simulator.snapshot().skipped_object_ids.begin(),
                                           m_simulator.snapshot().skipped_object_ids.end(),
                                           static_cast<std::uint32_t>(i)) != m_simulator.snapshot().skipped_object_ids.end();
            tooltip = wxString::Format(u8(u8"ID %zu\n%s\nEstado: %s"), i,
                                       wxString::FromUTF8(m_simulator.snapshot().print_objects[i]),
                                       skipped ? u8(u8"Ignorado") : u8(u8"Na tarefa"));
            break;
        }
        m_plate_preview->SetToolTip(tooltip);
        event.Skip();
    });
    camera.body->Add(m_plate_preview, 1, wxEXPAND | wxALL, 8);
    camera.body->Add(body_text(camera.surface, u8(u8"Pr\u00e9-visualiza\u00e7\u00e3o simulada")), 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);
    camera.body->Add(body_text(camera.surface, u8(u8"Legenda: contorno escuro = selecionado | seta = orienta\u00e7\u00e3o | cinza + IGN = ignorado.")), 0, wxLEFT | wxRIGHT | wxBOTTOM, 10);
    center->Add(camera.panel, 1, wxEXPAND | wxBOTTOM, kCardGap);
    content->Add(center, 2, wxEXPAND | wxRIGHT, kCardGap / 2);

    auto* right = new wxBoxSizer(wxVERTICAL);
    auto control = card(this, "Controle", kCyan);
    m_state = body_text(control.surface, wxEmptyString);
    m_temperatures = body_text(control.surface, wxEmptyString);
    m_state->SetForegroundColour(kGreen);
    control.body->Add(m_state, 0, wxLEFT | wxRIGHT | wxTOP, 12);
    m_temperatures->Hide();
    m_control_metrics = new wxPanel(control.surface, wxID_ANY, wxDefaultPosition, wxSize(-1, 126));
    m_control_metrics->SetBackgroundColour(kSurface);
    m_control_metrics->Bind(wxEVT_PAINT, [this](wxPaintEvent&) {
        wxAutoBufferedPaintDC dc(m_control_metrics);
        dc.SetBackground(wxBrush(kSurface)); dc.Clear();
        const auto& snapshot = m_simulator.snapshot();
        const wxSize size = m_control_metrics->GetClientSize();
        const int gap = 7;
        const int width = std::max(92, (size.x - gap * 2) / 3);
        const int height = 53;
        const wxString labels[] = {"Bocal", "Mesa aquecida", "Luz da câmera", "Vent. modelo", "Modo de velocidade", "Detecção"};
        const wxString values[] = {
            wxString::Format("%.0f°C / %.0f°C", snapshot.nozzle_temperature, snapshot.nozzle_target_temperature),
            wxString::Format("%.0f°C / %.0f°C", snapshot.bed_temperature, snapshot.bed_target_temperature),
            snapshot.camera_available ? u8(u8"LIGADA") : u8(u8"DESLIGADA"),
            wxString::Format("%.0f%%", snapshot.fan_percent),
            "Standard",
            snapshot.state == AnycubicWorkbenchState::printing ? u8(u8"Ativa") : u8(u8"Aguardando")};
        for (int i = 0; i < 6; ++i) {
            const int x = (i % 3) * (width + gap);
            const int y = (i / 3) * (height + gap);
            const wxRect tile(x, y, width, height);
            dc.SetBrush(wxBrush(wxColour(249, 250, 252)));
            dc.SetPen(wxPen(wxColour(232, 234, 238), 1));
            dc.DrawRoundedRectangle(tile, 9);
            dc.SetTextForeground(wxColour(128, 132, 140));
            dc.DrawText(labels[i], tile.x + 10, tile.y + 8);
            dc.SetTextForeground(kInk);
            dc.DrawText(values[i], tile.x + 10, tile.y + 28);
        }
    });
    control.body->Add(m_control_metrics, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 10);
    auto* actions = new wxBoxSizer(wxHORIZONTAL);
    m_pause_resume = new wxButton(control.surface, wxID_ANY, u8(u8"Pausar"));
    m_stop = new wxButton(control.surface, wxID_ANY, u8(u8"Parar simula\u00e7\u00e3o"));
    m_skip_objects = new wxButton(control.surface, wxID_ANY, u8(u8"Ignorar objetos..."));
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
    right->Add(control.panel, 0, wxEXPAND | wxTOP | wxBOTTOM, kCardGap);

    auto materials = card(this, u8(u8"Gest\u00e3o de materiais"), kMagenta);
    m_materials = body_text(materials.surface, u8(u8"ACE: 4 posi\u00e7\u00f5es simuladas\nPLA 1   PLA 2   PLA 3   PLA 4"));
    materials.body->Add(m_materials, 0, wxLEFT | wxRIGHT | wxTOP, 12);
    m_material_spools = new wxPanel(materials.surface, wxID_ANY, wxDefaultPosition, wxSize(-1, 108));
    m_material_spools->SetBackgroundColour(kSurface);
    m_material_spools->Bind(wxEVT_PAINT, [this](wxPaintEvent&) {
        wxAutoBufferedPaintDC dc(m_material_spools);
        dc.SetBackground(wxBrush(kSurface)); dc.Clear();
        const wxColour filament[] = {wxColour(247, 231, 220), wxColour(234, 31, 83), wxColour(255, 181, 135), wxColour(154, 82, 47)};
        const wxString names[] = {"PLA", "PLA", "PLA", "PLA"};
        const wxSize size = m_material_spools->GetClientSize();
        const int slot = std::max(54, size.x / 4);
        for (int i = 0; i < 4; ++i) {
            const int x = slot * i + slot / 2;
            dc.SetPen(wxPen(wxColour(215, 218, 223), 1));
            dc.SetBrush(wxBrush(wxColour(245, 246, 248)));
            dc.DrawEllipse(x - 18, 14, 36, 62);
            dc.SetBrush(wxBrush(filament[i]));
            dc.DrawRoundedRectangle(x - 13, 19, 26, 52, 8);
            dc.SetBrush(wxBrush(wxColour(248, 249, 251)));
            dc.DrawEllipse(x - 7, 34, 14, 18);
            dc.SetTextForeground(kInk); dc.DrawText(names[i], x - 11, 79);
            dc.SetTextForeground(wxColour(120, 124, 132)); dc.DrawText(wxString::Format("%d", i + 1), x - 3, 94);
        }
    });
    materials.body->Add(m_material_spools, 0, wxEXPAND | wxLEFT | wxRIGHT | wxTOP, 8);
    right->Add(materials.panel, 0, wxEXPAND | wxBOTTOM, kCardGap);
    auto device = card(this, u8(u8"Informa\u00e7\u00f5es do dispositivo"), kGreen);
    m_device = body_text(device.surface, wxEmptyString);
    auto* device_row = new wxBoxSizer(wxHORIZONTAL);
    m_printer_image = new wxStaticBitmap(device.surface, wxID_ANY, load_printer_cover(wxSize(132, 100)));
    m_printer_image->SetBackgroundColour(kSurface);
    device_row->Add(m_printer_image, 0, wxALIGN_CENTER_VERTICAL | wxRIGHT, 12);
    device_row->Add(m_device, 1, wxALIGN_CENTER_VERTICAL);
    device.body->Add(device_row, 0, wxEXPAND | wxALL, 12);
    right->Add(device.panel, 0, wxEXPAND | wxBOTTOM, kCardGap);
    auto axis = card(this, u8(u8"Mover eixo"), kCyan);
    auto* axis_note = body_text(axis.surface, u8(u8"Controle seguro dispon\u00edvel quando a impressora estiver conectada."));
    axis_note->SetForegroundColour(wxColour(105, 110, 118));
    axis.body->Add(axis_note, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);
    auto* steps = new wxBoxSizer(wxHORIZONTAL);
    for (const wxString& step : {"1 mm", "15 mm", "50 mm"}) {
        auto* button = new wxButton(axis.surface, wxID_ANY, step, wxDefaultPosition, wxSize(70, 28));
        button->Enable(false);
        steps->Add(button, 0, wxRIGHT, 6);
    }
    axis.body->Add(steps, 0, wxLEFT | wxRIGHT | wxBOTTOM, 8);
    auto* dpad = new wxGridSizer(2, 3, 4, 4);
    for (const wxString& direction : {"X -", "Y +", "Z +", "X +", "Y -", "Z -"}) {
        auto* button = new wxButton(axis.surface, wxID_ANY, direction, wxDefaultPosition, wxSize(62, 28));
        button->Enable(false);
        dpad->Add(button, 1, wxEXPAND);
    }
    axis.body->Add(dpad, 0, wxEXPAND | wxLEFT | wxRIGHT | wxBOTTOM, 8);
    right->Add(axis.panel, 0, wxEXPAND | wxBOTTOM, kCardGap);
    content->Add(right, 1, wxEXPAND | wxRIGHT, kCardGap);
    root->Add(content, 1, wxEXPAND | wxLEFT | wxRIGHT, kCardGap);

    SetSizer(root);
    Bind(wxEVT_TIMER, &AnycubicWorkbenchPanel::on_timer, this);
    m_pause_resume->Bind(wxEVT_BUTTON, &AnycubicWorkbenchPanel::on_pause_resume, this);
    m_stop->Bind(wxEVT_BUTTON, &AnycubicWorkbenchPanel::on_stop, this);
    m_skip_objects->Bind(wxEVT_BUTTON, &AnycubicWorkbenchPanel::on_skip_objects, this);
    refresh_snapshot();
    m_timer.Start(1000);
}

void AnycubicWorkbenchPanel::sync_project_objects()
{
    if (wxGetApp().plater() == nullptr)
        return;

    const auto& objects = wxGetApp().plater()->model().objects;
    if (objects.empty())
        return;

    std::vector<std::string> names;
    names.reserve(objects.size());
    for (std::size_t i = 0; i < objects.size(); ++i) {
        const std::string fallback = "Objeto " + std::to_string(i + 1);
        names.push_back(objects[i] != nullptr && !objects[i]->name.empty() ? objects[i]->name : fallback);
    }
    m_simulator.set_print_objects(std::move(names));
    const int selected = wxGetApp().plater()->get_selected_object_idx();
    m_selected_object = selected >= 0 && static_cast<std::size_t>(selected) < objects.size() ? selected : -1;
    m_selected_objects.clear();
    if (auto* canvas = wxGetApp().plater()->get_view3D_canvas3D(); canvas != nullptr) {
        for (const auto& entry : canvas->get_selection().get_selected_object_instances()) {
            if (entry.first >= 0 && static_cast<std::size_t>(entry.first) < objects.size() &&
                std::find(m_selected_objects.begin(), m_selected_objects.end(), entry.first) == m_selected_objects.end())
                m_selected_objects.push_back(entry.first);
        }
    }
    if (m_selected_objects.empty() && m_selected_object >= 0)
        m_selected_objects.push_back(m_selected_object);
}

void AnycubicWorkbenchPanel::refresh_snapshot()
{
    sync_project_objects();
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
    if (m_control_metrics != nullptr)
        m_control_metrics->Refresh();
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
    wxCheckListBox* list = nullptr;
    preview->Bind(wxEVT_PAINT, [&snapshot, preview, &list](wxPaintEvent&) {
        wxAutoBufferedPaintDC dc(preview); dc.SetBackground(wxBrush(kSlate)); dc.Clear();
        const wxSize size = preview->GetClientSize();
        const wxRect plate(28, 28, std::max(180, size.x - 56), std::max(160, size.y - 56));
        dc.SetBrush(wxBrush(wxColour(226, 229, 233))); dc.SetPen(wxPen(wxColour(155, 160, 166), 1)); dc.DrawRoundedRectangle(plate, 10);
        const wxColour colors[] = {kCyan, kMagenta, kGreen};
        const auto points = object_points(plate, snapshot.print_objects.size());
        const auto shapes = object_shapes(plate, snapshot.print_objects.size(), 38);
        const auto rotations = object_rotations(snapshot.print_objects.size());
        for (std::size_t i = 0; i < snapshot.print_objects.size() && i < 3; ++i) {
            const bool skipped = list != nullptr && list->IsChecked(static_cast<unsigned int>(i));
            dc.SetBrush(wxBrush(skipped ? wxColour(170, 174, 180) : colors[i])); dc.SetPen(wxPen(*wxWHITE, 2)); dc.DrawRoundedRectangle(shapes[i], 10);
            dc.SetTextForeground(*wxWHITE); dc.DrawText(wxString::Format("ID %zu", i), points[i].x - 18, points[i].y - 9);
            dc.SetTextForeground(kInk);
            dc.DrawText(short_object_name(snapshot.print_objects[i]), shapes[i].x, shapes[i].GetBottom() + 4);
            const int arrow_length = std::max(12, std::min(shapes[i].GetWidth(), shapes[i].GetHeight()) / 3);
            const wxPoint tip(points[i].x + static_cast<int>(std::cos(rotations[i]) * arrow_length),
                              points[i].y - static_cast<int>(std::sin(rotations[i]) * arrow_length));
            dc.SetPen(wxPen(*wxWHITE, 2));
            dc.DrawLine(points[i], tip);
            if (skipped)
                dc.DrawText("IGN", shapes[i].GetRight() - 28, shapes[i].GetTop() + 4);
        }
        dc.SetTextForeground(wxColour(45, 45, 48)); dc.DrawText(u8(u8"Placa de impress\u00e3o"), 42, 38);
    });
    body->Add(preview, 1, wxEXPAND | wxRIGHT, 18);
    list = new wxCheckListBox(&dialog, wxID_ANY);
    list->SetBackgroundColour(kSurface);
    list->SetForegroundColour(kInk);
    for (std::size_t i = 0; i < snapshot.print_objects.size(); ++i) {
        const bool skipped = std::find(snapshot.skipped_object_ids.begin(), snapshot.skipped_object_ids.end(), static_cast<std::uint32_t>(i)) != snapshot.skipped_object_ids.end();
        list->Append(wxString::Format(u8(u8"ID %zu  \u2014  %s%s"), i, wxString::FromUTF8(snapshot.print_objects[i]), skipped ? u8(u8" (j\u00e1 ignorado)") : wxString()));
        list->Check(static_cast<unsigned int>(i), skipped);
    }
    auto* selected_count = body_text(&dialog, wxEmptyString, kBackground);
    auto update_selected_count = [list, selected_count]() {
        unsigned int count = 0;
        for (unsigned int i = 0; i < list->GetCount(); ++i)
            count += list->IsChecked(i) ? 1u : 0u;
        selected_count->SetLabel(wxString::Format(u8(u8"%u objeto(s) selecionado(s) para ignorar"), count));
    };
    update_selected_count();
    list->Bind(wxEVT_CHECKLISTBOX, [update_selected_count](wxCommandEvent&) { update_selected_count(); });
    preview->Bind(wxEVT_LEFT_DOWN, [preview, list, &snapshot, update_selected_count](wxMouseEvent& event) {
        const wxSize size = preview->GetClientSize();
        const wxRect plate(28, 28, std::max(180, size.x - 56), std::max(160, size.y - 56));
        const auto shapes = object_shapes(plate, snapshot.print_objects.size(), 38);
        for (std::size_t i = 0; i < snapshot.print_objects.size() && i < 3; ++i) {
            wxRect hit_shape = shapes[i];
            hit_shape.Inflate(6);
            if (hit_shape.Contains(event.GetPosition())) {
                list->Check(static_cast<unsigned int>(i), !list->IsChecked(static_cast<unsigned int>(i)));
                preview->Refresh();
                update_selected_count();
                break;
            }
        }
    });
    auto* list_column = new wxBoxSizer(wxVERTICAL);
    list_column->Add(selected_count, 0, wxLEFT | wxRIGHT | wxBOTTOM, 6);
    list_column->Add(list, 1, wxEXPAND);
    body->Add(list_column, 1, wxEXPAND); root->Add(body, 1, wxEXPAND | wxLEFT | wxRIGHT, 18);
    root->Add(new wxStaticText(&dialog, wxID_ANY, u8(u8"Objetos marcados ser\u00e3o pulados pelo simulador e ficar\u00e3o identificados na placa.")), 0, wxLEFT | wxRIGHT | wxTOP, 18);
    auto* buttons = dialog.CreateStdDialogButtonSizer(wxOK | wxCANCEL);
    root->Add(buttons, 0, wxEXPAND | wxALL, 18);
    dialog.SetSizerAndFit(root); dialog.SetMinSize(wxSize(680, 460));
    if (dialog.ShowModal() != wxID_OK) return;
    std::vector<std::uint32_t> ids;
    for (unsigned int i = 0; i < list->GetCount(); ++i) if (list->IsChecked(i)) ids.push_back(static_cast<std::uint32_t>(i));
    if (wxMessageBox(u8(u8"As altera\u00e7\u00f5es ser\u00e3o aplicadas somente no simulador. Deseja continuar?"), u8(u8"Confirmar a\u00e7\u00e3o"), wxYES_NO | wxICON_WARNING, this) != wxYES) return;
    m_simulator.set_skipped_objects(ids); refresh_snapshot(); m_plate_preview->Refresh();
}

} // namespace Slic3r::GUI
