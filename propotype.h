#ifndef PROPOTYPE_H_
#define PROPOTYPE_H_

#ifndef PROPODEF
#define PROPODEF
#endif // PROPODEF

#endif // PROPOTYPE_H_

#ifdef PROPOTYPE_IMPLEMENTATION

#ifdef _WIN32
#    ifndef WIN32_LEAN_AND_MEAN
#        define WIN32_LEAN_AND_MEAN
#    endif // WIN32_LEAN_AND_MEAN
#    include <errhandlingapi.h>
#    include <inttypes.h>
#    include <stdint.h>
#    include <winsock2.h>
#    include <ws2tcpip.h>
#    ifdef _MSC_VER
#        pragma comment(lib, "Ws2_32.lib")
#    endif // _MSC_VER
#else
#    include <arpa/inet.h>
#    include <errno.h>
#    include <netinet/in.h>
#    include <string.h>
#    include <sys/socket.h>
#    include <unistd.h>
#endif

#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#ifndef PROPO_ASSERT
#    include <assert.h>
#    define PROPO_ASSERT assert
#endif // PROPO_ASSERT

PROPODEF void propo_panic(const char *msg)
{
     fprintf(stderr, msg);
     abort();
}

PROPODEF void propo_panicf(const char *fmt, ...)
{
     va_list args;
     va_start(args, fmt);
     vfprintf(stderr, fmt, args);
     va_end(args);
     fprintf(stderr, "\n");
     abort();
}

#ifndef PROPO_BUFFER_SIZE
#define PROPO_BUFFER_SIZE 16
#endif // PROPO_BUFFER_SIZE

typedef struct {
     unsigned char data[PROPO_BUFFER_SIZE];
     size_t count;
} Buffer;

static void buf_append_byte(Buffer *buffer, unsigned char byte)
{
     PROPO_ASSERT(buffer->count < PROPO_BUFFER_SIZE && "buf_append_byte: unable to append a full buffer");
     buffer->data[buffer->count++] = byte;
}

static Buffer buf_from_cstr(const char *cstr)
{
     Buffer buffer = {0};
     for (const unsigned char *ucstr = (const unsigned char *)cstr; *ucstr != '\0'; ucstr++) {
          buf_append_byte(&buffer, *ucstr);
     }

     return buffer;
}

#ifdef _WIN32
#    define PROPOTYPE_INIT() propotype__windows_init(__func__)
#    define PROPOTYPE_CLEANUP() propotype__windows_cleanup(__func__)
#else
#    define PROPOTYPE_INIT() do {} while(0)
#    define PROPOTYPE_CLEANUP() do {} while(0)
#endif

#ifdef _WIN32
static void propotype__windows_init(const char *func)
{
     WSADATA data;
     int result = WSAStartup(MAKEWORD(2,2), &data);
     if (result != 0) {
          propo_panicf("%s (%d): unable to initialize", func, result);
     }
}

static void propotype__windows_cleanup(const char *func)
{
     int result = WSACleanup();
     if (result == SOCKET_ERROR) {
          propo_panicf("%s (%d): unable to cleanup", func, result);
     }
}
#endif

PROPODEF void propotype_init(void)
{
     PROPOTYPE_INIT();
}

PROPODEF void propotype_cleanup(void)
{
     PROPOTYPE_CLEANUP();
}

#ifdef _WIN32
#    define SOCKET_INVALID INVALID_SOCKET
#    define SOCKET_GET_ERROR() WSAGetLastError()
#    define SOCKET_PRINT_ERROR(err) socket__windows_print_error(__func__, err)

#    define SOCKET_CREATE() (socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP))
#    define SOCKET_CLOSE(socket) (closesocket(socket))

     typedef SOCKET Socket;
#else
#    define SOCKET_INVALID (-1)
#    define SOCKET_ERROR (-1)
#    define SOCKET_GET_ERROR() errno
#    define SOCKET_PRINT_ERROR(err) fprintf(stderr, "%s (%d): ", __func__, err); fprintf(stderr, strerror(err)); fprintf(stderr, "\n")

#    define SOCKET_CREATE() (socket(AF_INET, SOCK_DGRAM, 0))
#    define SOCKET_CLOSE(socket) (close(socket))

     typedef int Socket;
#endif

#ifdef _WIN32
static void socket__windows_print_error(const char *func, int err)
{
     char *msg = NULL;
     DWORD count = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER |
                                  FORMAT_MESSAGE_FROM_SYSTEM |
                                  FORMAT_MESSAGE_IGNORE_INSERTS,
                                  NULL,
                                  err,
                                  0,
                                  (LPSTR)&msg,
                                  0,
                                  NULL);
     if (count == 0) {
          DWORD format_error = GetLastError();
          fprintf(stderr, "socket__windows_print_error (%"PRIu32"): Unable to print error code\n", (uint32_t)format_error);
          LocalFree(msg);
          return;
     }

     fprintf(stderr, "%s (%d): %s\n", func, err, msg);
     LocalFree(msg);
}
#endif

static Socket socket_create(void)
{
     Socket s = SOCKET_CREATE();
     if (s == SOCKET_INVALID) {
          int err = SOCKET_GET_ERROR();
          SOCKET_PRINT_ERROR(err);
          exit(1);
     }
     return s;
}

static void socket_close(Socket s)
{
     int result = SOCKET_CLOSE(s);
     if (result == SOCKET_ERROR) {
          int err = SOCKET_GET_ERROR();
          SOCKET_PRINT_ERROR(err);
          exit(1);
     }
}

static int socket_send(Socket s, const Buffer *buffer, const struct sockaddr_in *address)
{
#    ifdef _WIN32
     return sendto(s, (const char *)buffer->data, (int)buffer->count, 0, (struct sockaddr *)address, sizeof(struct sockaddr));
#    else
     return (int)sendto(s, (const void *)buffer->data, buffer->count, 0, (struct sockaddr *)address, sizeof(struct sockaddr));
#    endif
}

#define PROPO_DEFAULT_HOST "192.168.0.1"
#define PROPO_DEFAULT_PORT 9876

typedef struct {
     Socket socket;
     struct sockaddr_in address;
} Client;

PROPODEF Client client_create(const char *host, int port)
{
     Client client = {0};
     client.socket = socket_create();
     client.address.sin_family = AF_INET;
     client.address.sin_port = htons((uint16_t)port);
     int result = inet_pton(AF_INET, host, &client.address.sin_addr);
     PROPO_ASSERT(result == 1 && "client_create: invalid host");

     return client;
}

PROPODEF void client_close(const Client *client)
{
     socket_close(client->socket);
}

PROPODEF int client_send(const Client *client, const Buffer *buffer)
{
     return socket_send(client->socket, buffer, &client->address);
}

PROPODEF int client_send_with_cstr(const Client *client, const char *cstr)
{
     Buffer buffer = buf_from_cstr(cstr);
     return socket_send(client->socket, &buffer, &client->address);
}

#endif // PROPOTYPE_IMPLEMENTATION
