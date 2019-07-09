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

# Helper function to add preprocesor definition of __MSIM_FILENAME__ to pass
# the filename without directory path and a longest extension for
# debugging use.
#
# Example:
#
#   define_filename_for_sources(my_target)
#
# Will add -D__MSIM_FILENAME__="filename" for each source file depended on
# by my_target, where filename is the name of the file.
#
# Source: https://stackoverflow.com/a/27990434
#
function(define_filename_for_sources targetname)
	get_target_property(source_files "${targetname}" SOURCES)

	foreach(sourcefile ${source_files})
		# Get source file's current list of compile definitions.
		get_property(defs SOURCE "${sourcefile}" PROPERTY COMPILE_DEFINITIONS)

		# Add the filename compile definition to the list.
		get_filename_component(basename "${sourcefile}" NAME_WE)
		list(APPEND defs "__MSIM_FILENAME__=\"${basename}\"")

		# Set the updated compile definitions on the source file.
		set_property(SOURCE "${sourcefile}" PROPERTY COMPILE_DEFINITIONS ${defs})
	endforeach()
endfunction()
