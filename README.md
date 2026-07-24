# propotyperc.h
A simple, unofficial C library for controlling [Propotype RC](https://propotyperc.com/) cars

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

## API Reference
For API reference, go [here](docs/api_reference.md).
