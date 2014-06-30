/**
 * Copyright (c) 2014 William Light <wrl@illest.net>
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
 */

#include <stdio.h>
#include <jack/jack.h>

#include "jogma/jogma.h"

#include "config.h"
#include "jogma_build_config.h"

#include "jogma_private/util.h"

static void
connect_to_inports(struct jogma_state *state)
{
	struct jogma_jack_port *p;
	unsigned i, channels;

	channels = state->jack.channels;
	for (i = 0; i < channels; i++) {
		p = &state->jack.ports[i];

		if (!jogma_input_ports[i])
			continue;

		jack_connect(state->jack.client,
				jogma_input_ports[i], jack_port_name(p->port));
	}
}

int
main(int argc, char **argv)
{
	struct jogma_state state = {};

	printf(
		" :: jogma version %d.%d.%d-%s\n"
		" :: copyright 2014 william light <wrl@illest.net>\n\n",
		_JOGMA_VERSION_MAJOR, _JOGMA_VERSION_MINOR, _JOGMA_VERSION_PATCH_LEVEL,
		_JOGMA_GIT_REVISION);

	state.stream = &stream_config;

	if (jogma_init(&state))
		return -1;

	if (jogma_jack_init(&state, ARRAY_LENGTH(jogma_input_ports)))
		return -1;

	if (jogma_flac_init(&state))
		return -1;

	connect_to_inports(&state);

	jogma_event_loop(&state);

	jogma_flac_fini(&state);
	jogma_jack_fini(&state);
	jogma_fini(&state);

	return 0;
}
