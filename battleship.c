#include<curses.h>
#include<string.h>
#include<time.h>
#include<stdlib.h>
#include<unistd.h>

typedef struct{
    int row, col;
}coord;

int attack(int **board, int **boardDiscovered, int *playerTurn, int *ShipsDown, int withSleep, int *ShipsUp);
int calculateScore(int **board, int **boardDiscovered);
int checkDiag(int **board, int **boardDiscovered, int r, int c);
int chooseMap(WINDOW *wnd, int maxrow, int maxcol, int argc, char *argv[], int **playerBoard);
void copyBoard(char nume_fis[], int **playerBoard);
int destroyInAdvance(int **playerBoard, int **computerBoard, int **playerDiscovered, int **computerDiscovered, int * pShipsDown, int * cShipsDown, int *pShipsUp, int *cShipsUp);
void drawBackground(int maxrow, int maxcol);
void drawLogo(int maxrow, int maxcol, int logoNumber);
void drawUI(WINDOW * wnd, int maxrow, int maxcol, int **playerBoard, int **computerBoard, int **playerDiscovered, int **computerDiscovered, int *pShipsDown, int *cShipsDown);
void endScreen(int win, int score, int *pShipsDown, int *cShipsDown, int maxrow, int maxcol);
void generateBoard(int **board);
int genRandNumber(int intervalStart, int intervalEnd);
void highscores(int maxrow, int maxcol);
WINDOW *initWindow(int *maxrow, int *maxcol);
void mainMenu(WINDOW *wnd, int maxrow, int maxcol, int argc, char *argv[]);
void placeShip(int **board, int row, int col, int length, int direction);
void randomizeMap(int **board, int **boardDiscovered, int *ShipsUp);
int resumeGame(WINDOW * wnd, int maxrow, int maxcol, int **playerBoard, int **computerBoard, int **playerDiscovered, int **computerDiscovered, int *pShipsDown, int *cShipsDown, int *pShipsUp, int *cShipsUp, int difficulty);
void saveScore(int score, char *name);
int scanField(int **board, int row, int col, int length, int direction);
void sleepOwn(int seconds);
int smartAttack(int **board, int **boardDiscovered, int *playerTurn, int *ShipsDown, int *ShipsUp, int *hit, coord *lastHit);
int startNewGame(WINDOW *wnd, int maxrow, int maxcol, int **playerBoard, int **computerBoard, int **playerDiscovered, int **computerDiscovered, int *pShipsDown, int *cShipsDown, int *pShipsUp, int *cShipsUp, int difficulty);
int takeDown(int row, int col, int **board, int **boardDiscovered, int precRow, int precCol);
int waitForInput(void);
int wasTakenDown(int row, int col, int **board, int **boardDiscovered, int precRow, int precCol);

int main(int argc, char *argv[]) {
    int maxrow, maxcol;
    WINDOW *wnd;

    srand(time(0));
    if (argc <= 1) {
        printf("[Eroare]: Nu s-au dat argumente de comanda.");
        return 1;
    }
    wnd = initWindow(&maxrow, &maxcol);
    mainMenu(wnd, maxrow, maxcol, argc, argv);
    endwin();
    return 1;
}

int attack(int **board, int **boardDiscovered, int *playerTurn, int *ShipsDown, int withSleep, int *ShipsUp) { //functie de generare a unui atac random pe harta
    int row, col;

    row = genRandNumber(1, 10);
    col = genRandNumber(1, 10);
    while (boardDiscovered[row][col]) { //se cauta un punct nedescoperit
        row = genRandNumber(1, 10);
        col = genRandNumber(1, 10);
    }
    boardDiscovered[row][col] = 1;
    if (board[row][col] == 0)
        *playerTurn = 1;
    if (wasTakenDown(row, col, board, boardDiscovered, -1, -1) == 1 && board[row][col] == 1) {
        (*ShipsDown)++;
        ShipsUp[takeDown(row, col, board, boardDiscovered, -1, -1)]--;
    }
    if (withSleep)
        sleepOwn(1);
    return board[row][col]; //reurneaza daca punctul atacat este o barca sau nu
}

int calculateScore(int **board, int **boardDiscovered) { //calculeaza scorul player-ului
    int i, j, score = 0;

    for (i = 1; i <= 10; i++)
        for (j = 1; j <= 10; j++) {
            if (boardDiscovered[i][j]) {
                if (board[i][j] == 0)
                    score -= 100; //un spatiu gol descoperit scarde 100 de puncte
                else
                    score += 450; //o barca nimerita creste scorul cu 450 de puncte
            }
        }
    return score;
}

int checkDiag(int **board, int **boardDiscovered, int r, int c) {
    if(board[r + 1][c + 1] != 0 && boardDiscovered[r + 1][c + 1] == 1)
        return 0;
    if(board[r - 1][c - 1] != 0 && boardDiscovered[r - 1][c - 1] == 1)
        return 0;
    if(board[r + 1][c - 1] != 0 && boardDiscovered[r + 1][c - 1] == 1)
        return 0;
    if(board[r - 1][c + 1] != 0 && boardDiscovered[r - 1][c + 1] == 1)
        return 0;
    if(board[r + 1][c] == 2 && boardDiscovered[r + 1][c] == 1)
        return 0;
    if(board[r - 1][c] == 2 && boardDiscovered[r - 1][c] == 1)
        return 0;
    if(board[r][c + 1] == 2 && boardDiscovered[r][c + 1] == 1)
        return 0;
    if(board[r][c - 1] == 2 && boardDiscovered[r][c - 1] == 1)
        return 0;
    return 1;
}

