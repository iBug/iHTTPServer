#include <math.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include "config.h"
#include "signals.h"
#include "status_code.h"

void parse_request(char* request, ssize_t req_len, char* path, ssize_t* path_len)
{
    char* req = request;

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

void handle_clnt(int clnt_sock)
{
    // 一个粗糙的读取方法，可能有 BUG！
    // 读取客户端发送来的数据，并解析
    char* req_buf = (char*) malloc(MAX_RECV_LEN * sizeof(char));
    // 将 clnt_sock 作为一个文件描述符，读取最多 MAX_RECV_LEN 个字符
    // 但一次读取并不保证已经将整个请求读取完整
    ssize_t req_len = read(clnt_sock, req_buf, MAX_RECV_LEN);

    // 根据 HTTP 请求的内容，解析资源路径和 Host 头
    char* path = (char*) malloc(MAX_PATH_LEN * sizeof(char));
    ssize_t path_len;
    parse_request(req_buf, req_len, path, &path_len);

    // 构造要返回的数据
    // 这里没有去读取文件内容，而是以返回请求资源路径作为示例，并且永远返回 200
    // 注意，响应头部后需要有一个多余换行（\r\n\r\n），然后才是响应内容
    char* response = (char*) malloc(MAX_SEND_LEN * sizeof(char)) ;
    sprintf(response,
        "HTTP/1.0 %s\r\nContent-Length: %zd\r\n\r\n%s",
        STATUS_200, path_len, path);
    size_t response_len = strlen(response);

    // 通过 clnt_sock 向客户端发送信息
    // 将 clnt_sock 作为文件描述符写内容
    write(clnt_sock, response, response_len);

    // 关闭客户端套接字
    close(clnt_sock);

    // 释放内存
    free(req_buf);
    free(path);
    free(response);
}

int main(){
    register_signal_handlers();

    fprintf(stderr, "Starting server...\n");
    int serv_sock = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    struct sockaddr_in serv_addr = {
        .sin_family = AF_INET,
        .sin_addr.s_addr = inet_addr(BIND_IP_ADDR),
        .sin_port = htons(BIND_PORT)
    };
    bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr));

    listen(serv_sock, MAX_CONN);

    // 接收客户端请求，获得一个可以与客户端通信的新的生成的套接字 clnt_sock
    struct sockaddr_in clnt_addr;
    socklen_t clnt_addr_size = sizeof(clnt_addr);

    while (1) // 一直循环
    {
        // 当没有客户端连接时， accept() 会阻塞程序执行，直到有客户端连接进来
        int clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_size);
        // 处理客户端的请求
        handle_clnt(clnt_sock);
    }

    // 实际上这里的代码不可到达，可以在 while 循环中收到 SIGINT 信号时主动 break
    // 关闭套接字
    close(serv_sock);
    return 0;
}
