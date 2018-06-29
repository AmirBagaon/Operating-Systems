#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <wait.h>
#include <stdbool.h>

#define ROWS (20 + 2)
#define COLS 20


void printBoard();
void initBoard();
void startRound();
void updateShapeToBoard();
void playOneTurn();
void clear();
void refresh();
void wRecieved();
void sRecieved();
void dRecieved();
void aRecieved();
void handle_alarm( int sig );
void sig_handler(int num);

//Indicate '-' (part of shape) coordinate
typedef struct coordinate {
    int row;
    int col;
}coordinate;

//Indicate shape
typedef struct shape {
    coordinate top; // Also The most Right
    coordinate mid;
    coordinate low; // Also The most Left
}shape;


char board[ROWS][COLS]; //The board
shape s; // The Shape
volatile sig_atomic_t print_flag = true; //Flag that indicates if board should play one turn

/**
 * Init the empty board without shape
 */
void initBoard() {
    int i, j;
    //Init all with space
    for (i = 0; i < ROWS; i++)
        for(j=0; j < COLS; j++)
            board[i][j] = ' ';
    for(i = 0; i < ROWS; i ++) {
        board[i][0] = '*';
        board[i][COLS - 1] = '*';
        board[ROWS - 1][i] = '*';
    }
}
/**
 * Print the board
 */
void printBoard() {
    int i, j;
    for (i = 2; i < ROWS; i++) {
        for(j=0; j < COLS; j++)
            printf("%c",board[i][j]);
        printf("\n");
    }
}
/**
 * Randomize a starting col and update shape location
 */
void startRound() {
    int r = (rand() % (COLS -1)) + 1; //Generate col between 1 to cols-1, means not in the edges
    //Set shape rows
    s.top.col = r;
    s.mid.col = r;
    s.low.col = r;
    //Set shape cols
    s.top.row = 0;
    s.mid.row = 1;
    s.low.row = 2;
}

/**
 * Update the location of the shape to the board
 */
void updateShapeToBoard() {

    //If we reach to bottom - start again
    if ((s.low.row >= (ROWS - 1)) || (s.mid.row >= (ROWS - 1)) || s.top.row >= (ROWS - 1)) {
        startRound();
    }

    board[s.top.row][s.top.col] = '-';
    board[s.mid.row][s.mid.col] = '-';
    board[s.low.row][s.low.col] = '-';
}
/**
 * Move shape 1 step down
 */
void playOneTurn() {
    s.top.row = s.top.row + 1;
    s.mid.row = s.mid.row + 1;
    s.low.row = s.low.row + 1;
}
/**
 * When pressed d - move shape 1 step right if possible
 */
void dRecieved() {

    if(s.top.col >= (COLS - 2))
        return;
    s.top.col++;
    s.mid.col++;
    s.low.col++;

    refresh();
}
/**
 * When pressed a - move shape 1 step left if possible
 */
void aRecieved() {

    if(s.low.col <= 1)
        return;
    s.top.col--;
    s.mid.col--;
    s.low.col--;
    refresh();
}
/**
 * When pressed s - move shape 1 step down if possible. If reached to bottom - start again
 */
void sRecieved() {

    if(s.low.row >= (ROWS - 1)) {
        startRound();
        refresh();
        return;
    }
    s.top.row++;
    s.mid.row++;
    s.low.row++;
    refresh();
}
/**
 * When pressed w - rotate the shape if possible
 */
void wRecieved() {
    if(s.low.row == s.top.row) {
        if(s.low.row + 1 == (ROWS - 1))
            return;
        s.low.row++;
        s.low.col++;
        s.top.row--;
        s.top.col--;
        refresh();
        return;
    }
    if(s.low.col == s.top.col) {
        if ((s.low.col -1 == 0) || (s.top.col + 1) == (COLS-1))
            return;
        s.low.row--;
        s.low.col--;
        s.top.row++;
        s.top.col++;
        refresh();
    }
}

/**
 * Clear the board
 */
void clear() {
    pid_t status;
    if ( (status = fork()) == -1) {
        //SuperError
        printf("Couldn\'t \'fork()\' the current process into a child: %s\n", strerror(errno));
        exit(-1);

    }
    if(status == 0) {

        //Child
        if(execvp("clear", (char *const[]){"clear", NULL}) == -1)
        {
            printf("Couldn\'t \'exec()\' the command: %s\n", strerror(errno));
            exit(EXIT_FAILURE);
        }
        exit(EXIT_SUCCESS);
    }
    //Father
    wait(&status);
}
/**
 * Handle sigusr2
 * @param num
 */
void sig_handler(int num) {
    char c = getchar();
    if (c == 'q') {
        close(0);
        close(1);
        exit(0);
    }
    if(c == 'd')
        dRecieved();
    else if(c == 'a')
        aRecieved();
    else if(c == 'w')
        wRecieved();
    else if(c == 's')
        sRecieved();
}

/**
 * Handle alarm
 * @param sig
 */
void handle_alarm( int sig ) {
    print_flag = true;
}

/**
 * Refresh the board and print it again
 */
void refresh() {
    clear();
    initBoard();
    updateShapeToBoard();
    printBoard();
}
int main() {
    signal( SIGALRM, handle_alarm ); // Install handler first,
    signal(SIGUSR2,sig_handler);
    startRound();

    while(1) {
        if ( print_flag ) {
            print_flag = false;
            refresh();
            playOneTurn();
            alarm(1);
        }
    }
}



