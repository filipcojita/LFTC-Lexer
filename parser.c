#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>

#include "lexer.h"

int iTk;            // Iterator in the tokens array
Token *consumed;    // Last consumed token

// Reports a syntax error including the line number
_Noreturn void tkerr(const char *fmt, ...) {
    fprintf(stderr, "Syntax error on line %d: ", tokens[iTk].line);
    va_list va;
    va_start(va, fmt);
    vfprintf(stderr, fmt, va);
    va_end(va);
    fprintf(stderr, "\n");
    exit(EXIT_FAILURE);
}

// Consumes a token if it matches the expected code, returns true if successful
bool consume(int code) {
    if (tokens[iTk].code == code) {
        consumed = &tokens[iTk++];
        return true;
    }
    return false;
}

// Forward declarations for grammar functions
bool program();
bool defVar();
bool baseType();
bool defFunc();
bool funcParams();
bool funcParam();
bool block();
bool instr();
bool expr();
bool exprLogic();
bool exprAssign();
bool exprComp();
bool exprAdd();
bool exprMul();
bool exprPrefix();
bool factor();

// program ::= ( defVar | defFunc | block )* FINISH
bool program() {
    while (defVar() || defFunc() || block()) {}
    if (consume(FINISH)) {
        return true;
    } else {
        tkerr("Expected end of program");
    }
    return false;
}

// defVar ::= VAR ID COLON baseType SEMICOLON
bool defVar() {
    if (consume(VAR)) {
        if (consume(ID)) {
            if (consume(COLON)) {
                if (baseType()) {
                    if (consume(SEMICOLON)) {
                        return true;
                    } else {
                        tkerr("Expected ';' after variable declaration");
                    }
                } else {
                    tkerr("Expected base type after ':' in variable declaration");
                }
            } else {
                tkerr("Expected ':' after variable name in declaration");
            }
        } else {
            tkerr("Expected identifier after 'var'");
        }
    }
    return false;
}

// baseType ::= TYPE_INT | TYPE_REAL | TYPE_STR
bool baseType() {
    return consume(TYPE_INT) || consume(TYPE_REAL) || consume(TYPE_STR);
}

// defFunc ::= FUNCTION ID LPAR funcParams? RPAR COLON baseType defVar* block END
bool defFunc() {
    if (consume(FUNCTION)) {
        if (consume(ID)) {
            if (consume(LPAR)) {
                funcParams();
                if (consume(RPAR)) {
                    if (consume(COLON)) {
                        if (baseType()) {
                            while (defVar()) {}  // Optional variable declarations in function
                            if (block()) {
                                if (consume(END)) {
                                    return true;
                                } else {
                                    tkerr("Expected 'end' after function block");
                                }
                            } else {
                                tkerr("Expected block in function body");
                            }
                        } else {
                            tkerr("Expected base type after ':' in function return type");
                        }
                    } else {
                        tkerr("Expected ':' after function parameters");
                    }
                } else {
                    tkerr("Expected ')' after function parameters");
                }
            } else {
                tkerr("Expected '(' after function name");
            }
        } else {
            tkerr("Expected identifier after 'function'");
        }
    }
    return false;
}

// funcParams ::= funcParam ( COMMA funcParam )*
bool funcParams() {
    if (funcParam()) {
        while (consume(COMMA)) {
            if (!funcParam()) {
                tkerr("Expected parameter after ','");
            }
        }
        return true;
    }
    return false;
}

// funcParam ::= ID COLON baseType
bool funcParam() {
    if (consume(ID)) {
        if (consume(COLON)) {
            if (baseType()) {
                return true;
            } else {
                tkerr("Expected base type after ':' in parameter declaration");
            }
        } else {
            tkerr("Expected ':' after parameter name");
        }
    }
    return false;
}

// block ::= instr+
bool block() {
    if (!instr()) {
        return false;
    }
    while (instr()) {}
    return true;
}

