#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>
#include <unistd.h>

#define MAX_TRIES 6
#define MAX_WORD_LENGTH 20
#define NUM_ANIMALS 30

typedef struct {
    char word[MAX_WORD_LENGTH];
    char guessedWord[MAX_WORD_LENGTH];
    char guessedLetters[MAX_WORD_LENGTH];
    int wordLength;
    int tries;
    int correctGuess;
} GuessTheWord;

void initializeGame(GuessTheWord *game, int mode);
void displayWord(const char word[], int length);
void displayPlayer(int attempts);
void displayPlayer(int attempts);
void makeGuess(GuessTheWord *game, char guess);
void Singleplayer();
void Multiplayer();
void exitmode();
void greetings();
void menu();
void menuSP();
void menuMP();

int main() {
    int choice;
    greetings();
    sleep(2);
    system("clear");

    do {
        system("clear");
        menu();
        printf("Enter your choice: ");
        scanf("%d", &choice); getchar(); 

        switch (choice) {
            case 1:
                system("clear");
                menuSP();printf("enter to continue...");getchar();
                Singleplayer();
                break;
            case 2:
                system("clear");
                menuMP();printf("enter to continue...");getchar();
                Multiplayer();
                break;
            case 3:
                system("clear");
                printf("Exiting the game...\n");
                sleep(1);
                system("clear");
                exitmode();
                break;
            default:
                system("clear");
                printf("Invalid choice. Please enter a valid option.\n");
                printf("\nenter to continue...");
                break;
        }

       
    } while (choice != 3);

    return 0;
}

void initializeGame(GuessTheWord *game, int mode) {
    if (mode == 1) {
        char animals[NUM_ANIMALS][MAX_WORD_LENGTH] = {
            "elephant", "tiger", "giraffe", "lion", "zebra", "koala", "panda", "kangaroo", "dolphin", "fish", "octopus", "orangutan", "parrot", "cheetah", "camel", "otter", "gorrila", "monkey", "rhinoceros", "hippopotamus", "crocodile", "penguin", "leopard", "bison", "jaguar", "armadillo"
        };
        srand(time(NULL));
        int index = rand() % NUM_ANIMALS;

        strcpy(game->word, animals[index]);
        game->wordLength = strlen(game->word);
        game->tries = 0;
        game->correctGuess = 0;
        memset(game->guessedWord, '_', game->wordLength);
        game->guessedWord[game->wordLength] = '\0';
        game->guessedLetters[0] = '\0';
    } else if (mode == 2) {
        system("clear");
        menuMP();
        printf("Enter the word to be guessed (secret!): ");
        fgets(game->word, MAX_WORD_LENGTH, stdin);
        game->word[strcspn(game->word, "\n")] = '\0';

        while (strlen(game->word) < 1) {
            printf("Please enter at least one character.\n");
            system("clear");
            printf("\nEnter the word to be guessed (secret!): ");
            fgets(game->word, MAX_WORD_LENGTH, stdin);
            game->word[strcspn(game->word, "\n")] = '\0';
        }

        game->wordLength = strlen(game->word);
        game->tries = 0;
        game->correctGuess = 0;

        memset(game->guessedWord, '_', game->wordLength);
        game->guessedWord[game->wordLength] = '\0'; 
    }
}

void displayWord(const char word[], int length) {
    for (int i = 0; i < length; i++) {
        printf("%c ", word[i]);
    }
    printf("\n");
}

void displayPlayer(int attempts) {
    if (attempts > 0) {
        printf("Attempts Left: ");
        for (int i = 0; i < attempts; i++) {
            printf("♡ ");
        }
        printf("\n");
    }
}

void makeGuess(GuessTheWord *game, char guess) {
    system("clear");
    int found = 0;
    for (int i = 0; i < game->wordLength; i++) {
        if (tolower(game->word[i]) == guess) {
            if (game->guessedWord[i] == '_') {
                game->guessedWord[i] = game->word[i];
                game->correctGuess++;
                found = 1;

                int len = strlen(game->guessedLetters);
                game->guessedLetters[len] = guess;
                game->guessedLetters[len + 1] = '\0';
            } else {
                found = 1; 
            }
        }
    }

    if (!found) {
        game->tries++;
    }
}

