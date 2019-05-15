// File: reqhandler.c
// Author: iBug

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "config.h"
#include "globals.h"
#include "signals.h"
#include "status_code.h"

void parse_request(char* req, ssize_t req_len, char* path, ssize_t* path_len)
{
    // 一个粗糙的解析方法，可能有 BUG！
    // 获取第一个空格(s1)和第二个空格(s2)之间的内容，为 PATH
    ssize_t s1 = 0;
    while(s1 < req_len && req[s1] != ' ') s1++;
    ssize_t s2 = s1 + 1;
    while(s2 < req_len && req[s2] != ' ') s2++;

    memcpy(path, req + s1 + 1, (s2 - s1 - 1) * sizeof(char));
    path[s2 - s1 - 1] = 0;
    *path_len = (s2 - s1 - 1);
}

void handle_client(int clnt_sock)
{
    char* req_buf = (char*) malloc(MAX_RECV_LEN * sizeof(char));
    ssize_t req_len = read(clnt_sock, req_buf, MAX_RECV_LEN);

    char* path = (char*) malloc(MAX_PATH_LEN * sizeof(char));
    ssize_t path_len;
    parse_request(req_buf, req_len, path, &path_len);

    char* response = (char*) malloc(MAX_SEND_LEN * sizeof(char)) ;
    sprintf(response,
        "HTTP/1.0 %s\r\nContent-Length: %zd\r\n\r\n%s",
        STATUS_200, path_len, path);
    size_t response_len = strlen(response);

    write(clnt_sock, response, response_len);
    close(clnt_sock);

    free(req_buf);
    free(path);
    free(response);
}
