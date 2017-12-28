MCUSim
======
Simulator for microcontrollers with GDB Remote Serial Protocol interface.

Main purpose of the simulator is to reproduce microcontroller unit (MCU)
within electronic circuit. It is achieved by decoding opcodes and
maintaining state of MCU within a program and substituting peripherals
(sensors, external RAM, etc.) using external modules written in Lua.

Goal of the project is to simplify firmware debugging of a real hardware
when not all of its components are available, affordable or easy to
set up and run.

![Example of AVRSim and GDB RSP](https://i.imgur.com/vRkcXQR.gif)

Build and install
-----------------
AVRSim (part of MCUSim, simulator for AVR microcontrollers) requires Lua 5.2
or above to mimic external devices. Please, install it in your operating
system. This is how it can be done in macOS:

	# brew install lua
	# git clone https://github.com/dsalychev/mcusim.git
	# cd mcusim && mkdir build && cd build
	# cmake -DCMAKE_BUILD_TYPE=Release ..
	# make && make install

How to contribute
-----------------
If you're feeling ready to make any changes, please, read a
[Contribution Guide](https://github.com/dsalychev/mcusim/blob/master/CONTRIBUTING.md).
