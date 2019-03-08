/*
 * Copyright 2017-2019 The MCUSim Project.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the MCUSim or its parts nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
 * THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL COPYRIGHT HOLDER OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

/* $OpenBSD: getopt_long.c,v 1.24 2010/07/22 19:31:53 blambert Exp $	*/
/* $NetBSD: getopt_long.c,v 1.1.1.1 2014/07/09 19:38:35 riastradh Exp $	*/
/*
 * Copyright (c) 2002 Todd C. Miller <Todd.Miller@courtesan.com>
 *
 * Permission to use, copy, modify, and distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT OF
 * OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 *
 * Sponsored in part by the Defense Advanced Research Projects
 * Agency (DARPA) and Air Force Research Laboratory, Air Force
 * Materiel Command, USAF, under agreement number F39502-99-1-0512.
 */
/*-
 * Copyright (c) 2000 The NetBSD Foundation, Inc.
 * All rights reserved.
 *
 * This code is derived from software contributed to The NetBSD Foundation
 * by Dieter Baron and Thomas Klausner.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE NETBSD FOUNDATION, INC. AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED
 * TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE FOUNDATION OR CONTRIBUTORS
 * BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "mcusim/getopt.h"

int MSIM_OPT_opterr = 1;	/* if error message should be printed */
int MSIM_OPT_optind = 1;	/* index into parent argv vector */
int MSIM_OPT_optopt = '?';	/* character checked for validity */
int MSIM_OPT_optreset;		/* reset getopt */
char *MSIM_OPT_optarg;		/* argument associated with option */

#define PRINT_ERROR	((MSIM_OPT_opterr) && (*options != ':'))

#define FLAG_PERMUTE	0x01	/* permute non-options to the end of argv */
#define FLAG_ALLARGS	0x02	/* treat non-options as args to option "-1" */
#define FLAG_LONGONLY	0x04	/* operate as getopt_long_only */

/* return values */
#define	BADCH		(int)'?'
#define	BADARG		((*options == ':') ? (int)':' : (int)'?')
#define	INORDER 	(int)1

#define	EMSG		""

static int getopt_internal(int, char **, const char *,
                           const struct MSIM_OPT_Option *, int *, int);
static int parse_long_options(char *const *, const char *,
                              const struct MSIM_OPT_Option *, int *, int);
static int gcd(int, int);
static void permute_args(int, int, int, char **);

static char *place = EMSG; /* option letter processing */

/* XXX: set MSIM_OPT_optreset to 1 rather than these two */
static int nonopt_start = -1; /* first non option argument (for permute) */
static int nonopt_end = -1;   /* first option after non options (for permute) */

/* Error messages */
static const char recargchar[] = "option requires an argument -- %c\n";
static const char recargstring[] = "option requires an argument -- %s\n";
static const char ambig[] = "ambiguous option -- %.*s\n";
static const char noarg[] = "option doesn't take an argument -- %.*s\n";
static const char illoptchar[] = "unknown option -- %c\n";
static const char illoptstring[] = "unknown option -- %s\n";

/*
 * Compute the greatest common divisor of a and b.
 */
static int
gcd(int a, int b)
{
	int c;

	c = a % b;
	while (c != 0) {
		a = b;
		b = c;
		c = a % b;
	}

	return (b);
}

/*
 * Exchange the block from nonopt_start to nonopt_end with the block
 * from nonopt_end to opt_end (keeping the same order of arguments
 * in each block).
 */
static void
permute_args(int panonopt_start, int panonopt_end, int opt_end, char **nargv)
{
	int cyclelen, i, j, ncycle, nnonopts, nopts;
	char *swap;

	/*
	 * compute lengths of blocks and number and size of cycles
	 */
	nnonopts = panonopt_end - panonopt_start;
	nopts = opt_end - panonopt_end;
	ncycle = gcd(nnonopts, nopts);
	cyclelen = (opt_end - panonopt_start) / ncycle;

	for (i = 0; i < ncycle; i++) {
		int cstart, pos;

		cstart = panonopt_end+i;
		pos = cstart;
		for (j = 0; j < cyclelen; j++) {
			if (pos >= panonopt_end) {
				pos -= nnonopts;
			} else {
				pos += nopts;
			}
			swap = nargv[pos];
			((char **)nargv)[pos] = nargv[cstart];
			((char **)nargv)[cstart] = swap;
		}
	}
}

