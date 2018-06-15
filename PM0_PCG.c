/*
    Richard L Zarth III
    ri441328

    University of Central Florida
    COP 3402 - Summer 2017

    PL/0 Parser-Code Generator
    July 7, 2017

    PM0_PCG.c
*/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>
#include "PM0.h"

#define PRINT_CODE         1
#define PRINT_ERROR        1

#define PRINT_DEBUG        0
#define PRINT_TOKEN_ARRAY  0
#define PRINT_SYMBOL_TABLE 0

#define MAX_SYMBOL_TABLE_SIZE 32768
#define MAX_TOKEN_ARRAY_SIZE  32768
#define ERROR                 INT_MAX

#define LIT  1  // Literal
#define OPR  2  // Operation
#define LOD  3  // Load
#define STO  4  // Store
#define CAL  5  // Call prodecure
#define INC  6  // Increment
#define JMP  7  // Jump
#define JPC  8  // Jump Condition
#define SIO  9  // Start Input Output

#define RET  0  // Return
#define NEG  1  // Negate
#define ADD  2  // Add
#define SUB  3  // Subtract
#define MUL  4  // Multiply
#define DIV  5  // Divide
#define ODD  6  // Odd
#define MOD  7  // Modulo
#define EQL  8  // Equal
#define NEQ  9  // Not equal
#define LSS 10  // Less than
#define LEQ 11  // Less than or equal
#define GTR 12  // Greater than
#define GEQ 13  // Greater than or equal

#define WRTE 1
#define READ 2
#define HALT 3
#define TEMP 999

#define DECLARATION_STAGE_TRUE  1
#define DECLARATION_STAGE_FALSE 0

/* ************************************************** */

void production_program(tokenSymbolContainer *tables);
void production_block(tokenSymbolContainer *tables);
void production_constDeclaration(tokenSymbolContainer *tables);
int production_varDeclaration(tokenSymbolContainer *tables);
void production_procedureDeclaration(tokenSymbolContainer *tables);
void production_statement(tokenSymbolContainer *tables);
void production_condition(tokenSymbolContainer *tables);
int  production_relOp(tokenSymbolContainer *tables);
void production_expression(tokenSymbolContainer *tables);
void production_term(tokenSymbolContainer *tables);
void production_factor(tokenSymbolContainer *tables);
void production_number(tokenSymbolContainer *tables, int declarationStage);
void production_ident(tokenSymbolContainer *tables, int op, int m, int declarationStage);
void production_digit(tokenSymbolContainer *tables);
void production_letter(tokenSymbolContainer *tables);

/* ************************************************** */

void error(char errorMessage[])
{
    printf("ERROR: %s\n", errorMessage);

    exit(0);
}

void getNextToken(tokenSymbolContainer *tables)
{
    if (PRINT_DEBUG) printf("getNextToken()\n");

    tables->tokenIndex += 1;

    strcpy(tables->currentToken, tables->tokenArray[tables->tokenIndex].word);

    if (PRINT_DEBUG) printf("    Index: %d\n", tables->tokenIndex);
    if (PRINT_DEBUG) printf("    Token: %s\n", tables->currentToken);
}

void getPrevToken(tokenSymbolContainer *tables)
{
    if (PRINT_DEBUG) printf("getPrevToken()\n");

    tables->tokenIndex -= 1;

    strcpy(tables->currentToken, tables->tokenArray[tables->tokenIndex].word);

    if (PRINT_DEBUG) printf("    Index: %d\n", tables->tokenIndex);
    if (PRINT_DEBUG) printf("    Token: %s\n", tables->currentToken);
}

int convertTokenToInt(tokenSymbolContainer *tables)
{
    char *endptr = NULL;
    int base = 10;
    long number = 0;

    errno = 0;

    number = strtol(tables->currentToken, &endptr, base);

    if (tables->currentToken == endptr)
        error("[convertTokenToInt] No digits found");
    else if (errno != 0 && number == 0)
        error("[convertTokenToInt] Unknown error");

    return (int) number;
}

