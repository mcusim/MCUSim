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
#ifndef MSIM_MAIN_WINDOW_H_
#define MSIM_MAIN_WINDOW_H_ 1

#include <wx/wx.h>
#include <wx/menu.h>

class MCUSimMainWindow : public wxFrame
{
public:
	MCUSimMainWindow(const wxString &title, const wxSize &size);

	void OnQuit(wxCommandEvent &event);

	wxMenuBar *menubar;
	wxMenu *file_menu;
	wxMenu *edit_menu;
	wxMenu *view_menu;
	wxMenu *place_menu;
	wxMenu *pref_menu;
	wxMenu *tools_menu;
	wxMenu *help_menu;
};

#endif /* MSIM_MAIN_WINDOW_H_ */