/*
 * parse_long_options --
 *	Parse long options in argc/argv argument vector.
 * Returns -1 if short_too is set and the option does not match long_options.
 */
static int
parse_long_options(char *const *nargv, const char *options,
                   const struct MSIM_OPT_Option *long_options, int *idx, int short_too)
{
	char *current_argv, *has_equal;
	size_t current_argv_len;
	int i, match;

	current_argv = place;
	match = -1;

	MSIM_OPT_optind++;

	if ((has_equal = strchr(current_argv, '=')) != NULL) {
		/* argument found (--option=arg) */
		current_argv_len = (size_t)(has_equal - current_argv);
		has_equal++;
	} else {
		current_argv_len = strlen(current_argv);
	}

	for (i = 0; long_options[i].name; i++) {
		/* find matching long option */
		if (strncmp(current_argv, long_options[i].name,
		                current_argv_len)) {
			continue;
		}

		if (strlen(long_options[i].name) == current_argv_len) {
			/* exact match */
			match = i;
			break;
		}
		/*
		 * If this is a known short option, don't allow
		 * a partial match of a single character.
		 */
		if (short_too && current_argv_len == 1) {
			continue;
		}

		if (match == -1) {	/* partial match */
			match = i;
		} else {
			/* ambiguous abbreviation */
			if (PRINT_ERROR) {
				fprintf(stderr, ambig, (int)current_argv_len,
				        current_argv);
			}
			MSIM_OPT_optopt = 0;
			return (BADCH);
		}
	}
	if (match != -1) {		/* option found */
		if (long_options[match].has_arg == MSIM_OPT_NO_ARGUMENT &&
		                has_equal) {
			if (PRINT_ERROR) {
				fprintf(stderr, noarg, (int)current_argv_len,
				        current_argv);
			}
			/*
			 * XXX: GNU sets MSIM_OPT_optopt to val regardless of flag
			 */
			if (long_options[match].flag == NULL) {
				MSIM_OPT_optopt = long_options[match].val;
			} else {
				MSIM_OPT_optopt = 0;
			}
			return (BADARG);
		}
		if (long_options[match].has_arg == MSIM_OPT_REQUIRED_ARGUMENT ||
		                long_options[match].has_arg ==
		                MSIM_OPT_OPTIONAL_ARGUMENT) {
			if (has_equal) {
				MSIM_OPT_optarg = has_equal;
			} else if (long_options[match].has_arg ==
			                MSIM_OPT_REQUIRED_ARGUMENT) {
				/*
				 * optional argument doesn't use next nargv
				 */
				MSIM_OPT_optarg = nargv[MSIM_OPT_optind++];
			}
		}
		if ((long_options[match].has_arg == MSIM_OPT_REQUIRED_ARGUMENT) &&
		                (MSIM_OPT_optarg == NULL)) {
			/*
			 * Missing argument; leading ':' indicates no error
			 * should be generated.
			 */
			if (PRINT_ERROR) {
				fprintf(stderr, recargstring, current_argv);
			}
			/*
			 * XXX: GNU sets MSIM_OPT_optopt to val regardless of flag
			 */
			if (long_options[match].flag == NULL) {
				MSIM_OPT_optopt = long_options[match].val;
			} else {
				MSIM_OPT_optopt = 0;
			}
			--MSIM_OPT_optind;
			return (BADARG);
		}
	} else {			/* unknown option */
		if (short_too) {
			--MSIM_OPT_optind;
			return (-1);
		}
		if (PRINT_ERROR) {
			fprintf(stderr, illoptstring, current_argv);
		}
		MSIM_OPT_optopt = 0;
		return (BADCH);
	}
	if (idx) {
		*idx = match;
	}
	if (long_options[match].flag) {
		*long_options[match].flag = long_options[match].val;
		return (0);
	} else {
		return (long_options[match].val);
	}
}

