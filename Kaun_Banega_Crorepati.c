#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <conio.h>    // Required for Windows _kbhit() and _getch()
#include <windows.h>  // Required for Windows Sleep()
#include <time.h>

#define MAX_QUES_LEN 300
#define MAX_OPTION_LEN 100

volatile int timeout_happened = 0;

const char* COLOR_END = "\033[0m";
const char* RED = "\033[1;31m";
const char* GREEN = "\033[1;32m";
const char* YELLOW = "\033[1;33m";
const char* BLUE = "\033[1;34m";
const char* PINK = "\033[1;35m";
const char* AQUA = "\033[1;36m";

typedef struct {
    char text[MAX_QUES_LEN];
    char options[4][MAX_OPTION_LEN];
    char correct_option;
    int timeout;
    int prize_money;
} Question;

int read_questions(char* file_name, Question** questions);
void print_formatted_question(Question question);
void play_game(Question* questions, int no_of_questions);
int use_lifeline(Question* question, int* lifeline);

void set_terminal_attributes() {}
void reset_terminal_attributes() {}

int main() {
    srand(time(NULL));
    set_terminal_attributes();
    
    printf("\n\t\t%sChalo Kheltay hain KAUN BANEGA CROREPATI !!!%s\n", PINK, COLOR_END);
    
    Question* questions = NULL;
    int no_of_questions = read_questions("questions.txt", &questions);
    
    if (no_of_questions > 0 && questions != NULL) {
        play_game(questions, no_of_questions);
        free(questions);
    } else {
        printf("\nError: No questions found or loaded.\n");
    }
    return 0;
}

void play_game(Question* questions, int no_of_questions) {
    int money_won = 0;
    int lifeline[] = {1, 1};

    for (int i = 0; i < no_of_questions; i++) {
        print_formatted_question(questions[i]);
        
        timeout_happened = 0;
        char ch = '\0';
        int seconds_left = questions[i].timeout;
        
        // Clean Windows Timer Display
        printf("\nTime Remaining: [ ");
        while (seconds_left > 0) {
            printf("%d ", seconds_left);
            fflush(stdout);
            
            // Check for input every 100ms for high responsiveness
            for (int ms = 0; ms < 10; ms++) {
                if (_kbhit()) { 
                    ch = _getch(); 
                    break;
                }
                Sleep(100); 
            }
            
            if (ch != '\0') {
                break; // Exit timer loop if user pressed a key
            }
            seconds_left--;
        }
        
        if (seconds_left <= 0 && ch == '\0') {
            timeout_happened = 1;
            printf("]\n\n%sTime out!!!!! Game Over!%s\n", RED, COLOR_END);
            break;
        }

        // Complete the timer bracket visually and print input cleanly
        printf("]\n");
        printf("%sYour Input: %s%c\n", GREEN, COLOR_END, ch);
        ch = toupper(ch);

        if (ch == 'L') {
            int value = use_lifeline(&questions[i], lifeline);
            if (value != 2) {
                i--; // Repeat current question if they didn't choose 'Skip'
            }
            continue;
        }

        if (ch == toupper(questions[i].correct_option)) {
            printf("\n%sCorrect!%s", GREEN, COLOR_END);
            money_won = questions[i].prize_money;
            printf("\n%sYou have won: Rs %d%s\n", BLUE, money_won, COLOR_END);
        } else {
            printf("\n%sWrong! Correct answer is %c.%s\n", RED, questions[i].correct_option, COLOR_END);
            break;
        }
    }
    printf("\n\n%sGame Over! Your total winnings are: Rs %d%s\n", BLUE, money_won, COLOR_END);
}

int use_lifeline(Question* question, int* lifeline) {
    printf("\n%sAvailable Lifelines:%s", PINK, COLOR_END);
    if (lifeline[0]) printf("\n%s  1. Fifty-Fifty (50/50)%s", PINK, COLOR_END);
    if (lifeline[1]) printf("\n%s  2. Skip the Question%s", PINK, COLOR_END);
    printf("\n\n%sChoose a lifeline or 0 to return: %s", PINK, COLOR_END);

    char ch = _getch();
    printf("%c\n", ch);

    switch (ch) {
        case '1':
            if (lifeline[0]) {
                lifeline[0] = 0;
                int removed = 0;
                while (removed < 2) {
                    int num = rand() % 4;
                    if ((num + 'A') != toupper(question->correct_option) && question->options[num][0] != '\0') {
                        question->options[num][0] = '\0'; 
                        removed++;
                    }
                }
                return 1;
            } else {
                printf("\nFifty-Fifty Lifeline already used!\n");
            }
            break;
        case '2':
            if (lifeline[1]) {
                lifeline[1] = 0;
                return 2; 
            } else {
                printf("\nSkip Lifeline already used!\n");
            }
            break;
        default:
            printf("\n%sReturning to the Question.%s\n", PINK, COLOR_END);
            break;
    }
    return 0;
}

void print_formatted_question(Question question) {
    printf("\n============================================================\n");
    printf("%sQUESTION: %s%s\n\n", YELLOW, question.text, COLOR_END);
    
    for (int i = 0; i < 4; i++) {
        if (question.options[i][0] != '\0') {
            printf("  %s%c. %s%s\n", AQUA, ('A' + i), question.options[i], COLOR_END);
        }
    }
    printf("\n%sPress your answer key (A, B, C, D) or L for lifeline directly...%s", GREEN, COLOR_END);
}

int read_questions(char* file_name, Question** questions) {
    FILE *file = fopen(file_name, "r");
    if (file == NULL) {
        printf("\nUnable to open the questions bank. Make sure 'questions.txt' exists.");
        return 0;
    }
    char str[MAX_QUES_LEN];
    int no_of_lines = 0;
    while (fgets(str, MAX_QUES_LEN, file)) {
        no_of_lines++;
    }
    
    int no_of_questions = no_of_lines / 8;
    if (no_of_questions == 0) {
        fclose(file);
        return 0;
    }

    *questions = (Question *) malloc(no_of_questions * sizeof(Question));
    rewind(file);
    
    for (int i = 0; i < no_of_questions; i++) {
        fgets((*questions)[i].text, MAX_QUES_LEN, file);
        (*questions)[i].text[strcspn((*questions)[i].text, "\r\n")] = 0;
        
        for (int j = 0; j < 4; j++) {
            fgets((*questions)[i].options[j], MAX_OPTION_LEN, file);
            (*questions)[i].options[j][strcspn((*questions)[i].options[j], "\r\n")] = 0;
        }
        char option[10];
        fgets(option, 10, file);
        (*questions)[i].correct_option = option[0];

        char timeout[10];
        fgets(timeout, 10, file);
        (*questions)[i].timeout = atoi(timeout);

        char prize_money[10];
        fgets(prize_money, 10, file);
        (*questions)[i].prize_money = atoi(prize_money);
    }
    fclose(file);
    return no_of_questions;
}