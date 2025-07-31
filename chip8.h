#include <stdio.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdint.h>

#include "SDL3/SDL.h"
#include "SDL3/SDL_main.h"

/**
 * SDL Container object for the emulator
 */
typedef struct
{
    SDL_Window *window;
    SDL_Renderer *renderer;
} emulator_t;

/**
 * Emulator configuration object
 */
typedef struct
{
    uint32_t window_width;
    uint32_t window_height;
    uint32_t fg_color;
    uint32_t bg_color;
    uint32_t scale_factor;
    bool pixel_outlines; // Draw pixel outlines yes/no
} emulator_config_t;

/**
 * Emulator states
 */
typedef enum
{
    QUIT,
    RUNNING,
    PAUSED,
} emulator_state_t;

/**
 * CHIP8 Instruction format
 */
typedef struct
{
    uint16_t opcode;
    uint16_t NNN; // 12bit address/constant
    uint8_t NN;   // 8bit constant
    uint8_t N;    // 4bit constant
    uint8_t X;    // 4bit register identifier
    uint8_t Y;    // 4bit register identifier
} chip8_instruction_t;

/**
 * CHIP8 Machine object
 */
typedef struct
{
    emulator_state_t state;
    uint8_t ram[4096];
    bool display[64 * 32];    // Emulate original CHIP8 resolution pixels
    uint16_t stack[12];       // Subroutine stack
    uint16_t *stack_ptr;      // Stack pointer
    uint8_t V[16];            // Data registers V0-VF
    uint16_t I;               // Index register
    uint16_t PC;              // Program Counter
    uint8_t delay_timer;      // Decrements at 60hz when >0
    uint8_t sound_timer;      // Decrements at 60hz and plays tone when >0
    bool keypad[16];          // Hexadecimal keypad 0x0 - 0xF
    const char *rom_name;     // Currently running ROM
    chip8_instruction_t inst; // CUrrently executing instruction
    bool draw;                // Update the screen yes/no
} chip8_t;

/**
 * Initialize the SDL Window and Renderer, required for the emulator conatiner
 * \param emulator_t Emulator container object pointer
 * \param emulator_config_t Emulator config object
 * \returns true on success or false on failure; as long as the error message
 */
bool init_sdl(emulator_t *emu, const emulator_config_t config);

/**
 * Set the configurations for the emulator, both defaults and user configured
 * \param emulator_config_t Emulator config object
 * \param int agrc
 * \param char argv
 * \returns true on success or false on failure; as long as the error message
 */
bool set_config_from_args(emulator_config_t *config, const int agrc, char **argv);

/**
 * Clear and setup for screen render
 * \param emulator_config_t Emulator config object
 * \param emulator_t the emulator container
 */
void clear_screen(const emulator_config_t config, const emulator_t emu);

/**
 * Actual render & update the screen
 * \param emulator_t the emulator container
 */
void update_screen(const emulator_t emu, const emulator_config_t config, const chip8_t chip8);

/**
 * The poll events loop that handle various of input events
 * \param chip8_t the CHIP8 machine
 */
void handle_input(chip8_t *chip8);

/**
 * Initialize the CHIP8 machine
 * \param chip8_t the CHIP8 machine
 * \param char the name of the rom is being load
 * \returns true on success or false on failure; as long as the error message
 */
bool init_chip8(chip8_t *chip8, const char rom_name[]);

/**
 * Emulate the CHIP8 instructions
 * \param chip8_t the CHIP8 machine
 * \param emulator_config_t current config of emulator container
 */
void emulate_instruction(chip8_t *chip8, const emulator_config_t config);

/**
 * Final cleanup to get back memories to system
 * \param emulator_t the emulator container
 */
void clear_sdl(const emulator_t emu);