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
DELAY = 3			-- Number of system clock cycles between two
				-- neighbor rises and falls of the external
				-- clock.

clock_del = DELAY

function module_conf(mcu)
	TICK_TIME = (1.0/MSIM_Freq(mcu))*1000000.0
	if VERBOSE then
		print("MCU clock: " .. MSIM_Freq(mcu)/1000 .. "kHz")
		print("MCU clock period: " .. TICK_TIME .. "us")
	end
end

function module_tick(mcu)
	if clock_del == 0 and AVR_IOBit(mcu, PIND, 4) then
		AVR_SetIOBit(mcu, PIND, 4, 0)
		clock_del = DELAY
	elseif clock_del == 0 and not AVR_IOBit(mcu, PIND, 4) then
		AVR_SetIOBit(mcu, PIND, 4, 1)
		clock_del = DELAY
	elseif clock_del ~= 0 then
		clock_del = clock_del-1
	end
end
