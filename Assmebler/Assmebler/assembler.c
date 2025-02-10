#define _CRT_SECURE_NO_WARNINGS

#include "preAssembler.h"
#include "mainAssembler.h"

FILE* openFile(FILE*, char*);

int main(int argc, char* argv[])
{
	FILE *filePointer = NULL; /* a pointer to a file*/
	
	int numOfFiles = argc - 1; /*number of files to be read*/
	int currentFileIndex; /* the current file index the assembler works with*/
	int preAssemblerFlag; /* a flag that checks if the pre assembler of a file was succesful*/
	int fileLength; /* the length of the file*/
	int i;
	
	char* currentFile = (char*)malloc(MAX_FILE_NAME_LENGTH); /* the directory of a file*/
	char* argvFilePreAssembler = (char*)malloc(MAX_FILE_NAME_LENGTH); /* name of the file for the pre assembler*/
	char* argvFileMainAssembler = (char*)malloc(MAX_FILE_NAME_LENGTH); /* name of the file for the assembler*/
	
	char* reservedWords[] = {"mov", "cmp", "add", "sub", "not", "clr", "lea", "inc", "dec", "jmp", "bne", "red", "prn", "jsr", "rts", "stop", "mcr", "endmcr", ".data", ".string", ".entry", ".extern", "r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7"}; /* the reserved words of the language*/
	
	/*no files were given for the assembler*/
	if(numOfFiles == 0)
	{
		fprintf(stderr, "\nNo assembly files were found.\n\n");
		exit(1);
	}
	
	/* scanning all the given files*/
	for(currentFileIndex = 1; currentFileIndex <= numOfFiles; currentFileIndex++)
	{
		printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n");
		
		preAssemblerFlag = 1;
		
		for(i = strlen(currentFile) - 1; i >= 0; i--)
			strncpy(currentFile + i, "\0", 1);
			
		strcpy(argvFilePreAssembler, argv[currentFileIndex]);
		strcpy(argvFileMainAssembler, argv[currentFileIndex]);
		strcpy(currentFile, argv[currentFileIndex]);
		strncpy(currentFile + strlen(currentFile), ".as", 3);
		
		filePointer = openFile(filePointer, currentFile);
		
		if(filePointer == NULL)
			fprintf(stderr, "%s was not found.\n", currentFile);
			
		else
		{
			fseek(filePointer, 0, SEEK_END);
			fileLength = ftell(filePointer);
			fseek(filePointer, 0, SEEK_SET);
			
			if(fileLength == 0)
				fprintf(stderr, "%s is empty.\n", currentFile);
			
			else
			{
				/* Sending the assembly file to the pre assembler routine.*/
				filePointer = preAssemble(filePointer, currentFile, argvFilePreAssembler, reservedWords);
			
				/* There was an error with the pre assemble of the file*/
				if(filePointer == NULL)
					preAssemblerFlag = 0;

				/* The pre assembler of the file was succesfull*/
				if(preAssemblerFlag)
				{
					printf("Pre assemble of '%s' was succesfull.\n\n",currentFile);
					
					strncpy(currentFile + strlen(currentFile) - 2, "am", 2);
				
					/* SENDING TO THE NEXT PART OF THE ASSEMBLER*/
					fseek(filePointer, 0, SEEK_END);
					
					mainAssembler(filePointer, currentFile, argvFileMainAssembler, reservedWords);

					fclose(filePointer);
					
				}
			}
		}
		
		printf("~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~\n\n");
	}
	
	free(currentFile);
	free(argvFilePreAssembler);
	free(argvFileMainAssembler);
	
	return 1;
}

/* This function gets a file directory and tries to open it*/
FILE* openFile(FILE* filePointer, char* fileDirectory)
{
	filePointer = fopen(fileDirectory, "r");
	
	return filePointer;
}
