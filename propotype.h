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
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>

#ifndef PROPO_ASSERT
#    include <assert.h>
#    define PROPO_ASSERT assert
#endif // PROPO_ASSERT

PROPODEF void propo_panic(const char *msg)
{
     fprintf(stderr, "%s\n", msg);
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

static int propo_max(int a, int b)
{
     return (a > b) ? a : b;
}

static int propo_min(int a, int b)
{
     return (a < b) ? a : b;
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

/*
 * This function only accepts number around 0-9999
 */
static void buf_append_int(Buffer *buffer, int number)
{
     int thousand = (number / 1000) % 10;
     int hundred = (number / 100) % 10;
     int ten = (number / 10) % 10;
     int one = (number / 1) % 10;

     buf_append_byte(buffer, thousand + '0');
     buf_append_byte(buffer, hundred + '0');
     buf_append_byte(buffer, ten + '0');
     buf_append_byte(buffer, one + '0');
}

static void buf_append_cstr(Buffer *buffer, const char *cstr)
{
     for (const unsigned char *ucstr = (const unsigned char *)cstr; *ucstr != '\0'; ucstr++) {
          buf_append_byte(buffer, *ucstr);
     }
}

static Buffer buf_from_cstr(const char *cstr)
{
     Buffer buffer = {0};
     buf_append_cstr(&buffer, cstr);

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
#    define SOCKET_PRINT_ERROR(err) fprintf(stderr, "%s (%d): ", __func__, err); fprintf(stderr, "%s\n", strerror(err));

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

typedef struct {
     // Steering
     bool steering_auto_return;
     bool steering_reverse;
     int steering_limits[3];
     size_t steering_limits_idx;
     // Throttle
     bool throttle_auto_return;
     bool throttle_reverse;
     int throttle_limits[3];
     size_t throttle_limits_idx;
} Car_Config;

#define PROPO_DEFAULT_CAR_CONFIG                        \
     ((Car_Config){                                     \
          .steering_auto_return = true,                 \
          .steering_reverse = false,                    \
          .steering_limits = {200, 300, 500},           \
          .steering_limits_idx = 1,                     \
                                                        \
          .throttle_auto_return = true,                 \
          .throttle_reverse = false,                    \
          .throttle_limits = {150, 300, 500},           \
          .throttle_limits_idx = 2                      \
     })

#ifndef PROPO_CAR_NEUTRAL_VALUE
#define PROPO_CAR_NEUTRAL_VALUE 1500
#endif // PROPO_CAR_NEUTRAL_VALUE

// Car commands (ch0)
const char PROPO_CAR_COMMAND_L[]             = "1250";
const char PROPO_CAR_COMMAND_R[]             = "1900";
const char PROPO_CAR_COMMAND_LIGHTOFF[]      = "1200";
const char PROPO_CAR_COMMAND_CABINLIGHT[]    = "1350";
const char PROPO_CAR_COMMAND_HEADLIGHT[]     = "1650";
const char PROPO_CAR_COMMAND_ENGINE_OFF[]    = "1800";
const char PROPO_CAR_COMMAND_ENGINE_ON[]     = "1950";
const char PROPO_CAR_COMMAND_HORN[]          = "1050";
const char PROPO_CAR_COMMAND_EPA[]           = "1450";

typedef struct {
     Client *client;
     Car_Config config;
} Car;

PROPODEF Car car_create(Client *client, const Car_Config config)
{
     Car car = {0};
     car.client = client;
     car.config = config;
     return car;
}

PROPODEF void car_close(const Car *car)
{
     client_close(car->client);
}

static int car_command(const Car *car, const char *command)
{
     Buffer buffer = buf_from_cstr("ch0 ");
     buf_append_cstr(&buffer, command);
     return client_send(car->client, &buffer);
}

PROPODEF int car_L(const Car *car)
{
     return car_command(car, PROPO_CAR_COMMAND_L);
}

PROPODEF int car_R(const Car *car)
{
     return car_command(car, PROPO_CAR_COMMAND_R);
}

PROPODEF int car_lightoff(const Car *car)
{
     return car_command(car, PROPO_CAR_COMMAND_LIGHTOFF);
}

PROPODEF int car_cabinlight(const Car *car)
{
     return car_command(car, PROPO_CAR_COMMAND_CABINLIGHT);
}

PROPODEF int car_headlight(const Car *car)
{
     return car_command(car, PROPO_CAR_COMMAND_HEADLIGHT);
}

PROPODEF int car_engine_off(const Car *car)
{
     return car_command(car, PROPO_CAR_COMMAND_ENGINE_OFF);
}

PROPODEF int car_engine_on(const Car *car)
{
     return car_command(car, PROPO_CAR_COMMAND_ENGINE_ON);
}

PROPODEF int car_horn(const Car *car)
{
     return car_command(car, PROPO_CAR_COMMAND_HORN);
}

PROPODEF int car_epa(const Car *car)
{
     return car_command(car, PROPO_CAR_COMMAND_EPA);
}

PROPODEF int car_trim(const Car *car, int value)
{
     int trim_value = propo_max(-500, propo_min(value, 500)) + PROPO_CAR_NEUTRAL_VALUE;

     Buffer buffer = buf_from_cstr("ch1 ");
     buf_append_int(&buffer, trim_value);
     return client_send(car->client, &buffer);
}

PROPODEF int car_steer(const Car *car, int value)
{
     int steering_value = propo_max(-car->config.steering_limits[car->config.steering_limits_idx], propo_min(value, car->config.steering_limits[car->config.steering_limits_idx]));
     if (car->config.steering_reverse) {
          steering_value = -steering_value;
     }
     steering_value += PROPO_CAR_NEUTRAL_VALUE;

     Buffer buffer = buf_from_cstr("ch2 ");
     buf_append_int(&buffer, steering_value);
     return client_send(car->client, &buffer);
}

PROPODEF int car_throttle(const Car *car, int value)
{
     int throttle_value = propo_max(-car->config.throttle_limits[car->config.throttle_limits_idx], propo_min(value, car->config.throttle_limits[car->config.throttle_limits_idx]));
     if (car->config.throttle_reverse) {
          throttle_value = -throttle_value;
     }
     throttle_value += PROPO_CAR_NEUTRAL_VALUE;

     Buffer buffer = buf_from_cstr("ch3 ");
     buf_append_int(&buffer, throttle_value);
     return client_send(car->client, &buffer);
}

#endif // PROPOTYPE_IMPLEMENTATION
