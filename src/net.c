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

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <fcntl.h>
#include <netdb.h>
#include <poll.h>

#include "jogma/jogma.h"

static int
resolve_addr(struct jogma_state *state, struct addrinfo *out)
{
	struct addrinfo hints = {}, *res;
	uint16_t port;
	int err;

	hints.ai_family   = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags    = AI_PASSIVE;

	err = getaddrinfo(state->stream->server, NULL, &hints, &res);
	if (err) {
		fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(err));
		return -1;
	}

	port = htons(state->stream->port);

	switch (res->ai_family) {
	case AF_INET:
		((struct sockaddr_in *) res->ai_addr)->sin_port = port;
		break;

	case AF_INET6:
		((struct sockaddr_in6 *) res->ai_addr)->sin6_port = port;
		break;
	}

	*out = *res;
	freeaddrinfo(res);
	return 0;
}

static void
print_address(struct addrinfo *addr)
{
	char ip_str[INET6_ADDRSTRLEN];
	struct sockaddr_in6 *sin6;
	struct sockaddr_in *sin;
	void *sin_addr;
	int port;

	switch (addr->ai_family) {
	case AF_INET:
		sin = ((struct sockaddr_in *) addr->ai_addr);
		sin_addr = &sin->sin_addr;
		port = sin->sin_port;
		break;

	case AF_INET6:
		sin6 = ((struct sockaddr_in6 *) addr->ai_addr);
		sin_addr = &sin6->sin6_addr;
		port = sin6->sin6_port;
		break;
	}

	inet_ntop(addr->ai_family, sin_addr, ip_str, sizeof(ip_str));
	printf(" :: connecting to %s, port %d\n\n", ip_str, ntohs(port));
}

static int
set_nonblock(int fd)
{
	int flags = fcntl(fd, F_GETFL, 0);
	return fcntl(fd, F_SETFL, flags & O_NONBLOCK);
}

ssize_t
jogma_net_send(struct jogma_state *state, const void *buf, size_t nbytes)
{
	struct pollfd fd;
	ssize_t sent;

	fd.fd = state->socket_fd;
	fd.events = POLLOUT;

	while (nbytes > 0) {
		poll(&fd, 1, 0);

		if (!(fd.revents & POLLOUT))
			continue;

		sent    = send(fd.fd, buf, nbytes, 0);
		buf    += sent;
		nbytes -= sent;
	}

	return sent;
}

int
jogma_net_open(struct jogma_state *state)
{
	struct addrinfo addr;
	int s;

	if (resolve_addr(state, &addr))
		goto err_resolve;

	print_address(&addr);

	s = socket(addr.ai_family, addr.ai_socktype, addr.ai_protocol);
	if (!s) {
		perror("socket()");
		goto err_socket;
	}

	state->socket_fd = s;

	if (connect(s, addr.ai_addr, addr.ai_addrlen)) {
		perror("connect");
		goto err_connect;
	}

	set_nonblock(s);

	return 0;

err_connect:
	close(s);
	state->socket_fd = -1;
err_socket:
err_resolve:
	return -1;
}

void
jogma_net_close(struct jogma_state *state)
{
	close(state->socket_fd);
}
