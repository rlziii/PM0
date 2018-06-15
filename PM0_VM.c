/*
    Richard L Zarth III
    ri441328

    University of Central Florida
    COP 3402 - Summer 2017

    PM/0 Virtual Machine
    June 2, 2017

    PM0_VM.c
*/

#include <stdio.h>
#include <limits.h>
#include <string.h>
#include "PM0.h"

#define DEBUG_MODE 0    // Set to 1 for printing debug statements

#define MAX_STACK_HEIGHT  2000
#define MAX_CODE_LENGTH   500
#define MAX_LEXI_LEVELS   3
#define MAX_VERTICAL_BARS 64
#define ERROR             1
#define NO_ERROR          0

#define DEFAULT_INPUT_FILE  "vminput.txt"
#define DEFAULT_OUTPUT_FILE "vmoutput.txt"

#define OUTPUT_CHAR "o"
#define INPUT_CHAR "i"

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

// typedef struct instruction
// {
//     int op;     // Opeation code
//     int l;      // Lexicographical level
//     int m;      // Modifier/Argument (constant, address, or OPR)
//     int ln;     // Line number
// } instruction;

int base(int l, int *BP, int stack[])
{
    // find base L levels down

    // This is just an example; it needs to be implemented still

    int b1;

    b1 = *BP;

    while (l > 0)
    {
        // This is broken because lolglobalvarsucks
        b1 = stack[b1 + 1];
        l--;
    }

    return b1;
}

void fetch(instruction code[], int *PC, instruction *IR)
{
    // Fetch cycle

    // IR <- code[PC]
    // PC <- PC + 1

    IR->op = code[*PC].op;
    IR->l  = code[*PC].l;
    IR->m  = code[*PC].m;
    IR->ln = code[*PC].ln;
    
    *PC += 1;
}

void executeORP(int *SP, int *BP, int *PC,
                instruction *IR, int stack[])
{
    switch(IR->m)
    {
        case RET:
            *SP = *BP - 1;
            *PC = stack[*SP + 4];
            *BP = stack[*SP + 3];
            break;
        case NEG:
            stack[*SP] *= -1;
            break;
        case ADD:
            *SP -= 1;
            stack[*SP] = stack[*SP] + stack[*SP + 1];
            break;
        case SUB:
            *SP -= 1;
            stack[*SP] = stack[*SP] - stack[*SP + 1];
            break;
        case MUL:
            *SP -= 1;
            stack[*SP] = stack[*SP] * stack[*SP + 1];
            break;
        case DIV:
            *SP -= 1;
            stack[*SP] = stack[*SP] / stack[*SP + 1];
            break;
        case ODD:
            stack[*SP] = stack[*SP] % 2;
            break;
        case MOD:
            *SP -= 1;
            stack[*SP] = stack[*SP] % stack[*SP + 1];
            break;
        case EQL:
            *SP -= 1;
            stack[*SP] = stack[*SP] == stack[*SP + 1];
            break;
        case NEQ:
            *SP -= 1;
            stack[*SP] = stack[*SP] != stack[*SP + 1];
            break;
        case LSS:
            *SP -= 1;
            stack[*SP] = stack[*SP] < stack[*SP + 1];
            break;
        case LEQ:
            *SP -= 1;
            stack[*SP] = stack[*SP] <= stack[*SP + 1];
            break;
        case GTR:
            *SP -= 1;
            stack[*SP] = stack[*SP] > stack[*SP + 1];
            break;
        case GEQ:
            *SP -= 1;
            stack[*SP] = stack[*SP] >= stack[*SP + 1];
            break;
        default:
            break;
    }
}

void executeSIO(int *SP, int *BP, int *PC,
                instruction *IR, int stack[])
{
    int userInput;

    switch(IR->m)
    {
        case 1:
            printf("Output: %d\n", stack[*SP]);
            *SP -= 1;
            break;
        case 2:
            *SP += 1;
            printf("Input : ");
            scanf(" %d", &userInput);
            // This needs to contain error-handeling
            stack[*SP] = userInput;
            break;
        case 3:
            *SP = 0;
            *BP = 0;
            *PC = 0;
            // stop running the program somehow
            break;
        default:
            break;
    }
}

