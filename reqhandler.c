// File: reqhandler.c
// Author: iBug

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>

#include "config.h"
#include "globals.h"
#include "util.h"
#include "signals.h"
#include "status_code.h"

void handle_client(int client_sock)
{
    char* req = malloc(MAX_RECV_LEN * sizeof(char));
    ssize_t req_len = read(client_sock, req, MAX_RECV_LEN);

    char* path = malloc(MAX_PATH_LEN * sizeof(char));

    char* response = malloc(MAX_SEND_LEN * sizeof(char)) ;

    if (strncmp(req, "GET ", (size_t)4U)) {
        sprintf(response, "HTTP/1.0 " STATUS_501 "\r\nContent-Length: 0\r\n\r\n");
        write(client_sock, response, strlen(response));
        close(client_sock);
        free(req);
        free(path);
        free(response);
        return;
    }

    // Construct path
    *strchr(req + 4, ' ') = 0;
    strcpy(path, DOCUMENT_ROOT);
    strcat(path, req + 4);
    /*
    char* fullpath = realpath(path, NULL);
    size_t fullpath_len = strlen(fullpath);
    if (strncmp(fullpath, document_root, document_root_len) != 0) {
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
    free(fullpath);
    */

    // open(2) and stat(2) the file
    int fd = open(path, O_RDONLY);
    if (fd == -1) {
        if (errno == ENOENT) {
            sprintf(response, "HTTP/1.0 " STATUS_404 "\r\nContent-Length: 0\r\n\r\n");
        } else {
            sprintf(response, "HTTP/1.0 " STATUS_500 "\r\nContent-Length: 0\r\n\r\n");
        }
        write(client_sock, response, strlen(response));
        free(req);
        free(path);
        free(response);
        return;
    }
    struct stat st;
    if (fstat(fd, &st) == -1) {
        sprintf(response, "HTTP/1.0 " STATUS_500 "\r\nContent-Length: 0\r\n\r\n");
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

        const ssize_t size = st.st_size;
        void *mm;
        off_t off = 0L;
        while (size - off >= MAX_SEND_LEN) {
            mm = mmap(NULL, MAX_SEND_LEN, PROT_READ, MAP_PRIVATE, fd, off);
            write(client_sock, mm, MAX_SEND_LEN);
            munmap(mm, MAX_SEND_LEN);
            off += MAX_SEND_LEN;
        }
        if (size - off > 0) {
            int s = size - off;
            mm = mmap(NULL, s, PROT_READ, MAP_PRIVATE, fd, off);
            if (mm == (void*)-1) {
                // Something snarky!
                close(fd);
                close(client_sock);
                free(req);
                free(path);
                free(response);
                exit(1);
            }
            write(client_sock, mm, s);
            munmap(mm, s);
            off += s;
        }
    }
    close(fd);
    close(client_sock);

    free(req);
    free(path);
    free(response);
}
