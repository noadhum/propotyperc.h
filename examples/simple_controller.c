/*
 * Simple Controller in desktop for Propotype RC Car using raylib (https://github.com/raysan5/raylib)
 *
 * This example solve that my question ("Can i control a Propotype RC car from my laptop?") is possible.
 *
 * Keybinds:
 *     - F3 (Shift + F3 or other variations), debug mode
 *     - WASD/Arrow Keys, drive the RC Car
 *     - Q/E, trim left or right
 *     - Z/C, L or R
 *     - X, turn the light off
 *     - G, turn the cabin light on
 *     - F, turn the headlight on
 *     - I/O, turn the engine on and off
 *     - SPACE, horn
 *     - P, EPA
 *     - 1/2/3, set throttle speed limit
 *     - 4/5/6, set steering sharpness limit
 *     - R, reverse the steering control
 *     - T, reverse the throttle control
 *
 * Functions:
 *     - is_ready(), debounce-like function
 *     - car_update_values(), update the car's trimmer, steering, and throttle value
 *     - car_send_values(), send the car's trimmer, steering, and throttle value to the RC car
 *
 * Macros:
 *     - ready_to_send(), is just a wrapper for is_ready(0.055) (55ms)
 */

#include <raylib.h>
#include <stdbool.h>

#define PROPOTYPERC_IMPLEMENTATION
#include "../propotyperc.h"

static bool is_ready(double delay_sec)
{
     static double last = 0;
     double now = GetTime();
     if (now - last >= delay_sec) {
          last = now;
          return true;
     }
     return false;
}

#define ready_to_send() is_ready(0.055)

static void car_update_values(const Car *car, int *trimmer, int *steering, int *throttle)
{
     // Trimmer
     bool key_trim_left_pressed = IsKeyDown(KEY_Q);
     bool key_trim_right_pressed = IsKeyDown(KEY_E);

     if (!key_trim_left_pressed && key_trim_right_pressed) {
          if (*trimmer < 500) {
               *trimmer += 10;
          }
     }
     else if (key_trim_left_pressed && !key_trim_right_pressed) {
          if (*trimmer > -500) {
               *trimmer -= 10;
          }
     }

     // Steering
     bool key_left_pressed = IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT);
     bool key_right_pressed = IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT);

     if (!key_left_pressed && key_right_pressed) {
          *steering = car->config.steering_limits[car->config.steering_limits_idx];
     }
     else if (key_left_pressed && !key_right_pressed) {
          *steering = -car->config.steering_limits[car->config.steering_limits_idx];
     }
     else {
          *steering = 0;
     }

     // Throttle
     bool key_forward_pressed = IsKeyDown(KEY_W) || IsKeyDown(KEY_UP);
     bool key_backward_pressed = IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN);

     if (key_forward_pressed && !key_backward_pressed) {
          *throttle = car->config.throttle_limits[car->config.throttle_limits_idx];
     }
     else if (!key_forward_pressed && key_backward_pressed) {
          *throttle = -car->config.throttle_limits[car->config.throttle_limits_idx];
     }
     else {
          *throttle = 0;
     }

}

static void car_send_values(const Car *car, const char *command, int trimmer, int steering, int throttle)
{
     car_command(car, command);
     car_trim(car, trimmer);
     car_steer(car, steering);
     car_throttle(car, throttle);
}