void execute(int *SP, int *BP, int *PC,
             instruction *IR, int stack[])
{
    switch (IR->op)
    {
        case LIT:
            *SP += 1;
            stack[*SP] = IR->m;
            break;
        case OPR:
            executeORP(SP, BP, PC, IR, stack);
            break;
        case LOD:
            *SP += 1;
            stack[*SP] = stack[base(IR->l, BP, stack) + IR->m];
            break;
        case STO:
            stack[base(IR->l, BP, stack) + IR->m] = stack[*SP];
            *SP -= 1;
            break;
        case CAL:
            stack[*SP + 1] = 0;
            stack[*SP + 2] = base(IR->l, BP, stack);
            stack[*SP + 3] = *BP;
            stack[*SP + 4] = *PC;
            *BP = *SP + 1;
            *PC = IR->m;
            break;
        case INC:
            *SP += IR->m;
            break;
        case JMP:
            *PC = IR->m;
            break;
        case JPC:
            if (stack[*SP] == 0)
                *PC = IR->m;
            *SP -= 1;
            break;
        case SIO:
            executeSIO(SP, BP, PC, IR, stack);
            break;
        default:
            break;
    }
}

void setDefaultValues(int *SP, int *BP, int *PC, instruction *IR)
{
    *SP = 0;
    *BP = 1;
    *PC = 0;
    
    IR->op = 0;
    IR->l  = 0;
    IR->m  = 0;
}

void stringFromOP(char *stringOP, int OP)
{
    switch (OP)
    {
        case LIT:
            strcpy(stringOP, "lit");
            break;
        case OPR:
            strcpy(stringOP, "opr");
            break;
        case LOD:
            strcpy(stringOP, "lod");
            break;
        case STO:
            strcpy(stringOP, "sto");
            break;
        case CAL:
            strcpy(stringOP, "cal");
            break;
        case INC:
            strcpy(stringOP, "inc");
            break;
        case JMP:
            strcpy(stringOP, "jmp");
            break;
        case JPC:
            strcpy(stringOP, "jpc");
            break;
        case SIO:
            strcpy(stringOP, "sio");
            break;
        default:
            break;
    }
}

void printStack(int *SP, int stack[], int *BP, int verticalBars[],
                instruction *IR, FILE *outputFile)
{
    int i, j;

    int endOfStack;

    if (IR->op == CAL)
        endOfStack = *SP + 5;
    else
        endOfStack = *SP + 1;

    if (IR->op == CAL)
    {
        for (i = 0; i < MAX_VERTICAL_BARS; i++)
        {
            if (verticalBars[i] == 0)
            {
                verticalBars[i] = *SP;
                break;
            }
        }
    }

    if (IR->op == OPR && IR->m == RET)
    {
        for (i = (MAX_VERTICAL_BARS - 1); i >= 0; i--)
        {
            if (verticalBars[i] != 0)
            {
                verticalBars[i] = 0;
                break;
            }
        }
    }

    for (i = 1; i < endOfStack; i++)
    {
        fprintf(outputFile, "%2d ", stack[i]);

        for (j = 0; j < MAX_VERTICAL_BARS; j++)
        {
            if (i == verticalBars[j])
            {
                fprintf(outputFile, " | ");
                break;
            }
        }
    }
}

void printOutputHeader(int *PC, int *BP, int *SP, FILE *outputFile)
{
    fprintf(outputFile,
            "                    pc  bp  sp      stack\n");

    fprintf(outputFile,
           "Initial values      %2d  %2d  %2d\n", *PC, *BP, *SP);
}

