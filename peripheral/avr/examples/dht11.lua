--[[
   AVRSim - Simulator for AVR microcontrollers.
   This software is a part of MCUSim, interactive simulator for
   microcontrollers.
   Copyright (C) 2017 Dmitry Salychev <darkness.bsd@gmail.com>

   This program is free software: you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation, either version 3 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program.  If not, see <http://www.gnu.org/licenses/>.

   Simulated DHT11 digital temperature and humidity sensor.
--]]

-- These parameters of MCU and DHT11 are assumed:
-- 	Part: ATmega8
-- 	MCU clock: 16 MHz
--	Data pin: PORTB0 (to read from), PINB0 (to write to)
TICK_TIME = 0.0625			-- in us, depends on MCU clock
DDR = 0x37				-- data direction register (DDRx)
PORT_IN = 0x38				-- I/O register (PORTx)
PORT_OUT = 0x36				-- I/O register (PINx)
VERBOSE = false				-- Switch on to enable DHT11 output

-- Timings constants
INIT_TICKS = 1000000/TICK_TIME		-- unstable initial state, ~1s

STSIG_LOW_TICKS = 18000/TICK_TIME	-- 18ms, MCU pulls down
STSIG_LOW_LIM = 21000/TICK_TIME		-- 21ms, pull down limit
STSIG_HIGH_TICKS = 25/TICK_TIME		-- 25us, MCU pulls up

DHTSIG_LOW_TICKS = 80/TICK_TIME		-- 80us, DHT pulls down
DHTSIG_HIGH_TICKS = 80/TICK_TIME	-- 80us, DHT pulls up
DHT_DLOW_TICKS = 50/TICK_TIME		-- 50us, delay before "1" or "0"
DHT_D0_TICKS = 27/TICK_TIME		-- 27us, logical "0"
DHT_D1_TICKS = 70/TICK_TIME		-- 70us, logical "1"
-- END Timings constants

-- Global variables
state = "mcu-start-low"			-- DHT state
init_ticks = 0
ticks = 0
mis_ticks = 0
data =	"11011110" ..
	"10101101" ..
	"10111110" ..
	"11101111" ..
	"00111000"
ibit = 1				-- Index of a bit to send
datalen = #data				-- Length of the data
-- END Global variables

function module_tick(mcu)
	-- Initial (unstable) period of DHT
	if init_ticks < INIT_TICKS then
		init_ticks = init_ticks + 1
		return
	end

	-- MCU should configure its pin to output
	-- and ask DHT for data transmission
	if state == "mcu-start-low" then
		if ticks == 0 and msim_avr_isset(mcu, PORT_IN, 0) then
			return
		end
		-- Check low level of MCU output pin
		if ticks < STSIG_LOW_TICKS and
		   not msim_avr_isset(mcu, PORT_IN, 0) then
			ticks = ticks + 1
			return
		end
		if ticks < STSIG_LOW_TICKS and
		   msim_avr_isset(mcu, PORT_IN, 0) then
		   	if VERBOSE then
			   	print("MCU start low: unexpected rise, " ..
				      "ticks: " .. ticks)
			end
		   	ticks = 0
		   	state = "mcu-start-low"
			return
		end
		-- Check if MCU pulls up within a limited time
		if msim_avr_isset(mcu, PORT_IN, 0) then
			if VERBOSE then
			   	print "MCU start low: rised"
			end
			ticks = 0
			state = "mcu-start-high"
			return
		end
		if ticks > STSIG_LOW_LIM then
			if VERBOSE then
			   	print("MCU start low: no rise within " ..
				      "limited time")
			end
			ticks = 0
			state = "mcu-start-low"
			return
		end
		ticks = ticks + 1
	elseif state == "mcu-start-high" then
		-- Check MCU keeping output pin pulled up
		if ticks < STSIG_HIGH_TICKS and
		   not msim_avr_isset(mcu, PORT_IN, 0) then
		   	if VERBOSE then
			   	print("MCU start high: unexpected fall, " ..
				      "ticks: " .. ticks)
			end
			ticks = 0
			state = "mcu-start-low"
			return
		end
		if ticks < STSIG_HIGH_TICKS and
		   msim_avr_isset(mcu, PORT_IN, 0) then
			ticks = ticks + 1
		end
		if ticks >= STSIG_HIGH_TICKS then
			if VERBOSE then
			   	print "MCU start high: DHT11 pulled down"
			end
			ticks = 0;
			state = "dht-start-low"
			return
		end
	-- MCU should start to listen to DHT response
	-- and configure its pin to input
	elseif state == "dht-start-low" then
		-- Can we write to I/O register?
		if msim_avr_isset(mcu, DDR, 0) then
			mis_ticks = mis_ticks + 1
			return
		end

		if (mis_ticks+ticks) < DHTSIG_LOW_TICKS then
			msim_avr_setbit(mcu, PORT_OUT, 0, 0)
			ticks = ticks + 1
			return
		else
			if VERBOSE then
			   	print "DHT11 start low: DHT11 pulled up"
			end
			mis_ticks = 0
			ticks = 0
			state = "dht-start-high"
			return
		end
	elseif state == "dht-start-high" then
		-- Can we write to I/O register?
		if msim_avr_isset(mcu, DDR, 0) then
			mis_ticks = mis_ticks + 1
			return
		end

		if (mis_ticks+ticks) < DHTSIG_HIGH_TICKS then
			msim_avr_setbit(mcu, PORT_OUT, 0, 1)
			ticks = ticks + 1
			return
		else
			if VERBOSE then
			   	print "DHT11 start high: delay"
				print "DHT11 is sending: "
			end
			mis_ticks = 0
			ticks = 0
			state = "data-delay"
			return
		end
	-- DHT is ready to transmit 40-bits data
	elseif state == "data-delay" then
		-- Can we write to I/O register?
		if msim_avr_isset(mcu, DDR, 0) then
			mis_ticks = mis_ticks + 1
			return
		end

		if (mis_ticks+ticks) < DHT_DLOW_TICKS then
			msim_avr_setbit(mcu, PORT_OUT, 0, 0)
			ticks = ticks + 1
			return
		else
			if ibit > datalen then
				ibit = 1
				mis_ticks = 0
				ticks = 0
				state = "mcu-start-low"
				return
			end

			local bit = data:sub(ibit,ibit)
			ibit = ibit + 1
			mis_ticks = 0
			ticks = 0
			if VERBOSE then
				print(bit)
			end
			if (bit == "0") then
				state = "data-send0"
			else
				state = "data-send1"
			end
			return
		end
	elseif state == "data-send0" or state == "data-send1" then
		-- Can we write to I/O register?
		if msim_avr_isset(mcu, DDR, 0) then
			mis_ticks = mis_ticks + 1
			return
		end

		local wt = DHT_D0_TICKS
		if state == "data-send1" then
			wt = DHT_D1_TICKS
		end
		if (mis_ticks+ticks) < wt then
			msim_avr_setbit(mcu, PORT_OUT, 0, 1)
			ticks = ticks + 1
			return
		else
			mis_ticks = 0
			ticks = 0
			state = "data-delay"
			return
		end
	end
end
