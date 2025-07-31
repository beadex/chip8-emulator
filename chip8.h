#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

// SDL Container object for the emulator
typedef struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;
} emulator_t;

// Emulator configuration object
typedef struct
{
    uint32_t window_width;
    uint32_t window_height;
    uint32_t fg_color;
    uint32_t bg_color;
    uint32_t scale_factor;
} emulator_config_t;

// Emulator states
typedef enum
{
    QUIT,
    RUNNING,
    PAUSED,
} emulator_state_t;

// CHIP8 Machine object
typedef struct
{
    emulator_state_t state;
} chip8_t;

bool init_sdl(emulator_t *emu, const emulator_config_t config);
bool set_config_from_args(emulator_config_t *config, const int agrc, char **argv);
void clear_screen(const emulator_config_t config, const emulator_t emu);
void update_screen(const emulator_t emu);
void handle_input(chip8_t *chip8);
bool init_chip8(chip8_t *chip8);
void clear_sdl(const emulator_t emu);