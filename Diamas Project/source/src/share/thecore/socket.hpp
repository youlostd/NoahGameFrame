/*
 *    Filename: socket.c
 * Description: 소켓 관련 함수 헤더.
 *
 *      Author: 비엽 (server), myevan (Client)
 */
#ifndef __INC_LIBTHECORE_SOCKET_H__
#define __INC_LIBTHECORE_SOCKET_H__

#if !defined(_WIN32)
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#else
#include <winsock2.h>
#endif


#if defined(_WIN32)
typedef int socklen_t;
typedef uintptr_t socket_t;
#else
#define INVALID_SOCKET -1
typedef int socket_t;
#endif

extern int socket_read(socket_t desc, char* read_point, size_t space_left);
extern int socket_write(socket_t desc, const char* data, size_t length);

extern int socket_udp_read(socket_t desc, char* read_point, size_t space_left,
                           struct sockaddr* from, socklen_t* fromlen);

extern socket_t socket_tcp_bind(const char* ip, int port);
extern socket_t socket_udp_bind(const char* ip, int port);

extern socket_t socket_accept(socket_t s, struct sockaddr_in* peer);
extern void socket_close(socket_t s);
extern socket_t socket_connect(const char* host, uint16_t port);

extern void socket_nonblock(socket_t s);
extern void socket_block(socket_t s);
extern void socket_dontroute(socket_t s);
extern void socket_lingeroff(socket_t s);
extern void socket_lingeron(socket_t s);

extern void socket_sndbuf(socket_t s, unsigned int opt);
extern void socket_rcvbuf(socket_t s, unsigned int opt);

#endif
