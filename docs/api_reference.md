# API Reference
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
### Initialization
```c
void propotyperc_init(void);
```
`propotyperc_init()` initializes the networking environment, this function must be called first before using any other function,
otherwise those functions might not work as expected.
> This function is **required** for Windows, other operating systems are optional.
> But it's a good practice to call this anyway.

### Cleanup
```c
void propotyperc_cleanup(void);
```
After you're completely done with this library, you can call `propotyperc_cleanup()` to terminate the networking environment
> Similar to `propotyperc_init()`, this function is required for Windows, other operating systems are optional.

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

This function returns amount of bytes sent, otherwise -1 if failed.

#### client_send_with_cstr()
Similar to `client_send()`, the difference is that this function converts a string into a `Buffer` then send to the destination host and port.<br>
This function expect two parameters:
* `client` - A pointer to a `Client` struct.
* `cstr` - A string to be sent.

> I usually call a string `cstr`, which is stands for C string.

This function returns amount of bytes sent, otherwise -1 if failed.

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
     bool steering_reverse;
     int steering_limits[3];
     size_t steering_limits_idx;
     // Throttle
     bool throttle_reverse;
     int throttle_limits[3];
     size_t throttle_limits_idx;
} Car_Config;
```

The `Car_Config` struct represents the RC Car configuration, let's go through each struct member one by one.

### steering_reverse
```c
bool steering_reverse;
```
`steering_reverse` reverse the steering if it is set to `true`, otherwise the steering will work normally.
> Like, when you go left, the RC car go right and vice versa.

### steering_limits
```c
int steering_limits[3];
```
`steering_limits` is an array where you can fill it with steering limit, eg. (ramp, medium, sharp) as `{200, 300, 500}`.
> You can only fill `steering_limits` with only 3 elements, the Propotype RC car itself mostly already have it's own steering limit around or above `500`.

### steering_limits_idx
```c
size_t steering_limits_idx;
```
`steering_limits_idx` contains the current active steering limit, you can only set it in range `0` to `2`, otherwise you might get a weird behavior.

### throttle_reverse
```c
bool throttle_reverse;
```
`throttle_reverse` reverse the move direction if it is set to `true`, otherwise the move direction will work normally.
> when you want the RC car go forward, the RC car goes backwards.

### throttle_limits
```c
int throttle_limits[3];
```
`throttle_limits` is an array where you can fill it with throttle/speed limit, eg. (slow, medium, fast) as `{150, 300, 500}`.
> You can only fill `throttle_limits` with only 3 elements, the Propotype RC car itself mostly have it's own throttle limit around or above `500`

### throttle_limits_idx
```c
size_t throttle_limits_idx;
```
`throttle_limits_idx` contains the current active throttle/speed limit, you can only set it in range `0` to `2`, otherwise you might get a weird behavior.

## Car
The `Car` struct contains `Client` and `Car_Config` so we can throw instructions and configuration into our Propotype RC car.
### Create a Car
```c
Car car_create(Client *client, const Car_Config config);
```
To create a `Car`, you can call `car_create()` to create one.<br>
This function expect two parameters:
* `client` - A pointer to a `Client` struct
* `config` - The RC car configuration (`Car_Config` struct)
> You can throw `PROPO_DEFAULT_CAR_CONFIG` macro to `config` parameter to use the default configuration.

### Close a Car
```c
void car_close(const Car *car);
```
This function is basicly a wrapper of [`client_close()`](#close-a-client), once you're done with the `Car`,
you don't need to close the client you pass into the `Car`.

### Car Functions
These are the `Car` functions you can use:
```c
// This function behavior is currently unknown
int car_L(const Car *car);
int car_R(const Car *car);
int car_epa(const Car *car);
// Car Commands
int car_lightoff(const Car *car);
int car_cabinlight(const Car *car);
int car_headlight(const Car *car);
int car_engine_off(const Car *car);
int car_engine_on(const Car *car);
int car_horn(const Car *car);
// Car Control
int car_trim(const Car *car, int value);
int car_steer(const Car *car, int value);
int car_throttle(const Car *car, int value);
```
> Note that all `Car` functions return the amount of bytes sent.<br>
> Also, if you know what L or R does, maybe tell me in [Issues](../../../issues).

### Car Commands
#### car_lightoff()
```c
int car_lightoff(const Car *car);
```
The `car_lightoff()` function turns the car's light off (usually cabin/headlight).

#### car_cabinlight()
```c
int car_cabinlight(const Car *car);
```
The `car_cabinlight()` function turns the car's cabin light on.

#### car_headlight()
```c
int car_headlight(const Car *car);
```
The `car_headlight()` function turns the car's headlight on.

#### car_engine_off()
```c
int car_engine_off(const Car *car);
```
The `car_engine_off()` function turns the car's engine off.

#### car_engine_on()
```c
int car_engine_on(const Car *car);
```
The `car_engine_on()` function turns the car's engine on.

#### car_horn()
```c
int car_horn(const Car *car);
```
The `car_horn()` function does beep beep to the car.

### Car Control
#### car_trim()
```c
int car_trim(const Car *car, int value);
```
The `car_trim()` function does trim the car by given `value`.<br>
If `value` is negative, the car will trim to left, otherwise the car trim to right.

#### car_steer()
```c
int car_steer(const Car *car, int value);
```
The `car_steer()` function steers the car to left or right by given value.<br>
If `value` is negative, the car steering goes left, otherwise the car steering goes right.

#### car_throttle()
```c
int car_throttle(const Car *car, int value);
```
The `car_throttle()` function moves the car forward or backward by given value.<br>
If `value` is negative, the car moves backward, otherwise the car moves forward.
