--[[
Copyright 2017-2018 The MCUSim Project.

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

Script which stops simulation in 5 seconds.
--]]

VERBOSE = true			-- Switch on to enable verbose output
TICK_TIME = 0.0			-- Clock period, in us
TIMEOUT = 5000000		-- Stop simulation after timeout, in us

ticks_left = 0

-- This function will be called by the simulator only once to configure
-- model before start of a simulation.
function module_conf(mcu)
	-- Re-calculate clock period, in us
	TICK_TIME = (1.0/MSIM_Freq(mcu))*1000000.0
	ticks_left = TIMEOUT/TICK_TIME
	if VERBOSE then
		print("[stop-in-5s] Timeout: " .. TIMEOUT .. "us")
		print("[stop-in-5s] MCU clock period: " .. TICK_TIME .. "us")
		print("[stop-in-5s] Ticks left: " .. ticks_left)
		print("[stop-in-5s] Actual timeout: " .. ticks_left*TICK_TIME
		      .. "us")
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