int chooseMap(WINDOW *wnd, int maxrow, int maxcol, int argc, char *argv[], int **playerBoard) { //meniul de ales al hartii
    int index = 1, chosen = 0, i, j, keyPressed = 0, canBeChosen, difficulty = 0;
    char *line, *chooseMapText, *difficultyText, *aux;
    FILE *map;

    chooseMapText = malloc(30 * sizeof(char));
    difficultyText = malloc(30 * sizeof(char));
    strcpy(chooseMapText, "Choose your map:");
    while (!chosen) {
        drawBackground(maxrow, maxcol);
        if(!difficulty)
            sprintf(difficultyText, "Difficulty: EASY");
        else
            sprintf(difficultyText, "Difficulty: HARD");
        attron(COLOR_PAIR(5));
        mvprintw(maxrow / 2 - 8, maxcol / 2 - strlen(chooseMapText) / 2, chooseMapText);
        mvprintw(maxrow / 2 + 7, maxcol / 2 - strlen(difficultyText) / 2, difficultyText);
        mvprintw(maxrow-1, 0, "Side Arrows - Navigate maps | Up and Down Arrows - Change difficulty");
        map = fopen(argv[index], "r");
        if(map == NULL) { //se verifica daca fisierul-harta exista
            aux = malloc(30 * sizeof(char));
            sprintf(aux, "Map %d can't be opened.", index);
            mvprintw(maxrow / 2, maxcol / 2 - strlen(aux) / 2, aux);
            free(aux);
            canBeChosen = 0;
        } else {
            line = malloc(15 * sizeof(char));
            for (i = 1; i <= 10; i++) { //daca se poate deschide se afiseaza harta pe ecran
                fgets(line, 15, map);
                for (j = 0; j < 10; j++)
                    if (line[j] == '.') {
                        attron(COLOR_PAIR(1));
                        mvaddch(maxrow / 2 + i - 6, maxcol / 2 + 2 * j - 10, ' ');
                        mvaddch(maxrow / 2 + i - 6, maxcol / 2 + 2 * j + 1 - 10, ' ');
                    }
                else {
                    attron(COLOR_PAIR(4));
                    mvaddch(maxrow / 2 + i - 6, maxcol / 2 + 2 * j - 10, ' ');
                    mvaddch(maxrow / 2 + i - 6, maxcol / 2 + 2 * j + 1 - 10, ' ');
                }
            }
            free(line);
            canBeChosen = 1;
            fclose(map);
        }
        keyPressed = waitForInput();
        switch (keyPressed) { //decizie in functie de tasta apasata
        case 1: //Up-Arrow
            difficulty = 1 - difficulty;
            break;
        case 2: //Down-Arrow
            difficulty = 1 - difficulty;
            break;
        case 3: //Right-Arrow
            if (index < argc - 1)
                index++;
            break;
        case 4: //Left-Arrow
            if (index > 1)
                index--;
            break;
        case 5: //ENTER
            if(canBeChosen){ //daca harta poate fi deschisa se copiaza in memorie
                chosen = 1;
                copyBoard(argv[index], playerBoard);
            }
            break;
        }
    }
    free(chooseMapText);
    free(difficultyText);
    return difficulty;
}

void copyBoard(char nume_fis[], int **playerBoard) { //se copiaza tabla de joc dintr-un fisier
    char *line;
    int i, j;
    FILE *map;

    line = malloc(15 * sizeof(char));
    map = fopen(nume_fis, "r");
    for (i = 0; i <= 11; i++)
        for (j = 0; j <= 11; j++)
            playerBoard[i][j] = 0;
    for (i = 1; i <= 10; i++) {
        fgets(line, 15, map);
        for (j = 0; j < 10; j++)
            if (line[j] == '.') {
                playerBoard[i][j + 1] = 0;
            }
        else {
            playerBoard[i][j + 1] = 1;
        }
    }
    free(line);
    fclose(map);
}

int destroyInAdvance(int **playerBoard, int **computerBoard, int **playerDiscovered, int **computerDiscovered, int * pShipsDown, int * cShipsDown, int *pShipsUp, int *cShipsUp) { //functie care joaca 10 ture in avans
    int i, spare;

    for (i = 1; i <= 10; i++) {
        while (attack(computerBoard, computerDiscovered, & spare, cShipsDown, 0, cShipsUp));
        if ((*cShipsDown) >= 10)
            return 1; // player wins
        while (attack(playerBoard, playerDiscovered, & spare, pShipsDown, 0, pShipsUp));
        if ((*pShipsDown) >= 10)
            return 2; // computer wins
    }
    return 0; // no win
}

void drawBackground(int maxrow, int maxcol) { //deseneaza fundalul cu culoarea gri
    int i, j;

    attron(COLOR_PAIR(2));
    for (i = 0; i < maxrow; i++)
        for (j = 0; j < maxcol; j++)
            mvaddch(i, j, ' ');
}

