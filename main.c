#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <linux/limits.h>


void splitToArray(char*[], char[]);

const char *count(char[], int *, int *);

void exHistory(int,char*);

void history();
void loop();

void readFromFile(int);

void writeToFile(char*);

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
                fgets(str, 510, stdin);
                //because the user press enter so '\n' enter to the input string in the last index, so we put '\0'
                str[strlen(str) - 1] = '\0';
                if (str[0] == ' ' || str[strlen(str) - 1] == ' ')
                    fprintf(stderr,"You have entered space/s before/after the command \n");
                else if (str[0] == '!'&&str[1]!=' ') {
                    int k = 1;
                    int check = 0;
                    while (k < strlen(str) - 1) {
                        if (str[k] <= '0' || str[k] >= '9')
                            check = 1;
                        k++;
                    }
                    if (check == 1)
                        printf("enter only numbers after '!'\n");
                    else {
                    int line = atoi(&str[1]);
                    readFromFile(line);
                }
            }
                else {
                    const char *word = count(str, &charCount, &wordCount);
                    if (wordCount != 0) {
                        if(strcmp(word, "history") == 0){
                            writeToFile(str);
                            history();
                            cmdCount++;
                            TotalWord ++;
                        }
                        else if (strcmp(word, "cd") == 0) {
                            printf("command not supported (Yet)\n");
                            cmdCount++;
                            TotalWord += wordCount;
                        } else if (wordCount == 1 && strcmp(word, "done") == 0) {
                            printf("Num of commands: %d\n", (cmdCount + 1));
                            printf("Total number of words in all commands: %d !", TotalWord);
                            break;
                        } else{
                            writeToFile(str);
                            ex(wordCount, str, &TotalWord, &cmdCount);
                        }
                    }
                }
            }
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
        char c = (char)fgetc(readFile);
        while (c != EOF) {
            if (c == '\n') counter++;
            c = (char)fgetc(readFile);

        }
        fclose(readFile);
    }
    return counter;
}
void history() {
    FILE *readFile;
    readFile = fopen("file.txt", "r");
    if (readFile == NULL) {
        perror("file doesn't exist");
        exit(1);
    } else {
        char c =(char) fgetc(readFile);//read character by character and print it to the
        while (c != EOF) {
            printf("%c", c);
            c = (char)fgetc(readFile);
        }
        fclose(readFile);
    }
}
void readFromFile(int line){
    int numberOfLines=countLine();
    if(line>numberOfLines||line<1){
        printf("the line number doesn't exist\n");
        return;
    }
    FILE* read;
    read= fopen("file.txt","r");
    if(read==NULL){
        perror("can't open file");
        exit(1);
    } else{
        char cmd[512];
        int i=0;
        while(i<line){
            fgets(cmd,512,read);
            i++;
        }
        cmd[strlen(cmd)-1]='\0';
        int WC=0,CC=0;
        writeToFile(cmd);
        count(cmd,&CC,&WC);
        exHistory(WC,cmd);
    }
    fclose(read);
}
void exHistory(int WC,char* str){
    char *arrayOfWords[WC + 1];
    splitToArray(arrayOfWords, str);
    pid_t x = fork();
    if (x < 0) {
        perror("Fork unsuccessfully");
        for (int j = 0; j < WC; j++)
            free(arrayOfWords[j]);
        exit(1);
    }
    if (x == 0) {
        if (-1 == execvp(arrayOfWords[0], arrayOfWords)){
            perror("command not supported (Yet)");
            exit(1);
        }

        exit(0);
    } else {
        for (int j = 0; j < WC; j++)
            free(arrayOfWords[j]);
        wait(NULL);
    }
}
void writeToFile(char *str){
    FILE* write;
    write= fopen("file.txt","a");
    if(write==NULL){
        perror("can't create/open file");
        exit(1);
    }
    else{
        fprintf(write,"%s\n",str);
        fclose(write);
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
    if (strcmp(word, "history") == 0&&(*charCount)==7)
        return "history";
    if (strcmp(word, "cd") == 0)
        return "cd";
    return "";
}

void ex(int WC, char *str, int *Total, int *Cmd) {
    char *arrayOfWords[WC + 1];
    splitToArray(arrayOfWords, str);
    pid_t x = fork();
    if (x < 0) {
        perror("Fork unsuccessfully");
        for (int j = 0; j < WC; j++)
            free(arrayOfWords[j]);
        exit(1);
    }
    if (x == 0) {
        if (-1 == execvp(arrayOfWords[0], arrayOfWords)){
            perror("command not supported (Yet)");
            exit(1);
        }

        exit(0);
    } else {
        for (int j = 0; j < WC; j++)
            free(arrayOfWords[j]);
        wait(NULL);
        (*Cmd)++;
        (*Total) += WC;

    }
}

void splitToArray(char *splitArray[], char Str[]) {
    int i = 0, start = 0, end = 0, j = 0;

    while (i < strlen(Str) + 1) {
        if (end != 0) {
            int length = (end - start + 1);
            splitArray[j] = (char *) calloc(length , sizeof(char) );
            if (splitArray[j] == NULL) {
                for (int k = 0; k < j; ++k)
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