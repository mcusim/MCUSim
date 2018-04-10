#!/bin/sh
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