void drawLogo(int maxrow, int maxcol, int logoNumber) { //deseneaza logo-ul din meniul principal
    FILE *logo;
    char *line;
    int colStart, rowStart = 3, i, j = 0;

    line = malloc(100 * sizeof(char));
    logo = fopen("logo.in", "r"); //se foloseste un fisier in care exista o imagine a logo-ului
    colStart = (maxcol - 39) / 2;
    if(logoNumber == 0) {
        attron(COLOR_PAIR(3));
        while (fgets(line, 45, logo) != NULL && j <= 6) {
            line = realloc(line, (strlen(line)+1) * sizeof(char));
            for (i = 0; i < strlen(line); i++)
                if (line[i] == '@')
                    mvaddch(rowStart + j, colStart + i, ' ');
            j++;
            line = realloc(line, 100 * sizeof(char));
        }
    } else {
        attron(COLOR_PAIR(3));
        for(i = 0; i <= 7; i++)
            fgets(line, 45, logo);
        while (fgets(line, 45, logo) != NULL) {
            line = realloc(line, (strlen(line)+1) * sizeof(char));
            for (i = 0; i < strlen(line); i++)
                if (line[i] == '@')
                    mvaddch(rowStart + j, colStart + i, ' ');
            j++;
            line = realloc(line, 100 * sizeof(char));
        }
    }
    free(line);
}

void drawUI(WINDOW * wnd, int maxrow, int maxcol, int **playerBoard, int **computerBoard, int **playerDiscovered, int **computerDiscovered, int *pShipsDown, int *cShipsDown) { //functie care se ocupa de desenarea UI-ului
    int playerCornerRow, playerCornerCol, computerCornerRow, computerCornerCol, i, j;

    playerCornerRow = computerCornerRow = (maxrow - 10) / 2;
    playerCornerCol = (maxcol / 2 - 20) / 2;
    computerCornerCol = maxcol / 2 + (maxcol / 2 - 20) / 2;
    drawBackground(maxrow, maxcol);
    for (i = 0; i <= 11; i++) { // deseneaza marginile tablei de joc
        attron(COLOR_PAIR(3));
        mvaddch(playerCornerRow + i - 1, playerCornerCol - 1, ' ');
        mvaddch(computerCornerRow + i - 1, computerCornerCol - 1, ' ');
        mvaddch(playerCornerRow + i - 1, playerCornerCol + 20, ' ');
        mvaddch(computerCornerRow + i - 1, computerCornerCol + 20, ' ');
        mvaddch(playerCornerRow + i - 1, playerCornerCol - 2, ' ');
        mvaddch(computerCornerRow + i - 1, computerCornerCol - 2, ' ');
        mvaddch(playerCornerRow + i - 1, playerCornerCol + 21, ' ');
        mvaddch(computerCornerRow + i - 1, computerCornerCol + 21, ' ');

        mvaddch(playerCornerRow - 1, playerCornerCol - 2 + 2 * i, ' ');
        mvaddch(playerCornerRow - 1, playerCornerCol - 2 + 2 * i + 1, ' ');
        mvaddch(computerCornerRow - 1, computerCornerCol - 2 + 2 * i, ' ');
        mvaddch(computerCornerRow - 1, computerCornerCol - 2 + 2 * i + 1, ' ');
        mvaddch(playerCornerRow + 10, playerCornerCol - 2 + 2 * i, ' ');
        mvaddch(playerCornerRow + 10, playerCornerCol - 2 + 2 * i + 1, ' ');
        mvaddch(computerCornerRow + 10, computerCornerCol - 2 + 2 * i, ' ');
        mvaddch(computerCornerRow + 10, computerCornerCol - 2 + 2 * i + 1, ' ');
    }
    for (i = 0; i <= 9; i++) // deseneaza tabla de joc (barci si spatii libere)
        for (j = 0; j <= 19; j++) {
            if (playerBoard[i + 1][j / 2 + 1] == 0) {
                if (playerDiscovered[i + 1][j / 2 + 1]) {
                    attron(COLOR_PAIR(1));
                    if (j % 2 == 0)
                        mvaddch(playerCornerRow + i, playerCornerCol + j, '>');
                    else
                        mvaddch(playerCornerRow + i, playerCornerCol + j, '<');
                } else {
                    attron(COLOR_PAIR(1));
                    mvaddch(playerCornerRow + i, playerCornerCol + j, ' ');
                }

            } else if (playerBoard[i + 1][j / 2 + 1] == 1) {
                if (playerDiscovered[i + 1][j / 2 + 1]) {
                    attron(COLOR_PAIR(4));
                    if (j % 2 == 0)
                        mvaddch(playerCornerRow + i, playerCornerCol + j, '>');
                    else
                        mvaddch(playerCornerRow + i, playerCornerCol + j, '<');
                } else {
                    attron(COLOR_PAIR(4));
                    mvaddch(playerCornerRow + i, playerCornerCol + j, ' ');
                }

            } else if (playerBoard[i + 1][j / 2 + 1] == 2) {
                attron(COLOR_PAIR(4));
                if (j % 2 == 0)
                    mvaddch(playerCornerRow + i, playerCornerCol + j, '[');
                else
                    mvaddch(playerCornerRow + i, playerCornerCol + j, ']');
            }

            if (computerDiscovered[i + 1][j / 2 + 1] == 1) {
                if (computerBoard[i + 1][j / 2 + 1] == 0) {
                    attron(COLOR_PAIR(1));
                    mvaddch(computerCornerRow + i, computerCornerCol + j, ' ');
                } else if (computerBoard[i + 1][j / 2 + 1] == 1) {
                    attron(COLOR_PAIR(4));
                    mvaddch(computerCornerRow + i, computerCornerCol + j, ' ');
                } else if (computerBoard[i + 1][j / 2 + 1] == 2) {
                    attron(COLOR_PAIR(4));
                    if (j % 2 == 0)
                        mvaddch(computerCornerRow + i, computerCornerCol + j, '[');
                    else
                        mvaddch(computerCornerRow + i, computerCornerCol + j, ']');
                }
            } else {
                attron(COLOR_PAIR(2));
                mvaddch(computerCornerRow + i, computerCornerCol + j, ' ');
            }
        }
    attron(COLOR_PAIR(5)); //parte in care se deseneaza textele de pe ecran
    mvprintw(computerCornerRow - 3, computerCornerCol - 2, "Computer ships taken down: ");
    if (*cShipsDown < 10)
        mvaddch(computerCornerRow - 3, computerCornerCol - 2 + strlen("Computer ships taken down: "), (*cShipsDown) + '0');
    else
        mvprintw(computerCornerRow - 3, computerCornerCol - 2 + strlen("Computer ships taken down: "), "10");
    mvprintw(playerCornerRow - 3, playerCornerCol - 2, "Player ships taken down: ");
    if ((*pShipsDown) < 10)
        mvaddch(playerCornerRow - 3, playerCornerCol - 2 + strlen("Player ships taken down: "), (*pShipsDown) + '0');
    else
        mvprintw(playerCornerRow - 3, playerCornerCol - 2 + strlen("Player ships taken down: "), "10");
    mvprintw(maxrow - 1, 0, "Arrows - Move cursor | Enter - Select | Q - Pause | D - Destroy in advance | R - Randomize map");
    move(0, 0);
}

