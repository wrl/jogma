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

#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>
#include <math.h>

#include "jogma/jogma.h"
#include "jogma/http.h"

#define HTTP_CHUNK_MS 15

#define FLOAT_STRIDE 4

inline static int32_t
float_to_int16(float x)
{
	return lrintf(32767.f * fmin(1.f, fmax(-1.f, x)));
}

static size_t
spool_samples_to_flac(struct jogma_state *state)
{
	jack_ringbuffer_data_t read_vec[2];
	size_t min_available, available;
	struct jogma_jack_port *p;
	unsigned i, j, channels;
	FLAC__int32 *buffer;

	channels = state->jack.channels;
	min_available = -1;

	for (i = 0; i < channels; i++) {
		p = &state->jack.ports[i];

		available = jack_ringbuffer_read_space(p->buffer);
		if (min_available > available)
			min_available = available;
	}

	min_available /= FLOAT_STRIDE;

	for (i = 0; i < channels; i++) {
		p = &state->jack.ports[i];
		available = min_available;
		buffer = state->flac.in_buffers[i];

		jack_ringbuffer_get_read_vector(p->buffer, read_vec);

		for (j = 0; j < read_vec[0].len && available;
				j += FLOAT_STRIDE, available--) {
			*buffer++ = float_to_int16(*((float *) &read_vec[0].buf[j]));
		}

		for (j = 0; j < read_vec[1].len && available;
				j += FLOAT_STRIDE, available--)
			*buffer++ = float_to_int16(*((float *) &read_vec[1].buf[j]));

		jack_ringbuffer_read_advance(p->buffer, min_available * FLOAT_STRIDE);
	}

	return min_available;
}

static ssize_t
read_from_socket(struct jogma_state *state)
{
	char buf[512];
	ssize_t recvd;

	recvd = recv(state->socket_fd, buf, sizeof(buf), 0);

	if (recvd > 0) {
		jogma_http_process(state, buf, recvd);
		return recvd;
	}

	return recvd;
}

int
jogma_init(struct jogma_state *state)
{
	if (jogma_net_open(state))
		return -1;

	jogma_http_init(state);
	state->status = JOGMA_STATUS_STARTING;
	return 0;
}

void
jogma_event_loop(struct jogma_state *state)
{
	struct pollfd fd;
	size_t available;

	fd.fd = state->socket_fd;
	fd.events = POLLIN;

	jogma_http_send_headers(state);

	for (;;) {
		poll(&fd, 1, HTTP_CHUNK_MS);

		if (fd.revents & POLLIN)
			read_from_socket(state);

		switch (state->status) {
		case JOGMA_STATUS_STARTING:
			continue;

		case JOGMA_STATUS_RUNNING:
			break;

		case JOGMA_STATUS_STOPPING:
			return;
		}

		available = spool_samples_to_flac(state);

		FLAC__stream_encoder_process(state->flac.encoder,
				(const FLAC__int32 **) state->flac.in_buffers, available);
	}
}

int
jogma_transition(struct jogma_state *state, jogma_status_t new_status)
{
	switch (state->status) {
	case JOGMA_STATUS_STARTING:
		switch (new_status) {
		case JOGMA_STATUS_RUNNING:
			jogma_flac_init(state);
			break;

		case JOGMA_STATUS_STOPPING:
			break;

		default:
			return -1;
		}

		break;

	case JOGMA_STATUS_RUNNING:
		switch (new_status) {
		case JOGMA_STATUS_STOPPING:
			jogma_flac_fini(state);
			break;

		default:
			return -1;
		}
		break;

	case JOGMA_STATUS_STOPPING:
		return -1;
	}

	state->status = new_status;
	return 0;
}

void
jogma_fini(struct jogma_state *state)
{
	jogma_net_close(state);
}