void printOutput(instruction *IR, int *PC, int *BP, int verticalBars[],
                 int *SP, int stack[], FILE *outputFile)
{
    char stringOP[4];
    stringFromOP(stringOP, IR->op);

    fprintf(outputFile, "%2d  ", IR->ln);
    fprintf(outputFile, "%s  " , stringOP);
    fprintf(outputFile, "%1d  ", IR->l);
    fprintf(outputFile, "%2d"  , IR->m);

    fprintf(outputFile, "      ");

    fprintf(outputFile, "%2d  ", *PC);
    fprintf(outputFile, "%2d  ", *BP);
    fprintf(outputFile, "%2d"  , *SP);

    fprintf(outputFile, "      ");

    printStack(SP, stack, BP, verticalBars, IR, outputFile);

    fprintf(outputFile, "\n");
}

void storeCode(FILE *FP, instruction code[])
{
    int tempOP;
    int tempL;
    int tempM;
    int i = 0;

    // this is not safe; need to make it more safe
    while(1)
    {
        if ((fscanf(FP, "%d", &tempOP)) != 1)
            break;
        if ((fscanf(FP, "%d", &tempL)) != 1)
            break;
        if ((fscanf(FP, "%d", &tempM)) != 1)
            break;

        if(DEBUG_MODE)
        {
            printf("tempOP = %d\n", tempOP);
            printf("tempL  = %d\n", tempL);
            printf("tempM  = %d\n", tempM);
        }

        code[i].op = tempOP;
        code[i].l  = tempL;
        code[i].m  = tempM;
        code[i].ln = i;

        i++;
    }
}

void printCode(instruction code[], FILE *outputFile)
{
    int i;

    char stringOP[4];

    fprintf(outputFile, "Line  OP   L   M\n");

    for (i = 0; i < MAX_CODE_LENGTH; i++)
    {
        if (code[i].op != INT_MAX)
        {
            stringFromOP(stringOP, code[i].op);

            fprintf(outputFile, "%2d  ", code[i].ln);
            fprintf(outputFile, "%s  " , stringOP);
            fprintf(outputFile, "%2d  ", code[i].l);
            fprintf(outputFile, "%2d  ", code[i].m);
            fprintf(outputFile, "\n");
        }
        else
        {
            break;
        }
    }

    fprintf(outputFile, "\n");
}

void initCode(instruction code[])
{
    int i;

    for (i = 0; i < MAX_CODE_LENGTH; i++)
    {
        code[i].op = INT_MAX;
        code[i].l  = INT_MAX;
        code[i].m  = INT_MAX;
    }
}

void initStack(int stack[])
{
    int i;

    for (i = 0; i < MAX_STACK_HEIGHT; i++)
        stack[i] = 0;
}

int readCode(instruction code[], FILE *outputFile, char inputFileName[])
{
    static FILE *FP;    // File pointer (for "vminput.txt")

    if ((FP = fopen(inputFileName, "r")) == NULL)
    {
        // Checks to see if file opened correctly
        // If not, exit from the program with an error
        printf("Error: Cannot open/read input file.\n");
        return ERROR;
    }

    storeCode(FP, code);
    printCode(code, outputFile);

    fclose(FP);

    return NO_ERROR;
}

void printRegisters(int *SP, int *BP, int *PC, instruction *IR)
{
    if (DEBUG_MODE)
    {
        printf("SP    = %d\n", *SP);
        printf("BP    = %d\n", *BP);
        printf("PC    = %d\n", *PC);

        printf("IR.op = %d\n", IR->op);
        printf("IR.l  = %d\n", IR->l);
        printf("IR.m  = %d\n", IR->m);

        printf("\n");
    }
}

void printArgumentError()
{
    printf("Error: Incorrect arguments.\n\n");

    printf("Use one of the following formats:\n\n");

    printf("./PM0_VM\n");
    printf("[default input: vminput.txt]\n");
    printf("[default output: vmoutput.txt]\n\n");

    printf("./PM0_VM i file.txt\n");
    printf("[specify input file: file.txt]\n");
    printf("[default output: vmoutput.txt]\n\n");

    printf("./PM0_VM o file.txt\n");
    printf("[default input file: vminput.txt]\n");
    printf("[specify output: file.txt]\n\n");

    printf("./PM0_VM i file.txt o file.txt\n");
    printf("[specify input file: file.txt]\n");
    printf("[specify output: file.txt]\n\n");
}

