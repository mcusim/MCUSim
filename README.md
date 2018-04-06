MCUSim
======
[![travis-ci](https://img.shields.io/travis/dsalychev/mcusim.svg)](https://travis-ci.org/dsalychev/mcusim)
[![coverity](https://scan.coverity.com/projects/13784/badge.svg)](https://scan.coverity.com/projects/dsalychev-mcusim)
[![latest-tag](https://img.shields.io/github/tag/dsalychev/mcusim.svg)](https://github.com/dsalychev/mcusim/releases)

Main purpose of the simulator is to reproduce microcontroller unit (MCU)
within electronic circuit. It is achieved by decoding opcodes and
maintaining state of MCU within a program and substituting peripherals
(sensors, external RAM, etc.) using external modules written in Lua.

Goal of the project is to simplify firmware debugging of a real hardware
when not all of its components are available, affordable or easy to
set up and run.

Site: http://www.mcusim.org    
Issue tracker: https://trac.mcusim.org    
Roadmap: https://trac.mcusim.org/roadmap    

Build it from sources
------------------
MCUSim is intended to be a suite for different types of microcontrollers, like
AVR® (Atmel, Microchip), PIC® (Microchip), etc. It requires:

* CMake 3.2+ (to generate build files)
* Lua 5.2+ (to mimic external devices)
* astyle (to check code style in pre-commit hook)

### macOS
Debug version of the suite can be compiled and installed using package
manager `brew` to solve dependencies:

	# brew install cmake lua astyle
	# git clone https://github.com/dsalychev/mcusim.git
	# cd mcusim && scripts/install-hooks.sh && mkdir build && cd build
	# cmake ..
	# make install clean

Release version can be compiled the very similar way. You'll have to call
`cmake -DCMAKE_BUILD_TYPE=Release ..`.

### FreeBSD
Debug version of the suite can be compiled and installed using package
manager `pkg` (FreeBSD 10 and above) or via collection of ports to solve
dependencies:

	# pkg install cmake lua52 astyle
	# git clone https://github.com/dsalychev/mcusim.git
	# cd mcusim && scripts/install-hooks.sh && mkdir build && cd build
	# cmake ..
	# make install clean

It's highly likely that you'll need super user rights to perform
the last command. Release version can be compiled the very similar way.
You'll have to call `cmake -DCMAKE_BUILD_TYPE=Release ..`.

Availability matrix
-------------------
This matrix should give you an idea which parts of the microcontrollers are
already supported by the simulator.
(I'm still painting it...)

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
