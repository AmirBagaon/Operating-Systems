//Amir Bagaon 204313100

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
#define tempOutputPath "output88.txt"

int isItCFile(char* str);
int isConnector(char* str);
char* readConfig(char* path);
void setPaths(char **buff, char** input, char** output);
int traverseFolder(char* folderPath, char* inputPath, char* outputPath);
int compile(char* fileName, char* path);
int execute(char* nameOut, char* inputPath);
int compareFiles(char* correctOutputPath, char* studentOutputPath);
void addGrade(int resultCSV,int status, char* name);
void errorSysCall();
int primeFolder(char* folderPath, char* inputPath, char* outputPath);
void goUpperFolder(char* abc);

enum exitStatus{SystemFail = 12};
enum grades{NoCFile = 5, CompilationError = 6, Timeout = 7, GreatJob = 3,
    BadOutput = 1, SimilarOutput = 2};
int main(int argc , char **argv)
{
    //Check for Valid args
    if (argc != 2) {
        //printf("Not good args!");
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

    char fPath[maxLength];
    char iPath[maxLength];
    char oPath[maxLength];
    strcpy(fPath, folderPath);
    strcpy(iPath, inputPath);
    strcpy(oPath, outputPath);
    free(buff);
    int success = primeFolder(fPath, iPath, oPath);


    if (!success)
        errorSysCall();

    return 0;
}
/**
 * Take name of folder, and 'delete' its last slash, so it represent it's parent folder
 * @param folder folder path
 */
void goUpperFolder(char* folder) {
    char* next = strrchr(folder, '/');
    if (!next)
        return;
    next[0] = '\0';
}

/**
 *
 * @param folderPath The students folder path
 * @param inputPath future input path
 * @param outputPath correct output path
 * @return 1 on success, 0 otherwise
 */
int primeFolder(char* folderPath, char* inputPath, char* outputPath) {
    //Open Result.csv
    int resultCSV = open("results.csv", O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (resultCSV < 0) {
        ////printf("Problem -> open result.csv\n");
        //SuperError
    }

    //Open Prime folder
    DIR *dr = opendir(folderPath);
    if (dr == NULL)  // opendir returns NULL if couldn't open directory
    {
        ////printf("Problem -> open primeFolder dir" );
        return 0;
    }

    struct stat statbuf;
    struct dirent *de;
    int success = 1;
    char helperPath[maxLength];
    strcpy(helperPath, folderPath);

    while ((de = readdir(dr)) != NULL) {
        ////printf("FName: %s\n", de->d_name);
        if (isConnector(de->d_name))
            continue;
        strcat(helperPath, "/");
        strcat(helperPath, de->d_name);
        ////printf("completePathName: %s\n", helperPath);

        if (stat(helperPath, &statbuf) == -1) {
            goUpperFolder(folderPath);
            strcpy(helperPath, folderPath);
            continue;
        }
        if(S_ISDIR(statbuf.st_mode)){
          //  //printf("%s is directory\n", de->d_name);
            int grade = traverseFolder(helperPath, inputPath, outputPath);
            ////printf("\nGOT GRADE! %d\n", grade);
            if (grade == SystemFail) {
                success = 0;
                break;
            }
            addGrade(resultCSV, grade, de->d_name);
            strcpy(helperPath, folderPath);
        } else {
            goUpperFolder(helperPath);
            strcpy(helperPath, folderPath);
        }
    }
    closedir(dr); //Close prime dir

    //Close ResultCSV
    if (close(resultCSV) < 0 ) {
        ////printf("couldn't close result.csv file\n");
        return 0;
    }
    return success; //Return 1 if success, or 0 if failed
}

/**
 * Gets a folder path, and traverse its all file and sub-folders,
 * Compile all C files and return the desired grade
 * @param folderPath A path of the folder
 * @param inputPath Path of the supposed input
 * @param outputPath Path to a file which contains correct input
 * @return T/F on success
 */
int traverseFolder(char* folderPath, char* inputPath, char* outputPath) {
    //Start traverse folder path to compile each .c files
    ////printf("---------*In folder: %s    *---------------------\n\n", folderPath);
    DIR *dr = opendir(folderPath);
    if (dr == NULL)  // opendir returns NULL if couldn't open directory
    {
        //printf("Could not open current directory" );
        return 0;
    }
    struct stat statbuf;
    struct dirent *de;

    int grade = NoCFile;

    while ((de = readdir(dr)) != NULL) {
        //printf("FName: %s\n", de->d_name);
        if (isConnector(de->d_name)) {
            continue;
        }
        strcat(folderPath, "/");
        strcat(folderPath, de->d_name);
        //printf("completePathName: %s\n", folderPath);
        if (isItCFile(de->d_name)) {
            char nameOut[maxLength];
            strcpy(nameOut, de->d_name);
            strcat(nameOut,"1.out");
            //Compile the file.c
            int compileResult = compile(nameOut, folderPath);
            if (compileResult == SystemFail) {
                grade = SystemFail;
                break;
            }
            if(!compileResult) { //Means compile wasn't successful
                grade = CompilationError;
                break;
            }

            //Execute file
            int exeResult = execute(nameOut, inputPath);
            if ((exeResult == SystemFail) || (exeResult == Timeout)) {
                grade = exeResult; //Returns SystemFail or Timeout
            } else {
                grade = compareFiles(outputPath, tempOutputPath);
            }
            unlink(tempOutputPath);
            unlink(nameOut);
            break; //Returns grade or SystemFail
        }

        if (stat(folderPath, &statbuf) == -1) {
            goUpperFolder(folderPath);
            continue;
        }
        if(S_ISDIR(statbuf.st_mode)){
            //printf("%s is directory\n", de->d_name);
            int tempGrade = traverseFolder(folderPath, inputPath, outputPath);
            if (tempGrade != NoCFile) {
                grade = tempGrade;
                break;
            }
        } else {
            goUpperFolder(folderPath);
        }
    }
    closedir(dr);
    goUpperFolder(folderPath);
    return grade;
}

/**
 * Add the grade to the resultCSV file
 * @param resultCSV the file (opened before and will be close after)
 * @param status the grade
 * @param name student name
 */
void addGrade(int resultCSV,int status, char* name) {

    char toWrite[maxLength];
    strcpy(toWrite, name);

    switch (status) {
        case NoCFile:
            strcat(toWrite, ",0,NO_C_FILE\n");
            break;
        case CompilationError:
            strcat(toWrite, ",0,COMPILATION_ERROR\n");
            break;
        case Timeout:
            strcat(toWrite, ",0,TIMEOUT\n");
            break;
        case BadOutput:
            strcat(toWrite, ",60,BAD_OUTPUT\n");
            break;
        case SimilarOutput:
            strcat(toWrite, ",80,SIMILAR_OUTPUT\n");
            break;
        case GreatJob:
            strcat(toWrite, ",100,GREAT_JOB\n");
            break;
        default:
            break;
    }
    int length = strlen(toWrite);
    if (write(resultCSV, toWrite, length) < 0) {
        //printf("Error in writing to result.csv\n");
        //SuperError
        errorSysCall();
    }
}
/**
 * Compare 2 files using comp.out
 * checks if their output is same
 * @param correctOutputPath path of the correct output
 * @param studentOutputPath path of the student output
 * @return 3 if same, 2 if similar, 1 if bad-output, 0 if failed to find comp.out
 */
int compareFiles(char* correctOutputPath, char* studentOutputPath) {
    pid_t pid;
    if ((pid=fork()) < 0) {
        //SuperError
        errorSysCall();
        //printf("Fail in compareFiles: couldn't create child\n");
        return SystemFail;
    }
    if(pid == 0) {
        //Run comp.out
        char* args[] = {"./comp.out", correctOutputPath, studentOutputPath, NULL};
        execvp(args[0], args);
    } else {
        int status;
        if ( waitpid(pid, &status, 0) == -1 ) {
            perror("waitpid() failed");
            return SystemFail;
        }

        if ( WIFEXITED(status) ) {
            int es = WEXITSTATUS(status);
            //printf("Exit status was %d\n", es);
            return es;
        }

    }
}
/**
 * Execute and run the nameOut.out with the correct input
 * Also checks for timeout
 * @param nameOut filename to be filename.out
 * @param inputPath input for this file
 * @return 1 on success, 'timeout' enum on timeout, 0 on fail
 */
int execute(char* nameOut, char* inputPath) {
    pid_t pid;
    int inputFile ,outputFile, status;

    //Create child
    if ((pid=fork()) < 0) {
        //printf("Problem -> execute: couldn't create child\n");
        return SystemFail;
    }

    if(pid == 0) {
        //open input file
        if ((inputFile = open(inputPath, O_RDONLY)) < 0) {
            //printf("Problem -> open inputFile file\n");
            return SystemFail;
        }
        if (dup2(inputFile, STDIN_FILENO) == -1) {
            //printf("Problem -> dup2 input\n");
            return SystemFail;
        }
        outputFile = open(tempOutputPath,
                          O_CREAT | O_TRUNC | O_WRONLY, 0644);
        if (outputFile== -1) {
            //printf("Problem -> open output file\n");
            unlink(tempOutputPath);
            return SystemFail;
        }

        //Output of running file.out will be in outputFile
        if (dup2(outputFile, STDOUT_FILENO) == -1) {
            //printf("Problem -> dup2 out\n");
            unlink(tempOutputPath);
            return SystemFail;
        }

        //Execute and run the file
        char command [maxLength] = "./";
        strcat(command, nameOut);
        char* args[] = {command, NULL};
        execvp(args[0], args);
    } else { //Father
        sleep(5);
        int temp = (waitpid(pid, &status, WNOHANG));
        if (temp == -1) { //Fail with waitpid
            //printf("waitpid() wasn't successful");
            return SystemFail;
        }
        if(!temp) { //Timeout
            //printf("TimeOut");
            return Timeout;
        } else { //NotTimeOut
            return 1;
        }
    }
}

/**
 * Compile the file
 * @param nameOut the name that the file will be compiled to (nameOut.out)
 * @param path path of the file we wish to compile
 * @return success or compilation error
 */
int compile(char* nameOut, char* path) {
    char* args[] = {"gcc", "-o", nameOut, path, NULL};
    int pid;
    if ((pid = fork()) < 0) {
        //printf("Problem in compile -> fork\n");
        return SystemFail;
    }
    if (pid == 0) { //Son
        execvp(args[0], &args[0]);
    } else { //Father
        int status;
        if (waitpid(pid, &status, 0) == -1) {
            perror("waitpid() failed"); //waitpid wasn't successful
            return SystemFail;
        }

        if(status) //Means compile wasn't successful
        {
            //printf("Compile wasn't successfull -_-\n");
            return 0;
        }
        //printf("Compile was successfull!\n");
        return 1;
    }
}

/**
 * Take paths information from the buffer ot the configuration file,
 * and set the path for students folder path, for correct input path,
 * and for correct output path
 * @param buff The buff from configuration file, which represent folder path
 * @param input correct input path
 * @param output correct output path
 */
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

/**
 * Read the configuration file and allocate space to contain its content
 * @param path configuration file path
 * @return
 */
char* readConfig(char* path) {
    //Open file
    int configFile;
    if ((configFile = open(path, O_RDONLY)) < 0) {
        //printf("Couldn't open configuration file\n");
        //SuperError
        errorSysCall();
        exit(EXIT_FAILURE);
    }


    int readBytes;
    int size = numLines * maxLength;
    char* buff;
    if ((buff = (char*)malloc(size + 1)) == NULL) {
        //printf("Error in allocation");
        //superError
        errorSysCall();
        exit(EXIT_FAILURE);
    }
    if ((readBytes = read(configFile, buff, size)) < 0) {
        //printf("Error while reading configuration file\n");
        //SuperError
        free(buff);
        errorSysCall();
        exit(EXIT_FAILURE);
    }
    buff[readBytes] = '\0';
    //Close File
    if (close(configFile) == -1) {
        //printf("Failed to close configuration file\n");
        //SuperError
        free(buff);
        errorSysCall();
        exit(EXIT_FAILURE);
    }
    return buff;
}

/**
 * Returns whether the string is a connector, like '.' or ".."
 * @param str string
 * @return 1 or 0
 */
int isConnector(char* str) {
    return ((!strcmp(str, ".")) || (!strcmp(str, "..")));
}
/**
 * Gets file name and check whether its finished with '.c' or not
 * @param str file name
 * @return T/F
 */
int isItCFile(char* str) {
    if(!str)
        return 0;

    // if (strcmp(str, ".c") == 0) //Case the name is just '.c'
    //   return 0;
    char* next = strrchr(str, '.');
    if (!next)
        return 0;
    return (strcmp(next, ".c") == 0);
}

/**
 * Write error massage to system
 */
void errorSysCall() {
    char error[] = "Error in system call\n";
    int size = sizeof(error);
    write(2, error, size);
    exit(EXIT_FAILURE);
}