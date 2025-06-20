#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <ctype.h>

#if defined(PLATFORM_WEB)
    #include <emscripten/emscripten.h>
#endif

//----------------------------------------------------------------------------------
// Defines and Macros
//----------------------------------------------------------------------------------
#define MAX_TRIES 6
#define MAX_WORD_LENGTH 20
#define NUM_ANIMALS 30

//----------------------------------------------------------------------------------
// Types and Structures Definition
//----------------------------------------------------------------------------------
typedef struct {
    char word[MAX_WORD_LENGTH];
    char guessedWord[MAX_WORD_LENGTH];
    char guessedLetters[27];
    int wordLength;
    int triesLeft;
    int correctLettersGuessed;
    bool letterAlreadyGuessed[26];
} GuessTheWordGame;

typedef enum GameScreen {
    SCREEN_LOGO = 0,
    SCREEN_TITLE,
    SCREEN_MODE_SELECT,
    SCREEN_INSTRUCTIONS_SP,
    SCREEN_INSTRUCTIONS_MP,
    SCREEN_MP_SETUP_WORD,
    SCREEN_GAMEPLAY,
    SCREEN_GAME_OVER
} GameScreen;

//----------------------------------------------------------------------------------
// Global Variables
//----------------------------------------------------------------------------------
static const int screenWidth = 800;
static const int screenHeight = 600;

static Rectangle singlePlayerButtonRect;
static Rectangle multiPlayerButtonRect;
static Rectangle backToTitleButtonRect;
static Rectangle startButtonRect;

static GameScreen currentScreen = SCREEN_LOGO;
static GuessTheWordGame game;
static bool gameWon = false;
static bool isMultiplayer = false;

static char mpWordInputBuffer[MAX_WORD_LENGTH + 1] = { 0 };
static int mpWordInputLetterCount = 0;
static Rectangle mpWordInputBox = { screenWidth/2.0f - 150, screenHeight/2.0f - 20, 300, 40 };
static bool mpWordSubmitted = false;
static int cursorBlinkCounter = 0;
static bool showCursor = true;

static char animals[NUM_ANIMALS][MAX_WORD_LENGTH] = {
    "elephant", "tiger", "giraffe", "lion", "zebra", "koala", "panda", "kangaroo",
    "dolphin", "octopus", "orangutan", "parrot", "cheetah", "camel", "otter",
    "gorilla", "monkey", "rhinoceros", "hippopotamus", "crocodile", "penguin",
    "leopard", "bison", "jaguar", "armadillo", "snake", "shark", "eagle", "wolf", "bear"
};

static Font gameFont;

//----------------------------------------------------------------------------------
// Module Functions Declaration
//----------------------------------------------------------------------------------
static void InitGame(bool multiplayer);
static void UpdateGame(void);
static void DrawGame(void);
static void UnloadGame(void);
static void UpdateDrawFrame(void);

static void DrawHangman(int triesLeft);
static void DrawGuessedWord(void);
static void DrawAlphabet(void);
static void ProcessGuess(char letter);
static void ResetGame(void);

//------------------------------------------------------------------------------------
// Program main entry point
//------------------------------------------------------------------------------------
int main(void)
{
    InitWindow(screenWidth, screenHeight, "Guess The Word (Hangman)");
    gameFont = LoadFontEx("resources/custom_font.ttf", 32, 0, 250);
    if (gameFont.texture.id == 0) gameFont = GetFontDefault();

    InitGame(false);

#if defined(PLATFORM_WEB)
    emscripten_set_main_loop(UpdateDrawFrame, 0, 1);
#else
    SetTargetFPS(60);
    while (!WindowShouldClose())
    {
        UpdateDrawFrame();
    }
#endif

    UnloadGame();
    CloseWindow();

    return 0;
}

