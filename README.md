MCUSim
======
Simulator for microcontrollers with GDB Remote Serial Protocol interface.    
    
[![travis-ci](https://img.shields.io/travis/dsalychev/mcusim.svg)](https://travis-ci.org/dsalychev/mcusim)
[![coverity](https://scan.coverity.com/projects/13784/badge.svg)](https://scan.coverity.com/projects/dsalychev-mcusim)
[![latest-tag](https://img.shields.io/github/tag/dsalychev/mcusim.svg)](https://github.com/dsalychev/mcusim/releases)
[![Gitter](https://badges.gitter.im/Join%20Chat.svg)](https://gitter.im/mcusim/Lobby?utm_source=share-link&utm_medium=link&utm_campaign=share-link)

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

Third-party projects and libraries
----------------------------------
I'd like to thank all contributors to the projects listed below because their
code gave me some ideas or helped a lot.

	avr-libc by the University of California et al.
	Copyright (c) 1999-2010,
	Modified BSD License,
	http://www.nongnu.org/avr-libc

	libGIS by Ivan A. Sergeev
	Copyright (c) 2010,
	MIT License and Public Domain,
	https://github.com/vsergeev/libGIS

	getopt.h and getopt_long.c are derived from the software
	contributed to the NetBSD Foundation by Dieter Baron and
	Thomas Klausner,
	Copyright (c) 2000 The NetBSD Foundation, Inc.
	2-clause BSD license,
	http://netbsd.org
