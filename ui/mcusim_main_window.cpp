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
#include <wx/wx.h>
#include <wx/menu.h>
#include <wx/image.h>

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

	wxToolBar *toolbar = CreateToolBar();
	toolbar->AddTool(wxID_EXIT, zoomin_icon, wxT("Zoom in"));
	toolbar->AddTool(wxID_EXIT, zoomout_icon, wxT("Zoom out"));
	toolbar->AddTool(wxID_EXIT, zoomorig_icon, wxT("Zoom original"));
	toolbar->AddSeparator();
	toolbar->AddTool(wxID_EXIT, help_icon, wxT("Open help"));
	toolbar->Realize();

	Connect(wxID_EXIT, wxEVT_COMMAND_MENU_SELECTED,
		wxCommandEventHandler(MCUSimMainWindow::OnQuit));
	Connect(wxID_EXIT, wxEVT_COMMAND_TOOL_CLICKED,
		wxCommandEventHandler(MCUSimMainWindow::OnQuit));
	Centre();
};

void MCUSimMainWindow::OnQuit(wxCommandEvent &event)
{
	Close(true);
};