//----------------------------------------------------------------------------------
// Module Functions Definition
//----------------------------------------------------------------------------------
void InitGame(bool multiplayer)
{
    isMultiplayer = multiplayer;
    game.triesLeft = MAX_TRIES;
    game.correctLettersGuessed = 0;
    memset(game.guessedLetters, 0, sizeof(game.guessedLetters));
    for (int i = 0; i < 26; i++) game.letterAlreadyGuessed[i] = false;

    mpWordSubmitted = false;
    mpWordInputLetterCount = 0;
    memset(mpWordInputBuffer, 0, sizeof(mpWordInputBuffer));
    cursorBlinkCounter = 0;
    showCursor = true;

    if (!multiplayer) {
        srand(time(NULL));
        int index = rand() % NUM_ANIMALS;
        strcpy(game.word, animals[index]);
    }

    if (multiplayer && !mpWordSubmitted) {
        // Word will be set up later
    } else {
        game.wordLength = strlen(game.word);
        for (int i = 0; i < game.wordLength; i++) {
            if (isalpha(game.word[i])) {
                game.guessedWord[i] = '_';
            } else {
                game.guessedWord[i] = game.word[i];
                game.correctLettersGuessed++;
            }
        }
        game.guessedWord[game.wordLength] = '\0';
    }
}

void ResetGame() {
    gameWon = false;
    InitGame(isMultiplayer);
}


