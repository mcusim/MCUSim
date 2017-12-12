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

![AVRSim and GDB RSP example](https://imgur.com/a/3s5La)
