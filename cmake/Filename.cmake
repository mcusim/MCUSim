#
# Copyright (c) 2017, 2018, The MCUSim Contributors
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

