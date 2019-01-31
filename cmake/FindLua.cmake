#
# Copyright 2017-2019 The MCUSim Project.
# Copyright (c) 2013 Martin Felis martin@fysx.org
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions are met:
#
#     * Redistributions of source code must retain the above copyright
#       notice, this list of conditions and the following disclaimer.
#     * Redistributions in binary form must reproduce the above copyright
#       notice, this list of conditions and the following disclaimer in the
#       documentation and/or other materials provided with the distribution.
#     * Neither the name of the MCUSim or its parts nor the
#       names of its contributors may be used to endorse or promote products
#       derived from this software without specific prior written permission.
#
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
# Try to find Lua or LuaJIT depending on the variable LUA_USE_LUAJIT.
# Sets the following variables:
#   LUA_FOUND
#   LUA_INCLUDE_DIR
#   LUA_LIBRARIES
#
# Use it in a CMakeLists.txt script as:
#
#   OPTION (LUA_USE_LUAJIT "Use LuaJIT instead of default Lua" OFF)
#   UNSET(LUA_FOUND CACHE)
#   UNSET(LUA_INCLUDE_DIR CACHE)
#   UNSET(LUA_LIBRARIES CACHE)
#   FIND_PACKAGE (Lua REQUIRED)
#
SET (LUA_FOUND FALSE)

SET (LUA_INTERPRETER_TYPE "")

IF (LUA_USE_LUAJIT)
    SET (LUA_INTERPRETER_TYPE "LuaJIT")
    SET (LUA_LIBRARIES_NAME luajit-5.1)
    SET (LUA_INCLUDE_DIRS /usr/include/luajit-2.0 /usr/local/include/luajit-2.0)
ELSE (LUA_USE_LUAJIT)
    SET (LUA_INTERPRETER_TYPE "Lua")
    SET (LUA_LIBRARIES_NAME lua5.1)
    SET (LUA_INCLUDE_DIRS /usr/include/lua5.1 /usr/local/include/lua5.1)
ENDIF(LUA_USE_LUAJIT)

FIND_PATH (LUA_INCLUDE_DIR lua.h ${LUA_INCLUDE_DIRS} )
FIND_LIBRARY (LUA_LIBRARIES NAMES ${LUA_LIBRARIES_NAME} PATHS /usr/lib /usr/local/lib)

IF (LUA_INCLUDE_DIR AND LUA_LIBRARIES)
    SET (LUA_FOUND TRUE)
ENDIF (LUA_INCLUDE_DIR AND LUA_LIBRARIES)

IF (LUA_FOUND)
    IF (NOT Lua_FIND_QUIETLY)
        MESSAGE(STATUS "Found ${LUA_INTERPRETER_TYPE} library: ${LUA_LIBRARIES}")
    ENDIF (NOT Lua_FIND_QUIETLY)
ELSE (LUA_FOUND)
   IF (Lua_FIND_REQUIRED)
       MESSAGE(FATAL_ERROR "Could not find ${LUA_INTERPRETER_TYPE}")
   ENDIF (Lua_FIND_REQUIRED)
ENDIF (LUA_FOUND)

INCLUDE(FindPackageHandleStandardArgs)
FIND_PACKAGE_HANDLE_STANDARD_ARGS(Lua  DEFAULT_MSG LUA_LIBRARIES LUA_INCLUDE_DIR)

MARK_AS_ADVANCED ( LUA_INCLUDE_DIR LUA_LIBRARIES)
