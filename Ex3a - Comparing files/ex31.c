//Amir Bagaon 204313100
#include <stdlib.h>
#include <stdio.h>
#include <sys/fcntl.h>
#include <errno.h>
#include<stdlib.h>

#include <sys/stat.h>
#include <zconf.h>

struct stat st;
struct stat st2;
int exit_status = 3;


char* ignoreSpaces(char *s);
int isItSpace(char c);
int isItSameLetter(char c1, char c2);

int main(int argc , char **argv)
{

    char* filename1;
    char* filename2;

    //Validate that there's only 2 args
    if(argc != 3) {
        fprintf(stderr, "There should be 2 args!\n");
        exit(-1);
    } else {
        filename1 = argv[1];
        filename2 = argv[2];
    }
    int fd1;           /* file descriptor of file 1*/
    int fd2;          /* file descriptor of file 2*/
    char* p1 = NULL;
    char* p2 = NULL;

    //Get files's sizes
    stat(filename1, &st);
    ssize_t size1 = st.st_size;
    stat(filename2, &st2);
    ssize_t size2 = st2.st_size;

    //open file 1
    fd1 = open(filename1,O_RDONLY);
    if (fd1 < 0) /* means file open did not take place */
    {
        perror("error with opening first file");   /* text explaining why */
        exit(-1);
    }
    char buf1[size1 + 1]; // buffer for reading
    read(fd1,buf1,size1);
    close(fd1);		// free allocated structures
    buf1[size1] = 0;

    // open file 2
    fd2 = open(filename2,O_RDONLY);
    if (fd2 < 0)	/* means file open did not take place */
    {
        perror("error with opening second file");   /* text explaining why */
        exit(-1);
    }
    char buf2[size2 + 1]; // buffer for reading
    read(fd2,buf2,size2);
    close(fd2);		//free allocated structures
    buf2[size2] = 0; //In order to null-terminate it

    p1 = buf1;
    p2 = buf2;

    while((*p1) && (*p2)) {
        if(p1[0] != p2[0]) {
            exit_status = 2;
            if (isItSpace(p1[0]) || isItSpace(p2[0])) {
                p1 = ignoreSpaces(p1);
                p2 = ignoreSpaces(p2);
                continue;
            } else {
                if(!isItSameLetter(p1[0], p2[0])) {
                    exit(1);
                }
            }
        }
        p1++;
        p2++;
    }
    if ( (!*p1) && (!*p2) ) //If they both null
        exit(exit_status);
    //Now need to check that they both finished
    p1 = ignoreSpaces(p1);
    p2 = ignoreSpaces(p2);
    if ( (!*p1) && (!*p2) ) //If they both null
        exit(2);

    exit(1); //If only one of them finished and the other isn't space
}

/*
Returns string after skipping the first sequence of spaces
@param char *s - the string that should be space-skipped
*/
char* ignoreSpaces(char *s)
{
    while (isItSpace(*s)) ++s;
    return s;
}
/*
Checks if a char is a space-type
@param c - The char
*/
int isItSpace(char c) {
    if ((c == ' ') || c == '\n')
        return 1;
    return 0;
}
/*
Gets 2 chars and checks if they are the same lower/upper letter
@param c1 - a char
@param c2 - a char
*/
int isItSameLetter(char c1, char c2) {
    int a = 'A';
    int z = 'z';
    if((c1 < a) || (c1 > z) || (c2 < a) || (c2 > z)) {
        //In that case, they are not letters, so case of lower/upper cant be
        //And since they came to the function as different chars, we return 0
        return 0;
    }
    int difference = ('a' - 'A');
    if ( ((c1 - c2) == difference) || ((c2 - c1) == difference))
        return 1;
    return 0;

}

