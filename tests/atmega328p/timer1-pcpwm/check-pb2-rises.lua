--[[
 * Copyright 2017-2019 The MCUSim Project.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the MCUSim or its parts nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
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
		if not pb2_old and AVR_IOBit(mcu, PORTB, 2) then
			if tcnt1 == 896 and ocr1b == 896 then
				print("check_point: " .. check_point)
				check_point = check_point + 1
			end
		end
	elseif check_point == 1 then
		if not pb2_old and AVR_IOBit(mcu, PORTB, 2) then
			if tcnt1 == 768 and ocr1b == 768 then
				print("check_point: " .. check_point)
				check_point = check_point + 1
			end
		end
	elseif check_point == 2 then
		if not pb2_old and AVR_IOBit(mcu, PORTB, 2) then
			if tcnt1 == 640 and ocr1b == 640 then
				print("check_point: " .. check_point)
				check_point = check_point + 1
			end
		end
	elseif check_point == 3 then
		if not pb2_old and AVR_IOBit(mcu, PORTB, 2) then
			if tcnt1 == 512 and ocr1b == 512 then
				print("check_point: " .. check_point)
				check_point = check_point + 1
			end
		end
	elseif check_point == 4 then
		if not pb2_old and AVR_IOBit(mcu, PORTB, 2) then
			if tcnt1 == 384 and ocr1b == 384 then
				-- Test finished successfully
				--MSIM_SetState(mcu, AVR_MSIM_STOP)
				print("check_point: " .. check_point)
				print("ticks passed: " .. ticks_passed)
				print("check point reached!")
			end
		end
	end

	if ticks_passed > 100000 then
		-- Test failed
		MSIM_SetState(mcu, AVR_MSIM_TESTFAIL)
		print("ticks passed: " .. ticks_passed)
	end

	ticks_passed = ticks_passed + 1
	pb2_old = AVR_IOBit(mcu, PORTB, 2)
end
