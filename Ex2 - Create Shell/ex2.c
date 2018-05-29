//Amir Bagaon 204313100
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>


#define sizeLine 1024
#define sizeArgs 512



// The arguments. args[0] will represent the command
static char* args[sizeArgs];
//The pid
pid_t pid;

/**
 * Gets the command and run it
 * @param cmd the command
 */
static void run(char* cmd);

static char line[sizeLine]; //Will help to the input

static int isUppersent = 0; //A boolean to know if & is used
static int isNormal = 0; //A boolean to know if the command belonged to execv

//A Struct for background process(with &)
typedef struct bgprocess {
    pid_t bgPID;
    char* cmdName[sizeArgs];
    struct bgprocess* next;
}bgprocess_t;

char* prev = NULL; //Will help the CD - command

//2 Nodes for a linked list that will hold the bg processes
bgprocess_t * head = NULL;
bgprocess_t * current  = NULL;

/**
 * Get and print the args of the current job
 * @param temp args of a job
 */
static void printArgs(char** temp) {
    int i;
    for(i = 0; temp[i] != '\0'; i++) {
        printf("%s ", temp[i]);
    }
}

/**
* Kill running processes and free allocated memory
*/
static void toFree() {
    while (head != NULL) {
        current = head;
        head = head->next;
        kill(current->bgPID, SIGKILL);
        free(current);
    }
    current = NULL;
    free(prev);
}


/**
 * Creat the fork and run the command
 * If the command contained '&', it insert it to the linked list
 */
static void command() {

    if (isUppersent) {
        //allocate space
        current = NULL;
        current = malloc(sizeof(bgprocess_t));
        if (current == NULL) {
            fprintf(stderr, "Error in allocation\n");
        }
        int i = 0;
        for (i; args[i] != '\0'; i++) {
            //copy the args
            current->cmdName[i] = strdup(args[i]);
        }
        current->cmdName[i] = '\0';
        current->next = NULL;
        if (head == NULL) {
            head = current;
        } else {
            bgprocess_t * last = head;
            while (last->next != NULL) {
                last = last->next;
            }
            last->next = current;
        }
    }
    pid = fork();
    if (pid == 0) {
        printf("%d\n", getpid());
        if (execvp(args[0], args) == -1)
            fprintf(stderr, "Error in system call\n");
    } else {
        if (isUppersent) {
            //Parent got his child's pid
            current->bgPID = pid;
        }
    }
}


/**
 * Check if the arguments contains '&'
 */
static void checkUppersent() {
    int i;
    for(i = 0; args[i] != '\0'; i++) {
        if (strcmp(args[i], "&") == 0) {
            args[i] = NULL;
            isUppersent = 1;
            return;
        }
    }
}


int main()
{
    while (1) {
        //The command line
        printf("prompt> ");
        fflush(NULL);

        //Get command
        if (!fgets(line, sizeLine, stdin))
            return 0;

        //Run command
        char* cmd = line;
        run(cmd);

        //The waiting
        int status;
        if(!isUppersent) {
            if (isNormal) {

                //              printf("\nIn normal. Wait to: %d\n", pid);
                //If it wasn't execv, we would wait
                waitpid(pid, &status, 0);
            }
        } else {
            //If it was '&', we won't wait to the son,
            //But for convinient printing we do the follow
            waitpid(getpid(), &status, 0);
        }
        isUppersent = 0;
        isNormal = 0;
//        n = 0;
    }
    return 0;
}

static void split(char* cmd);

