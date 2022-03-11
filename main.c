#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <linux/limits.h>
#include <sys/wait.h>

void splitToArray(char *splitArray[], char Str[], int N);

void history();

const char *count(char[], int *, int *);

int countLine();

void loop();

int main() {

    loop();


}


void loop() {
    //I added 2 to the length because there's '\n\0' at the end of the str
    char str[512];
    char cwd[PATH_MAX];
    int cmdCount=0,TotalWord=0;
    FILE *writeFile;
    writeFile = fopen("file.txt", "a");
    if (writeFile == NULL) {
        perror("cant create a new file");
        exit(1);
    } else {
        int i = countLine();
        while (-1) {
            int charCount = 0, wordCount = 0;
//            printf("Enter string, or \"exit\" to end program:\n");
            if (getcwd(cwd, sizeof(cwd)) != NULL) {
                printf("%s>", cwd);

                fgets(str, 510, stdin);
                //because the user press enter so '\n' enter to the input string in the last index, so we put '\0'
                str[strlen(str) -1] = '\0';
                if (strcmp(str, "exit") == 0) break;
                else if (strcmp(str, "history") == 0) {
                    //close the file to save every input before "history" to the file, to can read it in the history function
                    fprintf(writeFile, "%d: %s\n", i, str);
                    i++;
                    fclose(writeFile);
                    history();
                    //open the file again to continue to append new input in it
                    writeFile = fopen("file.txt", "a");
                } else {
                    const char *word = count(str, &charCount, &wordCount);
                    if (wordCount != 0) {
                        // if their only one word, so I have to check if it "exit" or "history"
                        if (wordCount == 1 && strcmp(word, "exit") ==0)
                            break;
                        else if (wordCount == 1 && strcmp(word, "history") == 0) {
                            fprintf(writeFile, "%d: %s\n", i, str);
                            i++;
                            fclose(writeFile);
                            history();
                            writeFile = fopen("file.txt", "a");
                        }
                        else if(wordCount == 1 && strcmp(word, "done") ==0){
                            printf("Num of commands: %d\n",(cmdCount+1));
                            printf("Total number of words in all commands: %d !",(TotalWord));
                            break;
                        }
                        else {
//                        printf("%d words\n%d chars\n", wordCount, charCount);
                            fprintf(writeFile, "%d: %s\n", i, str);
                            i++;
                            char *arrayOfWords[wordCount + 1];
                            splitToArray(arrayOfWords, str, wordCount);
                            pid_t x=fork();
                            if(x<0){
                                perror("Fork unsuccessfully");
                                exit(1);
                            }
                            if(x==0){
                                if(-1==execvp(arrayOfWords[0],arrayOfWords))
                                    printf("This command Not Supported Yet (%s)\n",arrayOfWords[0]);
                                for (int j = 0; j < wordCount; j++)
                                    free(arrayOfWords[j]);
                                exit(1);
                            }
                                wait(NULL);
                            cmdCount++;
                            TotalWord+=wordCount;

                        }

                    }

                }

            }

        }

        fclose(writeFile);

        return;

    }
}

void history() {
    FILE *readFile;
    readFile = fopen("file.txt", "r");
    if (readFile == NULL) {
        perror("file doesn't exist");
        exit(1);
    } else {
        char c = fgetc(readFile);//read character by character and print it to the
        while (c != EOF) {
            printf("%c", c);
            c = fgetc(readFile);
        }
        fclose(readFile);
    }
}

int countLine() {
    //count how many lines in the file
    int counter = 0;
    FILE *readFile;
    readFile = fopen("file.txt", "r");
    if (readFile == NULL) {
        perror("file doesn't exist");
        exit(1);
    } else {
        char c = fgetc(readFile);
        while (c != EOF) {
            if (c == '\n') counter++;
            c = fgetc(readFile);

        }
        fclose(readFile);
    }
    return counter;
}

const char *count(char str[], int *charCount, int *wordCount) {
    int i = 0;
    int len = strlen(str);
    char word[7];
    int wordInd = 0;

    while (i < len) {
        if (str[i] != ' ') {//if the char is not ' ' so we have to count iy
            (*charCount)++;
            if ((*wordCount) < 1 && (*charCount) <
                                    8) { // if we are in the first word we enter the first 8 chars to the word to check if it "exit" or "history"
                word[wordInd] = str[i];
                wordInd++;
            }

        } else if ((*charCount) != 0 && str[i + 1] != ' ' && str[i + 1] !=
                                                             '\0')//if the str[i] is ' ' and the next element isn't ' ' or '\0' so it's an legal char for a new word
            (*wordCount)++;
        i++;
    }
    if ((*charCount) >
        0)//because we count the splits so the number of splits is more than 0, so there is (1+splits) words
        (*wordCount)++;
    word[wordInd] = '\0';// to finish the string if it less than 7 chars like "exit"
    if (strcmp(word, "exit") == 0)// if the first word in the input is "exit" return  "exit"
        return "exit";
    if (strcmp(word, "history") == 0 &&
        (*charCount) == 7)// if the first word in the input is "history" return  "history"
        return "history";
    if (strcmp(word, "done") == 0 )
        return "done";
    return "";
}


void splitToArray(char *splitArray[], char Str[], int N) {
    int i = 0, start = 0, end = 0, j = 0;

    while (i < strlen(Str) + 1) {
        if (end != 0) {
            int length = (end - start + 1);
            splitArray[j] = (char *) malloc(sizeof(char) * length);
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
    splitArray[j]=NULL;

}