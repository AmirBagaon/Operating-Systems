//
// Created by amir on 14/05/18.
//

#include <memory.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <zconf.h>
#include <dirent.h>
#include <sys/stat.h>
#include <wait.h>

#define maxLength 161
#define numLines 3
#define tempOutputPath "output.txt"

int isItCFile(char* str);
void completePathName(char* oldPathName, char* nameNewPath, char* buff);
int isConnector(char* str);
void levelUpPathName(char* newerPath, char* buff);
char* readConfig(char* path);
void setPaths(char **buff, char** input, char** output);
int traverseFolder(char* folderPath, char* inputPath, char* outputPath, char* tempPath);
int compile(char* fileName, char* path);
char* getName(char* folderPath);
int execute(char* nameOut, char* inputPath, char* correctOutputPath);
int compareFiles(char* correctOutputPath, char* studentOutputPath);
int containsEnter(char* path);
void errorSysCall();
void errorExit(char* tempPath, char* buff);

//למוצאי החג, לברר איפה לעשות אקסיט, מה להחזיר לכתיבה בקובץ ועוד

struct stat statbuf;
struct dirent *de;
//Malloc list: buff, tempPath

enum exitStatus{CompileError = 4, SysCallError};
enum grades{NoCFile = 5, CompilationError = 6, Timeout = 7, GreatJob = 3,
    BadOutput = 1, SimilarOutput = 2};
int main(int argc , char **argv)
{
    //char error[] = "Error in system call\n";
    int a = 20;
    if ( (a = 3) < 5)
        printf("Yes %d\n", a);

    //Check for Valid args
    if (argc < 2) {
        printf("Not good args!");
        return 0;
    }



    //Read from file
    char* buff = readConfig(argv[1]);
    if (!buff)
        return 0;

    //Pointers to each path
    char* folderPath = buff;
    char*  inputPath; char* outputPath;
    setPaths(&buff, &inputPath, &outputPath);

    printf("%s\n", folderPath);
    printf("%s\n", inputPath);
    printf("%s\n", outputPath);
    containsEnter(folderPath);
    containsEnter(inputPath);
    containsEnter(outputPath);
    char* tempPath;
    if ((tempPath = (char*)malloc(maxLength)) == NULL) {
        printf("Error In tempPath allocation");
        //superError
        errorSysCall();
        free(buff);
        return 0;
    }
    traverseFolder(folderPath, inputPath, outputPath,tempPath);
    free(buff);
    free(tempPath);

    return 0;
}

