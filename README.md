Readme for MCUSim
-----------------

 MCUSim is a digital simulator and NGSpice library with microcontrollers.
 It is created to assist in circuit simulation, firmware debugging, testing
 and signal tracing.

 Please note that the 8-bit AVR (RISC) microcontrollers are aimed at the
 moment. Feel free to start a discussion about any other family or
 architecture of the microcontrollers.

Screenshots
-----------

Description
-----------

 There is an mcusim.conf configuration file installed together with the mcusim
 binary and libmsim which can be used to tweak the program.

 The best way to prepare your own simulation is to copy mcusim.conf to a new
 directory, adjust the options and run the simulator. Firmware and Lua files
 should also be placed there.

 Lua scripts can be used to substitute models of the real devices during a
 simulation process. They may affect state of the chip in several ways, e.g.
 access registers, generate signals or terminate MCU.

 Scripts are supposed to be external devices connected to the MCU of the
 simulated circuit (external EEPROM, humidity sensor, MOSFET switch, etc).

 Registers of the simulated MCU can be saved into a VCD (value change dump)
 file and read using GTKWave viewer.

How can I start a discussion?
-----------------------------

 Feel free to ask questions and start a discussion in a mailing list for
 developers. Just subscribe and send a letter.

How can I join the development?
-------------------------------

 You may drop a note in the mailing list first or just code the feature you
 want to add and share your patch there. Before you start coding check the
 latest development release of MCUSim from our git repository or try to find
 a ticket at https://trac.mcusim.org/report. It might be that your feature has
 already been implemented.

 There is no bureaucracy here.

Mailing list
------------

 * mcusim-dev@freelists.org:
   This is a list for developers and anyone who is interested in MCUSim.
   Subscribe by sending 'subscribe' to mcusim-dev-request@freelists.org or
   visiting http://www.freelists.org/list/mcusim-dev.

Web sites
---------

 Source code is hosted at https://github.com/mcusim/MCUSim.
 Wiki and issue tracker are at https://trac.mcusim.org.
 Mailing list is at https://www.freelists.org/list/mcusim-dev.
