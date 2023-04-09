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

    input();

    // Obtain array from interrupts and compare with morse array
    char *interruptArray = grabArray();

    // Determine level based on input string
    for (int levelIndex = 1; levelIndex <= 4; levelIndex++) {
        char tempArray[6];

        // Assign values from input to the temporary array
        for (int i = 0; i < 6; i++) {
            tempArray[i] = *(interruptArray + (i * sizeof(int)));
        }

        tempArray[5] = '\0'; // Ensure the temporary array is null-terminated

        // Compare the temporary array with the morse strings
        if (strcmp(tempArray, morseStruct[levelIndex].morseCode) == 0) {
            level = levelIndex;
            break;
        }
    }
}

// Compares morse input with morse array
int get_Morse_Index(char *morse_Input)
{
    int i = 0;
	while (i !=36){
		if (!strcmp(morse_Input, morseStruct[i].morseCode))
		{
			return i;
		}
        i++;
	}

	return -1;
}

// Compares char input with char array
int get_Char_Index(char char_Input)
{
    int i = 0;
	while (i !=36){
		if (char_Input == morseStruct[i].letter)
		{
			return i;
		}
        i++;
	}

	return -1;
}

// Takes input array pointer and compares with char from challenge
int morse_Compare(char *input)
{
	char morse_Input[6];

	// Set values from input into char array
	for (int i = 0; i < 6; i++)
	{
		morse_Input[i] = *(input + (i *sizeof(int)));
	}

	// Make a new string
	char *string = morse_Input;

	// Find morse from user input
	int input_Index = get_Morse_Index(string);

	// Compares indexes
	if (input_Index == -1)
	{
		printf("||---------------------------------------------------||\n");
		printf("||                     NOT FOUND                     ||\n");
		printf("||---------------------------------------------------||\n");
		return 0;
	}
    if (challenge_Index == input_Index)	// If letter needed = letter input
	{
		printf("||---------------------------------------------------||\n");
		printf("||                 YOUR INPUT[ %s ]                  ||\n", morseStruct[input_Index].letter);
		printf("||---------------------------------------------------||\n");
		return 1;
	}
	else
	{
		printf("||---------------------------------------------------||\n");
		printf("||                 YOUR INPUT[ %s ]                  ||\n", morseStruct[input_Index].letter);
		printf("||---------------------------------------------------||\n");
		return 0;
	}
    return 0;
}

void hardcodedMorseCompareTest(){
    for(int i = 0; i < 8; i ++){
        challenge_Index = rand() % 36;
        if(morse_Compare(challenge_Index - 1) == 1){
            printf("An Error has occured with the comparison\n");
        } else if(morse_Compare(challenge_Index + 1) == 1){
            printf("An Error has occured with the comparison\n");
        } else if(morse_Compare(challenge_Index) == 0){
            printf("An Error has occured with the comparison\n");
        }
    }
}


