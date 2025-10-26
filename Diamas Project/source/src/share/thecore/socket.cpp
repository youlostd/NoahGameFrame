/*
 *    Filename: socket.c
 * Description: ¼ÒÄÏ °ü·Ã ¼Ò½º.
 *
 *      Author: ºñ¿± aka. Cronan
 */
#define __LIBTHECORE__
#include "stdafx.h"

#if !defined(_WIN32)
#include <unistd.h>
#include <fcntl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#else
#include <winsock2.h>
#endif
#include <SpdLog.hpp>

#include <cerrno>
#include <cstring>
#include <cctype>
#include <cstdio>
#include "spdlog/spdlog.h"


// Forwards
void socket_lingeron(socket_t s);
void socket_lingeroff(socket_t s);
void socket_timeout(socket_t s, int32_t sec, int32_t usec);
void socket_reuse(socket_t s);
void socket_keepalive(socket_t s);

int socket_udp_read(socket_t desc, char* read_point, size_t space_left,
                    struct sockaddr* from, socklen_t* fromlen)
{
	return recvfrom(desc, read_point, space_left, 0, from, fromlen);
}

int socket_read(socket_t desc, char* read_point, size_t space_left)
{
	int ret = recv(desc, read_point, space_left, 0);

	if (ret > 0)
		return ret;

	if (ret == 0) // 정상적으로 접속 끊김
		return -1;

#ifdef EINTR /* Interrupted system call - various platforms */
	if (errno == EINTR)
		return 0;
#endif

#ifdef EAGAIN /* POSIX */
	if (errno == EAGAIN)
		return 0;
#endif

#ifdef EWOULDBLOCK /* BSD */
	if (errno == EWOULDBLOCK)
		return 0;
#endif /* EWOULDBLOCK */

#ifdef EDEADLK /* Macintosh */
	if (errno == EDEADLK)
		return 0;
#endif

#if defined(_WIN32)
	int wsa_error = WSAGetLastError();
	if (wsa_error == WSAEWOULDBLOCK)
		return 0;

	SPDLOG_INFO( "socket_read: WSAGetLastError returned {0}", wsa_error);
#endif

	spdlog::error( "about to lose connection");
	return -1;
}

int socket_write_tcp(socket_t desc, const char* txt, int length)
{
	int bytes_written = send(desc, txt, length, 0);
	if (bytes_written > 0)
		return bytes_written;

	if (bytes_written == 0)
		return -1;

#ifdef EAGAIN /* POSIX */
	if (errno == EAGAIN)
		return 0;
#endif

#ifdef EWOULDBLOCK /* BSD */
	if (errno == EWOULDBLOCK)
		return 0;
#endif

#ifdef EDEADLK /* Macintosh */
	if (errno == EDEADLK)
		return 0;
#endif

#if defined(_WIN32)
	int wsa_error = WSAGetLastError();
	if (wsa_error == WSAEWOULDBLOCK)
		return 0;

	SPDLOG_INFO( "socket_write_tcp: WSAGetLastError returned {0}",
	          wsa_error);
#endif

	// Looks like the error was fatal.  Too bad.
	return -1;
}

int socket_write(socket_t desc, const char* data, size_t length)
{
	size_t total;

	total = length;

	do {
		int bytes_written = socket_write_tcp(desc, data, total);
		if (bytes_written < 0) {
			return -1;
		} else if (bytes_written == 0) {
			return 0;
		} else {
			data += bytes_written;
			total -= bytes_written;
		}
	} while (total > 0);

	return 0;
}

socket_t socket_bind(const char* ip, int port, int protocol)
{
	socket_t s = socket(AF_INET, protocol, 0);
	if (s < 0) {
		spdlog::error( "socket: {0}", strerror(errno));
		return -1;
	}

	socket_reuse(s);

#if defined(_WIN32)
	// Winsock2: SO_DONTLINGER, SO_KEEPALIVE, SO_LINGER, and SO_OOBINLINE are
	// not supported on sockets of type SOCK_DGRAM
	if (protocol == SOCK_STREAM)
#endif
		socket_lingeroff(s);

#if defined(_WIN32)
	SOCKADDR_IN sa;
#else
	struct sockaddr_in sa;
#endif

	memset(&sa, 0, sizeof(sa));
	sa.sin_family = AF_INET;
	sa.sin_addr.s_addr = inet_addr(ip);
	sa.sin_port = htons((unsigned short)port);

	if (bind(s, (struct sockaddr*)&sa, sizeof(sa)) < 0) {
		spdlog::error( "bind: {0}", strerror(errno));
		return -1;
	}

	socket_nonblock(s);

	if (protocol == SOCK_STREAM) {
		SPDLOG_INFO( "SYSTEM: BINDING TCP PORT ON [{0}] (fd {1})", port, s);
		listen(s, -1);
	} else {
		SPDLOG_INFO( "SYSTEM: BINDING UDP PORT ON [{0}] (fd {1})", port, s);
	}

	return s;
}