int main(void)
{
     propotyperc_init();

     Client client = client_create(PROPO_DEFAULT_HOST, PROPO_DEFAULT_PORT);
     Car car = car_create(&client, PROPO_DEFAULT_CAR_CONFIG);

     const char *command_value = PROPO_CAR_NEUTRAL_CSTR;

     int trimmer_value = 0;
     int steering_value = 0;
     int throttle_value = 0;

     const int window_width = 1200;
     const int window_height = 700;

     InitWindow(window_width, window_height, "Propotype RC Desktop Controller");
     SetTargetFPS(60);

     bool debug_mode = false;

     while (!WindowShouldClose()) {
          // Debug
          if (IsKeyPressed(KEY_F3)) {
               debug_mode = !debug_mode;
          }

          // Car Config
          if (IsKeyDown(KEY_ONE))        car.config.throttle_limits_idx = 0;
          else if (IsKeyDown(KEY_TWO))   car.config.throttle_limits_idx = 1;
          else if (IsKeyDown(KEY_THREE)) car.config.throttle_limits_idx = 2;

          if (IsKeyDown(KEY_FOUR))       car.config.steering_limits_idx = 0;
          else if (IsKeyDown(KEY_FIVE))  car.config.steering_limits_idx = 1;
          else if (IsKeyDown(KEY_SIX))   car.config.steering_limits_idx = 2;

          if (IsKeyPressed(KEY_R))       car.config.steering_reverse = !car.config.steering_reverse;
          else if (IsKeyPressed(KEY_T))  car.config.throttle_reverse = !car.config.throttle_reverse;

          // Commands
          if (IsKeyDown(KEY_Z))          command_value = PROPO_CAR_COMMAND_L;
          else if (IsKeyDown(KEY_C))     command_value = PROPO_CAR_COMMAND_R;
          else if (IsKeyDown(KEY_X))     command_value = PROPO_CAR_COMMAND_LIGHTOFF;
          else if (IsKeyDown(KEY_G))     command_value = PROPO_CAR_COMMAND_CABINLIGHT;
          else if (IsKeyDown(KEY_F))     command_value = PROPO_CAR_COMMAND_HEADLIGHT;
          else if (IsKeyDown(KEY_O))     command_value = PROPO_CAR_COMMAND_ENGINE_OFF;
          else if (IsKeyDown(KEY_I))     command_value = PROPO_CAR_COMMAND_ENGINE_ON;
          else if (IsKeyDown(KEY_SPACE)) command_value = PROPO_CAR_COMMAND_HORN;
          else if (IsKeyDown(KEY_P))     command_value = PROPO_CAR_COMMAND_EPA;
          else                           command_value = PROPO_CAR_NEUTRAL_CSTR;

          car_update_values(&car, &trimmer_value, &steering_value, &throttle_value);

          if (ready_to_send()) {
               car_send_values(&car, command_value, trimmer_value, steering_value, throttle_value);
          }

          BeginDrawing();
          ClearBackground(BLACK);

          if (debug_mode) {
               SetWindowTitle("Propotype RC Desktop Controller (DEBUG)");
               // RC Car Values
               DrawText("RC Car Values:", window_width * 0.02, window_height * 0.02, 24, RAYWHITE);
               DrawText(TextFormat("- Command: %s", command_value), window_width * 0.02, window_height * 0.06, 16, RAYWHITE);
               DrawText(TextFormat("- Trimmer: %d", trimmer_value), window_width * 0.02, window_height * 0.09, 16, RAYWHITE);
               DrawText(TextFormat("- Steering: %d", steering_value), window_width * 0.02, window_height * 0.12, 16, RAYWHITE);
               DrawText(TextFormat("- Throttle: %d", throttle_value), window_width * 0.02, window_height * 0.15, 16, RAYWHITE);

               // RC Car Config
               DrawText("RC Car Config:", window_width * 0.02, window_height * 0.2, 24, RAYWHITE);
               DrawText(TextFormat("- Steering Reverse: %s", (car.config.steering_reverse) ? "true" : "false"), window_width * 0.02, window_height * 0.24, 16, RAYWHITE);
               DrawText(TextFormat("- Steering Limit: %d (index %d)", car.config.steering_limits[car.config.steering_limits_idx], car.config.steering_limits_idx), window_width * 0.02, window_height * 0.27, 16, RAYWHITE);
               DrawText(TextFormat("- Throttle Reverse: %s", (car.config.throttle_reverse) ? "true" : "false"), window_width * 0.02, window_height * 0.30, 16, RAYWHITE);
               DrawText(TextFormat("- Throttle Limit: %d (index %d)", car.config.throttle_limits[car.config.throttle_limits_idx], car.config.throttle_limits_idx), window_width * 0.02, window_height * 0.33, 16, RAYWHITE);
          } else {
               SetWindowTitle("Propotype RC Desktop Controller");
               // RC Car Control
               DrawText("RC Car Control:", window_width * 0.02, window_height * 0.02, 24, RAYWHITE);
               DrawText("- WASD/Arrow Keys to drive the RC car", window_width * 0.02, window_height * 0.06, 16, RAYWHITE);
               DrawText("- Q/E to trim left and right", window_width * 0.02, window_height * 0.09, 16, RAYWHITE);
               DrawText("- Z/C to L or R", window_width * 0.02, window_height * 0.12, 16, RAYWHITE);
               DrawText("- X to turn the light off", window_width * 0.02, window_height * 0.15, 16, RAYWHITE);
               DrawText("- G to turn the cabin light on", window_width * 0.02, window_height * 0.18, 16, RAYWHITE);
               DrawText("- F to turn the headlight on", window_width * 0.02, window_height * 0.21, 16, RAYWHITE);
               DrawText("- I/O to turn the engine on and off", window_width * 0.02, window_height * 0.24, 16, RAYWHITE);
               DrawText("- SPACE to horn", window_width * 0.02, window_height * 0.27, 16, RAYWHITE);
               DrawText("- P to EPA", window_width * 0.02, window_height * 0.3, 16, RAYWHITE);
               DrawText("- 1/2/3 to set throttle speed limit", window_width * 0.02, window_height * 0.33, 16, RAYWHITE);
               DrawText("- 4/5/6 to set steering sharpness limit", window_width * 0.02, window_height * 0.36, 16, RAYWHITE);
               DrawText("- R to reverse the steering control", window_width * 0.02, window_height * 0.39, 16, RAYWHITE);
               DrawText("- T to reverse the throttle control", window_width * 0.02, window_height * 0.42, 16, RAYWHITE);

               // RC Car Config
               DrawText("RC Car Config:", window_width * 0.02, window_height * 0.47, 24, RAYWHITE);
               DrawText(TextFormat("- Steering Limit: %d", car.config.steering_limits[car.config.steering_limits_idx]), window_width * 0.02, window_height * 0.51, 16, RAYWHITE);
               DrawText(TextFormat("- Throttle Limit: %d", car.config.throttle_limits[car.config.throttle_limits_idx]), window_width * 0.02, window_height * 0.54, 16, RAYWHITE);
          }

          EndDrawing();
     }

     CloseWindow();
     propotyperc_cleanup();
     return 0;
}