void UpdateGame(void)
{
    if (currentScreen == SCREEN_MODE_SELECT) {
        const char *item1Text = "1. Single Player";
        const char *item2Text = "2. Multi Player";
        const char *item3Text = "3. Back";

        Vector2 item1Size = MeasureTextEx(gameFont, item1Text, 24, 1);
        Vector2 item2Size = MeasureTextEx(gameFont, item2Text, 24, 1);
        Vector2 item3Size = MeasureTextEx(gameFont, item3Text, 24, 1);

        float maxWidth = 0;
        if (item1Size.x > maxWidth) maxWidth = item1Size.x;
        if (item2Size.x > maxWidth) maxWidth = item2Size.x;
        if (item3Size.x > maxWidth) maxWidth = item3Size.x;

        float buttonTextPadding = 10.0f;
        float uniformButtonWidth = maxWidth + 2 * buttonTextPadding;
        float buttonHeight = item1Size.y + 2 * buttonTextPadding;
        float commonButtonX = screenWidth/2.0f - uniformButtonWidth/2.0f;

        float currentY = 190.0f;
        float ySpacing = buttonHeight + 10.0f;

        singlePlayerButtonRect = (Rectangle){ commonButtonX, currentY, uniformButtonWidth, buttonHeight };
        currentY += ySpacing;
        multiPlayerButtonRect = (Rectangle){ commonButtonX, currentY, uniformButtonWidth, buttonHeight };
        currentY += ySpacing;
        backToTitleButtonRect = (Rectangle){ commonButtonX, currentY, uniformButtonWidth, buttonHeight };
    }

    if (currentScreen == SCREEN_INSTRUCTIONS_SP || currentScreen == SCREEN_INSTRUCTIONS_MP) {
        const char *buttonTxt = (currentScreen == SCREEN_INSTRUCTIONS_SP) ? "Start Game" : "Continue to Word Input";
        Vector2 btnTextSize = MeasureTextEx(gameFont, buttonTxt, 24, 1);
        float buttonTextPadding = 10.0f;
        startButtonRect = (Rectangle){ screenWidth/2.0f - (btnTextSize.x + 2 * buttonTextPadding)/2.0f, 250, btnTextSize.x + 2 * buttonTextPadding, btnTextSize.y + 2 * buttonTextPadding };
    }


    switch(currentScreen) {
        case SCREEN_LOGO: {
            static int framesCounter = 0;
            framesCounter++;
            if (framesCounter > 120) {
                currentScreen = SCREEN_TITLE;
                framesCounter = 0;
            }
        } break;
        case SCREEN_TITLE: {
            if (IsKeyPressed(KEY_ENTER) || (IsMouseButtonPressed(MOUSE_BUTTON_LEFT))) {
                currentScreen = SCREEN_MODE_SELECT;
            }
        } break;
        case SCREEN_MODE_SELECT: {
            if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                if (CheckCollisionPointRec(GetMousePosition(), singlePlayerButtonRect)) {
                    isMultiplayer = false;
                    currentScreen = SCREEN_INSTRUCTIONS_SP;
                } else if (CheckCollisionPointRec(GetMousePosition(), multiPlayerButtonRect)) {
                    isMultiplayer = true;
                    currentScreen = SCREEN_INSTRUCTIONS_MP;
                } else if (CheckCollisionPointRec(GetMousePosition(), backToTitleButtonRect)) {
                    currentScreen = SCREEN_TITLE;
                }
            }
            if (IsKeyPressed(KEY_ONE)) { isMultiplayer = false; currentScreen = SCREEN_INSTRUCTIONS_SP; }
            else if (IsKeyPressed(KEY_TWO)) { isMultiplayer = true; currentScreen = SCREEN_INSTRUCTIONS_MP; }
            else if (IsKeyPressed(KEY_THREE) || IsKeyPressed(KEY_ESCAPE)) { currentScreen = SCREEN_TITLE; }
        } break;
        case SCREEN_INSTRUCTIONS_SP: {
             if (IsKeyPressed(KEY_ENTER) || (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), startButtonRect)) ) {
                ResetGame();
                currentScreen = SCREEN_GAMEPLAY;
            }
        } break;
        case SCREEN_INSTRUCTIONS_MP: {
            if (IsKeyPressed(KEY_ENTER) || (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && CheckCollisionPointRec(GetMousePosition(), startButtonRect)) ) {
                ResetGame();
                currentScreen = SCREEN_MP_SETUP_WORD;
            }
        } break;
        case SCREEN_MP_SETUP_WORD: {
            if (mpWordSubmitted) break;

            cursorBlinkCounter++;
            if (cursorBlinkCounter >= 30) {
                cursorBlinkCounter = 0;
                showCursor = !showCursor;
            }

            int key = GetCharPressed();
            while (key > 0) {
                if (((key >= 'a' && key <= 'z') || (key >= 'A' && key <= 'Z')) && (mpWordInputLetterCount < MAX_WORD_LENGTH -1) ) {
                    mpWordInputBuffer[mpWordInputLetterCount] = (char)tolower(key);
                    mpWordInputBuffer[mpWordInputLetterCount+1] = '\0';
                    mpWordInputLetterCount++;
                }
                key = GetCharPressed();
            }

            if (IsKeyPressed(KEY_BACKSPACE)) {
                mpWordInputLetterCount--;
                if (mpWordInputLetterCount < 0) mpWordInputLetterCount = 0;
                mpWordInputBuffer[mpWordInputLetterCount] = '\0';
            }

            if (IsKeyPressed(KEY_ENTER) && mpWordInputLetterCount > 0) {
                strcpy(game.word, mpWordInputBuffer);
                mpWordSubmitted = true;
                game.wordLength = strlen(game.word);
                game.correctLettersGuessed = 0;
                for (int i = 0; i < game.wordLength; i++) {
                    if (isalpha(game.word[i])) {
                        game.guessedWord[i] = '_';
                    } else {
                        game.guessedWord[i] = game.word[i];
                        game.correctLettersGuessed++;
                    }
                }
                game.guessedWord[game.wordLength] = '\0';
                currentScreen = SCREEN_GAMEPLAY;
            }
        } break;
        case SCREEN_GAMEPLAY: {
            if (game.triesLeft <= 0 || game.correctLettersGuessed == game.wordLength) {
                gameWon = (game.correctLettersGuessed == game.wordLength);
                currentScreen = SCREEN_GAME_OVER;
                break;
            }
            int keyPressed = GetKeyPressed();
            if ((keyPressed >= KEY_A && keyPressed <= KEY_Z)) {
                 ProcessGuess((char)tolower(keyPressed));
            }
        } break;
        case SCREEN_GAME_OVER: {
            if (IsKeyPressed(KEY_ENTER) || IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                currentScreen = SCREEN_MODE_SELECT;
            }
        } break;
        default: break;
    }
}

