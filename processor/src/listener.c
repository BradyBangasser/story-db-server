#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/un.h>
#include <string.h>
#include <stdio.h>
#include <errno.h>
#include <unistd.h>
#include <poll.h>

#include "socket-handler.h"
#include "logging.h"

#define SOCKET_PATH "/tmp/sniffy.socket"

void slisten() {
    struct sockaddr_un addr;
    int sock, result;
    const int enable = 1;

    if (!access(SOCKET_PATH, F_OK)) {
        DEBUGF("Removing existing socket at %s\n", SOCKET_PATH);
        remove(SOCKET_PATH);
    }

    sock = socket(AF_LOCAL, SOCK_STREAM, 0);
    if (sock == -1) {
        ERRORF("Socket creation failed, errno %d\n", errno);
        return;
    }

    if (setsockopt(sock, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
        ERRORF("Error setting socket option, errno: %d\n", errno);
        return;
    }
    
    memset(&addr, 0, sizeof(addr));
    addr.sun_family = AF_LOCAL;
    strcpy(addr.sun_path, SOCKET_PATH);

    result = bind(sock, (struct sockaddr *) &addr, sizeof(addr));
    if (result) {
        ERRORF("Bind failed, errno: %d\n", errno);
        return;
    }

    result = listen(sock, 5);
    if (result) {
        ERRORF("Listen failed, errno: %d\n", errno);
        return;
    }

    while (1) {
        struct pollfd p;
        p.events = POLLIN;
        p.fd = accept(sock, NULL, NULL);
        if (p.fd == -1) {
            ERRORF("Failed to accept connection, errno: %d\n", errno);
            break;
        }

        INFOF("Accepted socket connection: %d\n", p.fd);

        while (poll(&p, 1, 300) < 1);

        socket_handler(&p.fd);
        // knock knock, race condition
    }
}