void Singleplayer() {
    int playAgain;
    do {
        GuessTheWord game;
        initializeGame(&game, 1);
        char guess;
        char input[MAX_WORD_LENGTH];

        while (game.tries < MAX_TRIES && game.correctGuess < game.wordLength) {
            system("clear");
            printf("\n");
            displayWord(game.guessedWord, game.wordLength);
            displayPlayer(MAX_TRIES - game.tries);

            if (game.correctGuess == game.wordLength) {
                break;
            }

            printf("Enter a letter guess: ");
            fgets(input, sizeof(input), stdin);

            if (strlen(input) > 2 || input[0] == '\n') {
                printf("Please enter only one letter at a time.\n");
                printf("Press enter to continue...\n");
                while(getchar() != '\n');
            } else {
                guess = tolower(input[0]);

                if (isspace(input[1]) || input[1] == '\n' || input[1] == '\0') {
                    makeGuess(&game, guess);
                } else {
                    printf("Please enter only one letter at a time.\n");
                    printf("Press enter to continue...\n");
                    while(getchar() != '\n');
                }
            }
        }

        system("clear");

        displayWord(game.guessedWord, game.wordLength);
        displayPlayer(MAX_TRIES - game.tries);

        if (game.correctGuess == game.wordLength) {
            printf("\nCongratulations! You guessed the word: %s\n", game.word);
        } else {
            printf("\nSorry, you couldn't guess the word: %s\n", game.word);
        }

        printf("\nPlay again? (1 for Yes / 0 for No): ");
        while (scanf("%d", &playAgain) != 1 || (playAgain != 0 && playAgain != 1)) {
            printf("Invalid input. Please enter 1 for Yes or 0 for No: ");
            while (getchar() != '\n');
        }

        system("clear");

    } while (playAgain == 1);
}


void Multiplayer() {
    int playAgain ;
    do {
        GuessTheWord game;
        initializeGame(&game, 2);

        while (game.tries < MAX_TRIES && game.correctGuess < game.wordLength) {
            system("clear");
            printf("\n");
            displayWord(game.guessedWord, game.wordLength);
            displayPlayer(MAX_TRIES - game.tries);

            printf("Enter a letter guess: ");
            char input[MAX_WORD_LENGTH];
            fgets(input, sizeof(input), stdin);

            if (strlen(input) > 2 || input[0] == '\n') {
                printf("Please enter only one letter at a time.\n");
                printf("Press enter to continue...\n");
                while (getchar() != '\n');
            } else {
                char guess = tolower(input[0]);

                if (isspace(input[1]) || input[1] == '\n' || input[1] == '\0') {
                    makeGuess(&game, guess);
                } else {
                    printf("Please enter only one letter at a time.\n");
                    printf("Press enter to continue...\n");
                    while (getchar() != '\n');
                }
            }
        }

        system("clear");

        displayWord(game.guessedWord, game.wordLength);
        displayPlayer(MAX_TRIES - game.tries);

        if (game.correctGuess == game.wordLength) {
            printf("\nCongratulations! You guessed the word: %s\n", game.word);
        } else {
            printf("\nSorry, you couldn't guess the word: %s\n", game.word);
        }

        printf("\nDo you want to play again? (1 for Yes / 0 for No): ");
        while (scanf("%d", &playAgain) != 1 || (playAgain != 0 && playAgain != 1)) {
            printf("Invalid input. Please enter 1 for Yes or 0 for No: ");
            while (getchar() != '\n');
        }

        system("clear");

    } while (playAgain == 1);
}

