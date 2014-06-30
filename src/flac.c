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

#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <FLAC/all.h>

#include "jogma/jogma.h"

static FLAC__StreamEncoderWriteStatus
write_cb(const FLAC__StreamEncoder *env, const FLAC__byte *buf, size_t nbytes,
		unsigned samples, unsigned current_frame, void *ctx)
{
	struct jogma_state *state = ctx;
	jogma_net_send(state, buf, nbytes);
	return FLAC__STREAM_ENCODER_WRITE_STATUS_OK;
}

int
jogma_flac_init(struct jogma_state *state)
{
	FLAC__StreamEncoder *enc;
	int i;

	state->flac.input_buffer = calloc(state->jack.channels,
			JOGMA_BUFFER_FRAMES * sizeof(FLAC__int32));

	if (!state->flac.input_buffer)
		return -1;

	for (i = 0; i < state->jack.channels; i++)
		state->flac.in_buffers[i] =
			&state->flac.input_buffer[i * JOGMA_BUFFER_FRAMES];

	state->flac.encoder = enc = FLAC__stream_encoder_new();
	FLAC__stream_encoder_set_ogg_serial_number(enc, 4242);

	FLAC__stream_encoder_set_channels(enc, state->jack.channels);
	FLAC__stream_encoder_set_bits_per_sample(enc, 16);
	FLAC__stream_encoder_set_sample_rate(enc,
			jack_get_sample_rate(state->jack.client));

	FLAC__stream_encoder_init_ogg_stream(enc,
			NULL, write_cb, NULL, NULL, NULL, state);

	return 0;
}

void
jogma_flac_fini(struct jogma_state *state)
{
	FLAC__stream_encoder_finish(state->flac.encoder);
	FLAC__stream_encoder_delete(state->flac.encoder);

	free(state->flac.input_buffer);
}