void ProcessGuess(char letter) {
    if (letter < 'a' || letter > 'z') return;
    if (game.letterAlreadyGuessed[letter - 'a']) return;

    game.letterAlreadyGuessed[letter - 'a'] = true;
    strncat(game.guessedLetters, &letter, 1);

    bool found = false;
    for (int i = 0; i < game.wordLength; i++) {
        if (tolower(game.word[i]) == letter) {
            if (game.guessedWord[i] == '_') {
                game.guessedWord[i] = game.word[i];
                game.correctLettersGuessed++;
            }
            found = true;
        }
    }
    if (!found) game.triesLeft--;
}

void DrawGame(void)
{
    BeginDrawing();
    ClearBackground(RAYWHITE);

    float buttonTextPadding = 10.0f; // Consistent padding for text inside buttons

    switch(currentScreen) {
        case SCREEN_LOGO: {
            DrawTextEx(gameFont, "Hangman Game", (Vector2){ screenWidth/2 - MeasureTextEx(gameFont, "Hangman Game", 40, 1).x/2, screenHeight/2 - 40 }, 40, 1, DARKGRAY);
            DrawTextEx(gameFont, "by akmalhen", (Vector2){ screenWidth/2 - MeasureTextEx(gameFont, "by akmalhen", 20, 1).x/2, screenHeight/2 + 10 }, 20, 1, GRAY);
        } break;
        case SCREEN_TITLE: {
            DrawTextEx(gameFont, "Guess The Word (HANGMAN)", (Vector2){ screenWidth/2 - MeasureTextEx(gameFont, "Guess The Word (HANGMAN)", 40, 1).x/2, screenHeight/2 - 60 }, 40, 1, BLACK);
            DrawTextEx(gameFont, "Click or Press ENTER to Start", (Vector2){ screenWidth/2 - MeasureTextEx(gameFont, "Click or Press ENTER to Start", 20, 1).x/2, screenHeight/2 + 20 }, 20, 1, DARKGRAY);
        } break;
        case SCREEN_MODE_SELECT: {
            DrawTextEx(gameFont, "SELECT MODE:", (Vector2){ screenWidth/2 - MeasureTextEx(gameFont, "SELECT MODE:", 30, 1).x/2, 100 }, 30, 1, BLACK);

            DrawRectangleRec(singlePlayerButtonRect, CheckCollisionPointRec(GetMousePosition(), singlePlayerButtonRect) ? LIGHTGRAY : BLANK);
            DrawTextEx(gameFont, "1. Single Player", (Vector2){ singlePlayerButtonRect.x + buttonTextPadding, singlePlayerButtonRect.y + buttonTextPadding }, 24, 1, DARKGRAY);

            DrawRectangleRec(multiPlayerButtonRect, CheckCollisionPointRec(GetMousePosition(), multiPlayerButtonRect) ? LIGHTGRAY : BLANK);
            DrawTextEx(gameFont, "2. Multi Player", (Vector2){ multiPlayerButtonRect.x + buttonTextPadding, multiPlayerButtonRect.y + buttonTextPadding }, 24, 1, DARKGRAY);

            DrawRectangleRec(backToTitleButtonRect, CheckCollisionPointRec(GetMousePosition(), backToTitleButtonRect) ? LIGHTGRAY : BLANK);
            DrawTextEx(gameFont, "3. Back", (Vector2){ backToTitleButtonRect.x + buttonTextPadding, backToTitleButtonRect.y + buttonTextPadding }, 24, 1, DARKGRAY);
        } break;
        case SCREEN_INSTRUCTIONS_SP: {
            DrawTextEx(gameFont, "Single Player Instructions", (Vector2){50, 50}, 30, 1, BLACK);
            DrawTextEx(gameFont, "1. Theme: Animal Names.", (Vector2){50, 100}, 20, 1, DARKGRAY);
            DrawTextEx(gameFont, "2. Guess the letters one by one.", (Vector2){50, 130}, 20, 1, DARKGRAY);
            DrawTextEx(gameFont, "3. You have 6 attempts to guess.", (Vector2){50, 160}, 20, 1, DARKGRAY);

            DrawRectangleRec(startButtonRect, CheckCollisionPointRec(GetMousePosition(), startButtonRect) ? LIGHTGRAY : BLANK);
            DrawTextEx(gameFont, "Start Game", (Vector2){ startButtonRect.x + buttonTextPadding, startButtonRect.y + buttonTextPadding }, 24, 1, BLUE);
        } break;
        case SCREEN_INSTRUCTIONS_MP: {
            DrawTextEx(gameFont, "Multi Player Instructions", (Vector2){50, 50}, 30, 1, BLACK);
            DrawTextEx(gameFont, "1. Player 1 enters a secret word.", (Vector2){50, 100}, 20, 1, DARKGRAY);
            DrawTextEx(gameFont, "2. Player 2 then guesses the word.", (Vector2){50, 130}, 20, 1, DARKGRAY);
            DrawTextEx(gameFont, "3. You have 6 attempts to guess.", (Vector2){50, 160}, 20, 1, DARKGRAY);

            DrawRectangleRec(startButtonRect, CheckCollisionPointRec(GetMousePosition(), startButtonRect) ? LIGHTGRAY : BLANK);
            DrawTextEx(gameFont, "Continue to Word Input", (Vector2){ startButtonRect.x + buttonTextPadding, startButtonRect.y + buttonTextPadding }, 24, 1, BLUE);
        } break;
        case SCREEN_MP_SETUP_WORD: {
            DrawTextEx(gameFont, "Player 1: Enter Secret Word", (Vector2){screenWidth/2 - MeasureTextEx(gameFont, "Player 1: Enter Secret Word", 30,1).x/2, 100}, 30,1, BLACK);
            DrawRectangleRec(mpWordInputBox, LIGHTGRAY);
            DrawTextEx(gameFont, mpWordInputBuffer, (Vector2){mpWordInputBox.x + 10, mpWordInputBox.y + 5}, 30,1, MAROON);

            if (showCursor) {
                Vector2 textSize = MeasureTextEx(gameFont, mpWordInputBuffer, 30, 1);
                float cursorX = mpWordInputBox.x + 10 + textSize.x + 2;
                float cursorY_start = mpWordInputBox.y + 5;
                float cursorY_end = mpWordInputBox.y + 5 + MeasureTextEx(gameFont, "A", 30, 1).y;
                DrawLine((int)cursorX, (int)cursorY_start, (int)cursorX, (int)cursorY_end, MAROON);
            }

            DrawTextEx(gameFont, TextFormat("Length: %d/%d", mpWordInputLetterCount, MAX_WORD_LENGTH-1), (Vector2){mpWordInputBox.x, mpWordInputBox.y + 50}, 20,1, DARKGRAY);
            if (mpWordInputLetterCount > 0) {
                 DrawTextEx(gameFont, "Press ENTER to Confirm", (Vector2){screenWidth/2 - MeasureTextEx(gameFont, "Press ENTER to Confirm", 20,1).x/2, mpWordInputBox.y + 100}, 20,1, DARKBLUE);
            }
            DrawTextEx(gameFont, "(Don't let Player 2 see!)", (Vector2){screenWidth/2 - MeasureTextEx(gameFont, "(Don't let Player 2 see!)", 18,1).x/2, mpWordInputBox.y + 150}, 18,1, GRAY);
        } break;
        case SCREEN_GAMEPLAY: {
            DrawHangman(game.triesLeft);
            DrawGuessedWord();
            DrawAlphabet();
            DrawTextEx(gameFont, TextFormat("Attempts Left: %d", game.triesLeft), (Vector2){550, 500}, 24, 1, (game.triesLeft <=2 ? RED : DARKGREEN));
            DrawTextEx(gameFont, "Guess a letter (A-Z)", (Vector2){50, 500}, 24, 1, DARKGRAY);
        } break;
        case SCREEN_GAME_OVER: {
            if (gameWon) {
                DrawTextEx(gameFont, "CONGRATULATIONS, YOU WON!", (Vector2){screenWidth/2 - MeasureTextEx(gameFont, "CONGRATULATIONS, YOU WON!", 40,1).x/2, screenHeight/2 - 80}, 40,1, GREEN);
            } else {
                DrawTextEx(gameFont, "SORRY, YOU LOSE!", (Vector2){screenWidth/2 - MeasureTextEx(gameFont, "SORRY, YOU LOSE!", 40,1).x/2, screenHeight/2 - 80}, 40,1, RED);
            }
            DrawTextEx(gameFont, TextFormat("The word was: %s", game.word), (Vector2){screenWidth/2 - MeasureTextEx(gameFont, TextFormat("The word was: %s", game.word), 30,1).x/2, screenHeight/2 - 20}, 30,1, DARKGRAY);
            DrawTextEx(gameFont, "Click or Press ENTER to Return to Menu", (Vector2){screenWidth/2 - MeasureTextEx(gameFont, "Click or Press ENTER to Return to Menu", 20,1).x/2, screenHeight/2 + 40}, 20,1, SKYBLUE);
        } break;
        default: break;
    }
    EndDrawing();
}

