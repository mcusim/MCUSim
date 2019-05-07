--[[
Copyright (c) 2017, 2018,
Dmitry Salychev <darkness.bsd@gmail.com>,
Alexander Salychev <ppsalex@rambler.ru> et al.
All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:
    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the <organization> nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND
ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
DISCLAIMED. IN NO EVENT SHALL <COPYRIGHT HOLDER> BE LIABLE FOR ANY
DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

This scripts plays a role of external clock generator to let Timer/Counter0
of a microcontroller to be incremented.
--]]

VERBOSE = false			-- Switch on to enable verbose output
TICK_TIME = 0.0			-- clock period, in us

ticks_passed = 0

-- This function will be called by the simulator only once to configure
-- model before start of a simulation.
function module_conf(mcu)
	-- Re-calculate clock period, in us
	TICK_TIME = (1.0/MSIM_Freq(mcu))*1000000.0
	if VERBOSE then
		print("[check_OCR2_updated] MCU clock: " ..
		      MSIM_Freq(mcu)/1000 .. "kHz")
		print("[check_OCR2_updated] MCU clock period: " ..
		      TICK_TIME .. "us")
	end
end

-- This function will be called by the simulator periodically according to the
-- main simulation loop, i.e. time passed between two neighbor calls to the
-- function is a period.
function module_tick(mcu)
	if AVR_ReadIO(mcu, OCR2) == 67 then
		if ticks_passed < 5000 then
			-- Test failed
			-- OCR2 updated too early
			MSIM_SetState(mcu, AVR_MSIM_TESTFAIL)
			print("[check_OCR2_updated] OCR2 updated too early")
			print("[check_OCR2_updated] ticks passed: " ..
			      ticks_passed)
		else
			-- Test passed
			MSIM_SetState(mcu, AVR_MSIM_STOP)
			print("[check_OCR2_updated] OCR2 updated correctly")
			print("[check_OCR2_updated] ticks passed: " ..
			      ticks_passed)
		end
	end
	ticks_passed = ticks_passed + 1
end

