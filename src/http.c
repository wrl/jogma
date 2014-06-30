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
#include <string.h>
#include <stdio.h>

#include <b64/cencode.h>
#include <http_parser/http_parser.h>

#include "jogma/jogma.h"
#include "jogma/http.h"

#include "jogma_build_config.h"

static int
encode_auth_basic_header(struct jogma_state *state, char *buf, size_t nbytes)
{
	char unencoded[384];
	base64_encodestate b64st;
	size_t len;

	len = snprintf(unencoded, sizeof(unencoded), "%s:%s",
			"source", state->stream->password);

	base64_init_encodestate(&b64st);
	buf += base64_encode_block(unencoded, len, buf, &b64st);
	buf += base64_encode_blockend(buf, &b64st);

	*buf = '\0';
	return 0;
}

static int
on_status(http_parser *parser, const char *at, size_t length)
{
	struct jogma_state *state = parser->data;

	switch (parser->status_code) {
	case 200:
		puts(" :: server said \"OK\", starting stream...");
		state->status = JOGMA_STATUS_RUNNING;
		/* fall through */

	case 100:
		return 0;

	default:
		break;
	}

	fprintf(stderr, " !! icecast server said: %d: ", parser->status_code);
	fwrite(at, 1, length, stderr);
	fputs("\n !! bailing out...\n", stderr);

	state->status = JOGMA_STATUS_STOPPING;
	return 0;
}

const struct http_parser_settings parser_settings = {
	.on_message_begin    = NULL,
	.on_url              = NULL,
	.on_status           = on_status,
	.on_header_field     = NULL,
	.on_header_value     = NULL,
	.on_headers_complete = NULL,
	.on_body             = NULL,
	.on_message_complete = NULL
};

int
jogma_http_send_headers(struct jogma_state *state)
{
	const struct jogma_stream *stream = state->stream;
	char header_buf[256], auth[527];

#define ADD_HEADER(h) do {													\
	send(state->socket_fd, h, strlen(h), 0);								\
	send(state->socket_fd, "\n", 1, 0);										\
} while (0)

#define ADD_HEADER_SPRINTF(fmt, ...) do {									\
	snprintf(header_buf, sizeof(header_buf), fmt, __VA_ARGS__);				\
	ADD_HEADER(header_buf);													\
} while (0)

#define ADD_HEADER_METADATA_STR(fmt, str) do {								\
	if (str)																\
		ADD_HEADER_SPRINTF(fmt, str);										\
} while(0)

	encode_auth_basic_header(state, auth, sizeof(auth));

	ADD_HEADER_SPRINTF("PUT %s HTTP/1.1", "/stream");
	ADD_HEADER_SPRINTF("Authorization: Basic %s", auth);
	ADD_HEADER_SPRINTF("Host: %s:%d", stream->server, stream->port);
	ADD_HEADER("Content-type: application/ogg");

	ADD_HEADER_SPRINTF("User-Agent: Jogma %d.%d.%d-%s",
		_JOGMA_VERSION_MAJOR, _JOGMA_VERSION_MINOR, _JOGMA_VERSION_PATCH_LEVEL,
		_JOGMA_GIT_REVISION);

	ADD_HEADER_SPRINTF("ice-public: %d", stream->visibility);

	ADD_HEADER_METADATA_STR("ice-name: %s", stream->metadata.name);
	ADD_HEADER_METADATA_STR("ice-description: %s", stream->metadata.description);
	ADD_HEADER_METADATA_STR("ice-url: %s", stream->metadata.url);
	ADD_HEADER_METADATA_STR("ice-genre: %s", stream->metadata.genre);

	ADD_HEADER("Expect: 100-continue\n");

#undef ADD_HEADER_METADATA_STR
#undef ADD_HEADER_SPRINTF
#undef ADD_HEADER

	return 0;
}

int
jogma_http_process(struct jogma_state *state, const char *buf, size_t nbytes)
{
	return http_parser_execute(&state->http.parser, &parser_settings, buf, nbytes);
}

int
jogma_http_init(struct jogma_state *state)
{
	http_parser *p = &state->http.parser;

	http_parser_init(p, HTTP_RESPONSE);
	p->data = state;

	return 0;
}

int
jogma_http_fini(struct jogma_state *state)
{
	return 0;
}
