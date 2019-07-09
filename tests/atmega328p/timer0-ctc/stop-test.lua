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

VERBOSE = true			-- Switch on to enable verbose output
TICK_TIME = 0.0			-- clock period, in us

ticks_passed = 0
check_point = 0			-- TCNT0 is 0 at start, count up to 255 then
						-- and reset by overflow interrupt

-- This function will be called by the simulator only once to configure
-- model before start of a simulation.
function module_conf(mcu)
	-- Re-calculate clock period, in us
	TICK_TIME = (1.0/MSIM_Freq(mcu))*1000000.0
	if VERBOSE then
		print("[Test stopper] MCU clock: " .. MSIM_Freq(mcu)/1000 ..
		      "kHz")
		print("[Test stopper] MCU clock period: " .. TICK_TIME ..
		      "us")
	end
end

-- This function will be called by the simulator periodically according to the
-- main simulation loop, i.e. time passed between two neighbor calls to the
-- function is a period.
function module_tick(mcu)
	if ticks_passed > 400000 then
		-- Test finished successfully
		MSIM_SetState(mcu, AVR_MSIM_STOP)
		print("[Test stopper] ticks passed: " .. ticks_passed)
	end

	ticks_passed = ticks_passed + 1
end

