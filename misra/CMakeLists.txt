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

# MCUSim configuration file to check code to be compliant with MISRA C:2012
# rules.
cmake_minimum_required(VERSION 3.2)
project(MCUSim-misra C)

# Version
add_definitions(-DMSIM_VERSION="${MSIM_VERSION}")

# Load modules from our source tree too
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_SOURCE_DIR}/../cmake")

configure_file(
	${CMAKE_CURRENT_SOURCE_DIR}/misra.cmake.in
	${CMAKE_CURRENT_BINARY_DIR}/misra.cmake
	@ONLY
)
add_custom_command(
	OUTPUT MISRA
	COMMAND ${CMAKE_COMMAND} -P ${CMAKE_CURRENT_BINARY_DIR}/misra.cmake
)
add_custom_target(misra DEPENDS MISRA)
