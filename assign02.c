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
    stdio_init_all();              
    PIO pio = pio0;
    uint offset = pio_add_program(pio, & ws2812_program);
    ws2812_program_init(pio, 0, offset, WS2812_PIN, 800000, IS_RGBW);

    main_asm();                    
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

// FROM Example
static inline void put_pixel(uint32_t pixel_grb) {
    pio_sm_put_blocking(pio0, 0, pixel_grb << 8u);
}

// FROM Example
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) |
        ((uint32_t)(g) << 16) |
        (uint32_t)(b);
}

// Function to change LED based on current lives
void RGB_Change() {
    // From example
    if (lives == 0) {
        // Red when no lives
        printf("\033[1;31m");
        put_pixel(urgb_u32(0x7F, 0x00, 0x00));
    } else if (lives == 1) {
        // Orange for 1 life
        printf("\033[0;33m");
        put_pixel(urgb_u32(0x2F, 0xC, 0x00));
    } else if (lives == 2) {
        // Blue for 2 lives
        printf("\033[0;34m");
        put_pixel(urgb_u32(0x00, 0x00, 0x7F));
    } else if (lives == 3) {
        // Green for 3 lives
        printf("\033[1;32m");
        put_pixel(urgb_u32(0x00, 0x7F, 0x00));
    }
}


