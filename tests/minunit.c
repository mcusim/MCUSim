/*
 * MinUnit Test is a part of mcusim.
 *
 * Created with help of MinUnit described by John Brewer.
 * http://www.jera.com/techinfo/jtns/jtn002.html
 *
 * mcusim - Interactive simulator for microcontrollers.
 * Copyright (C) 2017 Dmitry Salychev <darkness.bsd@gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include <stdio.h>

#include "minunit.h"

int main(int argc, char *argv[])
{
	char *sn = suite_name();
	printf("\n--- MinUnit '%s' Suite ---\n", sn);
	setup_tests();
	int r = all_tests();
	if (r == 0)
		printf("\n  ALL TESTS PASSED\n");
	printf("  Tests run: %d\n", tests_run);
	printf("\n--- MinUnit '%s' End Suite ---\n", sn);

	return r != 0;
}
