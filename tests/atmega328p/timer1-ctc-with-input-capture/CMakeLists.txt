#
# This file is part of MCUSim, an XSPICE library with microcontrollers.
#
# Copyright (C) 2017-2019 MCUSim Developers, see AUTHORS.txt for contributors.
#
# MCUSim is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# MCUSim is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <https://www.gnu.org/licenses/>.
#
cmake_minimum_required(VERSION 3.2)
project(mcusim-test-atmega328-toggle-pin)

# Set common variables
set(TARGET_OUTPUT_BASENAME "firmware")
set(TARGET_OUTPUT_FILE "${TARGET_OUTPUT_BASENAME}.elf")
set(TARGET_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}")
set(AVR_MCU "atmega328")
set(AVR_FREQ 16000000UL)

# Remove '-rdynamic', '-Wl,-search_paths_first' (avr-gcc doesn't support this
# one and treats it as '-Wl,-s' which strips linked ELF)
set(CMAKE_C_LINK_FLAGS)
set(CMAKE_SHARED_LIBRARY_LINK_C_FLAGS)
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS)

# Set flags
if (CMAKE_BUILD_TYPE MATCHES Release)
	message(STATUS "Release version will be built.")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wall -pedantic -std=iso9899:1999")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wshadow -Wpointer-arith -Wcast-qual")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wcast-align -Wstrict-prototypes")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Wmissing-prototypes -Wconversion")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -mmcu=${AVR_MCU} -DF_CPU=${AVR_FREQ}")
	set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -mmcu=${AVR_MCU} -DF_CPU=${AVR_FREQ}")
	set(CMAKE_C_FLAGS_RELEASE "${CMAKE_C_FLAGS_RELEASE} -Os")
else()
	message(STATUS "Debug version will be built by default.")
	message(STATUS "Set CMAKE_BUILD_TYPE=Release to build a release.")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -pedantic -std=iso9899:1999")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wshadow -Wpointer-arith -Wcast-qual")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wcast-align -Wstrict-prototypes")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wmissing-prototypes -Wconversion")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -mmcu=${AVR_MCU} -DF_CPU=${AVR_FREQ}")
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -mmcu=${AVR_MCU} -DF_CPU=${AVR_FREQ}")
	set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g")
endif()

# Set linker flags
if (CMAKE_BUILD_TYPE MATCHES Release)
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mmcu=${AVR_MCU}")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=${TARGET_OUTPUT_DIR}/${TARGET_OUTPUT_BASENAME}.map,--cref,--section-start=.text=0")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -s")
	message(STATUS "Linker flags: ${CMAKE_EXE_LINKER_FLAGS}")
else()
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -mmcu=${AVR_MCU}")
	set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -Wl,-Map=${TARGET_OUTPUT_DIR}/${TARGET_OUTPUT_BASENAME}.map,--cref,--section-start=.text=0")
	message(STATUS "Linker flags: ${CMAKE_EXE_LINKER_FLAGS}")
endif()

# remove the '-rdynamic'
set(CMAKE_SHARED_LIBRARY_LINK_CXX_FLAGS "")

# Find executables
find_program(AVR_CC avr-gcc)
find_program(AVR_CXX avr-g++)
find_program(AVR_SIZE_TOOL avr-size)
find_program(AVR_OBJCOPY avr-objcopy)
find_program(AVR_OBJDUMP avr-objdump)
find_program(AVR_DUDE avrdude)
find_program(SREC_CAT srec_cat)

# Define mandatory variables
set(CMAKE_SYSTEM_NAME Generic)
set(CMAKE_SYSTEM_PROCESSOR avr)
set(CMAKE_C_COMPILER ${AVR_CC})
set(CMAKE_CXX_COMPILER ${AVR_CXX})

# Define includes
include_directories("./")

# Set sources here
set(SRCS fuse.c main.c)

add_executable(${TARGET_OUTPUT_FILE} ${SRCS})
add_custom_target("upload")
file(COPY "mcusim.conf" DESTINATION ${CMAKE_BINARY_DIR})
file(COPY "check-pin.lua" DESTINATION ${CMAKE_BINARY_DIR})