// instr ::= expr? SEMICOLON
//         | IF LPAR expr RPAR block ( ELSE block )? END
//         | RETURN expr SEMICOLON
//         | WHILE LPAR expr RPAR block END
bool instr() {
    if (expr()) {
        if (consume(SEMICOLON)) {
            return true;
        } else {
            tkerr("Expected ';' after expression");
        }
    } else if (consume(IF)) {
        if (consume(LPAR)) {
            if (expr()) {
                if (consume(RPAR)) {
                    if (block()) {
                        if (consume(ELSE)) {
                            if (!block()) {
                                tkerr("Expected block after 'else'");
                            }
                        }
                        if (consume(END)) {
                            return true;
                        } else {
                            tkerr("Expected 'end' after 'if' block");
                        }
                    } else {
                        tkerr("Expected block after 'if' condition");
                    }
                } else {
                    tkerr("Expected ')' after 'if' condition");
                }
            } else {
                tkerr("Expected expression after '(' in 'if' condition");
            }
        } else {
            tkerr("Expected '(' after 'if'");
        }
    } else if (consume(RETURN)) {
        if (expr()) {
            if (consume(SEMICOLON)) {
                return true;
            } else {
                tkerr("Expected ';' after 'return' expression");
            }
        } else {
            tkerr("Expected expression after 'return'");
        }
    } else if (consume(WHILE)) {
        if (consume(LPAR)) {
            if (expr()) {
                if (consume(RPAR)) {
                    if (block()) {
                        if (consume(END)) {
                            return true;
                        } else {
                            tkerr("Expected 'end' after 'while' block");
                        }
                    } else {
                        tkerr("Expected block after 'while' condition");
                    }
                } else {
                    tkerr("Expected ')' after 'while' condition");
                }
            } else {
                tkerr("Expected expression after '(' in 'while' condition");
            }
        } else {
            tkerr("Expected '(' after 'while'");
        }
    }
    return false;
}

// expr ::= exprLogic
bool expr() {
    return exprLogic();
}

// exprLogic ::= exprAssign ( ( AND | OR ) exprAssign )*
bool exprLogic() {
    if (exprAssign()) {
        while (consume(AND) || consume(OR)) {
            if (!exprAssign()) {
                tkerr("Expected expression after '&&' or '||'");
            }
        }
        return true;
    }
    return false;
}

// exprAssign ::= ( ID ASSIGN )? exprComp
bool exprAssign() {
    if (consume(ID)) {
        if (consume(ASSIGN)) {
            if (!exprComp()) {
                tkerr("Expected expression after '='");
            }
            return true;
        }
        iTk--;  // Backtrack if not an assignment
    }
    return exprComp();
}

// exprComp ::= exprAdd ( ( LESS | EQUAL ) exprAdd )?
bool exprComp() {
    if (exprAdd()) {
        if (consume(LESS) || consume(EQUAL)) {
            if (!exprAdd()) {
                tkerr("Expected expression after comparison operator");
            }
        }
        return true;
    }
    return false;
}

// exprAdd ::= exprMul ( ( ADD | SUB ) exprMul )*
bool exprAdd() {
    if (exprMul()) {
        while (consume(ADD) || consume(SUB)) {
            if (!exprMul()) {
                tkerr("Expected expression after '+' or '-'");
            }
        }
        return true;
    }
    return false;
}

// exprMul ::= exprPrefix ( ( MUL | DIV ) exprPrefix )*
bool exprMul() {
    if (exprPrefix()) {
        while (consume(MUL) || consume(DIV)) {
            if (!exprPrefix()) {
                tkerr("Expected expression after '*' or '/'");
            }
        }
        return true;
    }
    return false;
}

// exprPrefix ::= ( SUB | NOT )? factor
bool exprPrefix() {
    consume(SUB) || consume(NOT);
    return factor();
}

// factor ::= INT | REAL | STR | LPAR expr RPAR | ID ( LPAR ( expr ( COMMA expr )* )? RPAR )?
bool factor() {
    if (consume(INT) || consume(REAL) || consume(STR)) {
        return true;
    } else if (consume(LPAR)) {
        if (expr()) {
            if (consume(RPAR)) {
                return true;
            } else {
                tkerr("Expected ')' after expression");
            }
        } else {
            tkerr("Expected expression after '('");
        }
    } else if (consume(ID)) {
        if (consume(LPAR)) {
            if (expr()) {
                while (consume(COMMA)) {
                    if (!expr()) {
                        tkerr("Expected expression after ','");
                    }
                }
            }
            if (consume(RPAR)) {
                return true;
            } else {
                tkerr("Expected ')' after function call arguments");
            }
        }
        return true;
    }
    return false;
}

// Entry point for parsing
void parse() {
    iTk = 0;
    if (program()) {
        printf("Parsing completed successfully!\n");
    }
}