void endScreen(int win, int score, int *pShipsDown, int *cShipsDown, int maxrow, int maxcol) { //functie care deseneaza ecranul de final de joc
    int midrow, midcol, keyPressed = 0, currentLetter = 1;
    char *scoreText, *letter, *pDown, *cDown;

    scoreText = malloc(30 * sizeof(char));
    pDown = malloc(30 * sizeof(char));
    cDown = malloc(30 * sizeof(char));
    letter = malloc(5 * sizeof(char));
    letter[1] = letter[2] = letter[3] = 'A';
    letter[4] = '\0';
    sprintf(scoreText, "Player score: %d", score);
    sprintf(pDown, "Player ships taken down: %d", *pShipsDown);
    sprintf(cDown, "Computer ships taken down: %d", *cShipsDown);
    midrow = maxrow / 2;
    midcol = maxcol / 2;
    while (keyPressed != 5) {
        drawBackground(maxrow, maxcol);
        attron(COLOR_PAIR(1));
        if (win == 1) //afiseaza text in functie de castigator
            mvprintw(midrow - 6, midcol - strlen("Player won!") / 2, "Player won!");
        else
            mvprintw(midrow - 6, midcol - strlen("Computer won!") / 2, "Computer won!");
        attron(COLOR_PAIR(5));
        mvprintw(maxrow - 1, 0, "Side arrows - Navigate letters | Up and Down arrows - Change letter");
        mvprintw(midrow - 5, midcol - strlen(scoreText) / 2, scoreText);
        mvprintw(midrow - 3, midcol - strlen(pDown) / 2, pDown);
        mvprintw(midrow - 2, midcol - strlen(cDown) / 2, cDown);
        mvprintw(midrow, midcol - strlen("Please enter your name: ") / 2, "Please enter your name: ");
        switch (keyPressed) { //decizie in functie de tasta apasata
        case 1: //Up-Arrow
            if (letter[currentLetter] == 'A')
                letter[currentLetter] = 'Z';
            else
                letter[currentLetter] = (((letter[currentLetter] - 'A') - 1) % 26) + 'A';
            break;
        case 2: //Down-Arrow
            letter[currentLetter] = (((letter[currentLetter] - 'A') + 1) % 26) + 'A';
            break;
        case 3: //Right-Arrow
            if (currentLetter < 3)
                currentLetter++;
            break;
        case 4: //Left-Arrow
            if (currentLetter > 1)
                currentLetter--;
            break;
        }
        mvaddch(midrow + 1, midcol - 2, letter[1]);
        mvaddch(midrow + 1, midcol, letter[2]);
        mvaddch(midrow + 1, midcol + 2, letter[3]);
        move(midrow + 1, midcol - 4 + 2 * currentLetter);
        if (keyPressed != 0)
            keyPressed = waitForInput();
        else {
            keyPressed = -1;
        }
    }
    free(scoreText);
    saveScore(score, letter + 1); //salvam scorul actual
    free(letter);
}

void generateBoard(int **board) { //genereaza o tabla de joc valida in mod aleatoriu
    int i, j, row, col, direction;

    for (i = 4; i >= 1; i--) {
        for (j = 1; j <= 5 - i; j++) {
            row = genRandNumber(1, 10);
            col = genRandNumber(1, 10);
            direction = genRandNumber(1, 4);
            while (!scanField(board, row, col, i, direction)) {
                row = genRandNumber(1, 10);
                col = genRandNumber(1, 10);
                direction = genRandNumber(1, 4);
            }
            placeShip(board, row, col, i, direction);
        }
    }
}

