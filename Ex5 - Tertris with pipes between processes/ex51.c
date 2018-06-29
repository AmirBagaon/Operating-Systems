#include <stdio.h>
#include <unistd.h>
#include <termios.h>
#include <signal.h>
#include <sys/wait.h>


char getch();
int checkChar(char c);
void sysCallError();

/**
 * Returns pressed char
 * @return pressed char
 */

char getch() {
    char buf = 0;
    struct termios old = {0};
    if (tcgetattr(0, &old) < 0)
        perror("tcsetattr()");
    old.c_lflag &= ~ICANON;
    old.c_lflag &= ~ECHO;
    old.c_cc[VMIN] = 1;
    old.c_cc[VTIME] = 0;
    if (tcsetattr(0, TCSANOW, &old) < 0)
        perror("tcsetattr ICANON");
    if (read(0, &buf, 1) < 0)
        perror ("read()");
    old.c_lflag |= ICANON;
    old.c_lflag |= ECHO;
    if (tcsetattr(0, TCSADRAIN, &old) < 0)
        perror ("tcsetattr ~ICANON");
    return (buf);
}

/**
 * Error on system call
 */
void sysCallError() {
    char error[] = "Error in system call\n";
    write(2, error, sizeof(error));
}

/**
 * Returns if char is a/s/d/w/q
 * @return 1 or 0
 */
int checkChar(char c) {
    if ((c == 'a')
        ||(c == 'd')
        ||(c == 's')
        ||(c == 'w')
        ||(c == 'q'))
        return 1;
    return 0;

}

int main() {
    pid_t pid;
    int Pipe[2];
    pipe(Pipe);

    if ((pid = fork()) < 0) {
        sysCallError();
    }

    // Child process
    if (pid == 0) {
        // Change stdin to read from pipe
        dup2(Pipe[0], 0);
        if(execvp("./draw.out", (char *const[]){"./draw.out", NULL}) == -1)
            sysCallError();
    }

    /* Father Process */
    char c;
    while (1) {
        c = getch();
        // char from stdin is not a game key.
        if (!checkChar(c)) {
            continue;
        }

        if(write(Pipe[1], &c, 1) < 0) {
            sysCallError();
        }
        kill(pid, SIGUSR2);

        if (c == 'q') {
            break;
        }
    }
    wait(NULL);
    close(Pipe[0]);
    close(Pipe[1]);

    return 0;
}