static void run(char* cmd)
{
    split(cmd);
//    if (isUppersent) {
//        printf("Yes it is \n");
//    }
    if (args[0] != NULL) {
        //Exit case
        if (strcmp(args[0], "exit") == 0) {
            printf("%d\n", getpid());
            toFree();
            exit(0);
        }

        //CD case
        if (strcmp(args[0], "cd") == 0) {

            //If there is a param - change to it. Otherwise - change to home
            printf("%d\n", getpid());
            char* newDir;

            //If command is 'cd - '
            if ((args[1] != NULL) && (strcmp(args[1], "-") == 0)) {
                if (prev == NULL) {
                    fprintf(stderr, "Error: OLDPWD not set\n");
                    return;
                }
                newDir = strdup(prev);
                //Print the target dir location
                printf("%s\n", newDir);
            } else {
                newDir = ((args[1] == '\0') || ((strcmp(args[1], "~") == 0))?
                          getenv("HOME") : args[1]);
            }

            //Get current position before change
            prev = getcwd(NULL, 0);
            if (chdir(newDir) == -1) {
                printf("Couldn't change directory\n");
            }
            return;
        }

        //Jobs Command
        if (strcmp(args[0], "jobs") == 0) {
            int i,status;
            //If there is a param - change to it. Other - change to home
            bgprocess_t* temp = head;
            while (temp != NULL) {
                if(waitpid(temp->bgPID, &status, WNOHANG) == 0) {
                    printf("%d ", temp->bgPID);
                    printArgs(temp->cmdName);
                    printf("\n");
                } else {
//                    if (temp == head) {
//                        head = NULL;
//                        free(temp);
//                        return;
//                    }
//                    bgprocess_t* beforeTemp = head;
//                    while(beforeTemp->next != temp) {
//                        beforeTemp = beforeTemp->next;
//                    }
//                    beforeTemp->next = temp->next;
//                    free(temp);
                }
                temp = temp->next;
            }
            return;
        }

        //Other Command
        isNormal = 1;
        command();
    }
}

/**
 * skip bunch of spaces in a string
 * @param s the sring
 * @return the string after skipping
 */
static char* ignoreSpaces(char *s)
{
    while (isspace(*s)) ++s;
    return s;
}

/**
 * Split the command to different args without spaces and commas
 * @param cmd the command
 */
static void split(char* cmd)
{
    cmd = ignoreSpaces(cmd);
    char* next = strchr(cmd, ' ');
    if(next != NULL) {
        char* findCommas = next;
        ++findCommas;
        if(findCommas[0] == '"') {
            char* after = strchr(++findCommas, '"');
            if (after != NULL) {
                if (((after + 1)[0] == '\n') || ((after + 1) == NULL)) {
                    after[0] = '\0';
                    args[0] = cmd;
                    next[0] = '\0';
                    args[1] = findCommas;
                    args[2] = NULL;
                    return;
                }
            }
        }
//        printf("%s\n",++findCommas);
    }

    int i = 0;

    while(next != NULL) {
        next[0] = '\0';
        args[i] = cmd;
        ++i;
        cmd = ignoreSpaces(next + 1);
        next = strchr(cmd, ' ');
    }

    if (cmd[0] != '\0') {
        args[i] = cmd;
        next = strchr(cmd, '\n');
        next[0] = '\0';
        ++i;
    }
    checkUppersent();

    args[i] = NULL;
}


/*
 * static void command()
{
    bgprocess_t* current = NULL;
    if (isUppersent) {
        if (head != NULL) {
            current = head;
            while (current != NULL) {
                current = current->next;
            }
        }
        //allocate space
        current = malloc(sizeof(bgprocess_t));
        if (current == NULL) {
                fprintf(stderr, "Error in allocation\n");
        }
        if (head == NULL) {
            head = current;
        }
        current->next = NULL;

        //copy the args
        int i = 0;
        for(i; args[i] != '\0'; i++) {
            current->cmdName[i] = strdup(args[i]);
        }
        current->cmdName[i] = '\0';
        current->pidNum = getpid();
        last = current;
    }
    pid = fork();

    if (pid == 0) {
        //printf("My process ID : %d\n", getpid());
        if (execvp( args[0], args) == -1)
            fprintf(stderr, "Error in system call\n");
    }
}
 */

//‫‪12aB2     3‬‬
