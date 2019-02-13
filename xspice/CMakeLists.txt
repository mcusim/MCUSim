#
# Copyright (c) 2018, The MCUSim Contributors
# All rights reserved.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the MCUSim or its parts nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
# THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER OR CONTRIBUTORS
# BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
# CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
# SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
# INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
# CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
# ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
# POSSIBILITY OF SUCH DAMAGE.
#
# Configuration file to build MCUSim as XSPICE code model (mcusim.cm) library.
#
cmake_minimum_required(VERSION 3.2)
project(MCUSim-XSPICE NONE)

# Version
add_definitions(-DMSIM_VERSION="${MSIM_VERSION}")

# Default NGSpice tarball
set(_NGSPICE_TARBALL "https://netix.dl.sourceforge.net/project/ngspice/ng-spice-rework/30/ngspice-30.tar.gz")
set(_NGSPICE_TARBALL_MD5 272653b01dd4529c3c3a1dbd2e1588f1)

if (WITH_XSPICE)
	include(ExternalProject)
endif()

if (WITH_XSPICE)
	if (NGSPICE_TARBALL)
		set(_NGSPICE_TARBALL ${NGSPICE_TARBALL})
	endif()
	if (NGSPICE_TARBALL_MD5)
		set(_NGSPICE_TARBALL_MD5 ${NGSPICE_TARBALL_MD5})
	endif()

	message(STATUS "Building with XSPICE code model library (i.e. mcusim.cm)")
	message(STATUS "Building with NGSpice tarball: ${_NGSPICE_TARBALL}")
	message(STATUS "Building with NGSpice tarball MD5: ${_NGSPICE_TARBALL_MD5}")
	message(STATUS "Building with NGSpice configuration: --enable-xspice --disable-debug --with-readline=yes")

	if (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
		find_program(_NGSPICE_MAKE gmake)
		if (${_NGSPICE_MAKE} MATCHES "_NGSPICE_MAKE-NOTFOUND")
			message(STATUS "Building with NGSpice make: ${_NGSPICE_MAKE}")
			message(FATAL_ERROR "GNU make is required to build NGSpice!")
		endif()
	else()
		find_program(_NGSPICE_MAKE make)
	endif()
	if (NGSPICE_MAKE)
		set(_NGSPICE_MAKE ${NGSPICE_MAKE})
	endif()
	message(STATUS "Building with NGSpice make: ${_NGSPICE_MAKE}")

	# Set flags to link code model library against MCUSim library
	set(MSIM_CFLAGS "$ENV{CFLAGS} -I${CMAKE_BINARY_DIR}/include")
	set(MSIM_LDFLAGS "$ENV{LDFLAGS} -L${CMAKE_BINARY_DIR}")
	if (${CMAKE_SYSTEM_NAME} MATCHES "FreeBSD")
		set(MSIM_CFLAGS "${MSIM_CFLAGS} -I/usr/local/include")
		set(MSIM_LDFLAGS "${MSIM_LDFLAGS} -L/usr/local/lib")
	endif()

	# Copy script to apply NGSpice patches
	file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/patch-ngspice-sources DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ngspice/patches)

	# Copy all sub-directories to the binary directory
	file(COPY ${CMAKE_CURRENT_SOURCE_DIR}/src DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ngspice/patches)

	# Copy and configure all patches to the binary directory
	file(GLOB PATCHES ${CMAKE_CURRENT_SOURCE_DIR}/ngspice-*.patch)
	file(GLOB PATCHES_CONF ${CMAKE_CURRENT_SOURCE_DIR}/ngspice-*.patch.in)
	foreach(PATCH ${PATCHES})
		file(COPY ${PATCH} DESTINATION ${CMAKE_CURRENT_BINARY_DIR}/ngspice/patches)
	endforeach()
	foreach(PATCH ${PATCHES_CONF})
		configure_file(${PATCH} ${CMAKE_CURRENT_BINARY_DIR}/ngspice/patches @ONLY)
	endforeach()

	ExternalProject_Add(
		mcusim-ngspice-codemodel
		URL ${_NGSPICE_TARBALL}
		URL_MD5 ${_NGSPICE_TARBALL_MD5}
		PREFIX ${CMAKE_CURRENT_BINARY_DIR}/ngspice
		SOURCE_DIR ${CMAKE_CURRENT_BINARY_DIR}/ngspice/src
		TMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/ngspice/tmp
		STAMP_DIR ${CMAKE_CURRENT_BINARY_DIR}/ngspice/stamp
		BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/ngspice/release
		# Patch working directory: SOURCE_DIR
		PATCH_COMMAND "../patches/patch-ngspice-sources"
		# Configuration working directory: BINARY_DIR
		CONFIGURE_COMMAND "../src/configure" CFLAGS=${MSIM_CFLAGS} LDFLAGS=${MSIM_LDFLAGS} --enable-xspice --disable-debug --with-readline=yes
		# Build working directory: BINARY_DIR
		BUILD_COMMAND "${_NGSPICE_MAKE}"
		INSTALL_COMMAND ""
		TEST_COMMAND ""
	)
else()
	message(STATUS "Building without XSPICE code model library (i.e. mcusim.cm)")
endif()
