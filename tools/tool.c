/*
 * Copyright (c) 2011, Collabora Ltd.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 *     * Redistributions of source code must retain the above
 *       copyright notice, this list of conditions and the
 *       following disclaimer.
 *     * Redistributions in binary form must reproduce the
 *       above copyright notice, this list of conditions and
 *       the following disclaimer in the documentation and/or
 *       other materials provided with the distribution.
 *     * The names of contributors to this software may not be
 *       used to endorse or promote products derived from this
 *       software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 *
 * Author: Stef Walter <stefw@collabora.co.uk>
 */

#include "config.h"

#include "buffer.h"
#include "compat.h"
#include "debug.h"
#include "library.h"
#include "p11-kit.h"

#include <assert.h>
#include <ctype.h>
#include <getopt.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "tool.h"

struct {
	const char *name;
	int (*function) (int, char*[]);
	const char *text;
} commands[] = {
#ifdef WITH_ASN1
	{ "extract", p11_tool_extract, "Extract certificates" },
#endif
	{ "list-modules", p11_tool_list_modules, "List modules and tokens"},
	{ 0, }
};

static char
short_option (int opt)
{
	if (isalpha (opt) || isdigit (opt))
		return (char)opt;
	return 0;
}

static const struct option *
find_option (const struct option *longopts,
             int opt)
{
	int i;

	for (i = 0; longopts[i].name != NULL; i++) {
		if (longopts[i].val == opt)
			return longopts + i;
	}

	return NULL;
}

void
p11_tool_usage (const p11_tool_desc *usages,
                const struct option *longopts)
{
	const struct option *longopt;
	const int indent = 22;
	const char *long_name;
	const char *description;
	const char *next;
	char short_name;
	int spaces;
	int len;
	int i;

	for (i = 0; usages[i].text != NULL; i++) {

		/* If no option, then this is a heading */
		if (!usages[i].option) {
			printf ("%s\n\n", usages[i].text);
			continue;
		}

		longopt = find_option (longopts, usages[i].option);
		long_name = longopt ? longopt->name : NULL;
		short_name = short_option (usages[i].option);
		description = usages[i].text;

		if (short_name && long_name)
			len = printf ("  -%c, --%s", (int)short_name, long_name);
		else if (long_name)
			len = printf ("  --%s", long_name);
		else
			len = printf ("  -%c", (int)short_name);
		if (longopt && longopt->has_arg)
			len += printf ("%s<%s>",
			               long_name ? "=" : " ",
			               usages[i].arg ? usages[i].arg : "...");
		if (len < indent) {
			spaces = indent - len;
		} else {
			printf ("\n");
			spaces = indent;
		}
		while (description) {
			while (spaces-- > 0)
				fputc (' ', stdout);
			next = strchr (description, '\n');
			if (next) {
				next += 1;
				printf ("%.*s", (int)(next - description), description);
				description = next;
				spaces = indent;
			} else {
				printf ("%s\n", description);
				break;
			}
		}

	}
}

int
p11_tool_getopt (int argc,
                 char *argv[],
                 const struct option *longopts)
{
	p11_buffer buf;
	int ret;
	char opt;
	int i;

	if (!p11_buffer_init_null (&buf, 64))
		return_val_if_reached (-1);

	for (i = 0; longopts[i].name != NULL; i++) {
		opt = short_option (longopts[i].val);
		if (opt != 0) {
			p11_buffer_add (&buf, &opt, 1);
			assert (longopts[i].has_arg != optional_argument);
			if (longopts[i].has_arg == required_argument)
				p11_buffer_add (&buf, ":", 1);
		}
	}

	ret = getopt_long (argc, argv, buf.data, longopts, NULL);

	p11_buffer_uninit (&buf);

	return ret;
}

static void
command_usage (void)
{
	int i;

	printf ("usage: p11-kit command <args>...\n");
	printf ("\nCommon p11-kit commands are:\n");
	for (i = 0; commands[i].name != NULL; i++)
		printf ("  %-15s  %s\n", commands[i].name, commands[i].text);
	printf ("\nSee 'p11-kit <command> --help' for more information\n");
}

static void
exec_external (const char *command,
               int argc,
               char *argv[])
{
	char *filename;
	const char *path;
	char *env;

	if (!asprintf (&filename, "p11-kit-%s", command) < 0)
		return_if_reached ();

	/* Add our libexec directory to the path */
	path = getenv ("PATH");
	if (!asprintf (&env, "PATH=%s%s%s", path ? path : "", path ? ":" : "", PKGDATADIR))
		return_if_reached ();
	putenv (env);

	argv[0] = filename;
	execvp (filename, argv);
}

static void
verbose_arg (void)
{
	putenv ("P11_KIT_DEBUG=all");
	p11_kit_be_loud ();
	p11_message_loud ();
}

static void
quiet_arg (void)
{
	putenv ("P11_KIT_DEBUG=");
	p11_kit_be_quiet ();
	p11_message_quiet ();
}

int
main (int argc, char *argv[])
{
	char *command = NULL;
	bool skip;
	int in, out;
	int i;

	/*
	 * Parse the global options. We rearrange the options as
	 * necessary, in order to pass relevant options through
	 * to the commands, but also have them take effect globally.
	 */

	for (in = 1, out = 1; in < argc; in++, out++) {

		/* Already seen the command, keep the arguments */
		if (command) {
			skip = false;

		/* The non-option is the command, take it out of the arguments */
		} else if (argv[in][0] != '-') {
			skip = true;
			command = argv[in];

		/* The global long options */
		} else if (argv[in][1] == '-') {
			skip = false;

			if (strcmp (argv[in], "--") == 0) {
				p11_message ("no command specified");
				return 2;

			} else if (strcmp (argv[in], "--verbose") == 0) {
				verbose_arg ();

			} else if (strcmp (argv[in], "--quiet") == 0) {
				quiet_arg ();

			} else if (strcmp (argv[in], "--help") == 0) {
				command_usage ();
				return 0;

			} else {
				p11_message ("unknown option: %s", argv[in]);
				return 2;
			}

		/* The global short options */
		} else {
			skip = false;

			for (i = 1; argv[in][i] != '\0'; i++) {
				switch (argv[in][i]) {
				case 'h':
					command_usage ();
					return 0;

				/* Compatibility option */
				case 'l':
					command = "list-modules";
					break;

				case 'v':
					verbose_arg ();
					break;

				case 'q':
					quiet_arg ();
					break;

				default:
					p11_message ("unknown option: -%c", (int)argv[in][i]);
					return 2;
				}
			}
		}

		/* Skipping this argument? */
		if (skip)
			out--;
		else
			argv[out] = argv[in];
	}

	if (command == NULL) {
		/* As a special favor if someone just typed 'p11-kit', help them out */
		if (argc == 1)
			command_usage ();
		else
			p11_message ("no command specified");
		return 2;
	}

	argc = out;

	/* Look for the command */
	for (i = 0; commands[i].name != NULL; i++) {
		if (strcmp (commands[i].name, command) == 0) {
			argv[0] = command;
			return (commands[i].function) (argc, argv);
		}
	}

	/* Got here because no command matched */
	exec_external (command, argc, argv);

	/* At this point we have no command */
	p11_message ("'%s' is not a valid p11-kit command. See 'p11-kit --help'", command);
	return 2;
}
