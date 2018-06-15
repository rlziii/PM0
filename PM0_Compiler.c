/*
    Richard L Zarth III
    ri441328

    University of Central Florida
    COP 3402 - Summer 2017

    PL/0 Compiler
    July 7, 2017

    PM0_Compiler.c
*/

#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include "PM0.h"

void printFile(char *filename)
{
    int c;
    FILE *file;
    file = fopen(filename, "r");
    if (file) {
        while ((c = getc(file)) != EOF)
            putchar(c);
        fclose(file);
    }
}

int main(int argc, char *argv[])
{
    // -l Print list of lexemes/tokens (LA output) to screen
    // -a Print generated assembly code (PCG output) to screen
    // -v Print VM execution trace (VM output) to screen

    // LA
    // PCG
    // VM

    // if (argc != 3)
    // {
    //     printf("Format: ./PM0_Compiler [input file] [output file]\n");
    //     return 1;
    // }

    // printf("Com: %s\n", argv[1]);
    // printf("Com: %s\n\n", argv[2]);

    // Command-Line Argument Stuff
    int aflag = 0;
    int lflag = 0;
    int vflag = 0;
    char *inputValue = NULL;
    char *outputValue = NULL;
    int index;
    int c;

    opterr = 0;

    while ((c = getopt(argc, argv, ":alvi:o:")) != -1)
    {
        switch (c)
        {
            case 'a':
                aflag = 1;
                break;
            case 'l':
                lflag = 1;
                break;
            case 'v':
                vflag = 1;
                break;
            case 'i':
                inputValue = optarg;
                break;
            case 'o':
                outputValue = optarg;
                break;
            case '?':
                if (optopt == 'i')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else if (optopt == 'o')
                    fprintf (stderr, "Option -%c requires an argument.\n", optopt);
                else if (isprint (optopt))
                    fprintf (stderr, "Unknown option `-%c'.\n", optopt);
                else
                    fprintf (stderr, "Unknown option character `\\x%x'.\n", optopt);
                return 1;
            default:
                abort ();
        }
    }

    if (inputValue == NULL)
        strcpy(inputValue, "input.txt");

    if (outputValue == NULL)
        strcpy(outputValue, "output.txt");

    //printf("a=%d, l=%d, v=%d, i=%s, o=%s\n", aflag, lflag, vflag, inputValue, outputValue);

    PM0_LA(inputValue, "la_output.txt", lflag);

    if (lflag)
    {
        printf("\n--------------------\n\n");
        printf("Lexical Analyzer Output\n\n");
        printFile("la_output.txt");
        printf("\n--------------------\n\n");
    }

    PM0_PCG("la_output.txt", "pcg_output.txt", aflag);

    if (aflag)
    {
        printf("\n--------------------\n\n");
        printf("Parser-Code Generator Output\n\n");
        printFile("pcg_output.txt");
        printf("\n--------------------\n\n");
    }

    PM0_VM("pcg_output.txt", outputValue, vflag);
    
    if (vflag)
    {
        printf("\n--------------------\n\n");
        printf("Virtual Machine Output\n\n");
        printFile(outputValue);
        printf("\n--------------------\n\n");
    }

    return 0;
}

// Error handling
// Eustis
// Print setup
// Input/Output files
// Expression.txt needs to work
// Check while loop for count.txt ()
// Check to make sure that non-found idents give an error