void addToSymbolTable(tokenSymbolContainer *tables, kind type, int varCount)
{
    if (type == CONSTANT)
    {
        tables->symbolTable[tables->symbolIndex].kind = CONSTANT;
        strcpy(tables->symbolTable[tables->symbolIndex].name, tables->recentName);
        tables->symbolTable[tables->symbolIndex].val = tables->recentVal;
        tables->symbolTable[tables->symbolIndex].level = 0;
        tables->symbolTable[tables->symbolIndex].addr = 0;

        tables->symbolTableLength += 1;
        tables->symbolIndex += 1;
        tables->constCount += 1;
    }
    else if (type == VARIABLE)
    {
        tables->symbolTable[tables->symbolIndex].kind = VARIABLE;
        strcpy(tables->symbolTable[tables->symbolIndex].name, tables->recentName);
        tables->symbolTable[tables->symbolIndex].val = 0;
        tables->symbolTable[tables->symbolIndex].level = tables->currentLevel;
        tables->symbolTable[tables->symbolIndex].addr = varCount;

        tables->symbolTableLength += 1;
        tables->symbolIndex += 1;
    }
    else if (type == PROCEDURE)
    {
        tables->symbolTable[tables->symbolIndex].kind = PROCEDURE;
        strcpy(tables->symbolTable[tables->symbolIndex].name, tables->recentName);
        tables->symbolTable[tables->symbolIndex].val = 0;
        tables->symbolTable[tables->symbolIndex].level = tables->currentLevel;
        tables->symbolTable[tables->symbolIndex].addr = tables->codeIndex + 1;        

        tables->symbolTableLength += 1;
        tables->symbolIndex += 1;
    }
}

symbol searchSymbolTable(tokenSymbolContainer *tables)
{
    int i;

    for (i = tables->symbolIndex - 1; i >= 0; i--)
    {
        if (strcmp(tables->currentToken, tables->symbolTable[i].name) == 0)
        {
            return tables->symbolTable[i];
        }
    }

    error("[searchSymbolTable] Cannot find symbol in table");

    // this should never happen
    return tables->symbolTable[i];
}

int emitCode(tokenSymbolContainer *tables, int op, int l, int m)
{
    int index = tables->codeIndex;

    tables->code[index].op = op;
    tables->code[index].l  = l;
    tables->code[index].m  = m;
    tables->code[index].ln = index;

    tables->codeIndex += 1;

    return index;
}

/* ************************************************** */

void production_program(tokenSymbolContainer *tables)
{
    if (PRINT_DEBUG) printf("production_program()\n");

    getNextToken(tables);

    production_block(tables);

    if (strcmp(tables->currentToken, "periodsym") != 0)
        error("[program] Expected periodsym");

    // CODE: this is the code to halt the program
    //       should be the last line of code
    emitCode(tables, SIO, 0, HALT);
}

void production_block(tokenSymbolContainer *tables)
{
    if (PRINT_DEBUG) printf("production_block()\n");

    int previousSymbolIndex = tables->symbolIndex;
    int jumpAddress = 0;
    int space = 4;

    tables->currentLevel++;

    // CODE: this is a placeholder for the jump at the start of a block
    //       this should generate the first jump to MAIN
    //       and all subsequent jumps to other procedures
    jumpAddress = emitCode(tables, JMP, 0, TEMP);

    production_constDeclaration(tables);

    space += production_varDeclaration(tables);

    production_procedureDeclaration(tables);

    tables->code[jumpAddress].m = tables->codeIndex;

    // CODE: this should allocate space for this block
    //       this should work for the start of MAIN
    //       and all subsequent procedures
    emitCode(tables, INC, 0, space);

    production_statement(tables);

    if (tables->currentLevel != 1)
    {
        // CODE: this code will return from the procedure
        emitCode(tables, OPR, 0, 0);
    }

    tables->symbolIndex = previousSymbolIndex;

    tables->currentLevel--;
}

void production_constDeclaration(tokenSymbolContainer *tables)
{
    if (PRINT_DEBUG) printf("production_constDeclaration()\n");

    if (strcmp(tables->currentToken, "constsym") == 0)
    {
        getNextToken(tables);

        if (strcmp(tables->currentToken, "identsym") != 0)
                error("[constDeclaration] Expected identsym");
        production_ident(tables, 0, 0, DECLARATION_STAGE_TRUE);

        if (strcmp(tables->currentToken, "eqlsym") != 0)
            error("[constDeclaration] Expected eqlsym");
        getNextToken(tables);

        if (strcmp(tables->currentToken, "numbersym") != 0)
            error("[constDeclaration] Expected numbersym");
        production_number(tables, DECLARATION_STAGE_TRUE);

        addToSymbolTable(tables, CONSTANT, 0);

        while (strcmp(tables->currentToken, "commasym") == 0)
        {
            getNextToken(tables);

            if (strcmp(tables->currentToken, "identsym") != 0)
                error("[constDeclaration] Expected identsym");
            production_ident(tables, 0, 0, DECLARATION_STAGE_TRUE);

            if (strcmp(tables->currentToken, "eqlsym") != 0)
                error("[constDeclaration] Expected eqlsym");
            getNextToken(tables);

            if (strcmp(tables->currentToken, "numbersym") != 0)
                error("[constDeclaration] Expected numbersym");
            production_number(tables, DECLARATION_STAGE_TRUE);

            addToSymbolTable(tables, CONSTANT, 0);
        }

        if (strcmp(tables->currentToken, "semicolonsym") != 0)
            error("[constDeclaration] Expected semicolonsym");
        getNextToken(tables);
    }
}

