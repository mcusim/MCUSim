#!/bin/sh
#
# Script to install git hooks in the current repository. Please, run it
# from the repository root, i.e.
#
#	./scripts/install-hooks.sh
#
ln -s ../../scripts/pre-commit.sh ./.git/hooks/pre-commit
if [ $? -ne 0 ]; then
	echo "[!] Git hooks are not installed." >&2
	exit 1
fi