socket_t socket_tcp_bind(const char* ip, int port)
{
	return socket_bind(ip, port, SOCK_STREAM);
}

socket_t socket_udp_bind(const char* ip, int port)
{
	return socket_bind(ip, port, SOCK_DGRAM);
}

void socket_close(socket_t s)
{
#if defined(_WIN32)
	closesocket(s);
#else
	close(s);
#endif
}

socket_t socket_accept(socket_t s, struct sockaddr_in* peer)
{
	socklen_t i = sizeof(*peer);

	socket_t desc = accept(s, (struct sockaddr*)peer, &i);
	if (desc == -1) {
		spdlog::error( "accept: {0} (fd {1})", strerror(errno), s);
		return -1;
	}

	if (desc >= 65500) {
		spdlog::error( "SOCKET FD 65500 LIMIT! {0}", desc);
		socket_close(s);
		return -1;
	}

	socket_nonblock(desc);
	socket_lingeroff(desc);
	return desc;
}

socket_t socket_connect(const char* host, uint16_t port)
{
	socket_t s = 0;
	struct sockaddr_in server_addr;
	int rslt;

	/* 소켓주소 구조체 초기화 */
	memset(&server_addr, 0, sizeof(server_addr));

	if (isdigit(*host)) {
		server_addr.sin_addr.s_addr = inet_addr(host);
	} else {
		struct hostent* hp;

		if ((hp = gethostbyname(host)) == NULL) {
			spdlog::error( "socket_connect(): can not connect to {0}:{1}",
			          host, port);
			return -1;
		}

		memcpy((char*)&server_addr.sin_addr, hp->h_addr,
		       sizeof(server_addr.sin_addr));
	}

	server_addr.sin_family = AF_INET;
	server_addr.sin_port = htons(port);

	if ((s = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		perror("socket");
		return -1;
	}

	socket_keepalive(s);
	socket_sndbuf(s, 233016);
	socket_rcvbuf(s, 233016);
	socket_timeout(s, 10, 0);
	socket_lingeron(s);

	/*  연결요청 */
	if ((rslt = connect(s, (struct sockaddr*)&server_addr,
	                    sizeof(server_addr))) < 0) {
		socket_close(s);

#if defined(_WIN32)
		switch (WSAGetLastError()) {
#else
		switch (rslt) {
#endif
#if defined(_WIN32)
			case WSAETIMEDOUT:
#else
			case EINTR:
#endif
				spdlog::error( "HOST {0}:{1} connection timeout.", host,
				          port);
				break;
#if defined(_WIN32)
			case WSAECONNREFUSED:
#else
			case ECONNREFUSED:
#endif
				spdlog::error(
				          "HOST {0}:{1} port is not opened. connection "
				          "refused.",
				          host, port);
				break;
#if defined(_WIN32)
			case WSAENETUNREACH:
#else
			case ENETUNREACH:
#endif
				spdlog::error(
				          "HOST {0}:{1} is not reachable from this host.", host,
				          port);
				break;

			default:
				spdlog::error( "HOST {0}:{1}, could not connect.", host,
				          port);
				break;
		}

		perror("connect");
		return -1;
	}

	return s;
}

#ifndef WIN32

#ifndef O_NONBLOCK
#define O_NONBLOCK O_NDELAY
#endif

void socket_nonblock(socket_t s)
{
	int flags;

	flags = fcntl(s, F_GETFL, 0);
	flags |= O_NONBLOCK;

	if (fcntl(s, F_SETFL, flags) < 0)
		spdlog::error( "fcntl: nonblock: {0}", strerror(errno));
}

void socket_block(socket_t s)
{
	int flags;

	flags = fcntl(s, F_GETFL, 0);
	flags &= ~O_NONBLOCK;

	if (fcntl(s, F_SETFL, flags) < 0)
		spdlog::error( "fcntl: nonblock: {0}", strerror(errno));
}

#else

void socket_nonblock(socket_t s)
{
	unsigned long val = 1;
	ioctlsocket(s, FIONBIO, &val);
}

void socket_block(socket_t s)
{
	unsigned long val = 0;
	ioctlsocket(s, FIONBIO, &val);
}

#endif

void socket_dontroute(socket_t s)
{
	int set;

	if (setsockopt(s, SOL_SOCKET, SO_DONTROUTE, (const char*)&set,
	               sizeof(int)) < 0) {
		spdlog::error( "setsockopt: dontroute: {0}", strerror(errno));
		socket_close(s);
	}
}

void socket_lingeroff(socket_t s)
{
#if defined(_WIN32)
	int linger;
	linger = 0;
#else
	struct linger linger;

	linger.l_onoff = 0;
	linger.l_linger = 0;
#endif

	if (setsockopt(s, SOL_SOCKET, SO_LINGER, (const char*)&linger,
	               sizeof(linger)) < 0) {
		spdlog::error( "setsockopt: linger: {0}", strerror(errno));
		socket_close(s);
	}
}

void socket_lingeron(socket_t s)
{
#if defined(_WIN32)
	int linger;
	linger = 0;
#else
	struct linger linger;

	linger.l_onoff = 1;
	linger.l_linger = 0;
#endif

	if (setsockopt(s, SOL_SOCKET, SO_LINGER, (const char*)&linger,
	               sizeof(linger)) < 0) {
		spdlog::error( "setsockopt: linger: {0}", strerror(errno));
		socket_close(s);
	}
}

void socket_rcvbuf(socket_t s, unsigned int opt)
{
	socklen_t optlen = sizeof(opt);
	if (setsockopt(s, SOL_SOCKET, SO_RCVBUF, (const char*)&opt, optlen) < 0) {
		spdlog::error( "setsockopt: rcvbuf: {0}", strerror(errno));
		socket_close(s);
		return;
	}

	opt = 0;
	optlen = sizeof(opt);

	if (getsockopt(s, SOL_SOCKET, SO_RCVBUF, (char*)&opt, &optlen) < 0) {
		spdlog::error( "getsockopt: rcvbuf: {0}", strerror(errno));
		socket_close(s);
		return;
	}

	SPDLOG_DEBUG( "SYSTEM: {0}: receive buffer changed to {1}", s, opt);
}

void socket_sndbuf(socket_t s, unsigned int opt)
{
	socklen_t optlen = sizeof(opt);
	if (setsockopt(s, SOL_SOCKET, SO_SNDBUF, (const char*)&opt, optlen) < 0) {
		spdlog::error( "setsockopt: sndbuf: {0}", strerror(errno));
		return;
	}

	opt = 0;
	optlen = sizeof(opt);

	if (getsockopt(s, SOL_SOCKET, SO_SNDBUF, (char*)&opt, &optlen) < 0) {
		spdlog::error( "getsockopt: sndbuf: {1}", strerror(errno));
		return;
	}

	SPDLOG_DEBUG( "SYSTEM: {1}: send buffer changed to {1}", s, opt);
}

// sec : seconds, usec : microseconds
void socket_timeout(socket_t s, int32_t sec, int32_t usec)
{
#if !defined(_WIN32)
	struct timeval rcvopt;
	struct timeval sndopt;
	socklen_t optlen = sizeof(rcvopt);

	rcvopt.tv_sec = sndopt.tv_sec = sec;
	rcvopt.tv_usec = sndopt.tv_usec = usec;
#else
	socklen_t rcvopt, sndopt;
	socklen_t optlen = sizeof(rcvopt);
	sndopt = rcvopt = (sec * 1000) + (usec / 1000);
#endif

	if (setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (const char*)&rcvopt, optlen) <
	    0) {
		spdlog::error( "setsockopt: timeout: {0}", strerror(errno));
		socket_close(s);
		return;
	}

	if (getsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*)&rcvopt, &optlen) < 0) {
		spdlog::error( "getsockopt: timeout: {0}", strerror(errno));
		socket_close(s);
		return;
	}

	optlen = sizeof(sndopt);

	if (setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (const char*)&sndopt, optlen) <
	    0) {
		spdlog::error( "setsockopt: timeout: {0}", strerror(errno));
		socket_close(s);
		return;
	}

	if (getsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char*)&sndopt, &optlen) < 0) {
		spdlog::error( "getsockopt: timeout: {0}", strerror(errno));
		socket_close(s);
		return;
	}

#if !defined(_WIN32)
	spdlog::trace("SYSTEM: {0}: TIMEOUT RCV: {1}.{2}, SND: {3}.{4}", s,
	           rcvopt.tv_sec, rcvopt.tv_usec, sndopt.tv_sec, sndopt.tv_usec);
#endif
}

void socket_reuse(socket_t s)
{
	int opt = 1;
	if (setsockopt(s, SOL_SOCKET, SO_REUSEADDR, (const char*)&opt,
	               sizeof(opt)) < 0) {
		spdlog::error( "setsockopt: reuse: {0}", strerror(errno));
		socket_close(s);
	}
}

void socket_keepalive(socket_t s)
{
	int opt = 1;
	if (setsockopt(s, SOL_SOCKET, SO_KEEPALIVE, (const char*)&opt,
	               sizeof(opt)) < 0) {
		perror("setsockopt: keepalive");
		socket_close(s);
	}
}