int production_varDeclaration(tokenSymbolContainer *tables)
{
    if (PRINT_DEBUG) printf("production_varDeclaration()\n");

    int varCount = 0;

    if (strcmp(tables->currentToken, "varsym") == 0)
    {
        getNextToken(tables);

        if (strcmp(tables->currentToken, "identsym") != 0)
            error("[varDeclaration] Expected identsym");
        production_ident(tables, 0, 0, DECLARATION_STAGE_TRUE);

        addToSymbolTable(tables, VARIABLE, varCount);
        varCount++;

        while (strcmp(tables->currentToken, "commasym") == 0)
        {
            getNextToken(tables);

            if (strcmp(tables->currentToken, "identsym") != 0)
                error("[varDeclaration] Expected identsym");
            production_ident(tables, 0, 0, DECLARATION_STAGE_TRUE);

            addToSymbolTable(tables, VARIABLE, varCount);
            varCount++;
        }

        if (strcmp(tables->currentToken, "semicolonsym") != 0)
            error("[varDeclaration] Expected semicolonsym");
        getNextToken(tables);
    }

    return varCount;
}

void production_procedureDeclaration(tokenSymbolContainer *tables)
{
    if (PRINT_DEBUG) printf("production_procedureDeclaration()\n");

    // perhaps this whole thing should be in a while loop?
    while (strcmp(tables->currentToken, "prosym") == 0)
    //if (strcmp(tables->currentToken, "prosym") == 0)
    {
        getNextToken(tables);

        if (strcmp(tables->currentToken, "identsym") != 0)
            error("[procedureDeclaration] Expected identsym");
        production_ident(tables, 0, 0, DECLARATION_STAGE_TRUE); // this needs to be checked

        addToSymbolTable(tables, PROCEDURE, 0); // this needs to be checked

        if (strcmp(tables->currentToken, "semicolonsym") != 0)
            error("[procedureDeclaration] Expected semicolonsym");
        getNextToken(tables);

        production_block(tables); // this will need a lot of fixing

        if (strcmp(tables->currentToken, "semicolonsym") != 0)
            error("[procedureDeclaration] Expected semicolonsym");
        getNextToken(tables);
    }
}

