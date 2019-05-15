// File: reqhandler.c
// Author: iBug

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>

#include "config.h"
#include "globals.h"
#include "util.h"
#include "signals.h"
#include "status_code.h"

void handle_client(int client_sock)
{
    char* req = (char*) malloc(MAX_RECV_LEN * sizeof(char));
    ssize_t req_len = read(client_sock, req, MAX_RECV_LEN);

    char* path = (char*) malloc(MAX_PATH_LEN * sizeof(char));
    char* fullpath = (char*) malloc(MAX_PATH_LEN * sizeof(char));

    char* response = (char*) malloc(MAX_SEND_LEN * sizeof(char)) ;

    if (strncmp(req, "GET ", (size_t)4U)) {
        sprintf(response, "HTTP/1.0 " STATUS_501 "\r\nContent-Length: 0\r\n\r\n");
        write(client_sock, response, strlen(response));
        close(client_sock);
        free(req);
        free(path);
        free(fullpath);
        free(response);
        return;
    }

    // Construct path
    *strchr(req + 4, ' ') = 0;
    strcpy(path, DOCUMENT_ROOT);
    strcat(path, req + 4);
    realpath(path, fullpath);
    size_t fullpath_len = strlen(fullpath);
    if (fullpath_len > document_root_len)
        fullpath_len = document_root_len;
    if (strncmp(fullpath, document_root, fullpath_len) != 0) {
        // Something sneaky
        sprintf(response, "HTTP/1.0 " STATUS_403 "\r\nContent-Length: 0\r\n\r\n");
        write(client_sock, response, strlen(response));
        close(client_sock);
        free(req);
        free(path);
        free(fullpath);
        free(response);
        return;
    }
    eprintf("OK\n");

    // stat(2) the file
    struct stat st;
    if (stat(path, &st) == -1) {
        if (errno == ENOENT) {
            sprintf(response, "HTTP/1.0 " STATUS_404 "\r\nContent-Length: 0\r\n\r\n");
        } else {
            sprintf(response, "HTTP/1.0 " STATUS_500 "\r\nContent-Length: 0\r\n\r\n");
        }
        write(client_sock, response, strlen(response));
    } else if ((st.st_mode & S_IFMT) != S_IFREG && (st.st_mode & S_IFMT) != S_IFLNK) {
        sprintf(response, "HTTP/1.0 " STATUS_403 "\r\nContent-Length: 0\r\n\r\n");
        write(client_sock, response, strlen(response));
    } else {
        sprintf(response,
                "HTTP/1.0 %s\r\n"
                "Content-Length: %zd\r\n"
                "\r\n",
                // "Content-Type: application/octet-stream\r\n\r\n",
                STATUS_200, st.st_size);
        write(client_sock, response, strlen(response));

        eprintf("Size: %zd bytes\n", st.st_size);
        off_t fs = st.st_size;
        int fd = open(path, O_RDONLY);
        while (fs >= MAX_SEND_LEN) {
            read(fd, response, MAX_SEND_LEN);
            write(client_sock, response, MAX_SEND_LEN);
            fs -= MAX_SEND_LEN;
        }
        if (fs > 0) {
            read(fd, response, fs);
            write(client_sock, response, fs);
        }
        close(fd);
    }
    close(client_sock);

    free(req);
    free(path);
    free(fullpath);
    free(response);
}
