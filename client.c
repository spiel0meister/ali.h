#define ALI_NET_IMPLEMENTATION
#define ALI_NET_REMOVE_PREFIX
#include "ali_net.h"

int main(void) {
    i32 serverfd = ali_tcp_client_connect("127.0.0.1", 6969);
    if (serverfd < 0) return 1;

    char buffer[] = "This is a test!";
    isize n = write(serverfd, buffer, sizeof(buffer));
    if (n < 0) {
        logn_error("Couldn't write to server: %s", strerror(errno));
        return 1;
    }

    close(serverfd);
    return 0;
}
