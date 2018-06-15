#ifndef PM0_H
#define PM0_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <limits.h>
#include <errno.h>

#define MAX_SYMBOL_TABLE_SIZE 32768
#define MAX_TOKEN_ARRAY_SIZE  32768

typedef enum
{
    kindConst = 1,
    kindVar   = 2,
    kindProc  = 3
} symbolKind;

typedef struct
{
    char word[20];
} token;

typedef struct
{
    int kind;       // const = 1, var = 2, proc = 3
    char name[20];  // name up to 19 chars
    int val;        // number (ASCII) value
    int level;      // L level
    int addr;       // M address
} symbol;

typedef struct
{
    int op;     // Opeation code
    int l;      // Lexicographical level
    int m;      // Modifier/Argument (constant, address, or OPR)
    int ln;     // Line number
} instruction;

typedef struct 
{
    token tokenArray[500];
    char currentToken[20];
    int  tokenArrayLength;
    int  tokenIndex;

    symbol symbolTable[MAX_SYMBOL_TABLE_SIZE];
    int  symbolTableLength;
    int  symbolIndex;
    int  constCount;

    char recentName[20];
    int  recentVal;
    int  recentLevel;
    int  recentAddr;

    instruction code[500];
    int codeIndex;

    int jpcPosition;
    int jmpPosition;

    int aflag;

    int currentLevel;
} tokenSymbolContainer;

typedef enum
{
    CONSTANT = 1, VARIABLE = 2, PROCEDURE = 3
} kind;

int PM0_LA(char *inputFile, char *outputFile, int lflag);

int PM0_PCG(char *inputFile, char *outputFile, int aflag);

int PM0_VM(char *inputFile, char *outputFile, int vflag);

#endif