int genRandNumber(int intervalStart, int intervalEnd) { //genereaza un int din intervalul inclus specificat
    int result, swp;

    if (intervalEnd < intervalStart) {
        swp = intervalEnd;
        intervalEnd = intervalStart;
        intervalStart = swp;
    }
    result = rand();
    result = (result % (intervalEnd - intervalStart + 1)) + intervalStart;
    return result;
}

void highscores(int maxrow, int maxcol) { //un meniu care afiseaza highscore-urile
    char *line, *score, keyPressed = 0;
    FILE *scores;
    int i = maxrow / 2 - 4;

    line = malloc(10 * sizeof(char));
    score = malloc(20 * sizeof(char));
    drawBackground(maxrow, maxcol);
    drawLogo(maxrow, maxcol, 1);
    scores = fopen("scores", "r");
    attron(COLOR_PAIR(5));
    mvprintw(maxrow - 1, 0, "ENTER - Continue");
    while (fgets(line, 10, scores) && (i - maxrow / 2 + 5 <= 5)) {
        line[strlen(line) - 1] = '\0';
        sprintf(score, "%d. ", i - maxrow / 2 + 5);
        strcat(score, line);
        strcat(score, "pts");
        mvprintw(i, maxcol / 2 - strlen(score) / 2, score);
        i++;
    }
    free(line);
    free(score);
    fclose(scores);
    move(0, 0);
    while (keyPressed != 5)
        keyPressed = waitForInput();

}

WINDOW *initWindow(int *maxrow, int *maxcol) { //initializeaza fereastra gasindu-i limitele
    WINDOW *wnd;

    wnd = initscr();
    cbreak();
    noecho();
    getmaxyx(wnd, *maxrow, *maxcol);
    clear();
    refresh();
    if (has_colors() != FALSE) {
        start_color();
        init_pair(1, COLOR_RED, COLOR_WHITE); //MAIN MENU TEXT
        init_pair(2, COLOR_RED, 8); //BACKGROUND
        init_pair(3, COLOR_WHITE, 12); //LOGO
        init_pair(4, COLOR_RED, COLOR_BLACK); //SHIPS
        init_pair(5, COLOR_WHITE, 8); //GAME TEXT
    }
    return wnd;
}

void mainMenu(WINDOW *wnd, int maxrow, int maxcol, int argc, char *argv[]) { //meniul principal
    int **playerBoard, **computerBoard, **playerDiscovered, **computerDiscovered, pShipsDown = 0, cShipsDown = 0;
    int keyPressed = 0, i, buttonRow = 1, buttonNumber = 4, quit = 0, gameStarted = 0;
    int *pShipsUp, *cShipsUp, difficulty;
    char **buttonText;

    //alocarea memoriei
    playerBoard = malloc(12 * sizeof(int *));
    computerBoard = malloc(12 * sizeof(int *));
    playerDiscovered = malloc(12 * sizeof(int *));
    computerDiscovered = malloc(12 * sizeof(int *));
    for(i = 0; i < 12; i++) {
        playerBoard[i] = malloc(12 * sizeof(int));
        computerBoard[i] = malloc(12 * sizeof(int));
        playerDiscovered[i] = malloc(12 * sizeof(int));
        computerDiscovered[i] = malloc(12 * sizeof(int));
    }
    pShipsUp = malloc(5 * sizeof(int));
    cShipsUp = malloc(5 * sizeof(int));
    buttonText = malloc(5 * sizeof(char *));
    for(i = 0; i < 5; i++) {
        buttonText[i] = calloc(15, sizeof(char));
    }

    strcpy(buttonText[1], "New Game");
    strcpy(buttonText[2], "Continue Game");
    strcpy(buttonText[3], "Scores");
    strcpy(buttonText[4], "Quit");
    for(i = 0; i < 5; i++) {
        buttonText[i] = realloc(buttonText[i], (strlen(buttonText[i]) + 1) * sizeof(char));
    }
    while (!quit) {
        if (keyPressed != 0) {
            keyPressed = waitForInput();
        } else {
            keyPressed = -1;
            drawBackground(maxrow, maxcol);
            drawLogo(maxrow, maxcol, 0);
        }
        if (keyPressed == 1 && buttonRow > 1)
            buttonRow--;
        if (keyPressed == 2 && buttonRow < buttonNumber)
            buttonRow++;
        for (i = 1; i <= buttonNumber; i++) {
            attron(COLOR_PAIR(1));
            if (buttonRow == i)
                mvprintw((maxrow / 2) - (buttonNumber / 2) + i - 1, maxcol / 2 + 1 - strlen(buttonText[i]) - 1, ">");
            else {
                attron(COLOR_PAIR(2));
                mvprintw((maxrow / 2) - (buttonNumber / 2) + i - 1, maxcol / 2 + 1 - strlen(buttonText[i]) - 1, " ");
                attron(COLOR_PAIR(1));
            }
            if (i == 2 && !gameStarted)
                attron(COLOR_PAIR(5));
            mvprintw((maxrow / 2) - (buttonNumber / 2) + i - 1, maxcol / 2 + 1 - strlen(buttonText[i]), buttonText[i]);
        }
        move((maxrow / 2) - (buttonNumber / 2) + buttonRow - 1, maxcol / 2 + 1 - strlen(buttonText[buttonRow]) - 1);
        if (keyPressed == 5) {
            switch (buttonRow) {
            case 1: //NEW GAME
                difficulty = chooseMap(wnd, maxrow, maxcol, argc, argv, playerBoard);
                gameStarted = startNewGame(wnd, maxrow, maxcol, playerBoard, computerBoard, playerDiscovered, computerDiscovered, &pShipsDown, &cShipsDown, pShipsUp, cShipsUp, difficulty);
                keyPressed = 0;
                break;
            case 2: //CONTIUNE GAME
                if (gameStarted)
                    gameStarted = resumeGame(wnd, maxrow, maxcol, playerBoard, computerBoard, playerDiscovered, computerDiscovered, &pShipsDown, &cShipsDown, pShipsUp, cShipsUp, difficulty);
                keyPressed = 0;
                break;
            case 3: //SCORES
                highscores(maxrow, maxcol);
                keyPressed = 0;
                break;
            case 4: //QUIT
                quit = 1;
                break;
            }
        }
    }
    //eliberarea memoriei
    for(i = 0; i < 12; i++) {
        free(playerBoard[i]);
        free(computerBoard[i]);
        free(playerDiscovered[i]);
        free(computerDiscovered[i]);
    }
    free(playerBoard);
    free(computerBoard);
    free(playerDiscovered);
    free(computerDiscovered);
    free(pShipsUp);
    free(cShipsUp);
    for(i = 0; i < 5; i++) {
        free(buttonText[i]);
    }
    free(buttonText);
}

