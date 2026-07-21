# Getting Started
## Include the Library
To use the library, copy [propotyperc.h](https://github.com/noadhum/propotyperc.h/blob/master/propotyperc.h) into your project directory, and include it:
```c
#define PROPOTYPERC_IMPLEMENTATION
#include "propotyperc.h"
```
> Define the `PROPOTYPERC_IMPLEMENTATION` macro before including `propotyperc.h` is required to get access to `propotyperc.h`'s functions, structs, and more

## Quick Example
This is a simple, quick example of turning on the RC car's headlight:
```c
#define PROPOTYPERC_IMPLEMENTATION
#include "propotyperc.h"

int main(void)
{
     // Initialize the library
     propotyperc_init();

     // Initialize the car
     Client client = client_create(PROPO_DEFAULT_HOST, PROPO_DEFAULT_PORT);
     Car car = car_create(&client, PROPO_DEFAULT_CAR_CONFIG);

     // Turn on the car's headlight
     car_headlight(&car);

     // Cleanup
     car_close(&car);
     propotyperc_cleanup();
     return 0;
}
```

## Initialize and Cleanup
As we can see in example above, the `main()` function starts with `propotyperc_init()` and ends with `propotyperc_cleanup()`, but what does it even do?

### Initialize
```c
void propotyperc_init(void)
```
The `propotyperc_init()` function initializes the networking environment, you must call this first before using other functions, otherwise those functions might not work as expected.
> This function is required for Windows, for other operating systems is optional, but it's a best practice to call this function.

### Cleanup
```c
void propotyperc_cleanup(void)
```
The `propotyperc_cleanup()` function terminates the networking environment, call this function when you are completely done with the library.
> Similar to `propotyperc_init()`, this function is required for Windows, optional for other operating systems.

## Client
The `Client` struct sets the destination host and port so we could throw instructions to the car, to create a `Client` variable, assign it with `client_create()` function
```c
Client client_create(const char *host, int port)
```
where:
* `host` - Destination IPv4 host representation in cstr (string), eg. `"127.0.0.1"`.
* `port` - Destination port representation in `int`, eg. `8080`.

> For Propotype RC cars, you probably always use `PROPO_DEDAULT_HOST` (`"192.168.0.1"`) for host, and `PROPO_DEFAULT_PORT` (`9876`) for port.

After finished using `Client`, you probably want to free it with `client_close()`:
```c
void client_close(const Client *client)
```
> Once you close a `Client`, other `Client` functions might not work, that means we probably cannot use a closed/freed `Client`, unless reassign the variable, etc.

### Closed Client Example
```c
#define PROPOTYPERC_IMPLEMENTATION
#include "propotyperc.h"

int main(void)
{
     propotyperc_init();

     // Initialize the client
     Client client = client_create(PROPO_DEFAULT_HOST, PROPO_DEFAULT_PORT);

     // Sends a message and recieved by the destination
     client_send_with_cstr(&client, "This message is recieved!");

     // Close the client
     client_close(&client);

     // This message is probably never be recieved by the destination
     client_send_with_cstr(&client, "This message is probably not recieved");

     propotyperc_cleanup();
     return 0;
}
```
> For now, let's say that `client_send_with_cstr()` function sends sequence of bytes to the destination host and port.
