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

# - Try to find readline include dirs and libraries
#
# Usage of this module as follows:
#
#     find_package(Readline)
#
# Variables used by this module, they can change the default behaviour and need
# to be set before calling find_package:
#
#  Readline_ROOT_DIR         Set this variable to the root installation of
#                            readline if the module has problems finding the
#                            proper installation path.
#
# Variables defined by this module:
#
#  READLINE_FOUND            System has readline, include and lib dirs found
#  Readline_INCLUDE_DIR      The readline include directories.
#  Readline_LIBRARY          The readline library.

find_path(Readline_ROOT_DIR
	NAMES include/readline/readline.h
)
find_path(Readline_INCLUDE_DIR
	NAMES readline/readline.h
	HINTS ${Readline_ROOT_DIR}/include
)
find_library(Readline_LIBRARY
	NAMES readline
	HINTS ${Readline_ROOT_DIR}/lib
)

if(Readline_INCLUDE_DIR AND Readline_LIBRARY AND Ncurses_LIBRARY)
	set(READLINE_FOUND TRUE)
else(Readline_INCLUDE_DIR AND Readline_LIBRARY AND Ncurses_LIBRARY)
	FIND_LIBRARY(Readline_LIBRARY NAMES readline)
	include(FindPackageHandleStandardArgs)
	FIND_PACKAGE_HANDLE_STANDARD_ARGS(Readline DEFAULT_MSG Readline_INCLUDE_DIR Readline_LIBRARY)
	MARK_AS_ADVANCED(Readline_INCLUDE_DIR Readline_LIBRARY)
endif(Readline_INCLUDE_DIR AND Readline_LIBRARY AND Ncurses_LIBRARY)

mark_as_advanced(
	Readline_ROOT_DIR
	Readline_INCLUDE_DIR
	Readline_LIBRARY
)
