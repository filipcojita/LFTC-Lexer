#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "lexer.h"
#include "utils.h"

Token tokens[MAX_TOKENS];
int nTokens = 0;
int line = 1;  // Tracks the current line number

// Adds a token to the list, sets its code and line number, and returns it
Token *addTk(int code) {
    if (nTokens == MAX_TOKENS) {
        err("Too many tokens.");
    }
    Token *tk = &tokens[nTokens++];
    tk->code = code;
    tk->line = line;
    return tk;
}

// Copies a string from 'begin' to 'end' into 'dst' and returns 'dst'
char *copyn(char *dst, const char *begin, const char *end) {
    if (end - begin > MAX_STR) {
        err("String too long.");
    }
    char *p = dst;
    while (begin != end) {
        *p++ = *begin++;
    }
    *p = '\0';  // Null-terminate the string
    return dst;
}

// Tokenizes the input string and populates the tokens array
// Tokenizes the input string and populates the tokens array
void tokenize(const char *pch) {
    const char *start;
    Token *tk;
    char buf[MAX_STR + 1];

    while (*pch) {
        switch (*pch) {
            case ' ': case '\t': case '\r':  // Ignore spaces
                pch++;
                break;

            case '\n':  // Handle newlines and increment line
                line++;
                pch++;
                break;

            case '#':  // Comments: until newline
                while (*pch && *pch != '\n') pch++;
                break;

            case '"':  // Handle strings within quotes
                start = ++pch;
                while (*pch && *pch != '"') pch++;
                if (*pch == '"') {
                    tk = addTk(STR);
                    copyn(tk->text, start, pch);
                    pch++;
                } else {
                    err("Missing \" on line %d", line);
                }
                break;

            case ',': addTk(COMMA); pch++; break;
            case ':': addTk(COLON); pch++; break;
            case ';': addTk(SEMICOLON); pch++; break;
            case '(': addTk(LPAR); pch++; break;
            case ')': addTk(RPAR); pch++; break;

            case '=':  // Handle '=' and '=='
                if (pch[1] == '=') {
                    addTk(EQUAL); pch += 2;
                } else {
                    addTk(ASSIGN); pch++;
                }
                break;

            case '+': addTk(ADD); pch++; break;
            case '-': addTk(SUB); pch++; break;
            case '*': addTk(MUL); pch++; break;
            case '/': addTk(DIV); pch++; break;

            case '!':  // Handle '!' and '!='
                if (pch[1] == '=') {
                    addTk(NOTEQ); pch += 2;
                } else {
                    addTk(NOT); pch++;
                }
                break;

            case '<': addTk(LESS); pch++; break;
            case '>':  // Handle '>' and '>='
                if (pch[1] == '=') {
                    addTk(GREATEREQ); pch += 2;
                } else {
                    addTk(GREATER); pch++;
                }
                break;

            default:
                if (isdigit(*pch)) {  // Handle integer and real numbers
                    start = pch;
                    while (isdigit(*pch)) pch++;
                    if (*pch == '.') {  // Check for real numbers
                        pch++;
                        if (!isdigit(*pch)) {
                            err("Real number without digits after decimal on line %d", line);
                        }
                        while (isdigit(*pch)) pch++;
                        tk = addTk(REAL);
                        copyn(tk->text, start, pch);
                        tk->r = atof(tk->text);
                    } else {
                        tk = addTk(INT);
                        copyn(tk->text, start, pch);
                        tk->i = atoi(tk->text);
                    }
                } else if (isalpha(*pch) || *pch == '_') {  // Handle identifiers and keywords
                    start = pch++;
                    while (isalnum(*pch) || *pch == '_') pch++;
                    char *text = copyn(buf, start, pch);
                    if (strcmp(text, "int") == 0) addTk(TYPE_INT);
                    else if (strcmp(text, "real") == 0) addTk(TYPE_REAL);
                    else if (strcmp(text, "str") == 0) addTk(TYPE_STR);
                    else if (strcmp(text, "var") == 0) addTk(VAR);
                    else if (strcmp(text, "if") == 0) addTk(IF);
                    else if (strcmp(text, "else") == 0) addTk(ELSE);
                    else if (strcmp(text, "while") == 0) addTk(WHILE);
                    else if (strcmp(text, "return") == 0) addTk(RETURN);
                    else if (strcmp(text, "function") == 0) addTk(FUNCTION);
                    else if (strcmp(text, "end") == 0) addTk(END);
                    else if (strcmp(text, "and") == 0) addTk(AND);
                    else if (strcmp(text, "or") == 0) addTk(OR);
                    else {
                        tk = addTk(ID);
                        strcpy(tk->text, text);
                    }
                } else {
                    err("Unknown symbol '%c' (ASCII: %d) on line %d", *pch, *pch, line);
                    pch++;
                }
                break;
        }
    }
    addTk(FINISH);
}


// Displays the tokens with their codes, line numbers, and text (if any)
void showTokens() {
    for (int i = 0; i < nTokens; i++) {
        Token *tk = &tokens[i];

        // Print line number
        printf("%d ", tk->line);

        // Print token type based on its code
        switch (tk->code) {
            case FUNCTION:   printf("FUNCTION"); break;
            case IF:         printf("IF"); break;
            case ELSE:       printf("ELSE"); break;
            case WHILE:      printf("WHILE"); break;
            case RETURN:     printf("RETURN"); break;
            case END:        printf("END"); break;
            case COMMA:      printf("COMMA"); break;
            case COLON:      printf("COLON"); break;
            case SEMICOLON:  printf("SEMICOLON"); break;
            case LPAR:       printf("LPAR"); break;
            case RPAR:       printf("RPAR"); break;
            case ASSIGN:     printf("ASSIGN"); break;
            case EQUAL:      printf("EQUAL"); break;
            case ADD:        printf("ADD"); break;
            case SUB:        printf("SUB"); break;
            case MUL:        printf("MUL"); break;
            case DIV:        printf("DIV"); break;
            case NOT:        printf("NOT"); break;
            case NOTEQ:      printf("NOTEQ"); break;
            case LESS:       printf("LESS"); break;
            case GREATER:    printf("GREATER"); break;
            case GREATEREQ:  printf("GREATEREQ"); break;
            case ID:         printf("ID:%s", tk->text); break;
            case VAR:        printf("VAR"); break;
            case TYPE_INT:   printf("TYPE_INT"); break;
            case TYPE_REAL:  printf("TYPE_REAL"); break;
            case TYPE_STR:   printf("TYPE_STR"); break;
            case FINISH:     printf("FINISH"); break;

            // Handle integer and real literals
            case INT:        printf("INT:%d", tk->i); break;
            case REAL:       printf("REAL:%.5f", tk->r); break;

            // Handle string literals
            case STR:        printf("STR:%s", tk->text); break;

            default:         printf("UNKNOWN"); break;
        }

        // Newline after each token
        printf("\n");
    }
}