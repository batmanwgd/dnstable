/*
 * Copyright (c) 2012 by Internet Systems Consortium, Inc. ("ISC")
 *
 * Permission to use, copy, modify, and/or distribute this software for any
 * purpose with or without fee is hereby granted, provided that the above
 * copyright notice and this permission notice appear in all copies.
 *
 * THE SOFTWARE IS PROVIDED "AS IS" AND ISC DISCLAIMS ALL WARRANTIES
 * WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS.	IN NO EVENT SHALL ISC BE LIABLE FOR
 * ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL DAMAGES OR ANY DAMAGES
 * WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR PROFITS, WHETHER IN AN
 * ACTION OF CONTRACT, NEGLIGENCE OR OTHER TORTIOUS ACTION, ARISING OUT
 * OF OR IN CONNECTION WITH THE USE OR PERFORMANCE OF THIS SOFTWARE.
 */

#include <assert.h>
#include <errno.h>
#include <locale.h>
#include <inttypes.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <dnstable.h>
#include <mtbl.h>

#include "librsf/print_string.h"

static void
print_entry(struct dnstable_entry *ent)
{
	char *s = dnstable_entry_to_text(ent);
	assert(s != NULL);
	if (strlen(s) > 0) {
		fputs(s, stdout);
		if (dnstable_entry_get_type(ent) == DNSTABLE_ENTRY_TYPE_RRSET)
			putchar('\n');
		free(s);
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
	fprintf(stderr, "Dumped %'" PRIu64 " entries.\n", count);
}

int
main(int argc, char **argv)
{
	setlocale(LC_ALL, "");

	const char *m_fname;
	struct mtbl_reader *m_reader;
	struct dnstable_reader *d_reader;
	struct dnstable_iter *d_it;

	if (argc != 2) {
		fprintf(stderr, "Usage: %s <DB FILE>\n", argv[0]);
		exit(EXIT_FAILURE);
	}
	m_fname = argv[1];

	m_reader = mtbl_reader_init(m_fname, NULL);
	if (m_reader == NULL) {
		perror("mtbl_reader_init");
		fprintf(stderr, "dnstable_dump: unable to open database file %s\n",
			m_fname);
		exit(EXIT_FAILURE);
	}

	d_reader = dnstable_reader_init(mtbl_reader_source(m_reader));
	d_it = dnstable_reader_iter(d_reader);
	do_dump(d_it);
	dnstable_iter_destroy(&d_it);
	dnstable_reader_destroy(&d_reader);
	mtbl_reader_destroy(&m_reader);

	return (EXIT_SUCCESS);
}