void placeShip(int **board, int row, int col, int length, int direction) { //plaseaza nava pe tabla
    int i;

    switch (direction) { //se tine cont de directie si de punctul de plecare
    case 1:
        for (i = 0; i < length; i++)
            board[row - i][col] = 1;
        break;
    case 2:
        for (i = 0; i < length; i++)
            board[row + i][col] = 1;
        break;
    case 3:
        for (i = 0; i < length; i++)
            board[row][col + i] = 1;
        break;
    case 4:
        for (i = 0; i < length; i++)
            board[row][col - i] = 1;
        break;
    }
}

void randomizeMap(int **board, int **boardDiscovered, int *ShipsUp) { //amesteca aleator navele care nu au fost distruse complet
    int i, j, row, col, direction;

    for (i = 1; i <= 10; i++)
        for (j = 1; j <= 10; j++) {
            if (board[i][j] != 2) {
                boardDiscovered[i][j] = 0;
                board[i][j] = 0;
            }
        }
    for (i = 4; i >= 1; i--) {
        for (j = 1; j <= ShipsUp[i]; j++) {
            row = genRandNumber(1, 10);
            col = genRandNumber(1, 10);
            direction = genRandNumber(1, 4);
            while (!scanField(board, row, col, i, direction)) {
                row = genRandNumber(1, 10);
                col = genRandNumber(1, 10);
                direction = genRandNumber(1, 4);
            }
            placeShip(board, row, col, i, direction);
        }
    }
}

int resumeGame(WINDOW * wnd, int maxrow, int maxcol, int **playerBoard, int **computerBoard, int **playerDiscovered, int **computerDiscovered, int *pShipsDown, int *cShipsDown, int *pShipsUp, int *cShipsUp, int difficulty) { //functia principala a jocului
    int win = 0, pause = 0, keyPressed = 0, hit = 0;
    int playerTurn = 1, cursorRow = 1, cursorCol = 1;
    coord lastHit;

    clear();
    drawBackground(maxrow, maxcol); //deseneaza fundalul
    while (!win && !pause) {
        drawUI(wnd, maxrow, maxcol, playerBoard, computerBoard, playerDiscovered, computerDiscovered, pShipsDown, cShipsDown); //deseneaza UI-ul
        if (playerTurn) {
            attron(COLOR_PAIR(5));
            mvprintw(maxrow / 4, maxcol / 2 - strlen("Player is choosing...") / 2, "Player is choosing...");
            switch (keyPressed) { //decizie in functie de tasta apasata
            case 1: //Up-Arrow
                if (cursorRow > 1)
                    cursorRow--;
                break;
            case 2: //Down-Arrow
                if (cursorRow < 10)
                    cursorRow++;
                break;
            case 3: //Right-Arrow
                if (cursorCol < 10)
                    cursorCol++;
                break;
            case 4: //Left-Arrow
                if (cursorCol > 1)
                    cursorCol--;
                break;
            case 5: //ENTER
                if (computerDiscovered[cursorRow][cursorCol] == 0) {
                    if (computerBoard[cursorRow][cursorCol] == 0)
                        playerTurn = 0;
                    computerDiscovered[cursorRow][cursorCol] = 1;
                    if (wasTakenDown(cursorRow, cursorCol, computerBoard, computerDiscovered, -1, -1) == 1 && computerBoard[cursorRow][cursorCol] == 1) {
                        (*cShipsDown) ++;
                        cShipsUp[takeDown(cursorRow, cursorCol, computerBoard, computerDiscovered, -1, -1)]--;
                    }
                    if ((*cShipsDown) >= 10)
                        win = 1;
                    keyPressed = 0;
                }
                break;
            case 6: //Q
                keyPressed = 0;
                pause = 1;
                break;
            case 7: //D
                keyPressed = 0;
                win = destroyInAdvance(playerBoard, computerBoard, playerDiscovered, computerDiscovered, pShipsDown, cShipsDown, pShipsUp, cShipsUp);
                break;
            case 8: //R
                keyPressed = 0;
                randomizeMap(playerBoard, playerDiscovered, pShipsUp);
                randomizeMap(computerBoard, computerDiscovered, cShipsUp);
                break;
            }
            move((maxrow - 10) / 2 + cursorRow - 1, maxcol / 2 + (maxcol / 2 - 20) / 2 + cursorCol * 2 - 2);
        } else {
            attron(COLOR_PAIR(5));
            mvprintw(maxrow / 4, maxcol / 2 - strlen("Computer is choosing...") / 2, "Computer is choosing...");
            refresh();
            if(!difficulty)
                attack(playerBoard, playerDiscovered, &playerTurn, pShipsDown, 1, pShipsUp);
            else
                smartAttack(playerBoard, playerDiscovered, &playerTurn, pShipsDown, pShipsUp, &hit, &lastHit);
            if ((*pShipsDown) >= 10)
                win = 2;
            keyPressed = 0;
        }
        if (keyPressed != 0)
            keyPressed = waitForInput();
        else {
            keyPressed = -1;
        }
    }
    if (win) { //daca a gastigat cineva se afiseaza ecranul de sfarsit
        drawUI(wnd, maxrow, maxcol, playerBoard, computerBoard, playerDiscovered, computerDiscovered, pShipsDown, cShipsDown);
        refresh();
        sleepOwn(3);
        endScreen(win, calculateScore(computerBoard, computerDiscovered), pShipsDown, cShipsDown, maxrow, maxcol);
        return 0;
    }
    return 1;
}

