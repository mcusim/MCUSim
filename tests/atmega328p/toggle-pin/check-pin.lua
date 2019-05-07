--[[
Copyright 2017-2019 The MCUSim Project.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright
      notice, this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright
      notice, this list of conditions and the following disclaimer in the
      documentation and/or other materials provided with the distribution.
    * Neither the name of the MCUSim or its parts nor the
      names of its contributors may be used to endorse or promote products
      derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER OR CONTRIBUTORS
BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
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
