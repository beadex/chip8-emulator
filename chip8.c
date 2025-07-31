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
        .pixel_outlines = true, // draw pixel outlines by default
    };

    return true;
}

void clear_screen(const emulator_config_t config, const emulator_t emu)
{
    const uint8_t bg_r = (config.bg_color >> 24) & 0xFF;
    const uint8_t bg_g = (config.bg_color >> 16) & 0xFF;
    const uint8_t bg_b = (config.bg_color >> 8) & 0xFF;
    const uint8_t bg_a = (config.bg_color >> 0) & 0xFF;

    SDL_SetRenderDrawColor(emu.renderer, bg_r, bg_g, bg_b, bg_a);
    SDL_RenderClear(emu.renderer);
}

void update_screen(const emulator_t emu, const emulator_config_t config, const chip8_t chip8)
{
    SDL_FRect rect = {.x = 0, .y = 0, .w = config.scale_factor, .h = config.scale_factor};

    // Grab color values to draw
    const uint8_t bg_r = (config.bg_color >> 24) & 0xFF;
    const uint8_t bg_g = (config.bg_color >> 16) & 0xFF;
    const uint8_t bg_b = (config.bg_color >> 8) & 0xFF;
    const uint8_t bg_a = (config.bg_color >> 0) & 0xFF;

    const uint8_t fg_r = (config.fg_color >> 24) & 0xFF;
    const uint8_t fg_g = (config.fg_color >> 16) & 0xFF;
    const uint8_t fg_b = (config.fg_color >> 8) & 0xFF;
    const uint8_t fg_a = (config.fg_color >> 0) & 0xFF;

    // Loop through display pixels, draw a rectangle per pixel to the SDL window
    for (uint32_t i = 0; i < sizeof chip8.display; i++)
    {
        // Translate 1D index i value to 2D X/Y coordinates
        // X = i % window width
        // Y = i / window width
        rect.x = (i % config.window_width) * config.scale_factor;
        rect.y = (i / config.window_width) * config.scale_factor;

        if (chip8.display[i])
        {
            // If pixel is on, draw foreground color
            SDL_SetRenderDrawColor(emu.renderer, fg_r, fg_g, fg_b, fg_a);
            SDL_RenderFillRect(emu.renderer, &rect);

            if (config.pixel_outlines)
            {
                SDL_SetRenderDrawColor(emu.renderer, bg_r, bg_g, bg_b, bg_a);
                SDL_RenderRect(emu.renderer, &rect);
            }
        }
        else
        {
            // If pixel is off, draw background color
            SDL_SetRenderDrawColor(emu.renderer, bg_r, bg_g, bg_b, bg_a);
            SDL_RenderFillRect(emu.renderer, &rect);
        }
    }

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
    chip8->stack_ptr = &chip8->stack[0];

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
            case SDLK_ESCAPE:
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

#ifdef DEBUG
void print_debug_info(chip8_t *chip8)
{
    printf("Address: 0x%04X, Opcode: 0x%04X Desc: ", chip8->PC - 2, chip8->inst.opcode);

    switch ((chip8->inst.opcode >> 12) & 0x0F)
    {
    case 0x00:
        if (chip8->inst.NN == 0xE0)
        {
            // 0x00E0: Clear the screen
            printf("Clear screen\n");
        }
        else if (chip8->inst.NN == 0xEE)
        {
            // 0x00EE: Return from subroutine
            // Set program counter to last address on subroutine stack ("pop" it off the stack)
            //   so that next opcode will be gotten from that address.
            printf("Return from subroutine to address 0x%04X\n",
                   *(chip8->stack_ptr - 1));
        }
        else
        {
            printf("Unimplemented Opcode.\n");
        }
        break;
    case 0x01:
        // 0x1NNN: Jump to address NNN
        printf("Jump to address NNN (0x%04X)\n",
               chip8->inst.NNN);
        break;
    case 0x02:
        // 0x2NNN: Call subroutine at NNN
        // Store current address to return to on subroutine stack ("push" it on the stack)
        //   and set program counter to subroutine address so that the next opcode
        //   is gotten from there.
        printf("Call subroutine at NNN (0x%04X)\n",
               chip8->inst.NNN);
        break;
    case 0x06:
        // 0x6XNN: Set register VX to NN
        printf("Set register V%X = NN (0x%02X)\n",
               chip8->inst.X, chip8->inst.NN);
        break;
    case 0x07:
        // 0xFX07: VX = delay timer
        printf("Set V%X = delay timer value (0x%02X)\n",
               chip8->inst.X, chip8->delay_timer);
        break;
    case 0x0A:
        // 0xANNN: Set index register I to NNN
        printf("Set I to NNN (0x%04X)\n",
               chip8->inst.NNN);
        break;
    case 0x0D:
        // 0xDXYN: Draw N-height sprite at coords X,Y; Read from memory location I;
        //   Screen pixels are XOR'd with sprite bits,
        //   VF (Carry flag) is set if any screen pixels are set off; This is useful
        //   for collision detection or other reasons.
        printf("Draw N (%u) height sprite at coords V%X (0x%02X), V%X (0x%02X) "
               "from memory location I (0x%04X). Set VF = 1 if any pixels are turned off.\n",
               chip8->inst.N, chip8->inst.X, chip8->V[chip8->inst.X], chip8->inst.Y,
               chip8->V[chip8->inst.Y], chip8->I);
        break;
    default:
        printf("Unimplemented Opcode.\n");
        break; // Unimplemented or invalid opcode
    }
}
#endif

void emulate_instruction(chip8_t *chip8, const emulator_config_t config)
{
    // Get next opcode from ram
    chip8->inst.opcode = (chip8->ram[chip8->PC] << 8) | chip8->ram[chip8->PC + 1];
    chip8->PC += 2; // Pre-increment program counter for next opcode

    // Fill out current instruction format
    chip8->inst.NNN = chip8->inst.opcode & 0x0FFF;
    chip8->inst.NN = chip8->inst.opcode & 0x0FF;
    chip8->inst.N = chip8->inst.opcode & 0x0F;
    chip8->inst.X = (chip8->inst.opcode >> 8) & 0x0F;
    chip8->inst.Y = (chip8->inst.opcode >> 4) & 0x0F;

#ifdef DEBUG
    print_debug_info(chip8);
#endif

    // Emulate opcode
    switch ((chip8->inst.opcode >> 12) & 0x0F)
    {
    case 0x00:
        if (chip8->inst.NN == 0xE0)
        {
            // 0x00E0: Clear the screen
            memset(&chip8->display[0], false, sizeof chip8->display);
            chip8->draw = true; // Will update screen on next 60hz tick
        }
        else if (chip8->inst.NN == 0xEE)
        {
            // 0x00EE: Return from the subroutine
            // Set program counter to last address from subroutine stack ("pop" it off the stack)
            // and that next opcode will be gotten from that address
            chip8->PC = *--chip8->stack_ptr;
        }
        break;
    case 0x01:
        // 0x1NNN: Jump to address NNN
        chip8->PC = chip8->inst.NNN; // Set program counter so that next opcode is from NNN
        break;
    case 0x02:
        // 0x2NNN: Call subroutine at NNN
        // Store current address to return on subroutine stack ("push" it on the stack)
        // and set program counter to subroutine address so that the next opcode
        // is gotten from there
        *chip8->stack_ptr = chip8->PC;
        chip8->PC = chip8->inst.NNN;
        break;
    case 0x06:
        // 0x6NN: Set register VX to NN
        chip8->V[chip8->inst.X] = chip8->inst.NN;
        break;
    case 0x07:
        // 0x7XNN: Set register VX += NN
        chip8->V[chip8->inst.X] += chip8->inst.NN;
        break;
    case 0x0A:
        // 0xANNN: Set index register I to NNN
        chip8->I = chip8->inst.NNN;
        break;
    case 0x0D:
        // 0xDXYN: Draw N height sprite at coords X,Y; Read from memory location I;
        // Screen pixels are XOR'd with sprite bits,
        // VF (Carry Flag) is set if any screen pixels are set off; This is useful for collision detection or other reasons.
        uint8_t X_coord = chip8->V[chip8->inst.X] % config.window_width;
        uint8_t Y_coord = chip8->V[chip8->inst.Y] % config.window_width;
        const uint8_t orig_X = X_coord; // Original X value

        chip8->V[0xF] = 0; // Initialize carry flag to 0

        // Loop over all N rows of the sprite
        for (uint8_t i = 0; i < chip8->inst.N; i++)
        {
            // Get next byte/row of sprite data
            const uint8_t sprite_data = chip8->ram[chip8->I + i];
            X_coord = orig_X; // Reset X for next row to draw

            for (int8_t j = 7; j >= 0; j--)
            {
                // If sprite pixel/bit is on and display pixel is on, set carry flag
                bool *pixel = &chip8->display[Y_coord * config.window_width + X_coord];
                const bool sprite_bit = (sprite_data & (1 << j));

                if (sprite_bit && *pixel)
                {
                    chip8->V[0xF] = 1;
                }

                // XOR display pixel with sprite pixel/bit to set it on or off
                *pixel ^= sprite_bit;

                // Stop drawing if hit right edge of screen
                if (++X_coord >= config.window_width)
                {
                    break;
                }
            }
            if (++Y_coord >= config.window_height)
            {
                break;
            }
        }
        chip8->draw = true; // Will update screen on next 60hz tick
        break;
    default:
        break; // Unimplemented or invalid opcode
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
        // Handle user input
        handle_input(&chip8);

        // Disrupt the loop if the emulator is paused
        if (chip8.state == PAUSED)
        {
            continue;
        }

        emulate_instruction(&chip8, config);

        SDL_Delay(16);

        if (chip8.draw)
        {
            update_screen(emu, config, chip8);
            chip8.draw = false;
        }
    }

    clear_sdl(emu);
    exit(EXIT_SUCCESS);
}