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

#include <jack/jack.h>

#include "jogma/jogma.h"
#include "jogma_private/util.h"

static int
process(jack_nframes_t nframes, void *ctx)
{
	struct jogma_state *state = ctx;
	struct jogma_jack_port *p;
	unsigned i, channels;
	size_t written;
	float *buf;

	channels = state->jack.channels;
	for (i = 0; i < channels; i++) {
		p = &state->jack.ports[i];

		buf = jack_port_get_buffer(p->port, nframes);
		written = jack_ringbuffer_write(p->buffer,
				(void *) buf, nframes * sizeof(*buf));

		if (written < nframes)
			printf(
				" :: JACK buffer overflow on channel %d:"
				"tried to write %d frames, only wrote %zu\n",
				i, nframes, written);
	}

	return 0;
}

int
jogma_jack_init(struct jogma_state *state, unsigned channels)
{
	struct jogma_jack_port *p;
	char port_name_buf[32];
	jack_status_t status;
	jack_client_t *c;
	unsigned i;

	if (!channels)
		return -1;

	c = jack_client_open("jogma", JackNoStartServer, &status);
	if (!c)
		return -1;

	state->jack.client = c;

	for (i = 0; i < channels && i < ARRAY_LENGTH(state->jack.ports); i++) {
		p = &state->jack.ports[i];

		snprintf(port_name_buf, sizeof(port_name_buf), "input_%d", i);
		p->port = jack_port_register(c,
				port_name_buf, JACK_DEFAULT_AUDIO_TYPE, JackPortIsInput, 0);

		p->buffer =
			jack_ringbuffer_create(sizeof(float) * JOGMA_BUFFER_FRAMES);
	}

	state->jack.channels = i;
	jack_set_process_callback(c, process, state);
	jack_activate(c);

	return 0;
}

void
jogma_jack_fini(struct jogma_state *state)
{
	struct jogma_jack_port *p;
	jack_client_t *c = state->jack.client;
	unsigned i, channels;

	jack_deactivate(c);

	channels = state->jack.channels;
	for (i = 0; i < channels; i++) {
		p = &state->jack.ports[i];

		jack_port_unregister(c, p->port);
		jack_ringbuffer_free(p->buffer);

		p->port   = NULL;
		p->buffer = NULL;
	}

	jack_client_close(c);
	state->jack.client = NULL;
}