void production_statement(tokenSymbolContainer *tables)
{
    if (PRINT_DEBUG) printf("production_statement()\n");

    symbol tempSymbol;
    int conditionTrue, conditionFalse;
    int jumpAddress;
    int value;
    int lexLevel;

    if (strcmp(tables->currentToken, "identsym") == 0)
    {
        // peek ahead in order to get the addr of the variable
        getNextToken(tables);
        tempSymbol = searchSymbolTable(tables);
        getPrevToken(tables);

        production_ident(tables, 0, 0, DECLARATION_STAGE_FALSE);

        if (strcmp(tables->currentToken, "becomesym") != 0)
            error("[statement] Expected becomesym");
        getNextToken(tables);

        production_expression(tables);

        // CODE: this will store a value
        lexLevel = (tables->currentLevel - tempSymbol.level);
        emitCode(tables, STO, lexLevel, tempSymbol.addr);
    }
    else if (strcmp(tables->currentToken, "callsym") == 0)
    {
        getNextToken(tables);

        if (strcmp(tables->currentToken, "identsym") != 0)
            error("[statement] Expected identsym");
        production_ident(tables, CAL, READ, DECLARATION_STAGE_FALSE); // this needs to be fixed
    }
    else if (strcmp(tables->currentToken, "beginsym") == 0)
    {
        getNextToken(tables);

        production_statement(tables);

        while (strcmp(tables->currentToken, "semicolonsym") == 0)
        {
            getNextToken(tables);

            production_statement(tables);
        }

        if (strcmp(tables->currentToken, "endsym") != 0)
            error("[statement] Expected endsym");
        getNextToken(tables);
    }
    else if (strcmp(tables->currentToken, "ifsym") == 0)
    {
        getNextToken(tables);

        production_condition(tables);

        if (strcmp(tables->currentToken, "thensym") != 0)
            error("[statement] Expected thensym");
        getNextToken(tables);

        production_statement(tables);

        conditionFalse = tables->codeIndex;

        // peek ahead
        //getNextToken(tables);

        if (strcmp(tables->currentToken, "elsesym") == 0)
        {
            getNextToken(tables);

            // CODE: jump address to be updated later
            jumpAddress = emitCode(tables, JMP, 0, TEMP);

            // CODE: jump conditional for false
            tables->code[tables->jpcPosition].m = (conditionFalse + 1);

            value = tables->codeIndex;

            production_statement(tables);

            // CODE: update the previous placeholder jump address
            tables->code[jumpAddress].m = tables->codeIndex;
        }
        else
        {
            // if not "else", put it back
            // printf("HELLO\n");
            // getPrevToken(tables);

            // CODE: jump conditional for false
            tables->code[tables->jpcPosition].m = conditionFalse;
        }
    }
    else if (strcmp(tables->currentToken, "whilesym") == 0)
    {
        value = tables->codeIndex;

        getNextToken(tables);

        production_condition(tables);

        if (strcmp(tables->currentToken, "dosym") != 0)
            error("[statement] Expected dosym");
        getNextToken(tables);

        production_statement(tables);

        emitCode(tables, JMP, 0, value);
        tables->code[tables->jpcPosition].m = tables->codeIndex;
    }
    else if (strcmp(tables->currentToken, "readsym") == 0)
    {
        getNextToken(tables);

        if (strcmp(tables->currentToken, "identsym") != 0)
            error("[statement] Expected identsym");
        production_ident(tables, SIO, READ, DECLARATION_STAGE_FALSE);
    }
    else if (strcmp(tables->currentToken, "writesym") == 0)
    {
        getNextToken(tables);

        if (strcmp(tables->currentToken, "identsym") != 0)
            error("[statement] Expected identsym");
        production_ident(tables, SIO, WRTE, DECLARATION_STAGE_FALSE);
    }
    // else if (strcmp(tables->currentToken, "semicolonsym") == 0)
    // {
    //     getNextToken(tables);
    // }
    else
    {
        // empty string
        if (PRINT_DEBUG) printf("\n***EMPTY STRING***\n\n");
        //getNextToken(tables);
    }
}

void production_condition(tokenSymbolContainer *tables)
{
    if (PRINT_DEBUG) printf("production_condition()\n");

    int relOp;

    if (strcmp(tables->currentToken, "oddsym") == 0)
    {
        getNextToken(tables);

        production_expression(tables);

        // CODE: for operation ODD
        emitCode(tables, OPR, 0, ODD);
    }
    else
    {
        production_expression(tables);

        relOp = production_relOp(tables);

        production_expression(tables);

        // CODE: for relational operation
        emitCode(tables, OPR, 0, relOp);

        // CODE: placeholder for conditional
        tables->jpcPosition = emitCode(tables, JPC, 0, TEMP);
    }
}

int production_relOp(tokenSymbolContainer *tables)
{
    if (PRINT_DEBUG) printf("production_relOp()\n");

    int relOp;

    if (strcmp(tables->currentToken, "eqlsym") == 0)
        relOp = EQL;
    else if (strcmp(tables->currentToken, "neqsym") == 0)
        relOp = NEQ;
    else if (strcmp(tables->currentToken, "lessym") == 0)
        relOp = LSS;
    else if (strcmp(tables->currentToken, "leqsym") == 0)
        relOp = LEQ;
    else if (strcmp(tables->currentToken, "gtrsym") == 0)
        relOp = GTR;
    else if (strcmp(tables->currentToken, "geqsym") == 0)
        relOp = GEQ;
    else
        error("[relOp] Invalid relOp");

    getNextToken(tables);

    return relOp;
}

