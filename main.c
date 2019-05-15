// File: main.c
// Author: iBug

#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "config.h"
#include "globals.h"
#include "util.h"
#include "signals.h"
#include "status_code.h"
#include "reqhandler.h"

int main(){
    register_signal_handlers();
    realpath(DOCUMENT_ROOT, document_root);
    document_root_len = strlen(document_root);

    server_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in server_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = inet_addr(BIND_IP_ADDR),
        .sin_port = htons(BIND_PORT)
    };
    bind(server_sock, (struct sockaddr*)&server_addr, sizeof(server_addr));
    listen(server_sock, MAX_CONN);
    eprintf("Listening on %s port %d...\n", BIND_IP_ADDR, BIND_PORT);

    struct sockaddr_in client_addr;
    socklen_t client_addr_size = sizeof(client_addr);

    while (1)
    {
        int client_sock = accept(server_sock, (struct sockaddr*)&client_addr, &client_addr_size);
        pid_t pid = fork(); // Fork a child
        if (pid == 0) {
            handle_client(client_sock);
            return 0;
        }
        close(client_sock);
    }
}
