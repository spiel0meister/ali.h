#define ALI_NET_IMPLEMENTATION
#define ALI_NET_REMOVE_PREFIX
#include "ali_net.h"

int main(void) {
    i32 sockfd = tcp_server_socket("127.0.0.1", 6969, 1);
    if (sockfd < 0) return 1;

    logn_info("Listening on 127.0.0.1:6969");

    char* client_host;
    u16 client_port;
    i32 clientfd = tcp_server_accept(sockfd, &client_host, &client_port);
    if (clientfd < 0) return 1;

    logn_info("Connected on %s:%d", client_host, client_port);

    char buffer[1024];
    do {
        isize n = recv(clientfd, buffer, sizeof(buffer), 0);
        if (n == 0) break;
        if (n < 0) {
            logn_error("Couldn't read from client: %s", strerror(errno));
            break;
        }

        logn_info("Recieved %ld bytes", n);
        buffer[n] = 0;
        logn_info("%s", buffer);
    } while (true);

    close(clientfd);
    close(sockfd);
    return 0;
}
