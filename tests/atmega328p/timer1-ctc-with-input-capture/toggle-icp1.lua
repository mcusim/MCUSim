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
HALF_PERIOD = 1272330 -- in 8MHz clock cycles
ticks = 0
pb0 = 0
change = 0
change_tick = 0

-- This function will be called by the simulator only once to configure
-- model before start of a simulation.
function module_conf(mcu)
end

-- This function will be called by the simulator periodically according to the
-- main simulation loop, i.e. time passed between two neighbor calls to the
-- function is a period.
function module_tick(mcu)
	if AVR_IOBit(mcu, PORTB, 0) ~= pb0 then
		pb0 = AVR_IOBit(mcu, PORTB, 0)
		if change == 2 and (ticks-change_tick) == HALF_PERIOD then
			print("[check-pin] tick:" .. ticks ..
			      " previous tick:" .. change_tick ..
			      " delta:" .. ticks-change_tick)
		elseif change == 6 and (ticks-change_tick) == HALF_PERIOD then
			print("[check-pin] tick:" .. ticks ..
			      " previous tick:" .. change_tick ..
			      " delta:" .. ticks-change_tick)
		elseif change == 9 and (ticks-change_tick) == HALF_PERIOD then
			print("[check-pin] tick:" .. ticks ..
			      " previous tick:" .. change_tick ..
			      " delta:" .. ticks-change_tick)
			-- Test finished successfully
			MSIM_SetState(mcu, AVR_MSIM_STOP)
		elseif change == 10 then
			-- Test failed
			MSIM_SetState(mcu, AVR_MSIM_TESTFAIL)
			print("[check-pin] ticks passed: " .. ticks)
		end

		change = change+1
		change_tick = ticks
	end

	ticks = ticks+1
end