void production_expression(tokenSymbolContainer *tables)
{
    if (PRINT_DEBUG) printf("production_expression()\n");

    int m;

    if ((strcmp(tables->currentToken, "plussym")  == 0) ||
        (strcmp(tables->currentToken, "minussym") == 0)  )
    {
        if (strcmp(tables->currentToken, "plussym")  == 0)
            m = ADD;
        else if (strcmp(tables->currentToken, "minussym") == 0)
            m = SUB;

        getNextToken(tables);
    }

    production_term(tables);

    if (m == ADD)
        emitCode(tables, OPR, 0, ADD);
    if (m == SUB)
        emitCode(tables, OPR, 0, SUB);

    while ((strcmp(tables->currentToken, "plussym")  == 0) ||
           (strcmp(tables->currentToken, "minussym") == 0)  )
    {
        if (strcmp(tables->currentToken, "plussym")  == 0)
            m = ADD;
        else if (strcmp(tables->currentToken, "minussym") == 0)
            m = SUB;

        getNextToken(tables);

        production_term(tables);

        if (m == ADD)
            emitCode(tables, OPR, 0, ADD);
        if (m == SUB)
            emitCode(tables, OPR, 0, SUB);
    }
}

void production_term(tokenSymbolContainer *tables)
{
    if (PRINT_DEBUG) printf("production_term()\n");

    int m;

    production_factor(tables);

    while ((strcmp(tables->currentToken, "multsym")  == 0) ||
           (strcmp(tables->currentToken, "slashsym") == 0)  )
    {
        if (strcmp(tables->currentToken, "multsym")  == 0)
            m = MUL;
        else if (strcmp(tables->currentToken, "slashsym") == 0)
            m = DIV;

        getNextToken(tables);

        production_factor(tables);

        if (m == MUL)
            emitCode(tables, OPR, 0, MUL);
        else if (m == DIV)
            emitCode(tables, OPR, 0, DIV);
    }
}

void production_factor(tokenSymbolContainer *tables)
{
    if (PRINT_DEBUG) printf("production_factor()\n");

    // int value;

    if (strcmp(tables->currentToken, "identsym") == 0)
    {
        // getNextToken(tables);
        // value = searchSymbolTable(tables);
        // getPrevToken(tables);

        production_ident(tables, 0, 1, DECLARATION_STAGE_FALSE);
    }
    else if (strcmp(tables->currentToken, "numbersym") == 0)
    {
        production_number(tables, DECLARATION_STAGE_FALSE);
    }
    else if (strcmp(tables->currentToken, "lparentsym") == 0)
    {
        getNextToken(tables);

        production_expression(tables);

        if (strcmp(tables->currentToken, "rparentsym") != 0)
            error("[factor] Expected rparentsym");
        getNextToken(tables);
    }
    else
    {
        error("[factor] Expected identsym, numbersym, or lparntsym");
    }
}

void production_number(tokenSymbolContainer *tables, int declarationStage)
{
    if (PRINT_DEBUG) printf("production_number()\n");

    getNextToken(tables);

    if (declarationStage == 1)
        tables->recentVal = convertTokenToInt(tables);
    else if (declarationStage == 0)
        emitCode(tables, LIT, 0, convertTokenToInt(tables));

    getNextToken(tables);
}

void production_ident(tokenSymbolContainer *tables, int op, int m, int declarationStage)
{
    if (PRINT_DEBUG) printf("production_ident()\n");

    symbol tempSymbol;
    int lexLevel;

    getNextToken(tables);

    if (declarationStage == 1)
    {
        strcpy(tables->recentName, tables->currentToken);
    }
    else if (declarationStage == 0)
    {
        tempSymbol = searchSymbolTable(tables);
        lexLevel = (tables->currentLevel - tempSymbol.level);

        if (op == SIO && m != HALT)
        {
            if (tempSymbol.kind == CONSTANT)
                emitCode(tables, LIT, 0, tempSymbol.val);
            else if (tempSymbol.kind == VARIABLE)
                emitCode(tables, LOD, lexLevel, tempSymbol.addr);
            
            if (m == WRTE)
            {
                emitCode(tables, SIO, 0, WRTE);
            }
            else if (m == READ)
            {
                emitCode(tables, SIO, 0, READ);
                emitCode(tables, STO, lexLevel, tempSymbol.addr);
            }
        }
        else if (op == 0 && m == 1)
        {
            if (tempSymbol.kind == CONSTANT)
                emitCode(tables, LIT, 0, tempSymbol.val);
            else if (tempSymbol.kind == VARIABLE)
                emitCode(tables, LOD, lexLevel, tempSymbol.addr);
        }
        else if (op == CAL)
        {
            if (tempSymbol.kind == PROCEDURE)
                emitCode(tables, CAL, lexLevel, tempSymbol.addr - 1);
        }
    }

    getNextToken(tables);
}

/* ************************************************** */