void saveScore(int score, char *name) { //functie care salveaza scorul
    int i, lim, bec = 1;
    FILE *scores;
    char **text, *actualScore;

    actualScore = malloc(15 * sizeof(char));
    text = malloc(7 * sizeof(char *));
    for(i = 0; i < 7; i++)
        text[i] = malloc(11 * sizeof(char));
    i = 1;
    scores = fopen("scores", "r");
    while (fgets(text[i], 10, scores) && i <= 5) { //se citesc scorurile existente
        text[i][strlen(text[i]) - 1] = '\0';
        text[i] = realloc(text[i], (strlen(text[i]) + 1) * sizeof(char));
        i++;
    }
    lim = i - 1;
    fclose(scores);
    scores = fopen("scores", "w");
    for (i = 1; i <= lim; i++) {
        if (atoi(text[i] + 4) < score && bec) { //se verifica daca scorul actual poate intra in highscores
            sprintf(actualScore, "%s %d", name, score);
            fprintf(scores, "%s\n", actualScore);
            bec = 0;
        }
        fprintf(scores, "%s\n", text[i]);
    }
    if (bec == 1) {
        sprintf(actualScore, "%s %d", name, score);
        fprintf(scores, "%s\n", actualScore);
        bec = 0;
    }
    fclose(scores);

    for(i = 0; i < 7; i++)
        free(text[i]);
    free(text);
    free(actualScore);
}

int scanField(int **board, int row, int col, int length, int direction) { //scaneaza terenul si spune daca o barca poate fii plasata in acel loc
    int i, j;

    switch (direction) { //un caz pentru fiecare directie (N,S,E,W)
    case 1:
        for (i = -1; i <= length; i++) {
            for (j = -1; j <= 1; j++) {
                if (board[row - i][col + j] != 0)
                    return 0;
                if (row - i < 1 || row - i > 10)
                    return 0;
            }

        }
        break;
    case 2:
        for (i = -1; i <= length; i++) {
            for (j = -1; j <= 1; j++) {
                if (board[row + i][col + j] != 0)
                    return 0;
                if (row + i < 1 || row + i > 10)
                    return 0;
            }

        }
        break;
    case 3:
        for (i = -1; i <= length; i++) {
            for (j = -1; j <= 1; j++) {
                if (board[row + j][col + i] != 0)
                    return 0;
                if (col + i < 1 || col + i > 10)
                    return 0;
            }

        }
        break;
    case 4:
        for (i = -1; i <= length; i++) {
            for (j = -1; j <= 1; j++) {
                if (board[row + j][col - i] != 0)
                    return 0;
                if (col - i < 1 || col - i > 10)
                    return 0;
            }

        }
        break;
    }
    return 1;
}

void sleepOwn(int seconds) { //functie de sleep
    time_t a;

    a = time(NULL);
    while (time(NULL) - a < seconds);
}

