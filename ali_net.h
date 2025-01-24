#ifndef ALI_NET_H_
#define ALI_NET_H_

#ifndef ALI_TYPES_
#define ALI_TYPES_
#include <stdint.h>
typedef uint8_t  ali_u8;
typedef uint16_t ali_u16;
typedef uint32_t ali_u32;
typedef uint64_t ali_u64;

typedef int8_t  ali_i8;
typedef int16_t ali_i16;
typedef int32_t ali_i32;
typedef int64_t ali_i64;

typedef float ali_f32;

#ifdef __x86_64__
typedef double ali_f64;
#endif // __x86_64__

typedef ali_u64 ali_usize;
typedef ali_i64 ali_isize;
#endif // ALI_TYPES_

#ifndef ALI_ASSERT
#include <assert.h>
#define ALI_ASSERT assert
#endif // ALI_ASSERT

#ifndef ALI_RETURN_DEFER
#define ALI_RETURN_DEFER(thing) do { result = thing; goto defer; } while (0)
#endif // ALI_RETURN_DEFER

#ifndef ali_logn_info
#include <stdio.h>
#define ali_logn_info(...) do { printf(__VA_ARGS__); printf("\n"); } while (0)
#endif // ali_logn_info

#ifndef ali_logn_warn
#include <stdio.h>
#define ali_logn_warn(...) do { printf(__VA_ARGS__); printf("\n"); } while (0)
#endif // ali_log_warn

#ifndef ali_logn_error
#include <stdio.h>
#define ali_logn_error(...) do { fprintf(stderr, __VA_ARGS__); fprintf(stderr, "\n"); } while (0)
#endif // ali_log_error

ali_i32 ali_tcp_server_socket(const char* host, ali_u16 port, ali_i32 backlog);
ali_i32 ali_tcp_server_accept(ali_i32 sockfd, char** host, ali_u16* port);

ali_i32 ali_tcp_client_connect(const char* host, ali_u16 port);

#endif // ALI_NET_H_

#ifdef ALI_NET_IMPLEMENTATION
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

ali_i32 ali_tcp_server_socket(const char* host, ali_u16 port, ali_i32 backlog) {
    bool result = true;
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        ali_logn_error("Couldn't create socket: %s", strerror(errno));
        ALI_RETURN_DEFER(false);
    }

    int reuse = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0) {
        ali_logn_error("Couldn't set option SO_REUSEADDR on socket: %s", strerror(errno));
        ALI_RETURN_DEFER(false);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_aton(host, &addr.sin_addr);

    if (bind(sockfd, (void*)&addr, sizeof(addr)) < 0) {
        ali_logn_error("Couldn't bind socket to %s:%d: %s", host, port, strerror(errno));
        ALI_RETURN_DEFER(false);
    }

    if (listen(sockfd, backlog) < 0) {
        ali_logn_error("Couldn't listen on socket: %s", strerror(errno));
        ALI_RETURN_DEFER(false);
    }

defer:
    if (!result) {
        if (sockfd >= 0) close(sockfd);
        return -1;
    }
    return sockfd;
}

ali_i32 ali_tcp_server_accept(ali_i32 sockfd, char** host, ali_u16* port) {
    ALI_ASSERT(sockfd >= 0);

    struct sockaddr_in addr;
    socklen_t addrlen = sizeof(addr);
    ali_i32 clientfd = accept(sockfd, (void*)&addr, &addrlen);
    if (clientfd < 0) {
        ali_logn_error("Couldn't accept connection: %s", strerror(errno));
        return -1;
    }

    *host = inet_ntoa(addr.sin_addr);
    *port = ntohs(addr.sin_port);

    return clientfd;
}

ali_i32 ali_tcp_client_connect(const char* host, ali_u16 port) {
    bool result = true;
    ali_i32 sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        ali_logn_error("Couldn't create socket: %s", strerror(errno));
        ALI_RETURN_DEFER(false);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_aton(host, &addr.sin_addr);

    if (connect(sockfd, (void*)&addr, sizeof(addr)) < 0) {
        ali_logn_error("Couldn't connect on %s:%d: %s", host, port, strerror(errno));
        ALI_RETURN_DEFER(false);
    }

defer:
    if (!result) {
        if (sockfd >= 0) close(sockfd);
        return -1;
    }
    return sockfd;
}

#endif // ALI_IMPLEMENTATION

#ifdef ALI_NET_REMOVE_PREFIX
#ifndef ALI_NET_REMOVE_PREFIX_GUARD_
#define ALI_NET_REMOVE_PREFIX_GUARD_

#define logn_info ali_logn_info
#define logn_warn ali_logn_warn
#define logn_error ali_logn_error

#ifdef ALI_TYPES_
#ifndef ALI_TYPES_ALIASES_
#define ALI_TYPES_ALIASES_
typedef ali_u8 u8;
typedef ali_u16 u16;
typedef ali_u32 u32;
typedef ali_u64 u64;

typedef ali_i8 i8;
typedef ali_i16 i16;
typedef ali_i32 i32;
typedef ali_i64 i64;

typedef ali_f32 f32;

#ifdef __x86_64__
typedef ali_f64 f64;
#endif // __x86_64__

typedef ali_usize usize;
typedef ali_isize isize;
#endif // ALI_TYPES_ALIASES_
#endif // ALI_TYPES_

#define tcp_server_socket ali_tcp_server_socket
#define tcp_server_accept ali_tcp_server_accept

#define tcp_client_connect ali_tcp_client_connect

#endif // ALI_NET_REMOVE_PREFIX_GUARD_
#endif // ALI_NET_REMOVE_PREFIX
