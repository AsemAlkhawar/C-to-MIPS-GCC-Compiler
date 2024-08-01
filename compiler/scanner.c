/*
 * File: scanner.c
 * Author: Ahmad Gaber
 * Purpose: Implementation of the scanner for the G0 subset of C-- language.
 */

#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include "scanner.h"


char* lexeme = NULL;
int line_number = 1;


int get_token() {
    int current_char;
    size_t lexeme_size = 100;
    char* temp_lexeme = realloc(lexeme, lexeme_size);
    if (temp_lexeme == NULL) {
        free(lexeme);  // Free the original lexeme if realloc fails
        return UNDEF;  // Return an undefined token or appropriate error handling
    }
    lexeme = temp_lexeme;
    size_t lexeme_len = 0;

    while ((current_char = getchar()) != EOF) {

        if (current_char == '\n') {
            line_number++;
        }

        // Skip whitespace
        if (isspace(current_char)) {
            continue;
        }

        // Clear previous lexeme
        memset(lexeme, 0, lexeme_size);
        lexeme_len = 0;

        // Handle comments
        if (current_char == '/') {
            int next_char = getchar();
            if (next_char == '*') { // Start of a comment
                int prev_char = 0;
                while (1) {
                    current_char = getchar();
                    if (current_char == EOF || (prev_char == '*' && current_char == '/')) {
                        break;
                    }
                    prev_char = current_char;
                }
                continue; // Continue to next character after comment ends
            }
            else {
                ungetc(next_char, stdin); // It's not a comment
            }
        }

        // Handle identifiers (and keywords as special cases)
        if (isalpha(current_char) || current_char == '_') {
            do {
                lexeme[lexeme_len++] = current_char;
                if (lexeme_len == lexeme_size) {
                    lexeme_size *= 2;
                    temp_lexeme = realloc(lexeme, lexeme_size);
                    if (temp_lexeme == NULL) {
                        free(lexeme);
                        return UNDEF;
                    }
                    lexeme = temp_lexeme;
                }
                current_char = getchar();
            } while (isalnum(current_char) || current_char == '_');
            ungetc(current_char, stdin); // Put back the last non-identifier character

            lexeme[lexeme_len] = '\0'; // Null-terminate the string

            // Check for keywords
            if (strcmp(lexeme, "int") == 0) return kwINT;
            if (strcmp(lexeme, "if") == 0) return kwIF;
            if (strcmp(lexeme, "else") == 0) return kwELSE;
            if (strcmp(lexeme, "while") == 0) return kwWHILE;
            if (strcmp(lexeme, "return") == 0) return kwRETURN;

            return ID;
        }

        // Handle integer constants
        if (isdigit(current_char)) {
            do {
                lexeme[lexeme_len++] = current_char;
                if (lexeme_len == lexeme_size) {
                    lexeme_size *= 2;
                    temp_lexeme = realloc(lexeme, lexeme_size);
                    if (temp_lexeme == NULL) {
                        free(lexeme);
                        return UNDEF;
                    }
                    lexeme = temp_lexeme;
                }
                current_char = getchar();
            } while (isdigit(current_char));
            ungetc(current_char, stdin); // Put back the last non-digit character

            lexeme[lexeme_len] = '\0'; // Null-terminate the string
            return INTCON;
        }

        // Handle operators and punctuations
        switch (current_char) {
        case '(':
            lexeme[0] = '('; lexeme[1] = '\0'; return LPAREN;
        case ')':
            lexeme[0] = ')'; lexeme[1] = '\0'; return RPAREN;
        case '{':
            lexeme[0] = '{'; lexeme[1] = '\0'; return LBRACE;
        case '}':
            lexeme[0] = '}'; lexeme[1] = '\0'; return RBRACE;
        case ',':
            lexeme[0] = ','; lexeme[1] = '\0'; return COMMA;
        case ';':
            lexeme[0] = ';'; lexeme[1] = '\0'; return SEMI;
        case '=': {
            int next_char = getchar();
            if (next_char == '=') {
                lexeme[0] = '='; lexeme[1] = '='; lexeme[2] = '\0';
                return opEQ;
            }
            ungetc(next_char, stdin);
            lexeme[0] = '='; lexeme[1] = '\0';
            return opASSG;
        }
        case '+':
            lexeme[0] = '+'; lexeme[1] = '\0';
            return opADD;
        case '-':
            lexeme[0] = '-'; lexeme[1] = '\0'; return opSUB;
        case '*':
            lexeme[0] = '*'; lexeme[1] = '\0'; return opMUL;
        case '/':
            lexeme[0] = '/'; lexeme[1] = '\0'; return opDIV;
        case '!': {
            int next_char = getchar();
            if (next_char == '=') {
                lexeme[0] = '!'; lexeme[1] = '='; lexeme[2] = '\0';
                return opNE;
            }
            ungetc(next_char, stdin);
            lexeme[0] = '!'; lexeme[1] = '\0';
            return opNOT;
        }
        case '>': {
            int next_char = getchar();
            if (next_char == '=') {
                lexeme[0] = '>'; lexeme[1] = '='; lexeme[2] = '\0';
                return opGE;
            }
            ungetc(next_char, stdin);
            lexeme[0] = '>'; lexeme[1] = '\0';
            return opGT;
        }
        case '<': {
            int next_char = getchar();
            if (next_char == '=') {
                lexeme[0] = '<'; lexeme[1] = '='; lexeme[2] = '\0';
                return opLE;
            }
            ungetc(next_char, stdin);
            lexeme[0] = '<'; lexeme[1] = '\0';
            return opLT;
        }
        case '&': {
            int next_char = getchar();
            if (next_char == '&') {
                lexeme[0] = '&'; lexeme[1] = '&'; lexeme[2] = '\0';
                return opAND;
            }
            ungetc(next_char, stdin);
            // Handle error for single '&' (if necessary)
        }
        case '|': {
            int next_char = getchar();
            if (next_char == '|') {
                lexeme[0] = '|'; lexeme[1] = '|'; lexeme[2] = '\0';
                return opOR;
            }
            ungetc(next_char, stdin);
            // Handle error for single '|' (if necessary)
        }

        }
    }

    free(lexeme);
    return EOF; // End of input
}