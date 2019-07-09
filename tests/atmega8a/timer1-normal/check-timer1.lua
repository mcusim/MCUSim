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
ticks_passed = 0
check_point = 0

function module_conf(mcu)
end

function module_tick(mcu)
	if check_point == 0 and AVR_ReadIO(mcu, TCNT1H) == 0 then
		check_point = check_point + 1
	elseif check_point == 1 and AVR_ReadIO(mcu, TCNT1H) == 255 then
		check_point = check_point + 1
	elseif check_point == 2 and AVR_ReadIO(mcu, TCNT1H) == 0 then
		check_point = check_point + 1
	elseif check_point == 3 and AVR_ReadIO(mcu, TCNT1H) == 255 then
		check_point = check_point + 1
	elseif check_point == 4 and AVR_ReadIO(mcu, TCNT1H) == 64 then
		-- Test finished successfully
		MSIM_SetState(mcu, AVR_MSIM_STOP)
		print("ticks passed: " .. ticks_passed)
	elseif ticks_passed > 1500000 then
		-- Test failed
		MSIM_SetState(mcu, AVR_MSIM_TESTFAIL)
		print("ticks passed: " .. ticks_passed)
	end

	ticks_passed = ticks_passed + 1
end
