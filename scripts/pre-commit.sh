#!/bin/sh
#
# Pre-commit hook to check C coding style using Artistic Style.
# http://astyle.sourceforge.net
#

# Artisctic style configuration to check K&R coding style
OPTIONS="--style=kr -t8 -xC80 -xL -xg -H -k3 -xW -xb -j -xB"

# Check programs available within operating system
ASTYLE=$(which astyle)
if [ $? -ne 0 ]; then
	echo "[!] astyle not installed. Unable to check source file format policy." >&2
	exit 1
fi
DIFF=$(which diff)
if [ $? -ne 0 ]; then
	echo "[!] diff not installed. Unable to check source file format policy." >&2
	exit 1
fi
GREP=$(which grep)
if [ $? -ne 0 ]; then
	echo "[!] grep not installed. Unable to check source file format policy." >&2
	exit 1
fi
CMP=$(which cmp)
if [ $? -ne 0 ]; then
	echo "[!] cmp not installed. Unable to check source file format policy." >&2
	exit 1
fi
LESS=$(which less)
if [ $? -ne 0 ]; then
	echo "[!] less not installed. Unable to check source file format policy." >&2
	exit 1
fi
GIT=$(which git)

RETURN=0
FILES=`$GIT diff --cached --name-only --diff-filter=ACMR | $GREP -E "\.(c|cpp|h)$"`
for FILE in $FILES; do
	$ASTYLE $OPTIONS < $FILE | $CMP -s $FILE -
	if [ $? -ne 0 ]; then
		echo "[!]" >&2
		echo "[!] $FILE does not follow the agreed coding standards." >&2
		echo "[!] These are lines which should be adjusted:" >&2
		echo "[!]" >&2
		$ASTYLE $OPTIONS < $FILE | $DIFF -u $FILE - | $LESS

		RETURN=1
	fi
done

if [ $RETURN -eq 1 ]; then
	echo "[I]" >&2
	echo "[I] These options of astyle used to check the files above:" >&2
	echo "[I] $ASTYLE $OPTIONS < FILE" >&2
fi

exit $RETURN
