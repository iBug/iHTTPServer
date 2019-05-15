// File: reqhandler.h
// Author: iBug

#ifndef _REQHANDLER_H
#define _REQHANDLER_H

#include <stddef.h>

void parse_request(const char* request, ssize_t request_len, char* path, ssize_t* path_len);
void handle_client(int client_fd);

#endif
