#include "chip8.h"

bool init_sdl(emulator_t *emu, const emulator_config_t config)
{
    if (!SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO))
    {
        SDL_Log("Error initialize SDL3: %s\n", SDL_GetError());
        return false;
    }

    emu->window = SDL_CreateWindow("CHIP8 Emulator", config.window_width * config.scale_factor, config.window_height * config.scale_factor, 0);

    if (!emu->window)
    {
        SDL_Log("Error creating Window: %s\n", SDL_GetError());
        return false;
    }

    emu->renderer = SDL_CreateRenderer(emu->window, NULL);

    if (!emu->renderer)
    {
        SDL_Log("Error creating Renderer: %s\n", SDL_GetError());
        return false;
    }

    return true;
}

bool set_config_from_args(emulator_config_t *config, const int agrc, char **argv)
{
    *config = (emulator_config_t){
        .window_width = 64,
        .window_height = 32,
        .fg_color = 0xFFFFFFFF, // white
        .bg_color = 0x000000FF, // black
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

bool init_chip8(chip8_t *chip8, const char rom_name[])
{
    const uint32_t entry_point = 0x200; // CHIP8 ROMs will be loaded to 0x200
    const uint8_t font[] = {
        0xF0, 0x90, 0x90, 0x90, 0xF0, // 0
        0x20, 0x60, 0x20, 0x20, 0x70, // 1
        0xF0, 0x10, 0xF0, 0x80, 0xF0, // 2
        0xF0, 0x10, 0xF0, 0x10, 0xF0, // 3
        0x90, 0x90, 0xF0, 0x10, 0x10, // 4
        0xF0, 0x80, 0xF0, 0x10, 0xF0, // 5
        0xF0, 0x80, 0xF0, 0x90, 0xF0, // 6
        0xF0, 0x10, 0x20, 0x40, 0x40, // 7
        0xF0, 0x90, 0xF0, 0x90, 0xF0, // 8
        0xF0, 0x90, 0xF0, 0x10, 0xF0, // 9
        0xF0, 0x90, 0xF0, 0x90, 0x90, // A
        0xE0, 0x90, 0xE0, 0x90, 0xE0, // B
        0xF0, 0x80, 0x80, 0x80, 0xF0, // C
        0xE0, 0x90, 0x90, 0x90, 0xE0, // D
        0xF0, 0x80, 0xF0, 0x80, 0xF0, // E
        0xF0, 0x80, 0xF0, 0x80, 0x80, // F
    };

    // Load font
    memcpy(&chip8->ram[0], font, sizeof(font));

    // Open ROM file
    FILE *rom = fopen(rom_name, "rb");
    if (!rom)
    {
        SDL_Log("ROM file %s is invalid or does not exist\n", rom_name);
        return false;
    }

    // Get/check rom
    fseek(rom, 0, SEEK_END);
    const size_t rom_size = ftell(rom);
    const size_t max_size = sizeof chip8->ram - entry_point;
    rewind(rom);

    if (rom_size > max_size)
    {
        SDL_Log("ROM file %s is too big! Rom size %zu, Max size allowed: %zu\n", rom_name, rom_size, max_size);
        return false;
    }

    if (fread(&chip8->ram[entry_point], rom_size, 1, rom) != 1)
    {
        SDL_Log("Could not read ROM %s into CHIP8 memory\n", rom_name);
        return false;
    }

    fclose(rom);

    // Set CHIP8 machine defaults
    chip8->state = RUNNING;
    chip8->PC = entry_point; // Start PC at ROM entry point
    chip8->rom_name = rom_name;

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
            case SDLK_SPACE:
                if (chip8->state == RUNNING)
                {

                    chip8->state = PAUSED;
                    puts("==== PAUSE ====");
                }
                else
                {
                    chip8->state = RUNNING;
                }
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
    if (argc < 2)
    {
        fprintf(stderr, "Usage: %s <rom_name>\n", argv[0]);
        exit(EXIT_FAILURE);
    }
    emulator_t emu = {0};
    emulator_config_t config = {0};
    chip8_t chip8 = {0};
    const char *rom_name = argv[1];

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
    if (!init_chip8(&chip8, rom_name))
    {
        exit(EXIT_FAILURE);
    }

    // Initial screen clear
    clear_screen(config, emu);

    // Main loop
    while (chip8.state != QUIT)
    {
        handle_input(&chip8);

        if (chip8.state == PAUSED)
        {
            continue;
        }

        SDL_Delay(16);
        update_screen(emu);
    }

    clear_sdl(emu);
    exit(EXIT_SUCCESS);
}