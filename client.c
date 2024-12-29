#define ALI_NET_IMPLEMENTATION
#define ALI_NET_REMOVE_PREFIX
#include "ali_net.h"

ali_i32 ali_tcp_client_connect(const char* host, ali_u16 port) {
    bool result = true;
    ali_i32 sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0) {
        log_error("Couldn't create socket: %s", strerror(errno));
        ALI_RETURN_DEFER(false);
    }

    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(port);
    inet_aton(host, &addr.sin_addr);

    if (connect(sockfd, (void*)&addr, sizeof(addr)) < 0) {
        log_error("Couldn't connect on %s:%d: %s", host, port, strerror(errno));
        ALI_RETURN_DEFER(false);
    }

defer:
    if (!result) {
        if (sockfd >= 0) close(sockfd);
        return -1;
    }
    return sockfd;
}

int main(void) {
    i32 serverfd = ali_tcp_client_connect("127.0.0.1", 6969);
    if (serverfd < 0) return 1;

    char buffer[] = "This is a test!";
    isize n = write(serverfd, buffer, sizeof(buffer));
    if (n < 0) {
        log_error("Couldn't write to server: %s", strerror(errno));
        return 1;
    }

    close(serverfd);
    return 0;
}