# Prepare files for MCU
add_custom_command(
	TARGET ${TARGET_OUTPUT_FILE} POST_BUILD
	COMMAND ${AVR_SIZE_TOOL} ${TARGET_OUTPUT_DIR}/${TARGET_OUTPUT_FILE})
add_custom_command(
	TARGET ${TARGET_OUTPUT_FILE} POST_BUILD
	COMMAND ${AVR_OBJDUMP} -h -S ${TARGET_OUTPUT_FILE} > ${TARGET_OUTPUT_DIR}/${TARGET_OUTPUT_BASENAME}.lss)
add_custom_command(
	TARGET ${TARGET_OUTPUT_FILE} POST_BUILD
	COMMAND ${AVR_OBJCOPY} -R .eeprom -R .fuse -R .lock -R .signature -O ihex ${TARGET_OUTPUT_FILE} ${TARGET_OUTPUT_DIR}/${TARGET_OUTPUT_BASENAME}.hex)
add_custom_command(
	TARGET ${TARGET_OUTPUT_FILE} POST_BUILD
	COMMAND ${AVR_OBJCOPY} --no-change-warnings -j .fuse --change-section-lma .fuse=0 -O ihex ${TARGET_OUTPUT_FILE} ${TARGET_OUTPUT_DIR}/${TARGET_OUTPUT_BASENAME}.fuse)
add_custom_command(
	TARGET ${TARGET_OUTPUT_FILE} POST_BUILD
	COMMAND ${AVR_OBJCOPY} --no-change-warnings -j .eeprom --change-section-lma .eeprom=0 -O ihex ${TARGET_OUTPUT_FILE} ${TARGET_OUTPUT_DIR}/${TARGET_OUTPUT_BASENAME}.eep)
add_custom_command(
	TARGET ${TARGET_OUTPUT_FILE} POST_BUILD
	COMMAND ${AVR_OBJCOPY} --no-change-warnings -j .lock --change-section-lma .lock=0 -O ihex ${TARGET_OUTPUT_FILE} ${TARGET_OUTPUT_DIR}/${TARGET_OUTPUT_BASENAME}.lock)
add_custom_command(
	TARGET ${TARGET_OUTPUT_FILE} POST_BUILD
	COMMAND ${AVR_OBJCOPY} --no-change-warnings -j .signature --change-section-lma .signature=0 -O ihex ${TARGET_OUTPUT_FILE} ${TARGET_OUTPUT_DIR}/${TARGET_OUTPUT_BASENAME}.sig)
add_custom_command(
	TARGET ${TARGET_OUTPUT_FILE} POST_BUILD
	COMMAND ${SREC_CAT} ${TARGET_OUTPUT_DIR}/${TARGET_OUTPUT_BASENAME}.fuse -Intel -crop 0x00 0x01 -offset  0x00 -O ${TARGET_OUTPUT_DIR}/${TARGET_OUTPUT_BASENAME}.lfs -Intel)
add_custom_command(
	TARGET ${TARGET_OUTPUT_FILE} POST_BUILD
	COMMAND ${SREC_CAT} ${TARGET_OUTPUT_DIR}/${TARGET_OUTPUT_BASENAME}.fuse -Intel -crop 0x01 0x02 -offset -0x01 -O ${TARGET_OUTPUT_DIR}/${TARGET_OUTPUT_BASENAME}.hfs -Intel)
add_custom_command(
	TARGET ${TARGET_OUTPUT_FILE} POST_BUILD
	COMMAND ${AVR_OBJDUMP} -m avr -D ${TARGET_OUTPUT_DIR}/${TARGET_OUTPUT_BASENAME}.hex > ${TARGET_OUTPUT_DIR}/${TARGET_OUTPUT_BASENAME}.hex.txt)

add_custom_command(
	TARGET "upload" POST_BUILD
	COMMAND ${AVR_DUDE} -p m328 -b 115200 -P /dev/tty.usbmodem00204652 -c avrispv2 -Uflash:w:${TARGET_OUTPUT_DIR}/${TARGET_OUTPUT_BASENAME}.hex -U hfuse:w:0xC9:m -U lfuse:w:0xEF:m)