void DrawHangman(int triesLeft) {
    DrawLineEx((Vector2){100, 400}, (Vector2){300, 400}, 5.0f, BLACK);
    DrawLineEx((Vector2){150, 400}, (Vector2){150, 100}, 5.0f, BLACK);
    DrawLineEx((Vector2){150, 100}, (Vector2){250, 100}, 5.0f, BLACK);
    DrawLineEx((Vector2){250, 100}, (Vector2){250, 150}, 3.0f, BROWN);

    if (triesLeft <= 5) { DrawCircle(250, 175, 25, DARKGRAY); }
    if (triesLeft <= 4) { DrawLineEx((Vector2){250, 200}, (Vector2){250, 300}, 5.0f, DARKGRAY); }
    if (triesLeft <= 3) { DrawLineEx((Vector2){250, 225}, (Vector2){200, 250}, 5.0f, DARKGRAY); }
    if (triesLeft <= 2) { DrawLineEx((Vector2){250, 225}, (Vector2){300, 250}, 5.0f, DARKGRAY); }
    if (triesLeft <= 1) { DrawLineEx((Vector2){250, 300}, (Vector2){200, 350}, 5.0f, DARKGRAY); }
    if (triesLeft <= 0) {
        DrawLineEx((Vector2){250, 300}, (Vector2){300, 350}, 5.0f, DARKGRAY);
        DrawLine(240, 168, 246, 174, RED); DrawLine(240, 174, 246, 168, RED);
        DrawLine(254, 168, 260, 174, RED); DrawLine(254, 174, 260, 168, RED);
    }
}
void DrawGuessedWord(void) {
    char displayString[MAX_WORD_LENGTH * 2 + 1] = {0};
    int len = 0;
    for (int i = 0; i < game.wordLength; i++) {
        displayString[len++] = game.guessedWord[i];
        if (i < game.wordLength - 1) {
             displayString[len++] = ' ';
        }
    }
    displayString[len] = '\0';

    Vector2 textSize = MeasureTextEx(gameFont, displayString, 36, 1);
    DrawTextEx(gameFont, displayString, (Vector2){screenWidth/2.0f - textSize.x/2.0f + 100, 200}, 36, 1, BLACK);
}
void DrawAlphabet(void) {
    DrawTextEx(gameFont, "Letters Guessed:", (Vector2){50, 420}, 20, 1, DARKGRAY);
    char displayAlpha[54] = {0};
    int currentPos = 0;
    for (char c = 'a'; c <= 'z'; c++) {
        if (game.letterAlreadyGuessed[c - 'a']) {
            displayAlpha[currentPos++] = c;
            displayAlpha[currentPos++] = ' ';
        }
    }
    if (currentPos > 0) displayAlpha[currentPos-1] = '\0';

    DrawTextEx(gameFont, displayAlpha, (Vector2){50, 450}, 24, 1, MAROON);
}

void UnloadGame(void) {
    UnloadFont(gameFont);
}

void UpdateDrawFrame(void) {
    UpdateGame();
    DrawGame();
}

//    gcc tebakkata_raylib.c -o tebak_kata_game $(pkg-config --cflags --libs raylib)
//    ./tebak_kata_game