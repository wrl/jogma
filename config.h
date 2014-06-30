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

#include "jogma/jogma.h"

/*****************************************************************************
 * change the things below this line
 ****************************************************************************/

/* number of samples to buffer.
 * default is 8 seconds at 44100. */
const size_t JOGMA_BUFFER_FRAMES = (44100 * 8);

struct jogma_stream stream_config = {
	/**
	 * the host, port, and mountpoint of your stream
	 */
	.server      = "localhost",
	.port        = 8000,
	.mount_point = "/stream",

	/**
	 * server source password
	 */
	.password = "wow-very-icecast",

	/**
	 * one of either
	 *   JOGMA_STREAM_PUBLIC, or
	 *   JOGMA_STREAM_UNLISTED
	 */
	.visibility = JOGMA_STREAM_PUBLIC,

	.metadata = {
		.name = "zombo",
		.description = "you can do anything at zombocom",
		.url = "http://www.zombo.com/",
		.genre = "1999"
	}
};

/**
 * connect to these JACK ports on startup. this array also determines
 * the channel count.
 *
 * to avoid auto-connecting to port, put a NULL for that channel.
 *
 * for example, for a stereo input connecting to the soundcard's capture
 * ports:
 *
 *     const char *jogma_input_ports[] = {
 *         "system:capture_1",
 *         "system:capture_2"
 *     };
 *
 * for a stereo input connecting to nothing:
 *
 *     const char *jogma_input_ports[] = {
 *         NULL,
 *         NULL
 *     };
 *
 * for a 4-channel stream connecting to the soundcard's capture ports:
 *
 *     const char *jogma_input_ports[] = {
 *         "system:capture_1",
 *         "system:capture_2",
 *         "system:capture_3",
 *         "system:capture_4"
 *     };
 *
 */

const char *jogma_input_ports[] = {
	"system:capture_1",
	"system:capture_2"
};
