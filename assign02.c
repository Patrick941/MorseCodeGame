#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <time.h>

#include "pico/stdlib.h"

#include "hardware/gpio.h"

#include "hardware/pio.h"

#include "hardware/clocks.h"

#include "ws2812.pio.h"

void main_asm();
void asm_gpio_init(uint pin);
void asm_gpio_set_dir(uint pin, bool out);
bool asm_gpio_get(uint pin);
void asm_gpio_put(uint pin, bool value);
void asm_gpio_set_irq(uint pin);
static inline void put_pixel(uint32_t pixel_grb);
static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b);
void RGB_Change();
void hardcodedMorseCompareTest();
int morse_Compare(char * input);
int verify_Input();
void branch_to_asm();
char * getPointerToArray();
void Game_Over_Screen();
void Game_Win_Screen();
void Game_STATS();
void Game_Flow_Control();
void print_level_header();


#define ARRAY_SIZE_FOR_MORSE 36

void setUpArrays();
void setUpArrays();
void printArrays();
void playGame();

typedef struct morseChar morseChar;
struct morseChar {
    char * morseCode;
    char letter;
    char * word;
};

morseChar morseStruct[40];

// From Example
#define IS_RGBW true // Will use RGBW format
#define NUM_PIXELS 1 // There is 1 WS2812 device in the chain
#define WS2812_PIN 28 // The GPIO pin that the WS2812 connected to

// Global Variables
int challenge_Index; // Index of char for each challenge
int lives = 3; // Amount of lives for player
// Stats variables
int total_Correct = 0; // Total Correct Guesses
// (For level 3 and 4 its the full word not each char)

int total_Lives_Lost = 0; // Total Lives Lost
int total_Lives_Gained = 0; // Total Lives Gained
int completed_Levels = 0;

int guess_Correct = 0; // To control 5 correct guesses 
int guess_Current = 0; // Current guess for work level 3 and 4
int level = 1; // Starting level and level select
char Game_Over = '0'; // Bool for game over scene

// Main function
int main() {

    // Initialise all STDIO as we will be using the GPIOs
    stdio_init_all();

    // Initialise the PIO interface with the WS2812 code
    PIO pio = pio0;
    uint offset = pio_add_program(pio, & ws2812_program);
    ws2812_program_init(pio, 0, offset, WS2812_PIN, 800000, IS_RGBW);

    // Run Main arm function in assign02.S
    main_asm();

    return 0;
}

