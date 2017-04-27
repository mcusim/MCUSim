/*
 * MCUSim - Interactive simulator for microcontrollers.
 * Copyright (C) 2017 Dmitry Salychev <darkness.bsd@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdint.h>

#include <wx/wx.h>
#include <wx/menu.h>
#include <wx/image.h>
#include <wx/sizer.h>
#include <wx/dcbuffer.h>

#include "mcusim/ui/mcusim_main_window.h"

MCUSimMainWindow::MCUSimMainWindow(const wxString &title, const wxSize &size)
	: wxFrame(NULL, wxID_ANY, title, wxDefaultPosition, size)
{
	menubar = new wxMenuBar();
	file_menu = new wxMenu();
	file_menu->Append(wxID_EXIT, wxT("&Quit"));
	edit_menu = new wxMenu();
	view_menu = new wxMenu();
	place_menu = new wxMenu();
	pref_menu = new wxMenu();
	tools_menu = new wxMenu();
	help_menu = new wxMenu();

	menubar->Append(file_menu, wxT("&File"));
	menubar->Append(edit_menu, wxT("&Edit"));
	menubar->Append(view_menu, wxT("&View"));
	menubar->Append(place_menu, wxT("&Place"));
	menubar->Append(pref_menu, wxT("&Preferences"));
	menubar->Append(tools_menu, wxT("&Tools"));
	menubar->Append(help_menu, wxT("&Help"));
	SetMenuBar(menubar);

	wxImage::AddHandler(new wxPNGHandler());
	wxBitmap zoomin_icon(wxT("resources/zoom-in.png"),
			wxBITMAP_TYPE_PNG);
	wxBitmap zoomout_icon(wxT("resources/zoom-out.png"),
			wxBITMAP_TYPE_PNG);
	wxBitmap zoomorig_icon(wxT("resources/zoom-original.png"),
			wxBITMAP_TYPE_PNG);
	wxBitmap help_icon(wxT("resources/help.png"),
			wxBITMAP_TYPE_PNG);

	/* Create window layout built on sizers */
	vbox = new wxBoxSizer(wxVERTICAL);
	top_hbox = new wxBoxSizer(wxHORIZONTAL);
	mid_hbox = new wxBoxSizer(wxHORIZONTAL);
	bot_hbox = new wxBoxSizer(wxHORIZONTAL);
	vbox->Add(top_hbox, wxSizerFlags().Proportion(0).Expand());
	vbox->Add(mid_hbox, wxSizerFlags().Proportion(1).Expand());
	vbox->Add(bot_hbox, wxSizerFlags().Proportion(0).Expand());
	SetSizer(vbox);

	/* Top toolbar */
	top_toolbar = new wxToolBar(this, wxID_ANY);
	top_toolbar->AddTool(wxID_ANY, zoomin_icon, wxT("Zoom in"));
	top_toolbar->AddTool(wxID_ANY, zoomout_icon, wxT("Zoom out"));
	top_toolbar->AddTool(wxID_ANY, zoomorig_icon, wxT("Original size"));
	top_toolbar->AddTool(wxID_ANY, help_icon, wxT("Open manual"));
	top_toolbar->Realize();
	top_hbox->Add(top_toolbar, wxSizerFlags().Proportion(0).Expand());

	/* Left toolbar */
	left_toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition,
			wxDefaultSize, wxTB_VERTICAL | wxTB_FLAT);
	left_toolbar->AddTool(wxID_ANY, zoomin_icon, wxT("Zoom in"));
	left_toolbar->AddTool(wxID_ANY, zoomout_icon, wxT("Zoom out"));
	left_toolbar->AddTool(wxID_ANY, zoomorig_icon, wxT("Original size"));
	left_toolbar->AddTool(wxID_ANY, help_icon, wxT("Open manual"));
	left_toolbar->Realize();
	mid_hbox->Add(left_toolbar, wxSizerFlags().Proportion(0).Expand());

	/* Panel to draw on */
	draw_panel = new wxPanel(this, wxID_ANY);
	draw_panel->SetBackgroundStyle(wxBG_STYLE_PAINT);
	mid_hbox->Add(draw_panel, wxSizerFlags().Proportion(1).Expand());

	/* Right toolbar */
	right_toolbar = new wxToolBar(this, wxID_ANY, wxDefaultPosition,
			wxDefaultSize, wxTB_VERTICAL | wxTB_FLAT);
	right_toolbar->AddTool(wxID_ANY, zoomin_icon, wxT("Zoom in"));
	right_toolbar->AddTool(wxID_ANY, zoomout_icon, wxT("Zoom out"));
	right_toolbar->AddTool(wxID_ANY, zoomorig_icon, wxT("Original size"));
	right_toolbar->AddTool(wxID_ANY, help_icon, wxT("Open manual"));
	right_toolbar->Realize();
	mid_hbox->Add(right_toolbar, wxSizerFlags().Proportion(0).Expand());

	/* Status bar */
	status_bar = new wxStatusBar(this, wxID_ANY);
	status_bar->SetStatusText("Ready to simulate!", 0);
	bot_hbox->Add(status_bar, wxSizerFlags().Proportion(0).Expand());

	draw_panel->Connect(wxEVT_PAINT,
			wxPaintEventHandler(MCUSimMainWindow::OnPaint));
	Connect(wxID_ANY, wxEVT_COMMAND_MENU_SELECTED,
			wxCommandEventHandler(MCUSimMainWindow::OnQuit));

	Centre();
};

void MCUSimMainWindow::OnQuit(wxCommandEvent &event)
{
	Close(true);
};

void MCUSimMainWindow::OnPaint(wxPaintEvent &event)
{
	wxAutoBufferedPaintDC dc(draw_panel);
	wxColour white, gray;
	int dp_width, dp_height;

	draw_panel->GetSize(&dp_width, &dp_height);

	white.Set(wxT("#FFFFFF"));
	gray.Set(wxT("#D4D4D4"));

	dc.SetPen(wxPen(gray));
	dc.SetBrush(wxBrush(white));

	dc.DrawRectangle(0, 0, dp_width, dp_height);
};
