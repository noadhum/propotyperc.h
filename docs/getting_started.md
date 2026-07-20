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
     // Initialize
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
The `propotyperc_init()` function initialize the networking environment, you must call this first before using other functions, otherwise those functions might not work as expected.
> This function is required for Windows, for other operating systems is optional, but it's a best practice to call this function.

### Cleanup
```c
void propotyperc_cleanup(void)
```
The `propotyperc_cleanup()` function terminates the networking environment, call this function when you are completely done with the library.
> Similar to `propotyperc_init()`, this function is required for Windows, optional for other operating systems.