void parser(tokenSymbolContainer *tables)
{
    if (PRINT_DEBUG) printf("parser()\n");

    if (PRINT_DEBUG) printf("    Index: %d\n", tables->tokenIndex);
    if (PRINT_DEBUG) printf("    Token: %s\n", tables->currentToken);

    production_program(tables);

    if (PRINT_DEBUG) printf("SUCCESS\n");
    if (tables->aflag) printf("\nNo errors. Program is syntactically correct.\n");
}

/* ************************************************** */

void initTables(tokenSymbolContainer *tables, int aflag)
{
    strcpy(tables->currentToken, "");
    tables->tokenArrayLength = 0;
    tables->tokenIndex = -1;

    tables->symbolTableLength = 0;
    tables->symbolIndex = 0;
    tables->constCount = 0;

    strcpy(tables->recentName, "");
    tables->recentVal = 0;
    tables->recentLevel = 0;
    tables->recentAddr = 0;

    tables->codeIndex = 0;

    tables->aflag = aflag;

    tables->currentLevel = 0;
}

void createTokenArray(tokenSymbolContainer *tables, char *inputFile)
{
    if (PRINT_DEBUG) printf("createTokenArray()\n");

    FILE *fp = fopen(inputFile, "r");

    if (fp == NULL)
    {
        error("PCG: Could not open input file");
    }

    char s[20];

    int i = 0;

    while (fscanf(fp, " %15s", s) == 1)
    {
        // Skip over the first three words
        // "Symbolic", "Lexeme", and "List:"
        if (i == 0 || i == 1 || i == 2)
        {
            i++;
            continue;
        }

        // Have reached the second part of the output
        // "Lexeme List:"
        if (strcmp(s, "Lexeme") == 0)
            break;

        strcpy(tables->tokenArray[i-3].word, s);
        i++;
    }

    tables->tokenArrayLength = i - 3;

    fclose(fp);
}

void printTokenArray(tokenSymbolContainer *tables)
{
    if (PRINT_DEBUG) printf("printTokenArray()\n");

    int i;

    printf("Token Array Length: %d\n", tables->tokenArrayLength);

    for (i = 0; i < tables->tokenArrayLength; i++)
        printf("tokenArray[%d]: %s\n", i, tables->tokenArray[i].word);
}

void printSymbolTable(tokenSymbolContainer *tables)
{
    printf("printSymbolTable()\n");

    int i;

    printf("Symbol Table Length : %d\n", tables->symbolTableLength);
    printf("Symbol Table Index  : %d\n", tables->symbolIndex);
    printf("Constant Count      : %d\n", tables->constCount);
    //printf("Variable Count      : %d\n\n", tables->varCount);

    for (i = 0; i < tables->symbolTableLength; i++)
        printf("[%02d] Kind: %d  Name: %10s  Value: %2d  Level: %d  Address: %2d\n",
               i, tables->symbolTable[i].kind, tables->symbolTable[i].name, tables->symbolTable[i].val,
               tables->symbolTable[i].level, tables->symbolTable[i].addr);
}

void printCodeArray(tokenSymbolContainer *tables)
{
    if (PRINT_DEBUG) printf("printCodeArray()\n");

    printf("\n--------------------\n");
    printf("\n PL/0 Assembly Code\n\n");

    int i;

    for (i = 0; i < tables->codeIndex; i++)
        printf("%d %d %d\n", tables->code[i].op, tables->code[i].l, tables->code[i].m);
}

void outputCode(tokenSymbolContainer *tables, char *outputFile)
{
    FILE *fp = fopen(outputFile, "w");

    int i;

    for (i = 0; i < tables->codeIndex; i++)
        fprintf(fp, "%d %d %d\n", tables->code[i].op, tables->code[i].l, tables->code[i].m);

    fclose(fp);
}

/* ************************************************** */

int PM0_PCG(char *inputFile, char *outputFile, int aflag)
{
    tokenSymbolContainer tables;

    // if (argc != 3)
    // {
    //     printf("Format: ./PM0_PCG [input file] [output file]\n");
    //     return 1;
    // }

    // printf("PCG: %s\n", inputFile);
    // printf("PCG: %s\n\n", outputFile);

    initTables(&tables, aflag);

    createTokenArray(&tables, inputFile);

    if (PRINT_TOKEN_ARRAY) printTokenArray(&tables);

    parser(&tables);

    if (PRINT_SYMBOL_TABLE) printSymbolTable(&tables);

    //if (aflag) printCodeArray(&tables);

    outputCode(&tables, outputFile);

    return 0;
}