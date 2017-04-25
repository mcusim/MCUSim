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

#include "mcusim/ui/mcusim_app.h"
#include "mcusim/ui/mcusim_main_window.h"

IMPLEMENT_APP(MCUSimApp)

bool MCUSimApp::OnInit()
{
	int width, height;

	width = wxSystemSettings::GetMetric(wxSYS_SCREEN_X);
	height = wxSystemSettings::GetMetric(wxSYS_SCREEN_Y);

	MCUSimMainWindow *mw = new MCUSimMainWindow(
			wxT("MCUSim"), wxSize(width, height));
	mw->Show(true);

	return true;
}