void exitmode() {
    system("clear");
    printf("\n");
    printf("###### ##  ##  ####  ##  ## ##  ##   ##  ##  ####  ##  ##  \n");
    printf("  ##   ##  ## ##  ## ### ## ## ##     ####  ##  ## ##  ##  \n");
    printf("  ##   ###### ###### ## ### #####      ##   ##  ## ##  ##  \n");
    printf("  ##   ##  ## ##  ## ##  ## ##  ##     ##   ##  ## ##  ##  ###  ###  ###\n");
    printf("  ##   ##  ## ##  ## ##  ## ##  ##     ##    ####   ####   ###  ###  ###\n");

}
void greetings(){
    system("clear");
    printf("\n");
    printf("##  ## ###### ##     ##      ####  \n");
    printf("##  ## ##     ##     ##     ##  ## \n");
    printf("###### ####   ##     ##     ##  ## \n");
    printf("##  ## ##     ##     ##     ##  ##  ## ## ##\n");
    printf("##  ## ###### ###### ######  ####   ## ## ##\n");
    printf("\n hello... Welcome to GUESS THE WORD game...\n");

}
void menu() {
    char underscore = '_';
    system("clear");
    printf("||=================================={INSTRUCTIONS}=================================|| \n");
    printf("|| 1. The game consists of two modes: singleplayer and multiplayer                 || \n");
    printf("|| 2. select an option on the \"choose\" menu to choose a playing method             || \n");
    printf("|| 3. Try to guess the words that appear by guessing one letter for each guess.    || \n");
    printf("|| 4. The available trial limit is 6 times.                                        || \n");
    printf("|| 5. The \"_\" sign can consist of numbers, letters, characters and spaces.         || \n");
    printf("|| 6. Uppercase and lowercase letters are the same                                 || \n");
    printf("|| 7. If you guess all the words, you win!                                         || \n");
    printf("|| 7. good luck !!!                                                                || \n");
    printf("||=================================={   CHOOSE   }=================================|| \n");
    printf("||    1. SINGLEPLAYER       ||     2. MULTIPLAYER       ||         3. QUIT         || \n");
    printf("||=================================================================================|| \n");
    printf("\n");
}
void menuSP() {
    system("clear");
    char underscore = '_';
    printf("||==================================={INSTRUCTIONS}==================================|| \n");
    printf("|| 1. The theme of this game is guess the name of the animal                         || \n");
    printf("|| 2. Numbers entered twice will be wrong                                            || \n");
    printf("|| 3. Uppercase and lowercase letters are the same                                   || \n");
    printf("|| 4. Try to guess the words that appear by guessing one letter for each guess.      || \n");
    printf("|| 5. The available trial limit is 6 times.                                          || \n");
    printf("|| 6. The \"_\" sign can consist of numbers, letters, characters and spaces.           || \n");
    printf("|| 7. When finished, select enter to return to the menu and '1' to continue the game || \n");
    printf("|| 8. If you guess all the words, you win!                                           || \n");
    printf("|| 9. good luck !!!                                                                  || \n");
    printf("||===================================================================================|| \n");
    printf("\n");
}
void menuMP() {
    system("clear");
    char underscore = '_';
    printf("||==================================={INSTRUCTIONS}==================================|| \n");
    printf("|| 1. The keywords that will be guessed are filled in by other players               || \n");
    printf("|| 2. Numbers entered twice will be wrong                                            || \n");
    printf("|| 3. Uppercase and lowercase letters are the same                                   || \n");
    printf("|| 4. Try to guess the words that appear by guessing one letter for each guess.      || \n");
    printf("|| 5. The available trial limit is 6 times.                                          || \n");
    printf("|| 6. The \"_\" sign can consist of numbers, letters, characters and spaces.           || \n");
    printf("|| 7. When finished, select '0' to return to the menu and '1' to continue the game   || \n");
    printf("|| 8. If you guess all the words, you win!                                           || \n");
    printf("|| 9. good luck !!!                                                                  || \n");
    printf("||===================================================================================|| \n");
    printf("\n");
}