void playGame() {
    printf("\033[1;34m");
    put_pixel(urgb_u32(0x00, 0x00, 0x00));
    printf("||===================================================||\n");
    printf("||  _       _   _______   _______   _______   _____  ||\n");
    printf("|| |:|     |:| |:::::::| |:::::::| |:::::::| |:::::| ||\n");
    printf("|| |::|   |::| |:|   |:| |:|   |:| |:|       |:|     ||\n");
    printf("|| |:|:| |:|:| |:|   |:| |:|___|:| |:|_____  |:|___  ||\n");
    printf("|| |:||:|:||:| |:|   |:| |::::::|  |:::::::| |:::::| ||\n");
    printf("|| |:| |:| |:| |:|   |:| |:||:|          |:| |:|     ||\n");
    printf("|| |:|     |:| |:|___|:| |:| |:|    _____|:| |:|___  ||\n");
    printf("|| |:|     |:| |:::::::| |:|  |:|  |:::::::| |:::::| ||\n");
    printf("||      _________     ___     _       _   _____      ||\n");
    printf("||     |:::::::::|   |:::|   |:|     |:| |:::::|     ||\n");
    printf("||     |:|     |:|  |:| |:|  |::|   |::| |:|         ||\n");
    printf("||     |:|         |:|   |:| |:|:| |:|:| |:|___      ||\n");
    printf("||     |:|    ___  |:|___|:| |:||:|:||:| |:::::|     ||\n");
    printf("||     |:|   |:::| |:::::::| |:| |:| |:| |:|         ||\n");
    printf("||     |:|_____|:| |:|   |:| |:|     |:| |:|___      ||\n");
    printf("||     |:::::::::| |:|   |:| |:|     |:| |:::::|     ||\n");
    printf("||===================================================||\n");
    printf("||                   [HOW TO PLAY]                   ||\n");
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
                    printf("    Your Character Is < %c >    Morse: (%s)\n", morseStruct[challenge_Index].letter, morseStruct[challenge_Index].morseCode);
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
                    printf("    Your Character is < %c >\n", morseStruct[challenge_Index].letter);
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
                        printf("    Your Word Is < %s > and it is of size %i \n", buff, size);
                        printf("||---------------------------------------------------||\n");

                        while (guess_Current < 5) {
                            challenge_Index = wordLetterIndexes[guess_Current];

                            printf("||---------------------------------------------------||\n");
                            printf("    Current Letter: < %c > = (%s)\n", morseStruct[challenge_Index].letter, morseStruct[challenge_Index].morseCode);
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
                                    printf("||              [ + WORD COMPLETED + ]               ||\n");
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
                                    Game_Over_Screen();
                                    Game_STATS();
                                    return;
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
                        printf("    Your Word Is < %s > and it is of size %i \n", buff, size);
                        printf("||---------------------------------------------------||\n");

                        while (guess_Current < 4) {
                            challenge_Index = wordLetterIndexes[guess_Current];

                            printf("||---------------------------------------------------||\n");
                            printf("    Current Letter:< %c > = (%s)\n", morseStruct[challenge_Index].letter, morseStruct[challenge_Index].morseCode);
                            printf("||---------------------------------------------------||\n");

                            // printf("%i is the value of size\n", size);
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
                                    printf("||              [ + WORD COMPLETED + ]               ||\n");
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
                                    Game_Over_Screen();
                                    Game_STATS();
                                    return;
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
                                    printf("||              [ + WORD COMPLETED + ]               ||\n");
                                    printf("||---------------------------------------------------||\n");
                                    level++;
                                    completed_Levels++;
                                    Game_Win_Screen();
                                    Game_STATS();
                                    return;
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
                                    printf("||              [ + WORD COMPLETED + ]               ||\n");
                                    printf("||---------------------------------------------------||\n");
                                    printf("\033[1;32m");
                                    completed_Levels++;
                                    Game_Win_Screen();
                                    Game_STATS();
                                    return;
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

// Initialise a GPIO pin - see SDK for detail on gpio_init()
void asm_gpio_init(uint pin) {
    gpio_init(pin);
}

// Set direction of a GPIO pin - see SDK for detail on gpio_set_dir()
void asm_gpio_set_dir(uint pin, bool out) {
    gpio_set_dir(pin, out);
}

// Get the value of a GPIO pin - see SDK for detail on gpio_get()
bool asm_gpio_get(uint pin) {
    return gpio_get(pin);
}

void setUpArrays() {
    for (int i = 0; i < ARRAY_SIZE_FOR_MORSE; i++) {
        if (i < 10) {
            morseStruct[i].letter = (char)(i + '0'); // assign the character corresponding to the integer value
            switch (i) {
            case 0:
                morseStruct[i].morseCode = "-----";
                break;
            case 1:
                morseStruct[i].morseCode = ".----";
                break;
            case 2:
                morseStruct[i].morseCode = "..---";
                break;
            case 3:
                morseStruct[i].morseCode = "...--";
                break;
            case 4:
                morseStruct[i].morseCode = "....-";
                break;
            case 5:
                morseStruct[i].morseCode = ".....";
                break;
            case 6:
                morseStruct[i].morseCode = "-....";
                break;
            case 7:
                morseStruct[i].morseCode = "--...";
                break;
            case 8:
                morseStruct[i].morseCode = "---..";
                break;
            case 9:
                morseStruct[i].morseCode = "----.";
                break;
            default:
                break;
            }
        } else {
            morseStruct[i].letter = (char)(i + 55); // assign the character corresponding to the integer value
            switch (i) {
            case 10:
                morseStruct[i].morseCode = ".-";
                break;
            case 11:
                morseStruct[i].morseCode = "-...";
                break;
            case 12:
                morseStruct[i].morseCode = "-.-.";
                break;
            case 13:
                morseStruct[i].morseCode = "-..";
                break;
            case 14:
                morseStruct[i].morseCode = ".";
                break;
            case 15:
                morseStruct[i].morseCode = "..-.";
                break;
            case 16:
                morseStruct[i].morseCode = "--.";
                break;
            case 17:
                morseStruct[i].morseCode = "....";
                break;
            case 18:
                morseStruct[i].morseCode = "..";
                break;
            case 19:
                morseStruct[i].morseCode = ".---";
                break;
            case 20:
                morseStruct[i].morseCode = "-.-";
                break;
            case 21:
                morseStruct[i].morseCode = ".-..";
                break;
            case 22:
                morseStruct[i].morseCode = "--";
                break;
            case 23:
                morseStruct[i].morseCode = "-.";
                break;
            case 24:
                morseStruct[i].morseCode = "---";
                break;
            case 25:
                morseStruct[i].morseCode = ".--.";
                break;
            case 26:
                morseStruct[i].morseCode = "--.-";
                break;
            case 27:
                morseStruct[i].morseCode = ".-.";
                break;
            case 28:
                morseStruct[i].morseCode = "...";
                break;
            case 29:
                morseStruct[i].morseCode = "-";
                break;
            case 30:
                morseStruct[i].morseCode = "..-";
                break;
            case 31:
                morseStruct[i].morseCode = "...-";
                break;
            case 32:
                morseStruct[i].morseCode = ".--";
                break;
            case 33:
                morseStruct[i].morseCode = "-..-";
                break;
            case 34:
                morseStruct[i].morseCode = "-.--";
                break;
            case 35:
                morseStruct[i].morseCode = "--.";
                break;
            case 36:
                morseStruct[i].morseCode = "NULL";
                break;
            case 37:
                morseStruct[i].morseCode = "NULL";
                break;
            case 38:
                morseStruct[i].morseCode = "NULL";
                break;
            case 39:
                morseStruct[i].morseCode = "NULL";
                break;
            default:
                break;
            }
        }
        if (i < 20) {
            switch (i) {
            case 0:
                morseStruct[i].word = "RADIO";
                break;
            case 1:
                morseStruct[i].word = "RELAY";
                break;
            case 2:
                morseStruct[i].word = "WAVES";
                break;
            case 3:
                morseStruct[i].word = "ALARM";
                break;
            case 4:
                morseStruct[i].word = "SOUND";
                break;
            case 5:
                morseStruct[i].word = "MORSE";
                break;
            case 6:
                morseStruct[i].word = "HERTZ";
                break;
            case 7:
                morseStruct[i].word = "BLEEP";
                break;
            case 8:
                morseStruct[i].word = "BURST";
                break;
            case 9:
                morseStruct[i].word = "NOISE";
                break;
            case 10:
                morseStruct[i].word = "SIREN";
                break;
            case 11:
                morseStruct[i].word = "TONES";
                break;
            case 12:
                morseStruct[i].word = "SPARK";
                break;
            case 13:
                morseStruct[i].word = "CABLE";
                break;
            case 14:
                morseStruct[i].word = "CODES";
                break;
            case 15:
                morseStruct[i].word = "DOTS";
                break;
            case 16:
                morseStruct[i].word = "DASH";
                break;
            case 17:
                morseStruct[i].word = "BEEP";
                break;
            case 18:
                morseStruct[i].word = "BUZZ";
                break;
            case 19:
                morseStruct[i].word = "TAPS";
                break;
            default:
                break;
            }
        }
    }
}

void printArrays() {
    for (int i = 0; i < ARRAY_SIZE_FOR_MORSE; i++) {
        if (morseStruct[i].morseCode != NULL) {
            printf("Character: %c, Morse String: %s\n", morseStruct[i].letter, morseStruct[i].morseCode);
        }
    }
    for (int i = 0; i < 20; i++) {
        if (morseStruct[i].word != NULL) {
            printf("Word is: %s\n", morseStruct[i].word);
        }
    }
}

// Set the value of a GPIO pin - see SDK for detail on gpio_put()
void asm_gpio_put(uint pin, bool value) {
    gpio_put(pin, value);
}

// Enable falling-edge interrupt - see SDK for detail on gpio_set_irq_enabled()
void asm_gpio_set_irq(uint pin) {
    gpio_set_irq_enabled(pin, GPIO_IRQ_EDGE_FALL | GPIO_IRQ_EDGE_RISE, true);
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

void hardcodedMorseCompareTest() {
    for (int i = 0; i < 8; i++) {
        challenge_Index = rand() % 36;
        if (morse_Compare(morseStruct[challenge_Index - 1].morseCode) == 1) {
            printf("An Error has occured with the comparison\n");
        } else if (morse_Compare(morseStruct[challenge_Index + 1].morseCode) == 1) {
            printf("An Error has occured with the comparison\n");
        } else if (morse_Compare(morseStruct[challenge_Index].morseCode) == 0) {
            printf("An Error has occured with the comparison\n");
        }
    }
}

// Takes input array pointer and compares with char from challenge
int morse_Compare(char * input) {
    char morse_Input[6];

    // Set values from input into char array
    for (int i = 0; i < 6; i++) {
        morse_Input[i] = * (input + (i * sizeof(int)));
    }

    // Find morse from user input
    int input_Index;

    int i = 0;
    while (i != 36) {
        if (!strcmp(morse_Input, morseStruct[i].morseCode)) {
            input_Index = i;
            // Compares indexes
            if (input_Index == -1) {
                printf("||---------------------------------------------------||\n");
                printf("||                     NOT FOUND                     ||\n");
                printf("||---------------------------------------------------||\n");
                return 0;
            }
            if (challenge_Index == input_Index) // If letter needed = letter input
            {
                printf("||---------------------------------------------------||\n");
                printf("||                 YOUR INPUT [ %c ]                  ||\n", morseStruct[input_Index].letter);
                printf("||---------------------------------------------------||\n");
                return 1;
            } else {
                printf("||---------------------------------------------------||\n");
                printf("||                 YOUR INPUT [ %c ]                  ||\n", morseStruct[input_Index].letter);
                printf("||---------------------------------------------------||\n");
                return 0;
            }
            return 0;
        }
        i++;
    }
    return 0;
}

// Verifies user input
int verify_Input() {
    if (morse_Compare(getPointerToArray()) == 1) {
        printf("||==================[ + CORRECT + ]==================||\n");
        return 1;
    } else {
        printf("||###################[ - WRONG - ]###################||\n");
        return 0;
    }
}

void Game_Over_Screen() {
    printf("#######################################################\n");
    printf("#           _________     ___     _       _   _____   #\n");
    printf("#          |:::::::::|   |:::|   |:|     |:| |:::::|  #\n");
    printf("#         |:|     |:|  |:| |:|  |::|   |::| |:|       #\n");
    printf("#        |:|         |:|   |:| |:|:| |:|:| |:|__      #\n");
    printf("#       |:|   ___   |:|___|:| |:||:||:|:| |:::::|     #\n");
    printf("#      |:|   |:::| |:::::::| |:| |:| |:| |:|          #\n");
    printf("#     |:|____|:|  |:|   |:| |:|     |:| |:| _         #\n");
    printf("#    |:::::::::| |:|   |:| |:|     |:| |:::::|        #\n");
    printf("#            _______   _      _   _____   _______     #\n");
    printf("#           |:::::::| |:|    |:| |:::::| |:::::::|    #\n");
    printf("#          |:|   |:| |:|    |:| |:|     |:|   |:|     #\n");
    printf("#         |:|   |:|  |:|  |:|  |:|__   |:|___|:|      #\n");
    printf("#        |:|   |:|  |:|  |:|  |:::::| |::::::|        #\n");
    printf("#       |:|   |:|   |:||:|   |:|     |:||:|           #\n");
    printf("#      |:|_|:|     |:|:|    |:|     |:| |:|           #\n");
    printf("#     |:::::::|    |:|     |:::::| |:|  |:|           #\n");
    printf("#######################################################\n");
}

// Function to display Victory screen
void Game_Win_Screen() {
    printf("||===================================================||\n");
    printf("||           _       _   _______   _     _           ||\n");
    printf("||          |:|     |:| |:::::::| |:|   |:|          ||\n");
    printf("||           |:|   |:|  |:|   |:| |:|   |:|          ||\n");
    printf("||            |:| |:|   |:|   |:| |:|   |:|          ||\n");
    printf("||             |:|:|    |:|   |:| |:|   |:|          ||\n");
    printf("||              |:|     |:|   |:| |:|   |:|          ||\n");
    printf("||              |:|     |:|___|:| |:|___|:|          ||\n");
    printf("||              |:|     |:::::::| |:::::::|          ||\n");
    printf("||         _             _______   __      _         ||\n");
    printf("||        |:|       |:| |:::::::| |::|    |:|        ||\n");
    printf("||        |:|       |:|    |:|    |:|:|   |:|        ||\n");
    printf("||        |:|  |:|  |:|    |:|    |:||:|  |:|        ||\n");
    printf("||        |:| |:|:| |:|    |:|    |:| |:| |:|        ||\n");
    printf("||        |:||:| |:||:|    |:|    |:|  |:||:|        ||\n");
    printf("||        |:|:|   |:|:|    |:|    |:|   |:|:|        ||\n");
    printf("||        |::|     |::| |:::::::| |:|    |::|        ||\n");
    printf("||===================================================||\n");
}

// Function for endgame Game_STATS
void Game_STATS() {
    // Calculate percentage of correct answers
    int percentage_Correct = ((total_Correct * 100) / (total_Correct + total_Lives_Lost));
    // Print Stats
    printf("||===================================================||\n");
    printf("||                       STATS                       ||\n");
    printf("||===================================================||\n");
    printf("    Levels Completed: %d \n", completed_Levels);
    printf("    Lives Lost: %d \n", total_Lives_Lost);
    printf("    Lives Gained: %d \n", total_Lives_Gained);
    printf("    Correct Guesses: %d \n", total_Correct);
    printf("    Percent Correct: %d %% \n", percentage_Correct);
    printf("||===================================================||\n");
}

void print_level_header(int level) {
    printf("||===================================================||\n");
    printf("||                  [LEVEL: %i of 4]                  ||\n", level);
    printf("||===================================================||\n");
}
