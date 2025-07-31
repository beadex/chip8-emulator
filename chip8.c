#include "chip8.h"

bool init_sdl(emulator_t *emu, const emulator_config_t config)
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    {
        fprintf(stderr, "Error initialize SDL3: %s\n", SDL_GetError());
        return false;
    }

    emu->window = SDL_CreateWindow("CHIP8 Emulator", config.window_width * config.scale_factor, config.window_height * config.scale_factor, 0);

    if (!emu->window)
    {
        fprintf(stderr, "Error creating Window: %s\n", SDL_GetError());
        return false;
    }

    emu->renderer = SDL_CreateRenderer(emu->window, NULL);

    if (!emu->renderer)
    {
        fprintf(stderr, "Error creating Renderer: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

bool set_config_from_args(emulator_config_t *config, const int agrc, char **argv)
{
    *config = (emulator_config_t){
        .window_width = 64,
        .window_height = 32,
        .fg_color = 0xFFFF00FF, // yellow
        .bg_color = 0xF1ABC2FF, // black
        .scale_factor = 20,
    };

    return true;
}

void clear_screen(const emulator_config_t config, const emulator_t emu)
{
    const uint8_t r = (config.bg_color >> 24) & 0xFF;
    const uint8_t g = (config.bg_color >> 16) & 0xFF;
    const uint8_t b = (config.bg_color >> 8) & 0xFF;
    const uint8_t a = (config.bg_color >> 0) & 0xFF;

    SDL_SetRenderDrawColor(emu.renderer, r, g, b, a);
    SDL_RenderClear(emu.renderer);
}

void update_screen(const emulator_t emu)
{
    SDL_RenderPresent(emu.renderer);
}

bool init_chip8(chip8_t *chip8)
{
    chip8->state = RUNNING;
    return true;
}

void handle_input(chip8_t *chip8)
{
    SDL_Event event;

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            chip8->state = QUIT;
            return;
        case SDL_EVENT_KEY_DOWN:
            switch (event.key.key)
            {
            case SDLK_Q:
                chip8->state = QUIT;
                return;
            default:
                break;
            }
            break;
        case SDL_EVENT_KEY_UP:
            break;
        default:
            break;
        }
    }
}

void clear_sdl(const emulator_t emu)
{
    SDL_DestroyWindow(emu.window);
    SDL_DestroyRenderer(emu.renderer);
    SDL_Quit();
}

int main(int argc, char **argv)
{
    emulator_t emu = {0};
    emulator_config_t config = {0};
    chip8_t chip8 = {0};

    // Hydrate the emulator container with default & user config
    if (!set_config_from_args(&config, argc, argv))
    {
        exit(EXIT_FAILURE);
    }

    // Initialize the emulator container
    if (!init_sdl(&emu, config))
    {
        exit(EXIT_FAILURE);
    }

    // Initialize the CHIP8 machine
    if (!init_chip8(&chip8))
    {
        exit(EXIT_FAILURE);
    }

    // Initial screen clear
    clear_screen(config, emu);

    // Main loop
    while (chip8.state != QUIT)
    {
        handle_input(&chip8);
        SDL_Delay(16);
        update_screen(emu);
    }

    clear_sdl(emu);
    exit(EXIT_SUCCESS);
}