#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/syslimits.h>

void splitToArray(char*[], char[], int);

const char *count(char[], int *, int *);

void loop();

void ex(int, char *, int*, int*);

int main() {
    loop();
}

void loop() {
    //I added 2 to the length because there's '\n\0' at the end of the str
    char str[512];
    char cwd[PATH_MAX];
    int cmdCount = 0, TotalWord = 0;

    while (-1) {
        int charCount = 0, wordCount = 0;
        if (getcwd(cwd, sizeof(cwd)) != NULL) {
            printf("%s>", cwd);
            fgets(str, 510, stdin);
            //because the user press enter so '\n' enter to the input string in the last index, so we put '\0'
            str[strlen(str) - 1] = '\0';
            if (str[0] == ' ' || str[strlen(str) - 1] == ' ')
                printf("You have entered space/s before/after the command \n");
            else {
                const char *word = count(str, &charCount, &wordCount);
                if (wordCount != 0) {
                    if (strcmp(word, "cd") == 0) {
                        printf("command not supported (Yet)\n");
                        cmdCount++;
                        TotalWord += wordCount;
                    } else if (wordCount == 1 && strcmp(word, "done") == 0) {
                        printf("Num of commands: %d\n", (cmdCount + 1));
                        printf("Total number of words in all commands: %d !", TotalWord);
                        break;
                    } else
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

void ex(int WC, char *str, int *Total, int *cmds) {
    char *arrayOfWords[WC + 1];
    splitToArray(arrayOfWords, str, WC);
    pid_t x = fork();
    if (x < 0) {
        perror("Fork unsuccessfully");
        exit(1);
    }
    if (x == 0) {
        if (-1 == execvp(arrayOfWords[0], arrayOfWords))
            perror("command not supported (Yet)");

        exit(0);
    } else {
        for (int j = 0; j < WC; j++)
            free(arrayOfWords[j]);
        wait(NULL);
        (*cmds)++;
        (*Total) += WC;
    }
}

void splitToArray(char *splitArray[], char Str[], int N) {
    int i = 0, start = 0, end = 0, j = 0;

    while (i < strlen(Str) + 1) {
        if (end != 0) {
            int length = (end - start + 1);
            splitArray[j] = (char *) calloc(length , sizeof(char) );
            if (splitArray[j] == NULL) {
                for (int k = 0; k < N; ++k)
                    free(splitArray[k]);
                perror("error can't allocate space in memory");
                exit(1);
            } else {
                strncpy(splitArray[j], &Str[start], length);
                j++;
            }
            end = 0;
        }

        if (i > 0 && Str[i] != ' ' && Str[i - 1] == ' ')
            start = i;
        if (Str[i] != ' ' && (Str[i + 1] == ' ' || Str[i + 1] == '\0'))
            end = i;

        i++;
    }
    splitArray[j] = NULL;
}