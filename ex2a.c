#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <linux/limits.h>

void splitToArray(char*[], char[]);

const char *count(char[], int *, int *);

void loop();

void ex(int, char *, int*, int*);

int main() {
    loop();
}

void loop() {
    //I added 2 to the length because there's '\n\0' at the end of the str
    char str[514];
    char cwd[PATH_MAX];
    int cmdCount = 0, TotalWord = 0;

    while (-1) {
        int charCount = 0, wordCount = 0;
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s>", cwd);
            fgets(str, 514, stdin);
            //because the user press enter so '\n' enter to the input string in the last index, so we put '\0'
            str[strlen(str) - 1] = '\0';
            //check if there's space after or before the command
            if (str[0] == ' ' || str[strlen(str) - 1] == ' ')
                fprintf(stderr,"You have entered space/s before/after the command \n");
            else {
                //count method return an "cd", "done" or ""
                const char *word = count(str, &charCount, &wordCount);
                if (wordCount != 0) {
                    //if it's cd, so we don't need to fork and execvp because cd not supported yet
                    if (strcmp(word, "cd") == 0) {
                        printf("command not supported (Yet)\n");
                        cmdCount++;
                        TotalWord += wordCount;
                    }// else if it "done" print the total of commands and the words at the commands and stop the shell
                    else if (wordCount == 1 && strcmp(word, "done") == 0) {
                        printf("Num of commands: %d\n", (cmdCount + 1));
                        printf("Total number of words in all commands: %d !\n", TotalWord);
                        break;
                    } else
                        //else its another command we have to execute it using ex method
                        ex(wordCount, str, &TotalWord, &cmdCount);
                }
            }
        }
    }
}

const char *count(char str[], int *charCount, int *wordCount) {
    int i = 0;
    int len =(int)strlen(str);
    char word[7];
    int wordInd = 0;

    while (i < len) {
        if (str[i] != ' ') {//if the char is not ' ' so we have to count i
            (*charCount)++;
            if ((*wordCount) < 1 && (*charCount) <
                                    8) { // if we are in the first word we enter the first 8 chars to the word to check if it "exit" or "history"
                word[wordInd] = str[i];
                wordInd++;
            }
        }//if the str[i] is ' ' and the next element isn't ' ' or '\0' so it's an legal char for a new word
        else if ((*charCount) != 0 && str[i + 1] != ' ' && str[i + 1] != '\0')
            (*wordCount)++;
        i++;
    }
    if ((*charCount) >0)//because we count the splits so the number of splits is more than 0, so there is (1+splits) words
        (*wordCount)++;
    word[wordInd] = '\0';// to finish the string if it less than 7 chars like "exit"
    if (strcmp(word, "done") == 0)
        return "done";
    if (strcmp(word, "cd") == 0)
        return "cd";
    return "";
}
/*
 * this method execute the command using fork and execvp
 */
void ex(int WC, char *str, int *Total, int *cmds) {
    char *arrayOfWords[WC + 1];
    splitToArray(arrayOfWords, str);
    //create a new process to run the command
    pid_t x = fork();
    //if there's problem to create a new process
    if (x < 0) {
        perror("Fork unsuccessfully");
        exit(1);
    }
    /*
     * if this the new process (child)
     * run the command using execvp method
     */
    if (x == 0) {
        //check if the execvp doesn't work probably
        if (-1 == execvp(arrayOfWords[0], arrayOfWords))
            perror("command not supported (Yet)");

        exit(0);
    } else {
        //wait for the child to end the process
        wait(NULL);
        //after the finishing of execvp (child) we don't want the array of stings anymore, so we have to free it from the memory
        for (int j = 0; j < WC; j++)
            free(arrayOfWords[j]);
        //increase the command and sum the words at it
        (*cmds)++;
        (*Total) += WC;
    }
}
/*
 * split the string to an array and the last element is NULL, so the command will be ready to execvp method after this method
 */
void splitToArray(char *splitArray[], char Str[]) {
    int i = 0, start = 0, end = 0, j = 0;

    while (i < strlen(Str) + 1) {
        //if the end changed so we arrived to the last char at the word
        if (end != 0) {
            // +1 for the '\0'
            int length = (end - start + 1);
            //every element in the array we need to allocate it exactly  to size of the word
            splitArray[j] = (char *) calloc(length , sizeof(char) );
            //if we cannot allocate the memory, so we have to free every thing we allocate it and exit from the program
            if (splitArray[j] == NULL) {
                for (int k = 0; k < j; ++k)
                    free(splitArray[k]);
                perror("error can't allocate space in memory");
                exit(1);
            } else {
                //copy the word to the element into the array
                strncpy(splitArray[j], &Str[start], length);
                j++;
            }
            end = 0;
        }
        //check if it's the first char at the word
        if (i > 0 && Str[i] != ' ' && Str[i - 1] == ' ')
            start = i;
        //check if it's the last char at the word
        if (Str[i] != ' ' && (Str[i + 1] == ' ' || Str[i + 1] == '\0'))
            end = i;

        i++;
    }
    //add the last element NULL for execvp method
    splitArray[j] = NULL;
}
