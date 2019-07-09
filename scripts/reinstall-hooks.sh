#!/bin/sh
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


# Script to re-install git hooks in the current repository. Please, run it
# from the repository root, i.e.
#
#	./scripts/reinstall-hooks.sh
#
rm -f ./.git/hooks/pre-commit
ln -s ../../scripts/pre-commit.sh ./.git/hooks/pre-commit
if [ $? -ne 0 ]; then
	echo "[!] Git hooks are not re-installed." >&2
	echo "[!] Please, run this script from the repository root." >&2
	exit 1
fi
