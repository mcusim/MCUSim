--[[

  This file is part of MCUSim, an XSPICE library with microcontrollers.

  Copyright (C) 2017-2019 MCUSim Developers, see AUTHORS.txt for contributors.

  MCUSim is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  MCUSim is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <https://www.gnu.org/licenses/>.

--]]

--[[ Script which stops simulation in 5 seconds. --]]

VERBOSE = true			-- Switch on to enable verbose output
TICK_TIME = 0.0			-- Clock period, in us
TIMEOUT = 5000000		-- Stop simulation after timeout, in us

ticks_left = 0

-- This function will be called by the simulator only once to configure
-- model before start of a simulation.
function module_conf(mcu)
	TICK_TIME = (1.0/MSIM_Freq(mcu))*1000000.0
	ticks_left = TIMEOUT/TICK_TIME

	local timeout = ticks_left*TICK_TIME
	if VERBOSE then
		print("[stop-in-5s] Clock period: " .. TICK_TIME .. "us")
		print("[stop-in-5s] Timeout: " .. TIMEOUT .. "us")
		print("[stop-in-5s] Ticks left: " .. ticks_left)
		print("[stop-in-5s] Actual timeout: " .. timeout .. "us")
	end
end

-- This function will be called by the simulator periodically according to the
-- main simulation loop, i.e. time passed between two neighbor calls to the
-- function is a period.
function module_tick(mcu)
	ticks_left = ticks_left - 1
	if ticks_left == 0 then
		MSIM_SetState(mcu, AVR_MSIM_STOP)
	end
end