void addGrade(int status, char* name) {

    char toWrite[maxLength];
    strcpy(toWrite, name);

    int result_file = open("results.csv", O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (result_file < 0) {
        printf("Couldn't opening result.csv\n");
        //SuperError
        errorSysCall();
        return;
    }
    switch (status) {
        case NoCFile:
            strcat(toWrite, ",0,NO_C_FILE");
            break;
        case CompilationError:
            strcat(toWrite, ",0,COMPILATION_ERROR");
            break;
        case Timeout:
            strcat(toWrite, ",0,TIMEOUT");
            break;
        case BadOutput:
            strcat(toWrite, ",60,BAD_OUTPUT");
            break;
        case SimilarOutput:
            strcat(toWrite, ",80,SIMILAR_OUTPUT");
            break;
        case GreatJob:
            strcat(toWrite, ",100,GREAT_JOB");
            break;
        default:
            break;
    }
    int length = strlen(toWrite);
    if (write(result_file, toWrite, length) < 0) {
        printf("Error in writing to result.csv\n");
     //SuperError
        errorSysCall();
    }

    if (close(result_file) < 0 ) {
        printf("couldn't close result.csv file\n");
        return;
    }
}

int containsEnter(char* path) {
    char *input = strchr(path, '\n');
    if (input != NULL) {
        printf("Yes\n");
        return 1;
    }
    printf("No\n");
    return 1;
}
int traverseFolder(char* folderPath, char* inputPath, char* outputPath, char* tempPath) {
    //Start traverse folder path to compile each .c files
    printf("In folder: %s\n\n", folderPath);
    DIR *dr = opendir(folderPath);
    if (dr == NULL)  // opendir returns NULL if couldn't open directory
    {
        printf("Could not open current directory" );
        return 0;
    }

    int foundCFile = 0;

    while ((de = readdir(dr)) != NULL) {
        char* name = de->d_name;
	printf("Name: %s\n", name);
 printf("\nfolder: %s\n",folderPath);
            printf("temp: %s\n",tempPath);
            
        if (isConnector(name)) {
            printf("%s is connector\n", de->d_name);
            continue;
        }
        completePathName(folderPath, de->d_name, tempPath);
        if (isItCFile(name)) {
	printf("P1 \n");
            foundCFile = 1;
            name = getName(folderPath);
            char nameOut[maxLength];
            strcpy(nameOut, name);
            strcat(nameOut,"1.out");
            //Compile the file.c
            if(!compile(nameOut, tempPath)) {
                addGrade(CompilationError, name);
                return 0;
            }
            int exeResult = execute(nameOut, inputPath, outputPath);
            if(exeResult == Timeout) {
                addGrade(Timeout, name);
                return 0;
            }
            printf("\n\n\n\n\nHere100\n");
            printf("%s\n", outputPath);
            printf("%s\n", tempOutputPath);
            int es;
            if (!(es = compareFiles(outputPath, tempOutputPath))) {
                //superError
                errorSysCall();
                return 0;
            }
            unlink(nameOut);
            addGrade(es, name);

            continue;

        }

        if (stat(tempPath, &statbuf) == -1) {
printf("P2 \n");
            printf("SPECIALLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLLL %s\n", de->d_name);
            levelUpPathName(folderPath, tempPath);
            continue;
        }
        if(S_ISDIR(statbuf.st_mode)){
printf("P3 \n");
            printf("%s is directory\n", de->d_name);
            completePathName(folderPath,de->d_name, tempPath);
            traverseFolder(tempPath, inputPath, outputPath, tempPath);
        } else {
printf("P4 \n");
levelUpPathName(folderPath, tempPath);
}
    }
    closedir(dr);
    if(!foundCFile) {
        char* sudentName = getName(folderPath);
        addGrade(NoCFile, sudentName);
    }
    levelUpPathName(folderPath, tempPath);
    printf("%s\n", tempPath);
    return 1;
}
int compareFiles(char* correctOutputPath, char* studentOutputPath) {
    pid_t pid;
    if ((pid=fork()) < 0) {
        //SuperError
        errorSysCall();
        printf("Fail in compareFiles: couldn't create child\n");
        return 0;
    }
    if(pid == 0) {
        //Run comp.out
        char* args[] = {"./comp.out", correctOutputPath, studentOutputPath, NULL};
        execvp(args[0], args);
    } else {
        int status;
        if ( waitpid(pid, &status, 0) == -1 ) {
            perror("waitpid() failed");
            exit(EXIT_FAILURE);
        }

        if ( WIFEXITED(status) ) {
            int es = WEXITSTATUS(status);
            printf("Exit status was %d\n", es);

            //Remove temp outputfile
            unlink(studentOutputPath);
            return es;
        }

    }
}
int execute(char* nameOut, char* inputPath, char* correctOutputPath) {
    pid_t pid;
    int inputFile;
    int outputFile;
    int status;

    //Create child
    if ((pid=fork()) < 0) {
        //SuperError
        errorSysCall();
        printf("Fail in execute: couldn't create child\n");
        return 0;
    }

    if(pid == 0) {
        printf("Son: %d\n", getpid());

        //open input file
        if ((inputFile = open(inputPath, O_RDONLY)) < 0) {
            printf("Couldn't open inputFile file\n");
            //SuperError
            errorSysCall();
            return 0;
        }
        if (dup2(inputFile, STDIN_FILENO) == -1) {
            printf("Failed to operate dup2 for input\n");
            //SuperError
            errorSysCall();
            return 0;
        }
        outputFile = open(tempOutputPath,
                          O_CREAT | O_TRUNC | O_WRONLY, 0644);
        if (outputFile== -1) {
            //SuperError
            errorSysCall();
            printf("couldn't open output file\n");
            return 0;
        }

        //Output of running file.out will be in outputFile
        if (dup2(outputFile, STDOUT_FILENO) == -1) {
            printf("Failed to operate dup2 for out\n");
            //SuperError
            errorSysCall();
            return 0;
        }

        //Execute and run the file
        char command [maxLength] = "./";
        strcat(command, nameOut);
        char* args[] = {command, NULL};
        execvp(args[0], args);
    } else { //Father

        //Check For timeOut
        sleep(5);
        int temp = (waitpid(pid, &status, WNOHANG));
        if(!temp) {
            printf("TimeOut");
            return Timeout;

        } else {
            printf("FINISHED\n");
            return 1;
        }

//
//        if (waitpid(pid, &status, WNOHANG) == 0) {
//            printf("\n\nTimeOut\n");
//        } else {
//            printf("\n\nFinished\n");
//        }

    }
}

int compile(char* nameOut, char* path) {
    char* args[] = {"gcc", "-o", nameOut, path, NULL};
    int pid;
    if ((pid = fork()) < 0) {
        printf("Error in fork command in order to compile .c file\n");
        //SuperError
        errorSysCall();
        return 0;
    }
    if (pid == 0) {
        execvp(args[0], &args[0]);
    } else {
        int status;
        waitpid(pid, &status, 0); //Wait for compile to be done
        if(status) //Means compile wasn't successful
        {
            //SuperError
            errorSysCall();
            printf("Error in fork command in order to compile .c file\n");
            return 0;
        }
        printf("Compile was successfull!\n");
        return 1;
    }
}
char* getName(char* folderPath) {
    char* first = strchr(folderPath, '/');
    if(!first) // So there are no '/', we just return the folderPath
        return folderPath;
    char* next;
    while (first != NULL) {
        next = ++first;
        first = strchr(first, '/');
    }
    return next;
}

void setPaths(char **buff, char** input, char** output) {
    *input = strchr(*buff, '\n');

    *input[0] = '\0';
    *output = strchr(++(*input), '\n');
    *output[0] = '\0';
    // (*output)++;
    //Valid that output not finished with '\n'
    char* temp = strchr(++(*output), '\n');
    if(temp != NULL) {
        temp[0] = '\0';
    }
}

char* readConfig(char* path) {
    //Open file
    int configFile;
    if ((configFile = open(path, O_RDONLY)) < 0) {
        printf("Couldn't open configuration file\n");
        //SuperError
        errorSysCall();
        exit(EXIT_FAILURE);
    }


    int readBytes;
    int size = numLines * maxLength;
    char* buff;
    if ((buff = (char*)malloc(size + 1)) == NULL) {
        printf("Error in allocation");
        //superError
        errorSysCall();
        exit(EXIT_FAILURE);
    }
    if ((readBytes = read(configFile, buff, size)) < 0) {
        printf("Error while reading configuration file\n");
        //SuperError
        errorSysCall();
        free(buff);
        exit(EXIT_FAILURE);
    }
    buff[readBytes] = '\0';
    //Close File
    if (close(configFile) == -1) {
        printf("Failed to close configuration file\n");
        //SuperError
        errorSysCall();
        free(buff);
        exit(EXIT_FAILURE);
    }
    return buff;
}

//void searchFile(char* folder) {
//
//}
void completePathName(char* oldPathName, char* nameNewPath, char* buff) {
    strcpy(buff, oldPathName);
    strcat(buff, "/");
    strcat(buff, nameNewPath);
}
void levelUpPathName(char* newerPath, char* buff) {
    char* lastSlash = strchr(newerPath, '/');
    //printf("%s\n", newerPath);
    //printf("%s\n", lastSlash);
    char* temp = NULL;
    while (lastSlash != NULL) {
        temp = lastSlash;
        lastSlash = strchr(++lastSlash, '/');
    }
    if(!temp)
        return;
    //printf("%s\n", temp);
    //printf("%s\n", newerPath);
    temp[0] = '\0';
    strcpy(buff, newerPath);
}
int isConnector(char* str) {
    return ((!strcmp(str, ".")) || (!strcmp(str, "..")));
}
int isItCFile(char* str) {
    if(!str)
        return 0;

    if (strcmp(str, ".c") == 0) //Case the name is just '.c'
        return 0;
    char* next = strchr(str, '.');
    if (!next)
        return 0;
    return (strcmp(next, ".c") == 0);
}

void errorSysCall() {
    char error[] = "Error in system call\n";
    int size = sizeof(error);
    write(2, error, size);
    exit(EXIT_FAILURE);
}
void errorExit(char* tempPath, char* buff) {
    free(tempPath);
    free(buff);
    errorSysCall();
}

