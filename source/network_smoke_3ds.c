#include <3ds.h>

#include <arpa/inet.h>
#include <errno.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>

#define SOC_BUFFER_SIZE (1024u * 1024u)
#define RESPONSE_CAPACITY 2048

static u32 *soc_buffer;

static void wait_for_exit(void)
{
    printf("\nPress START to exit.\n");
    while (aptMainLoop()) {
        hidScanInput();
        if ((hidKeysDown() & KEY_START) != 0) {
            break;
        }
        gfxFlushBuffers();
        gfxSwapBuffers();
        gspWaitForVBlank();
    }
}

static int run_http_probe(void)
{
    static const char request[] =
        "GET / HTTP/1.1\r\n"
        "Host: example.com\r\n"
        "User-Agent: Links-3DS-Network-Smoke/1.0\r\n"
        "Connection: close\r\n\r\n";
    struct addrinfo hints;
    struct addrinfo *result = NULL;
    struct addrinfo *entry;
    char response[RESPONSE_CAPACITY + 1];
    ssize_t received;
    size_t total = 0;
    int socket_fd = -1;
    int status;

    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;

    printf("Resolving example.com...\n");
    status = getaddrinfo("example.com", "80", &hints, &result);
    if (status != 0) {
        printf("DNS failed: %s\n", gai_strerror(status));
        return 1;
    }

    for (entry = result; entry != NULL; entry = entry->ai_next) {
        socket_fd = socket(entry->ai_family, entry->ai_socktype,
                           entry->ai_protocol);
        if (socket_fd < 0) {
            continue;
        }
        if (connect(socket_fd, entry->ai_addr, entry->ai_addrlen) == 0) {
            break;
        }
        close(socket_fd);
        socket_fd = -1;
    }
    freeaddrinfo(result);

    if (socket_fd < 0) {
        printf("TCP connect failed: errno=%d (%s)\n", errno, strerror(errno));
        return 1;
    }

    printf("Connected. Sending HTTP request...\n");
    if (send(socket_fd, request, sizeof(request) - 1, 0)
        != (ssize_t)(sizeof(request) - 1)) {
        printf("send failed: errno=%d (%s)\n", errno, strerror(errno));
        close(socket_fd);
        return 1;
    }

    while (total < RESPONSE_CAPACITY) {
        received = recv(socket_fd, response + total,
                        RESPONSE_CAPACITY - total, 0);
        if (received == 0) {
            break;
        }
        if (received < 0) {
            printf("recv failed: errno=%d (%s)\n", errno, strerror(errno));
            close(socket_fd);
            return 1;
        }
        total += (size_t)received;
    }
    close(socket_fd);
    response[total] = '\0';

    printf("Received %lu bytes.\n", (unsigned long)total);
    if (total == 0) {
        printf("No HTTP response data received.\n");
        return 1;
    }

    {
        char *line_end = strstr(response, "\r\n");
        if (line_end != NULL) {
            *line_end = '\0';
        }
        printf("Response: %s\n", response);
    }
    return 0;
}

int main(void)
{
    Result result;
    int probe_status;

    gfxInitDefault();
    consoleInit(GFX_TOP, NULL);
    consoleInit(GFX_BOTTOM, NULL);
    consoleSelect(NULL);

    printf("Links 3DS Network Smoke Test\n\n");
    soc_buffer = (u32 *)linearAlloc(SOC_BUFFER_SIZE);
    if (soc_buffer == NULL) {
        printf("linearAlloc failed.\n");
        wait_for_exit();
        gfxExit();
        return 1;
    }

    result = socInit(soc_buffer, SOC_BUFFER_SIZE);
    if (R_FAILED(result)) {
        printf("socInit failed: 0x%08lx\n", (unsigned long)result);
        linearFree(soc_buffer);
        wait_for_exit();
        gfxExit();
        return 1;
    }

    probe_status = run_http_probe();
    printf("\nProbe result: %s\n", probe_status == 0 ? "PASS" : "FAIL");

    socExit();
    linearFree(soc_buffer);
    soc_buffer = NULL;
    wait_for_exit();
    gfxExit();
    return probe_status;
}