int smartAttack(int **board, int **boardDiscovered, int *playerTurn, int *ShipsDown, int *ShipsUp, int *hit, coord *lastHit) { //functie de generare a unui atac inteligent pe harta
    int r, c;
    if (*hit) {
        if (!boardDiscovered[lastHit -> row + 1][lastHit -> col] && (lastHit -> row) + 1 <= 10 && checkDiag(board, boardDiscovered, (lastHit -> row) + 1, lastHit -> col)) {
            r = (lastHit -> row) + 1;
            c = lastHit -> col;
        } else if (!boardDiscovered[lastHit -> row - 1][lastHit -> col] && (lastHit -> row) - 1 >= 1 && checkDiag(board, boardDiscovered, (lastHit -> row) - 1, lastHit -> col)) {
            r = (lastHit -> row) - 1;
            c = lastHit -> col;
        } else if (!boardDiscovered[lastHit -> row][lastHit -> col + 1] && (lastHit -> col) + 1 <= 10 && checkDiag(board, boardDiscovered, lastHit -> row, (lastHit -> col) + 1)) {
            r = lastHit -> row;
            c = (lastHit -> col) + 1;
        } else if (!boardDiscovered[lastHit -> row][lastHit -> col - 1] && (lastHit -> col) - 1 >= 1 && checkDiag(board, boardDiscovered, lastHit -> row, (lastHit -> col) - 1)) {
            r = lastHit -> row;
            c = (lastHit -> col) - 1;
        } else {
            *hit = 0;
            r = genRandNumber(1, 10);
            c = genRandNumber(1, 10);
            while (boardDiscovered[r][c] || !checkDiag(board, boardDiscovered, r, c)) { //se cauta un punct nedescoperit
                r = genRandNumber(1, 10);
                c = genRandNumber(1, 10);
            }
        }
    } else {
        r = genRandNumber(1, 10);
        c = genRandNumber(1, 10);
        while (boardDiscovered[r][c] || !checkDiag(board, boardDiscovered, r, c)) { //se cauta un punct nedescoperit
            r = genRandNumber(1, 10);
            c = genRandNumber(1, 10);
        }
    }
    boardDiscovered[r][c] = 1;
    if (board[r][c] == 0)
        *playerTurn = 1;
    else {
        *hit = 1;
        lastHit -> row = r;
        lastHit -> col = c;
    }
    if (wasTakenDown(r, c, board, boardDiscovered, -1, -1) == 1 && board[r][c] == 1) {
        (*ShipsDown)++;
        ShipsUp[takeDown(r, c, board, boardDiscovered, -1, -1)]--;
        *hit = 0;
    }
    sleepOwn(1);
    return board[r][c]; //reurneaza daca punctul atacat este o barca sau nu
}

int startNewGame(WINDOW *wnd, int maxrow, int maxcol, int **playerBoard, int **computerBoard, int **playerDiscovered, int **computerDiscovered, int *pShipsDown, int *cShipsDown, int *pShipsUp, int *cShipsUp, int difficulty) { //functie care se apeleaza doar la inceputul unui joc
    int i, j;

    for (i = 0; i <= 11; i++) //se initializeaza tablele de joc
        for (j = 0; j <= 11; j++) {
            computerBoard[i][j] = 0;
            playerDiscovered[i][j] = 0;
            computerDiscovered[i][j] = 0;
        }
    (*pShipsDown) = (*cShipsDown) = 0;
    pShipsUp[1] = cShipsUp[1] = 4;
    pShipsUp[2] = cShipsUp[2] = 3;
    pShipsUp[3] = cShipsUp[3] = 2;
    pShipsUp[4] = cShipsUp[4] = 1;
    generateBoard(computerBoard); //se genereaza tabla de joc a computer-ului random
    return resumeGame(wnd, maxrow, maxcol, playerBoard, computerBoard, playerDiscovered, computerDiscovered, pShipsDown, cShipsDown, pShipsUp, cShipsUp, difficulty); //se da resume jocului de-abia initializat
}

int takeDown(int row, int col, int **board, int **boardDiscovered, int precRow, int precCol) { //distruge barca si returneaza lungimea ei
    int a = 0, b = 0, c = 0, d = 0;

    if (board[row][col] == 0)
        return 0;
    if (board[row][col] == 1 && boardDiscovered[row][col] == 0)
        return 0;
    board[row][col] = 2;
    if (row + 1 != precRow || col != precCol)
        a = takeDown(row + 1, col, board, boardDiscovered, row, col);
    if (row - 1 != precRow || col != precCol)
        b = takeDown(row - 1, col, board, boardDiscovered, row, col);
    if (row != precRow || col + 1 != precCol)
        c = takeDown(row, col + 1, board, boardDiscovered, row, col);
    if (row != precRow || col - 1 != precCol)
        d = takeDown(row, col - 1, board, boardDiscovered, row, col);
    return 1 + a + b + c + d;
}

int waitForInput(void) { //functia de citire a inputului
    char c;

    c = getch();
    if (c == 27) {
        c = getch();
        if (c == 91) {
            c = getch();
            switch (c) {
            case 65:
                return 1; //Up-Arrow
                break;
            case 66:
                return 2; //Down-Arrow
                break;
            case 67:
                return 3; //Right-Arrow
                break;
            case 68:
                return 4; //Left-Arrow
                break;
            }
        }
    } else if (c == 10)
        return 5; //ENTER
    else if (c == 81 || c == 113)
        return 6; //Q
    else if (c == 68 || c == 100)
        return 7; //D
    else if (c == 82 || c == 114)
        return 8; //R
    return 0;
}

int wasTakenDown(int row, int col, int **board, int **boardDiscovered, int precRow, int precCol) { //functie recursiva care verifica daca o barca a fost distrusa
    int a = 1, b = 1, c = 1, d = 1;

    if (board[row][col] == 0)
        return 1;
    if (board[row][col] == 1 && boardDiscovered[row][col] == 0)
        return 0;
    if (row + 1 != precRow || col != precCol)
        a = wasTakenDown(row + 1, col, board, boardDiscovered, row, col);
    if (row - 1 != precRow || col != precCol)
        b = wasTakenDown(row - 1, col, board, boardDiscovered, row, col);
    if (row != precRow || col + 1 != precCol)
        c = wasTakenDown(row, col + 1, board, boardDiscovered, row, col);
    if (row != precRow || col - 1 != precCol)
        d = wasTakenDown(row, col - 1, board, boardDiscovered, row, col);
    return (a && b && c && d);
}
