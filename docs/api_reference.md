# API Reference
Hellow! Have you ever wonder how can you control a Propotype RC car outside it's app or physical remote?
or maybe control the Propotype RC car using C?
Then this library is (probably) what you're looking for!

## What's this?
This is a simple, single-header library i write because of this simple question, "Can i control a Propotype RC car from my laptop?".<br>
What's **Propotype RC**? [Propotype RC](https://propotyperc.com) is an Indonesian brand that specialize in micro and custom RC cars.
> This library might only work for Propotype RC cars only.

## Quick Example
This is a very simple example to move the RC car forward (make sure to connect to the RC car's Wi-Fi first):
```c
#define PROPOTYPERC_IMPLEMENTATION
#include "propotyperc.h"

int main(void)
{
     // Initialize the library
     propotyperc_init();

     // Initialize the RC car
     Client client = client_create(PROPO_DEFAULT_HOST, PROPO_DEFAULT_PORT);
     Car car = car_create(&client, PROPO_DEFAULT_CAR_CONFIG);

     // Move the car forward (throttle the car)
     car_throttle(&car, 300);

     // Cleanup
     car_close(&car);
     propotyperc_cleanup();
     return 0;
}
```
> You might didn't want the RC car move forever,
> change `car_throttle(&car, 300)` to `car_throttle(&car, 0)` to stop the RC car for moving forward forever, or simply turn off the RC car.

## Include the Library
Let's start from how do we include the library. To include it, copy [propotyperc.h](https://github.com/noadhum/propotyperc.h/blob/master/propotyperc.h) into your project directory and include it like this:
```c
#define PROPOTYPERC_IMPLEMENTATION
#include "propotyperc.h"
```
> If you're wondering why do we even need to define `PROPOTYPERC_IMPLEMENTATION`,
> it is needed if you want to get access of `propotyperc.h`'s function, structs, and more,
> otherwise we cannot access those functions and structs.

## Core Functions
In the [example](#quick-example) above, we can see that `main()` function starts with `propotyperc_init()` and ends with `propotyperc_cleanup()`, but what does it even do?

### Initialization
```c
void propotyperc_init(void);
```
`propotyperc_init()` initializes the networking environment, this function must be called first before using any other function,
otherwise those functions might not work as expected.
> This function is **required** for Windows, other operating systems is optional.
> But it's a good practice to call this anyway.

### Cleanup
```c
void propotyperc_cleanup(void);
```
After you're completely done with this library, you can call `propotyperc_cleanup()` to terminate the networking environment
> Similar to `propotyperc_init()`, this function is required for Windows, other operating systems is optional.

## Client
The `Client` struct contains destination host and port, so we can throw instructions to the RC car.
> Don't worry if you didn't understand these `Client` functions, you mostly only call `client_create()` all of the time.<br>
> If you want to skip this section, go to [Car_Config](#car_config).

### Create a Client
```c
Client client_create(const char *host, int port);
```
To create a new `Client`, you can call `client_create()` function to create one.<br>
This function expect two parameters:
* `host` - IPv4 host representation in string, e.g. `"127.0.0.1"`.
* `port` - Port representation in number, e.g. `8080`.
> If you put an invalid host to `host` parameter, it will throw an assertion.

### Close a Client
```c
void client_close(const Client *client);
```
Once you're completely done with a `Client`, you might want to free/close it by calling `client_close()` function.
> After calling `client_close()`, you might cannot reuse that `Client` anymore, unless you create or assign a new one.

### Send using Client
Now, how do we even send a packet using our `Client`?<br>
There are two functions that `Client` have to send:
```c
int client_send(const Client *client, const Buffer *buffer);
int client_send_with_cstr(const Client *client, const char *cstr);
```
Uuh, what's the difference between `client_send()` and `client_send_with_cstr()`??

#### client_send()
```c
int client_send(const Client *client, const Buffer *buffer);
```
The `client_send()` function sends a sequence of bytes using a `Buffer` struct to the destination host and port
(the `Buffer` struct might will be explained later).<br>
This function expect two parameters:
* `client` - A pointer to a `Client` struct.
* `buffer` - A pointer to a `Buffer` struct to be sent (just think of it as a sequence of bytes for now).

This function returns amount of bytes sended, otherwise -1 if failed.

#### client_send_with_cstr()
Similar to `client_send()`, the difference is that this function converts a string into a `Buffer` then send to the destination host and port.<br>
This function expect two parameters":
* `client` - A pointer to a `Client` struct.
* `cstr` - A string to be sent.

> I usually call a string `cstr`, which is stands for C string.

This function returns amount if bytes sended, otherwise -1 if failed.

## Buffer
The `Buffer` struct is basically contains a sequence of bytes (or array) that you can throw into `client_send()` function.<br>
Since `Buffer` isn't a dynamic array, it is set to a static size (16 by default), to change its static size, you can define `PROPO_BUFFER_SIZE` macro and set the size to whatever you want, but most of the time, you will only use the default or 8 for buffer size.
> You probably want to define `PROPO_BUFFER_SIZE` macro before call any library function.

### Buffer Struct Members
The `Buffer` struct has only two members, `data` and `count`, where:
* `data` - An array, contains bytes (`unsigned char`).
* `count` - Amount of bytes in `data` member.

### Create a Buffer
To create an empty `Buffer`, you can simply write:
```c
Buffer buffer = {0}; // This creates an empty Buffer
```
or if you want a buffer to contain a string, you can write:
```c
Buffer buffer = buf_from_cstr("Hello"); // buffer.data contains "Hello"
```

### Append a Buffer
There are alot of ways to append a buffer, these are the functions you can use:
```c
static void buf_append_byte(Buffer *buffer, unsigned char byte);
static void buf_append_uint(Buffer *buffer, unsigned int number);
static void buf_append_cstr(Buffer *buffer, const char *cstr);
```

#### buf_append_byte()
```c
static void buf_append_byte(Buffer *buffer, unsigned char byte);
```
`buf_append_byte()` function appends a single byte into given buffer.<br>
This function expect two parameters:
* `buffer` - A pointer to a `Buffer` struct to get appended.
* `byte` - A single byte to append into given buffer.

A `byte` could be a `char`, hexadecimal literal, or integer literal:
```c
buf_append_byte(&buffer, 'A');
buf_append_byte(&buffer, 0x41);
buf_append_byte(&buffer, 65);
```

#### buf_append_uint()
```c
static void buf_append_uint(Buffer *buffer, unsigned int number);
```
`buf_append_uint()` function convert `number` into bytes, then append it into given buffer.<br>
This function expect two parameters:
* `buffer` - A pointer to a `Buffer` struct to get appended.
* `number` - An unsigned integer to get converted into bytes and append into given buffer.

The `buf_append_uint()` expects the `number` parameter to be in range of 0 to 9999, otherwise only last four digits get appended.
```c
buf_append_uint(&buffer, 12345); // This only append 2345
```

#### buf_append_cstr()
```c
static void buf_append_cstr(Buffer *buffer, const char *cstr);
```
`buf_append_cstr()` functions appends a C string into given buffer.<br>
This function expect two parameters:
* `buffer` - A pointer to a `Buffer` struct to get appended.
* `cstr` - A C string to get appended into given buffer.

```c
// Append "Hello World!" into buffer
buf_append_cstr(&buffer, "Hello ");
buf_append_cstr(&buffer, "World!");
```

## Car_Config
```c
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
```
The `Car_Config` struct contains the RC car configuration, let's go through the struct members one by one.
