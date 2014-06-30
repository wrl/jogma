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

#pragma once

#include <jack/jack.h>
#include <jack/ringbuffer.h>
#include <FLAC/all.h>

#include <http_parser/http_parser.h>

#define JOGMA_MAX_CHANNELS	8

typedef enum {
	JOGMA_STREAM_UNLISTED = 0,
	JOGMA_STREAM_PUBLIC
} jogma_stream_visibility_t;

struct jogma_jack_port {
	jack_port_t *port;
	jack_ringbuffer_t *buffer;
};

typedef enum {
	JOGMA_STATUS_STARTING,
	JOGMA_STATUS_RUNNING,
	JOGMA_STATUS_STOPPING
} jogma_status_t;

struct jogma_stream {
	char *server;
	int port;
	char *mount_point;
	char *password;

	jogma_stream_visibility_t visibility;

	struct {
		char *name;
		char *description;
		char *url;
		char *genre;
	} metadata;
};

struct jogma_state {
	jogma_status_t status;

	int socket_fd;

	struct {
		jack_client_t *client;
		struct jogma_jack_port ports[JOGMA_MAX_CHANNELS];

		int channels;
	} jack;

	struct {
		FLAC__StreamEncoder *encoder;
		FLAC__int32 *input_buffer;

		FLAC__int32 *in_buffers[JOGMA_MAX_CHANNELS];
	} flac;

	struct {
		http_parser parser;
	} http;

	const struct jogma_stream *stream;
};

int  jogma_flac_init(struct jogma_state *);
void jogma_flac_fini(struct jogma_state *);

int  jogma_jack_init(struct jogma_state *, unsigned channels);
void jogma_jack_fini(struct jogma_state *);

ssize_t jogma_net_send(struct jogma_state *, const void *buf, size_t nbytes);
int  jogma_net_open(struct jogma_state *);
void jogma_net_close(struct jogma_state *);

int  jogma_init(struct jogma_state *);
void jogma_event_loop(struct jogma_state *);
void jogma_fini(struct jogma_state *);