void initVerticalBars(int verticalBars[])
{
    int i;

    for (i = 0; i < MAX_VERTICAL_BARS; i++)
        verticalBars[i] = 0;
}

int PM0_VM(char *inputFile, char *outputFile, int vflag)
{
    int         SP;    // Stack pointer
    int         BP;    // Base pointer
    int         PC;    // Program Counter
    instruction IR;    // Instruction register

    int stack[MAX_STACK_HEIGHT];        // Simulation stack
    instruction code[MAX_CODE_LENGTH];  // Instruction storage

    int step = 1;
    int i = 0;

    int verticalBars[64];

    // printf(" VM: %s\n", inputFile);
    // printf(" VM: %s\n\n", outputFile);

    printf("\n--------------------\n");
    printf("\nPM/0 Virtual Machine\n\n");

    initCode(code);
    initStack(stack);
    setDefaultValues(&SP, &BP, &PC, &IR);

    FILE *outputFilePointer;

    // if (argc <= 1)
    // {
    //     strcpy(outputFileName, DEFAULT_OUTPUT_FILE);
    //     strcpy(inputFileName, DEFAULT_INPUT_FILE);
    // }
    // else if (argc == 3)
    // {
    //     if (strcmp(argv[1], OUTPUT_CHAR) == 0)
    //     {
    //         strcpy(outputFileName, argv[2]);
    //         strcpy(inputFileName, DEFAULT_INPUT_FILE);
    //     }
    //     else if (strcmp(argv[1], INPUT_CHAR) == 0)
    //     {
    //         strcpy(outputFileName, DEFAULT_OUTPUT_FILE);
    //         strcpy(inputFileName, argv[2]);
    //     }
    //     else
    //     {
    //         printArgumentError();
    //         return ERROR;
    //     }
    // }
    // else if (argc == 5)
    // {
    //     if (strcmp(argv[1], OUTPUT_CHAR) == 0)
    //         strcpy(outputFileName, argv[2]);
    //     else if (strcmp(argv[1], INPUT_CHAR) == 0)
    //         strcpy(inputFileName, argv[2]);
    //     else
    //     {
    //         printArgumentError();
    //         return ERROR;
    //     }

    //     if (strcmp(argv[3], OUTPUT_CHAR) == 0)
    //         strcpy(outputFileName, argv[4]);
    //     else if (strcmp(argv[3], INPUT_CHAR) == 0)
    //         strcpy(inputFileName, argv[4]);        
    //     else
    //     {
    //         printArgumentError();
    //         return ERROR;
    //     }
    // }
    // else
    // {
    //     printArgumentError();
    //     return ERROR;
    // }

    if ((outputFilePointer = fopen(outputFile, "w")) == NULL)
    {
        // Checks to see if file opened correctly
        // If not, exit from the program with an error
        printf("Error: Cannot open/write output file.\n");
        return ERROR;
    }

    initVerticalBars(verticalBars);

    if (readCode(code, outputFilePointer, inputFile) == ERROR)
        return ERROR;

    printRegisters(&SP, &BP, &PC, &IR);

    printOutputHeader(&PC, &BP, &SP, outputFilePointer);

    while (1)
    {
        if (IR.op == SIO && IR.m == 3)
        {
            //printf("\n        HALT        \n");
            printf("\n--------------------\n\n");
            break;
        }

        fetch(code, &PC, &IR);

        execute(&SP, &BP, &PC, &IR, stack);

        if (DEBUG_MODE) printf("Loop #%d\n", i++);

        printRegisters(&SP, &BP, &PC, &IR);

        printOutput(&IR, &PC, &BP, verticalBars, &SP, stack, outputFilePointer);
    }

    fclose(outputFilePointer);

    return 0;
}