void playGame() {
    printf("\033[1;34m");
    put_pixel(urgb_u32(0x00, 0x00, 0x00));
    printf("||===================================================||\n");
    printf("||  _       _   ____3___   _______   _______   _____  ||\n");
    printf("|| |:|     |:| |:::::::| |:::::::| |:::::::| |:::::| ||\n");
    printf("|| |::|   |::| |:|   |:| |:|   |:| |:|       |:|     ||\n");
    printf("|| |:|:| |:|:| |:|   |:| |:|_|:| |:|___  |:|___  ||\n");
    printf("|| |:||:|:||:| |:|   |:| |::::::|  |:::::::| |:::::| ||\n");
    printf("|| |:| |:| |:| |:|   |:| |:||:|          |:| |:|     ||\n");
    printf("|| |:|     |:| |:|_|:| |:| |:|    _____|:| |:|_  ||\n");
    printf("|| |:|     |:| |:::::::| |:|  |:|  |:::::::| |:::::| ||\n");
    printf("||      _________     ___     _       _   _____      ||\n");
    printf("||     |:::::::::|   |:::|   |:|     |:| |:::::|     ||\n");
    printf("||     |:|     |:|  |:| |:|  |::|   |::| |:|         ||\n");
    printf("||     |:|         |:|   |:| |:|:| |:|:| |:|___      ||\n");
    printf("||     |:|    ___  |:|___|:| |:||:|:||:| |:::::|     ||\n");
    printf("||     |:|   |:::| |:::::::| |:| |:| |:| |:|         ||\n");
    printf("||     |:|___|:|   |:|   |:| |:|     |:| |:| _       ||\n");
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

    // Receive input from arm interrupts
    branch_to_asm();

    // Obtain array from interrupts and compare with morse array
    char * interruptArray = getPointerToArray();

    // Determine level based on input string
    for (int levelIndex = 1; levelIndex <= 4; levelIndex++) {
        char tempArray[6];

        // Assign values from input to the temporary array
        for (int i = 0; i < 6; i++) {
            tempArray[i] = * (interruptArray + (i * sizeof(int)));
        }

        tempArray[5] = '\0'; // Ensure the temporary array is null-terminated

        // Compare the temporary array with the morse strings
        if (strcmp(tempArray, morseStruct[levelIndex].morseCode) == 0) {
            level = levelIndex;
            break;
        }
    }

    while (Game_Over == '0') {
        // To run levels based on cases
        switch (level) {
        case 1:
            print_level_header(1);
            lives = 3;
            guess_Correct = 0;
            RGB_Change(); // indicates the game has begun

            for (;;) {
                {
                    challenge_Index = rand() % 36;

                    // Print Level 1
                    printf("||---------------------------------------------------||\n");
                    printf("    Your Character is<%c >    Morse: (%s)\n", morseStruct[challenge_Index].letter, morseStruct[challenge_Index].morseCode);
                    printf("||---------------------------------------------------||\n");

                    // Get input
                    branch_to_asm();

                    if (verify_Input() == 1) {
                        // Increment Number of Correct Guesses
                        guess_Correct++;

                        // Increment Total Number of Correct Guesses
                        total_Correct++;

                        if (lives < 3) {
                            // Increase Lives
                            lives++;

                            // Increase lives gained stat
                            total_Lives_Gained++;

                            // Change Colour of LED
                            RGB_Change();
                        }

                        if (guess_Correct == 5) {
                            level++;
                            completed_Levels++;
                            break;
                        }
                    } else {
                        // Reduce Lives
                        lives--;

                        // Increment Amount of Lives Lost
                        total_Lives_Lost++;

                        // Change Colour of LED
                        RGB_Change();

                        // If Lost Last Life, Game Over
                        if (lives == 0) {
                            Game_Over = '1';
                            break;
                        }
                    }
                }
            }

            break;
        case 2:
            print_level_header(2);
            lives = 3;
            guess_Correct = 0;
            RGB_Change(); // indicates the game has begun

            for (;;) {
                {
                    challenge_Index = rand() % 36;

                    printf("||---------------------------------------------------||\n");
                    printf("    Your Character is<%c >\n", morseStruct[challenge_Index].letter);
                    printf("||---------------------------------------------------||\n");

                    // Get input
                    branch_to_asm();

                    if (verify_Input() == 1) {
                        // Increment Number of Correct Guesses
                        guess_Correct++;

                        // Increment Total Number of Correct Guesses
                        total_Correct++;

                        if (lives < 3) {
                            // Increase Lives
                            lives++;

                            // Increase lives gained stat
                            total_Lives_Gained++;

                            // Change Colour of LED
                            RGB_Change();
                        }

                        if (guess_Correct == 5) {
                            level++;
                            completed_Levels++;
                            break;
                        }
                    } else {
                        // Reduce Lives
                        lives--;

                        // Increment Amount of Lives Lost
                        total_Lives_Lost++;

                        // Change Colour of LED
                        RGB_Change();

                        // If Lost Last Life, Game Over
                        if (lives == 0) {
                            Game_Over = '1';
                            break;
                        }
                    }
                }
            }

            break;
        case 3:
            lives = 3;
            guess_Correct = 0;
            print_level_header(3);
            RGB_Change(); // indicates the game has begun
            int wordLetterIndexes[5]; // Array of indexes

            for (;;) {
                {
                    guess_Current = 0;
                    challenge_Index = rand() % 20; // Get random word
                    char * buff = morseStruct[challenge_Index].word; // Set buffer as that word
                    int size = 0; // Set size of that word to 0

                    if (challenge_Index < 15) {
                        // While buffer is not end string
                        while (strcmp( & buff[size], "\0")) {
                            int i = 0;
                            while (i != 36) {
                                if (buff[size] == morseStruct[i].letter) {
                                    wordLetterIndexes[size] = i;
                                }
                                i++;
                            }
                            size++;
                        }

                        printf("||---------------------------------------------------||\n");
                        printf("    Your Word Is<%s > and it is of size %i \n", buff, size);
                        printf("||---------------------------------------------------||\n");

                        while (guess_Current < 5) {
                            challenge_Index = wordLetterIndexes[guess_Current];

                            printf("||---------------------------------------------------||\n");
                            printf("    Current Letter:<%c > = (%s)\n", morseStruct[challenge_Index].letter, morseStruct[challenge_Index].morseCode);
                            printf("||---------------------------------------------------||\n");

                            //printf("%i is the value of size\n", size);
                            // Get Input

                            branch_to_asm();

                            if (verify_Input() == 1) {
                                guess_Current++;
                                //printf("incremented guess_current. New value %i\n", guess_Current);
                                //printf("%i is the value of size\n", 5);

                                // Increment Number of Correct Guesses
                                guess_Correct++;

                                // Increment Total Number of Correct Guesses
                                total_Correct++;

                                if (lives < 3) {
                                    // Increase Lives
                                    lives++;

                                    // Increase lives gained stat
                                    total_Lives_Gained++;

                                    // Change Colour of LED
                                    RGB_Change();
                                }

                                if (guess_Correct == 5) {

                                    printf("||---------------------------------------------------||\n");
                                    printf("||              [ + WORD COMPLETED +]               ||\n");
                                    printf("||---------------------------------------------------||\n");
                                    level++;
                                    completed_Levels++;
                                    break;
                                }
                            } else {
                                // Reduce Lives
                                lives--;

                                // Increment Amount of Lives Lost
                                total_Lives_Lost++;

                                // Change Colour of LED
                                RGB_Change();

                                // If Lost Last Life, Game Over
                                if (lives == 0) {
                                    Game_Over = '1';
                                    break;
                                }
                            }
                        }
                    } else {
                        // While buffer is not end string
                        while (strcmp( & buff[size], "\0")) {
                            int i = 0;
                            while (i != 36) {
                                if (buff[size] == morseStruct[i].letter) {
                                    wordLetterIndexes[size] = i;
                                }
                                i++;
                            }
                            size++;
                            size++;
                        }

                        printf("||---------------------------------------------------||\n");
                        printf("    Your Word Is<%s > and it is of size %i \n", buff, size);
                        printf("||---------------------------------------------------||\n");

                        while (guess_Current < 4) {
                            challenge_Index = wordLetterIndexes[guess_Current];

                            printf("||---------------------------------------------------||\n");
                            printf("    Current Letter:<%c > = (%s)\n", morseStruct[challenge_Index].letter, morseStruct[challenge_Index].morseCode);
                            printf("||---------------------------------------------------||\n");

                            printf("%i is the value of size\n", size);
                            // Get Input

                            branch_to_asm();

                            if (verify_Input() == 1) {
                                guess_Current++;
                                //printf("incremented guess_current. New value %i\n", guess_Current);
                                //printf("%i is the value of size\n", 5);

                                // Increment Number of Correct Guesses
                                guess_Correct++;

                                // Increment Total Number of Correct Guesses
                                total_Correct++;

                                if (lives < 3) {
                                    // Increase Lives
                                    lives++;

                                    // Increase lives gained stat
                                    total_Lives_Gained++;

                                    // Change Colour of LED
                                    RGB_Change();
                                }

                                if (guess_Correct == 4) {

                                    printf("||---------------------------------------------------||\n");
                                    printf("||              [ + WORD COMPLETED +]               ||\n");
                                    printf("||---------------------------------------------------||\n");
                                    level++;
                                    completed_Levels++;
                                    break;
                                }
                            } else {
                                // Reduce Lives
                                lives--;

                                // Increment Amount of Lives Lost
                                total_Lives_Lost++;

                                // Change Colour of LED
                                RGB_Change();

                                // If Lost Last Life, Game Over
                                if (lives == 0) {
                                    Game_Over = '1';
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            break;
        case 4:
            lives = 3;
            guess_Correct = 0;
            print_level_header(4);
            RGB_Change(); // indicates the game has begun
            int wordLetterIndex[5]; // Array of indexes

            for (;;) {
                {
                    guess_Current = 0;
                    challenge_Index = rand() % 20; // Get random word
                    char * buff = morseStruct[challenge_Index].word; // Set buffer as that word
                    int size = 0; // Set size of that word to 0

                    if (challenge_Index < 15) {
                        // While buffer is not end string
                        while (strcmp( & buff[size], "\0")) {
                            int i = 0;
                            while (i != 36) {
                                if (buff[size] == morseStruct[i].letter) {
                                    wordLetterIndexes[size] = i;
                                }
                                i++;
                            }
                            size++;
                            size++;
                        }

                        printf("||---------------------------------------------------||\n");
                        printf("    Your Word Is<%s > and it is of size %i \n", buff, size);
                        printf("||---------------------------------------------------||\n");

                        while (guess_Current < 5) {
                            challenge_Index = wordLetterIndex[guess_Current];

                            printf("||---------------------------------------------------||\n");
                            printf("    Current Letter:<%c >\n", morseStruct[challenge_Index].letter);
                            printf("||---------------------------------------------------||\n");

                            //printf("%i is the value of size\n", size);
                            // Get Input

                            branch_to_asm();

                            if (verify_Input() == 1) {
                                guess_Current++;
                                //printf("incremented guess_current. New value %i\n", guess_Current);
                                //printf("%i is the value of size\n", 5);

                                // Increment Number of Correct Guesses
                                guess_Correct++;

                                // Increment Total Number of Correct Guesses
                                total_Correct++;

                                if (lives < 3) {
                                    // Increase Lives
                                    lives++;

                                    // Increase lives gained stat
                                    total_Lives_Gained++;

                                    // Change Colour of LED
                                    RGB_Change();
                                }

                                if (guess_Correct == 5) {

                                    printf("||---------------------------------------------------||\n");
                                    printf("||              [ + WORD COMPLETED +]               ||\n");
                                    printf("||---------------------------------------------------||\n");
                                    level++;
                                    completed_Levels++;
                                    break;
                                }
                            } else {
                                // Reduce Lives
                                lives--;

                                // Increment Amount of Lives Lost
                                total_Lives_Lost++;

                                // Change Colour of LED
                                RGB_Change();

                                // If Lost Last Life, Game Over
                                if (lives == 0) {
                                    Game_Over = '1';
                                    break;
                                }
                            }
                        }
                    } else {
                        // While buffer is not end string
                        while (strcmp( & buff[size], "\0")) {
                            int i = 0;
                            while (i != 36) {
                                if (buff[size] == morseStruct[i].letter) {
                                    wordLetterIndexes[size] = i;
                                }
                                i++;
                            }
                            size++;
                            size++;
                        }

                        printf("||---------------------------------------------------||\n");
                        printf("    Your Word Is<%s > and it is of size %i \n", buff, size);
                        printf("||---------------------------------------------------||\n");

                        while (guess_Current < 4) {
                            challenge_Index = wordLetterIndex[guess_Current];

                            printf("||---------------------------------------------------||\n");
                            printf("    Current Letter:<%c > = (%s)\n", morseStruct[challenge_Index].letter, morseStruct[challenge_Index].morseCode);
                            printf("||---------------------------------------------------||\n");

                            //printf("%i is the value of size\n", size);
                            // Get Input

                            branch_to_asm();

                            if (verify_Input() == 1) {
                                guess_Current++;
                                //printf("incremented guess_current. New value %i\n", guess_Current);
                                //printf("%i is the value of size\n", 5);

                                // Increment Number of Correct Guesses
                                guess_Correct++;

                                // Increment Total Number of Correct Guesses
                                total_Correct++;

                                if (lives < 3) {
                                    // Increase Lives
                                    lives++;

                                    // Increase lives gained stat
                                    total_Lives_Gained++;

                                    // Change Colour of LED
                                    RGB_Change();
                                }

                                if (guess_Correct == 4) {

                                    printf("||---------------------------------------------------||\n");
                                    printf("||              [ + WORD COMPLETED +]               ||\n");
                                    printf("||---------------------------------------------------||\n");
                                    printf("\033[1;32m");
                                    completed_Levels++;
                                    Game_Win_Screen();
                                    Game_STATS();
                                    break;
                                }
                            } else {
                                // Reduce Lives
                                lives--;

                                // Increment Amount of Lives Lost
                                total_Lives_Lost++;

                                // Change Colour of LED
                                RGB_Change();

                                // If Lost Last Life, Game Over
                                if (lives == 0) {
                                    Game_Over = '1';
                                    break;
                                }
                            }
                        }
                    }
                }
            }

            break;

        default:
            printf("Tip: try entering the level properly next time dumb dumb!");
        }
    }

    Game_Over_Screen();
    Game_STATS();
    return;

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


