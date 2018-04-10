#!/bin/sh
#
# Interactive script to create a simulated board (MCU + peripherals).
#
# We usually create a PCB in real life to test our ideas or proof a concept.
# Purpose of the boards created within MCUSim is the same. You may:
#
#         - Test firmware with specific MCU and peripherals;
#         - Test a simulator feature (phase-correct PWM generating,
#           for instance);
#         - Create and run a board which resembles a part of your schematic or
#           even a full one.
#
# Each board contains MCU which is backed by the simulator itself and
# peripherals which are defined using Lua. You may find examples of such
# modules in "scripts/<MCU-family>" directory. Only a very limited set of
# them is destributed with MCUSim.
#
MCU_FAMILY_DEFAULT="avr"
BRD_DIR_DEFAULT="./boards"
BRD_NAME_SUFF="board"

# Ask user about a directory to store a board
printf "Directory to store a board [$BRD_DIR_DEFAULT]: "
read BRD_DIR
if [ -z "$BRD_DIR" ]; then
	BRD_DIR=$BRD_DIR_DEFAULT
fi

# Ask user about a family of simulated MCU
printf "Microcontroller family [$MCU_FAMILY_DEFAULT]: "
read MCU_FAMILY
if [ -z "$MCU_FAMILY" ]; then
	MCU_FAMILY=$MCU_FAMILY_DEFAULT
fi

# Ask user about a name for the board
printf "Name of the board [$MCU_FAMILY-$BRD_NAME_SUFF]: "
read BRD_NAME
if [ -z "$BRD_NAME" ]; then
	BRD_NAME=$MCU_FAMILY-$BRD_NAME_SUFF
fi

echo "Please, verify the board configuration:"
echo "---------------------------------------"
echo "Board directory: $BRD_DIR"
echo "Board name: $BRD_NAME"
echo "MCU family: $MCU_FAMILY"
echo "MCU fuses: EXT=0x00, HIGH=0x00, LOW=0x00"
echo "MCU lock bits: 0xFF"
echo "---------------------------------------"
printf "Is it valid? [y/N] "
read VALID_BRD
if [ "$VALID_BRD" = "y" ] || [ "$VALID_BRD" = "Y" ] ; then
	echo "Board will be created!"
	exit 0
else
	echo "Board won't be created"
	exit 0
fi
