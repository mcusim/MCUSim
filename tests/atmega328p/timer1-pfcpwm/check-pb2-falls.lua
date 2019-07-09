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
VERBOSE = false			-- Switch on to enable verbose output
TICK_TIME = 0.0			-- clock period, in us

ticks_passed = 0
check_point = 0
pb2_old = 1

function module_conf(mcu)
	-- Re-calculate clock period, in us
	TICK_TIME = (1.0/MSIM_Freq(mcu))*1000000.0
	if VERBOSE then
		print("MCU clock: " .. MSIM_Freq(mcu)/1000 .. "kHz")
		print("MCU clock period: " .. TICK_TIME .. "us")
	end
end

function module_tick(mcu)
	local tcnt1 = AVR_ReadIO16(mcu, TCNT1H, TCNT1L)
	local ocr1b = AVR_ReadIO16(mcu, OCR1BH, OCR1BL)

	if check_point == 0 then
		if pb2_old and not AVR_IOBit(mcu, PORTB, 2) then
			if tcnt1 == 896 and ocr1b == 768 then
				print("check_point: " .. check_point)
				check_point = check_point + 1
			end
		end
	elseif check_point == 1 then
		if pb2_old and not AVR_IOBit(mcu, PORTB, 2) then
			if tcnt1 == 768 and ocr1b == 640 then
				print("check_point: " .. check_point)
				check_point = check_point + 1
			end
		end
	elseif check_point == 2 then
		if pb2_old and not AVR_IOBit(mcu, PORTB, 2) then
			if tcnt1 == 640 and ocr1b == 512 then
				print("check_point: " .. check_point)
				check_point = check_point + 1
			end
		end
	elseif check_point == 3 then
		if pb2_old and not AVR_IOBit(mcu, PORTB, 2) then
			if tcnt1 == 512 and ocr1b == 384 then
				print("check_point: " .. check_point)
				check_point = check_point + 1
			end
		end
	elseif check_point == 4 then
		if pb2_old and not AVR_IOBit(mcu, PORTB, 2) then
			if tcnt1 == 384 and ocr1b == 256 then
				-- Test finished successfully
				MSIM_SetState(mcu, AVR_MSIM_STOP)
				print("check_point: " .. check_point)
				print("ticks passed: " .. ticks_passed)
				--print("last check point reached!")
			end
		end
	end

	if ticks_passed > 75000 then
		-- Test failed
		MSIM_SetState(mcu, AVR_MSIM_TESTFAIL)
		print("ticks passed: " .. ticks_passed)
	end

	ticks_passed = ticks_passed + 1
	pb2_old = AVR_IOBit(mcu, PORTB, 2)
end
