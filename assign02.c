#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "assign02Arrays.h"
#include "pico/stdlib.h"
#include "hardware/gpio.h"
#include "hardware/pio.h"

void asm_gpio_init(uint pin);
void asm_gpio_set_dir(uint pin, bool out);
bool asm_gpio_get(uint pin);
void asm_gpio_put(uint pin, bool value);
void asm_gpio_set_irq(uint pin);
void playGame();
void main_asm();

// Main entry point of the application
int main() {
    setUpArrays();
    stdio_init_all();              // Initialise all basic IO
    main_asm();                    // Jump into the ASM code
    playGame();
    return 0;                      // Application return code
}

// Initialise a GPIO pin – see SDK for detail on gpio_init()
void asm_gpio_init(uint pin) {
    gpio_init(pin);
}

// Set direction of a GPIO pin – see SDK for detail on gpio_set_dir()
void asm_gpio_set_dir(uint pin, bool out) {
    gpio_set_dir(pin, out);
}

// Get the value of a GPIO pin – see SDK for detail on gpio_get()
bool asm_gpio_get(uint pin) {
    return gpio_get(pin);
}

// Set the value of a GPIO pin – see SDK for detail on gpio_put()
void asm_gpio_put(uint pin, bool value) {
    gpio_put(pin, value);
}

// Enable falling-edge interrupt – see SDK for detail on gpio_set_irq_enabled()
void asm_gpio_set_irq(uint pin) {
    gpio_set_irq_enabled(pin, GPIO_IRQ_EDGE_FALL, true);
}

void playGame(){
    printf("\033[1;34m");
	put_pixel(urgb_u32(0x00, 0x00, 0x00));
	printf("||===================================================||\n");
	printf("||  _       _   _______   _______   _______   _____  ||\n");
	printf("|| |:|     |:| |:::::::| |:::::::| |:::::::| |:::::| ||\n");
	printf("|| |::|   |::| |:|   |:| |:|   |:| |:|       |:|     ||\n");
	printf("|| |:|:| |:|:| |:|   |:| |:|__|:| |:|____  |:|___  ||\n");
	printf("|| |:||:|:||:| |:|   |:| |::::::|  |:::::::| |:::::| ||\n");
	printf("|| |:| |:| |:| |:|   |:| |:||:|          |:| |:|     ||\n");
	printf("|| |:|     |:| |:|__|:| |:| |:|    _____|:| |:|__  ||\n");
	printf("|| |:|     |:| |:::::::| |:|  |:|  |:::::::| |:::::| ||\n");
	printf("||      _________     ___     _       _   _____      ||\n");
	printf("||     |:::::::::|   |:::|   |:|     |:| |:::::|     ||\n");
	printf("||     |:|     |:|  |:| |:|  |::|   |::| |:|         ||\n");
	printf("||     |:|         |:|   |:| |:|:| |:|:| |:|___      ||\n");
	printf("||     |:|    ___  |:|___|:| |:||:|:||:| |:::::|     ||\n");
	printf("||     |:|   |:::| |:::::::| |:| |:| |:| |:|         ||\n");
	printf("||     |:|____|:| |:|   |:| |:|     |:| |:|__      ||\n");
	printf("||     |:::::::::| |:|   |:| |:|     |:| |:::::|     ||\n");
	printf("||===================================================||\n");
	printf("||                  [HOW TO PLAY]                   ||\n");
	printf("||===================================================||\n");
	printf("||       Characters Will Appear On Your Screen       ||\n");
	printf("||    You Must Enter The Corresponding Morse Code    ||\n");
	printf("||      Press The GP21 Button Quickly For A Dot      ||\n");
	printf("||      Press The GP21 Button Longer For A Dash      ||\n");
	printf("||          If Correct You Will Gain A Life          ||\n");
	printf("||               (Maximum Of 3 lives)                ||\n");
	printf("||         If Incorrect You Will Lose A Life         ||\n");
	printf("||      When You Lose All Lives, it's Game Over      ||\n");
	printf("||    After 5 Correct Sequences You Win The Level    ||\n");
	printf("||===================================================||\n");
	printf("\033[0;39m");

    printf("||===================================================||\n");
	printf("||                   Select Level:                   ||\n");
	printf("||===================================================||\n");
	printf("||             Enter '.----' for level 1             ||\n");
	printf("||             Enter '..---' for level 2             ||\n");
	printf("||             Enter '...--' for level 3             ||\n");
	printf("||             Enter '....-' for level 4             ||\n");
	printf("||===================================================||\n");
}


