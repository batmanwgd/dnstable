/*
 * Copyright (c) 2012, 2014 by Farsight Security, Inc.
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *    http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <assert.h>
#include <errno.h>
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <dnstable.h>
#include <mtbl.h>

#include "libmy/argv.h"
#include "libmy/print_string.h"

static bool g_json;
static bool g_rrset;
static bool g_rdata;
static char *g_fname;

static argv_t args[] = {
	{ 'j',	"json",
		ARGV_BOOL,
		&g_json,
		NULL,
		"output in JSON format (default: text)" },

	{ 'r',	"rrset",
		ARGV_BOOL,
		&g_rrset,
		NULL,
		"output rrset records" },

	{ ARGV_ONE_OF },

	{ 'd',	"rdata",
		ARGV_BOOL,
		&g_rdata,
		NULL,
		"output rdata records" },

	{ ARGV_MAND, NULL,
		ARGV_CHAR_P,
		&g_fname,
		"filename",
		"input file" },

	{ ARGV_LAST }
};

static void
print_entry(struct dnstable_entry *ent)
{
	if (dnstable_entry_get_type(ent) == DNSTABLE_ENTRY_TYPE_RRSET ||
	    dnstable_entry_get_type(ent) == DNSTABLE_ENTRY_TYPE_RDATA)
	{
		char *s;
		if (g_json)
			s = dnstable_entry_to_json(ent);
		else
			s = dnstable_entry_to_text(ent);
		if (s != NULL) {
			fputs(s, stdout);
			if (!(!g_json &&
			      dnstable_entry_get_type(ent) == DNSTABLE_ENTRY_TYPE_RDATA))
			{
				putchar('\n');
			}
			free(s);
		}
	}
}

static void
do_dump(struct dnstable_iter *it)
{
	struct dnstable_entry *ent;
	uint64_t count = 0;

	while (dnstable_iter_next(it, &ent) == dnstable_res_success) {
		assert(ent != NULL);
		print_entry(ent);
		dnstable_entry_destroy(&ent);
		count++;
	}
}

int
main(int argc, char **argv)
{
	struct mtbl_reader *m_reader;
	struct dnstable_reader *d_reader;
	struct dnstable_iter *d_it = NULL;

	argv_process(args, argc, argv);

	m_reader = mtbl_reader_init(g_fname, NULL);
	if (m_reader == NULL) {
		perror("mtbl_reader_init");
		fprintf(stderr, "dnstable_dump: unable to open database file %s\n",
			g_fname);
		exit(EXIT_FAILURE);
	}

	d_reader = dnstable_reader_init(mtbl_reader_source(m_reader));

	if (g_rrset)
		d_it = dnstable_reader_iter_rrset(d_reader);
	else if (g_rdata)
		d_it = dnstable_reader_iter_rdata(d_reader);

	assert(d_it != NULL);
	do_dump(d_it);
	dnstable_iter_destroy(&d_it);
	dnstable_reader_destroy(&d_reader);
	mtbl_reader_destroy(&m_reader);

	argv_cleanup(args);

	return (EXIT_SUCCESS);
}