/*
 * getopt_internal --
 *	Parse argc/argv argument vector.  Called by user level routines.
 */
static int
getopt_internal(int nargc, char **nargv, const char *options,
                const struct MSIM_OPT_Option *long_options, int *idx, int flags)
{
	char *oli;				/* option letter list index */
	int optchar, short_too;
	static int posixly_correct = -1;

	if (options == NULL) {
		return (-1);
	}

	/*
	 * Disable GNU extensions if POSIXLY_CORRECT is set or options
	 * string begins with a '+'.
	 */
	if (posixly_correct == -1) {
		posixly_correct = (getenv("POSIXLY_CORRECT") != NULL);
	}
	if (posixly_correct || *options == '+') {
		flags &= ~FLAG_PERMUTE;
	} else if (*options == '-') {
		flags |= FLAG_ALLARGS;
	}
	if (*options == '+' || *options == '-') {
		options++;
	}

	/*
	 * XXX Some GNU programs (like cvs) set MSIM_OPT_optind to 0 instead of
	 * XXX using MSIM_OPT_optreset.  Work around this braindamage.
	 */
	if (MSIM_OPT_optind == 0) {
		MSIM_OPT_optind = MSIM_OPT_optreset = 1;
	}

	MSIM_OPT_optarg = NULL;
	if (MSIM_OPT_optreset) {
		nonopt_start = nonopt_end = -1;
	}
start:
	if (MSIM_OPT_optreset || !*place) {		/* update scanning pointer */
		MSIM_OPT_optreset = 0;
		if (MSIM_OPT_optind >= nargc) {          /* end of argument vector */
			place = EMSG;
			if (nonopt_end != -1) {
				/* do permutation, if we have to */
				permute_args(nonopt_start, nonopt_end,
				             MSIM_OPT_optind, nargv);
				MSIM_OPT_optind -= nonopt_end - nonopt_start;
			} else if (nonopt_start != -1) {
				/*
				 * If we skipped non-options, set MSIM_OPT_optind
				 * to the first of them.
				 */
				MSIM_OPT_optind = nonopt_start;
			}
			nonopt_start = nonopt_end = -1;
			return (-1);
		}
		if (*(place = nargv[MSIM_OPT_optind]) != '-' ||
		                (place[1] == '\0' && strchr(options, '-') ==
		                 NULL)) {
			place = EMSG;		/* found non-option */
			if (flags & FLAG_ALLARGS) {
				/*
				 * GNU extension:
				 * return non-option as argument to option 1
				 */
				MSIM_OPT_optarg = nargv[MSIM_OPT_optind++];
				return (INORDER);
			}
			if (!(flags & FLAG_PERMUTE)) {
				/*
				 * If no permutation wanted, stop parsing
				 * at first non-option.
				 */
				return (-1);
			}
			/* do permutation */
			if (nonopt_start == -1) {
				nonopt_start = MSIM_OPT_optind;
			} else if (nonopt_end != -1) {
				permute_args(nonopt_start, nonopt_end,
				             MSIM_OPT_optind, nargv);
				nonopt_start = MSIM_OPT_optind -
				               (nonopt_end - nonopt_start);
				nonopt_end = -1;
			}
			MSIM_OPT_optind++;
			/* process next argument */
			goto start;
		}
		if (nonopt_start != -1 && nonopt_end == -1) {
			nonopt_end = MSIM_OPT_optind;
		}

		/*
		 * If we have "-" do nothing, if "--" we are done.
		 */
		if (place[1] != '\0' && *++place == '-' && place[1] == '\0') {
			MSIM_OPT_optind++;
			place = EMSG;
			/*
			 * We found an option (--), so if we skipped
			 * non-options, we have to permute.
			 */
			if (nonopt_end != -1) {
				permute_args(nonopt_start, nonopt_end,
				             MSIM_OPT_optind, nargv);
				MSIM_OPT_optind -= nonopt_end - nonopt_start;
			}
			nonopt_start = nonopt_end = -1;
			return (-1);
		}
	}

	/*
	 * Check long options if:
	 *  1) we were passed some
	 *  2) the arg is not just "-"
	 *  3) either the arg starts with -- we are getopt_long_only()
	 */
	if (long_options != NULL && place != nargv[MSIM_OPT_optind] &&
	                (*place == '-' || (flags & FLAG_LONGONLY))) {
		short_too = 0;
		if (*place == '-') {
			place++;		/* --foo long option */
		} else if (*place != ':' && strchr(options, *place) != NULL) {
			short_too = 1;		/* could be short option too */
		}

		optchar = parse_long_options(nargv, options, long_options,
		                             idx, short_too);
		if (optchar != -1) {
			place = EMSG;
			return (optchar);
		}
	}

	if ((optchar = (int)*place++) == (int)':' ||
	                (optchar == (int)'-' && *place != '\0') ||
	                (oli = strchr(options, optchar)) == NULL) {
		/*
		 * If the user specified "-" and  '-' isn't listed in
		 * options, return -1 (non-option) as per POSIX.
		 * Otherwise, it is an unknown option character (or ':').
		 */
		if (optchar == (int)'-' && *place == '\0') {
			return (-1);
		}
		if (!*place) {
			++MSIM_OPT_optind;
		}
		if (PRINT_ERROR) {
			fprintf(stderr, illoptchar, optchar);
		}
		MSIM_OPT_optopt = optchar;
		return (BADCH);
	}
	if (long_options != NULL && optchar == 'W' && oli[1] == ';') {
		/* -W long-option */
		if (*place) {			/* no space */
			/* NOTHING */;
		} else if (++MSIM_OPT_optind >= nargc) {	/* no arg */
			place = EMSG;
			if (PRINT_ERROR) {
				fprintf(stderr, recargchar, optchar);
			}
			MSIM_OPT_optopt = optchar;
			return (BADARG);
		} else {			/* white space */
			place = nargv[MSIM_OPT_optind];
		}
		optchar = parse_long_options(nargv, options, long_options,
		                             idx, 0);
		place = EMSG;
		return (optchar);
	}
	if (*++oli != ':') {			/* doesn't take argument */
		if (!*place) {
			++MSIM_OPT_optind;
		}
	} else {				/* takes (optional) argument */
		MSIM_OPT_optarg = NULL;
		if (*place) {			/* no white space */
			MSIM_OPT_optarg = place;
		} else if (oli[1] != ':') {	/* arg not optional */
			if (++MSIM_OPT_optind >= nargc) {	/* no arg */
				place = EMSG;
				if (PRINT_ERROR) {
					fprintf(stderr, recargchar, optchar);
				}
				MSIM_OPT_optopt = optchar;
				return (BADARG);
			} else {
				MSIM_OPT_optarg = nargv[MSIM_OPT_optind];
			}
		}
		place = EMSG;
		++MSIM_OPT_optind;
	}
	/* dump back option letter */
	return (optchar);
}

/*
 * getopt --
 *	Parse argc/argv argument vector.
 *
 * [eventually this will replace the BSD getopt]
 */
int MSIM_OPT_Getopt(int nargc, char **nargv, const char *options)
{

	/*
	 * We don't pass FLAG_PERMUTE to getopt_internal() since
	 * the BSD getopt(3) (unlike GNU) has never done this.
	 *
	 * Furthermore, since many privileged programs call getopt()
	 * before dropping privileges it makes sense to keep things
	 * as simple (and bug-free) as possible.
	 */
	return (getopt_internal(nargc, nargv, options, NULL, NULL, 0));
}

/*
 * getopt_long --
 *	Parse argc/argv argument vector.
 */
int MSIM_OPT_Getopt_long(int nargc, char **nargv, const char *options,
                         const struct MSIM_OPT_Option *long_options, int *idx)
{
	return (getopt_internal(nargc, nargv, options, long_options, idx,
	                        FLAG_PERMUTE));
}
