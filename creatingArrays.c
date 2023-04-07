#include <stdlib.h>
#include <stdio.h>


#define ARRAY_SIZE_FOR_MORSE 36

void setUpArrays();

void printArrays();

typedef struct morseChar morseChar;
struct morseChar{
    char *morseCode;
    char letter;
    char *word;
};

morseChar morseStruct[40];



// Array of morse code
char *morse[] = {"-----", ".----", "..---", "...--", "....-", ".....", "-....",
                 "--...", "---..", "----.", ".-", "-...", "-.-.", "-..", ".", 
                 "..-.", "--.", "....", "..", ".---", "-.-", ".-..", "--", "-.",
                 "---", ".--.", "--.-", ".-.", "...", "-", "..-", "...-", ".--",
                 "-..-", "-.--", "--."};

// Array of letters
char *letters[] = {"0", "1", "2", "3", "4", "5", "6", "7", "8", "9", "A", "B",
                   "C", "D", "E", "F", "G", "H", "I", "J", "K", "L", "M", "N",
                   "O", "P", "Q", "R", "S", "T", "U", "V", "W", "X", "Y", "Z"};

// Array of words for level 3 and 4
char *words[] = {"RADIO", "RELAY", "WAVES", "ALARM", "SOUND", "MORSE",
                 "HERTZ", "BLEEP", "BURST", "NOISE", "SIREN", "TONES",
                 "SPARK", "CABLE", "CODES", "DOTS", "DASH", "BEEP", 
                 "BUZZ", "TAPS"};


int main(){
    setUpArrays();
    printArrays();
}

void setUpArrays(){
    for(int i = 0; i < ARRAY_SIZE_FOR_MORSE; i++){
        if(i < 10){
            morseStruct[i].letter = (char)(i + '0'); // assign the character corresponding to the integer value
            switch (i) {
            case 0: morseStruct[i].morseCode = "-----"; break;
            case 1: morseStruct[i].morseCode = ".----"; break;
            case 2: morseStruct[i].morseCode = "..---"; break;
            case 3: morseStruct[i].morseCode = "...--"; break;
            case 4: morseStruct[i].morseCode = "....-"; break;
            case 5: morseStruct[i].morseCode = "....."; break;
            case 6: morseStruct[i].morseCode = "-...."; break;
            case 7: morseStruct[i].morseCode = "--..."; break;
            case 8: morseStruct[i].morseCode = "---.."; break;
            case 9: morseStruct[i].morseCode = "----."; break;
            default:
                break;
            }
        } else{
            morseStruct[i].letter = (char)(i + 55); // assign the character corresponding to the integer value
            switch (i) {
            case 10: morseStruct[i].morseCode = ".-";  break;
            case 11: morseStruct[i].morseCode = "-...";  break;
            case 12: morseStruct[i].morseCode = "-.-.";  break;
            case 13: morseStruct[i].morseCode = "-..";  break;
            case 14: morseStruct[i].morseCode = ".";   break;
            case 15: morseStruct[i].morseCode = "..-.";  break;
            case 16: morseStruct[i].morseCode = "--.";  break;
            case 17: morseStruct[i].morseCode = "....";  break;
            case 18: morseStruct[i].morseCode = "..";  break;
            case 19: morseStruct[i].morseCode = ".---";  break;
            case 20: morseStruct[i].morseCode = "-.-";  break;
            case 21: morseStruct[i].morseCode = ".-..";  break;
            case 22: morseStruct[i].morseCode = "--";  break;
            case 23: morseStruct[i].morseCode = "-.";  break;
            case 24: morseStruct[i].morseCode = "---";  break;
            case 25: morseStruct[i].morseCode = ".--.";  break;
            case 26: morseStruct[i].morseCode = "--.-";  break;
            case 27: morseStruct[i].morseCode = ".-.";  break;
            case 28: morseStruct[i].morseCode = "...";  break;
            case 29: morseStruct[i].morseCode = "-";  break;
            case 30: morseStruct[i].morseCode = "..-";  break;
            case 31: morseStruct[i].morseCode = "...-";  break;
            case 32: morseStruct[i].morseCode = ".--";  break;
            case 33: morseStruct[i].morseCode = "-..-";  break;
            case 34: morseStruct[i].morseCode = "-.--"; break;
            case 35: morseStruct[i].morseCode = "--."; break;
            case 36: morseStruct[i].morseCode = "NULL"; break;
            case 37: morseStruct[i].morseCode = "NULL"; break;
            case 38: morseStruct[i].morseCode = "NULL"; break;
            case 39: morseStruct[i].morseCode = "NULL"; break;
            default:
                break;
            }
        }
        if(i < 20){
            switch (i) {
                case  0: morseStruct[i].word = "RADIO";   break;
                case  1: morseStruct[i].word = "RELAY";   break;
                case  2: morseStruct[i].word = "WAVES";   break;
                case  3: morseStruct[i].word = "ALARM";   break;
                case  4: morseStruct[i].word = "SOUND";   break;
                case  5: morseStruct[i].word = "MORSE";   break;
                case  6: morseStruct[i].word = "HERTZ";   break;
                case  7: morseStruct[i].word = "BLEEP";   break;
                case  8: morseStruct[i].word = "BURST";   break;
                case  9: morseStruct[i].word = "NOISE";   break;
                case 10: morseStruct[i].word = "SIREN";   break;
                case 11: morseStruct[i].word = "TONES";   break;
                case 12: morseStruct[i].word = "SPARK";   break;
                case 13: morseStruct[i].word = "CABLE";   break;
                case 14: morseStruct[i].word = "CODES";   break;
                case 15: morseStruct[i].word = "DOTS";   break;
                case 16: morseStruct[i].word = "DASH";   break;
                case 17: morseStruct[i].word = "BEEP";   break;
                case 18: morseStruct[i].word = "BUZZ";   break;
                case 19: morseStruct[i].word = "TAPS";  break;
                default:
                    break;
            }
        }
    }
}


void printArrays(){
    for(int i = 0; i < ARRAY_SIZE_FOR_MORSE; i++){
        if(morseStruct[i].morseCode != NULL){
            printf("Character: %c, Morse String: %s\n", morseStruct[i].letter, morseStruct[i].morseCode);
        }
    }
    for(int i = 0; i < 20; i++){
        if(morseStruct[i].word != NULL){
            printf("Word is: %s\n", morseStruct[i].word);
        }
    }
}