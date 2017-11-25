#
# MCUSim - Simulator for microcontrollers.
# Copyright (C) 2017 Dmitry Salychev <darkness.bsd@gmail.com>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
cmake_minimum_required(VERSION 3.2)
project(MCUSim_Handbook NONE)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/../cmake)

find_package(LATEX)
if (LATEX_FOUND)
	include(UseLATEX)
	set(HANDBOOK_SRC	mcusim-handbook.tex
				chapter1/introduction.tex
				chapter2/preparing-binaries.tex
				chapter3/avrsim.tex)

	add_latex_document(mcusim-handbook.tex INPUTS ${HANDBOOK_SRC})
endif()
