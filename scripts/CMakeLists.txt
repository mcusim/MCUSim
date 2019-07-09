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

# Configuration file for Lua scripts.
cmake_minimum_required(VERSION 3.2)
project(mcusim-scripts NONE)

# Version
add_definitions(-DMSIM_VERSION="${MSIM_VERSION}")

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/mcusim.conf.in
               ${CMAKE_BINARY_DIR}/mcusim.conf @ONLY)

# Install configuration file
install(FILES ${CMAKE_BINARY_DIR}/mcusim.conf DESTINATION ${MSIM_CONF_DIR})

# Install script models
install(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/lua DESTINATION ${MSIM_SCRIPT_DIR})
