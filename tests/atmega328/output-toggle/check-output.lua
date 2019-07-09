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

--[[
This script checks for a simple pattern of port outputs.
--]]

VERBOSE = true			-- Switch on to enable verbose output
TICK_TIME = 0.0			-- clock period, in us

ticks_passed = 0
check_point = 0			-- PORTB is 0 at start, count up to 255 then
				-- and reset by overflow interrupt

-- This function will be called by the simulator only once to configure
-- model before start of a simulation.
function module_conf(mcu)
	-- Re-calculate clock period, in us
	TICK_TIME = (1.0/MSIM_Freq(mcu))*1000000.0
	if VERBOSE then
		print("[PORTB checker] MCU clock: " .. MSIM_Freq(mcu)/1000 ..
		      "kHz")
		print("[PORTB checker] MCU clock period: " .. TICK_TIME ..
		      "us")
	end
end

-- This function will be called by the simulator periodically according to the
-- main simulation loop, i.e. time passed between two neighbor calls to the
-- function is a period.
function module_tick(mcu)
	if check_point == 0 and AVR_ReadIO(mcu, PORTB) == 0 then
		print("[PORTB checker] check 1")
		check_point = check_point + 1
	elseif check_point == 1 and AVR_ReadIO(mcu, PORTB) == 255 then
		print("[PORTB checker] check 2")
		check_point = check_point + 1
	elseif check_point == 2 and AVR_ReadIO(mcu, PORTB) == 254 then
		print("[PORTB checker] check 3")
		check_point = check_point + 1
	elseif check_point == 3 and AVR_ReadIO(mcu, PORTB) == 255 then
		-- Test finished successfully
		MSIM_SetState(mcu, AVR_MSIM_STOP)
		print("[PORTB checker] ticks passed: " .. ticks_passed)
	elseif ticks_passed > 200 then
		-- Test failed
		MSIM_SetState(mcu, AVR_MSIM_TESTFAIL)
		print("[PORTB checker] ticks passed: " .. ticks_passed)
	end

	ticks_passed = ticks_passed + 1
end
