#include "data.h"
#include "helpingFunctions.c"


struct SymbolTable /* the table containing the symbols*/
{
	char* symbolName; /* name of the symbol*/
	int symbolValue; /* value of the symbol*/
	char* type; /* type can be 'external' for .extern or 'relocatble' for .entry*/
	int dataType; /* data type can be 0 for data or 1 for command or 2 for .extern*/
	struct SymbolTable* next; /* next symbol of the table*/
};

struct Image /* the list of the image. for data image and for instruction image porpuses*/
{
	int content[14]; /* the content represented in binary*/
	struct Image* next; /* next image*/
};

int secondLoop(FILE*, int, struct SymbolTable*, struct Image*, char*[]);
void makeObjectFile(char*, struct Image*, struct Image*, int, int);
void makeEntryFile(char*,struct SymbolTable*);
void makeExternFile(FILE*, int, char*, struct SymbolTable*, char*[]);
int updateInstructionNode(struct SymbolTable*, struct Image*, char*, int, char*[]);
struct SymbolTable* getSymbol(struct SymbolTable*, char*);
struct SymbolTable* newSymbol(char*, int, int);
struct Image* newDataImage(int);
struct Image* newInstructionImage(int*);
int commandLine(struct SymbolTable*, char*, char*, char*, int, int, char*[]);
int entryExternLine(struct SymbolTable*,char*, char*, char*, int, int, char*[]);
int dataStringLine(char*, char*, char*, struct SymbolTable*, int, int);
struct SymbolTable* insertSymbol(struct SymbolTable*, struct SymbolTable*);
struct Image* insertImage(struct Image*, struct Image*);
void convertInstructionToBinary(int* , char*, char*, char*[]);
void makeObjectFile(char*, struct Image*, struct Image*, int, int);

/* This is the main function of the post macro assembler. It reads the lines from the
given file and runs all the necessary checks and translations of the text. */
void mainAssembler(FILE* filePointer, char* currentFile, char* newFileName, char* reservedWords[])
{
	int fileLength; /* the length of the file*/
	int lineCount = 1; /* the current line the assembler read*/
	int errorFlag = 0; /* a flag that checks if there was an error in the asembly code*/
	int functionErrorFlag = 0; /* a flag that checks if the line translated correctly inside function*/
	int symbolFlag = 0; /* a flag that checks for symbol at the start of the line*/
	int insertFlag = 0; /* a flag that checks if need to insert a new data to the data image*/
	int commandFlag; /* a flag that checks if a line is command line*/
	int instructionFlag = 0; /* a flag that checks if a line is instruction *.entry/.extern/.data/.string*/
	int onlyTokenFlag = 0; /* a flag that checks if the line consists of a symbol decleration: [symbol]: [null]*/ 
	int instructionArr[14]; /* an array representation of the instruction line*/
	int i;
	
	int IC = 0; /* instruction counter*/
	int DC = 0; /* data counter*/
	int L; /* how much data is needed to be stored for the instruction*/
	
	char* line = (char*)malloc(MAX_LINE_LENGTH); /* a line from the file*/
	char* symbolToken = (char*)malloc(MAX_LABEL_LENGTH); /* a token that represents a symbol decleration at the start of the line*/
	char* token; /* token for strtok*/
	char* token2; /* token for strtok*/
	char* restOfLineToken; /* token for the rest of the line strtok*/
	
	struct SymbolTable* symbolHead = NULL; /* the head of the symbol table list*/
	struct SymbolTable* symbolNode = NULL; /* a node of the symbol table list*/
	struct SymbolTable* newSymbolNode = NULL; /* a temporary node for new symbol storage*/
	
	struct Image* dataHead = NULL; /* the head of the data image list*/
	struct Image* dataNode = NULL; /* a node of the data image list*/
	struct Image* newDataNode = NULL; /* a temporary node for new data storage*/
	
	struct Image* instructionHead = NULL; /* the head of the instruction list*/
	struct Image* instructionNode = NULL; /* a node of the instruction list*/
	struct Image* newInstructionNode = NULL; /* a temporary node for new instruction storage*/	
	
	fseek(filePointer, 0, SEEK_END);
	fileLength = ftell(filePointer);
	fseek(filePointer, 0, SEEK_SET);
	
	/* starting to read the file*/
	while(ftell(filePointer) < fileLength)
	{
		symbolFlag = 0;
		onlyTokenFlag = 0;
		
		fgets(line, MAX_LINE_LENGTH, filePointer);
		
		/* avoiding empty lines and ';' lines*/
		if(line != NULL && strlen(line) > 1 && strncmp(line, ";", 1))
		{
			token = strtok(line, " \n\t");
	
			/*checking if first word is a command*/
			commandFlag = checkIfCommand(token, reservedWords);
	
			/* if not command, checking if first word is instruction*/
			if(!commandFlag)
				instructionFlag = checkIfInstruction(token, reservedWords);
				
			/* a command or an instruction*/
			if(commandFlag || instructionFlag)
				restOfLineToken = strtok(NULL, "\n");
			
			/* checking for symbol decleration*/
			if(!commandFlag && !instructionFlag && !strncmp(token + strlen(token) - 1, ":", 1))
			{
				strncpy(token + strlen(token) - 1, "\0", 1);
					
				symbolFlag = checkIfSymbol(token, reservedWords);
				
				/* only ':' was entered as symbol decleration*/
				if(symbolFlag == 0)
				{
					fprintf(stderr, "Error in Line %d: empty symbol was entered.\n", lineCount);
					errorFlag = 1;
				}
				
				/* symbol is a reserved word, this is an error*/
				if(symbolFlag == -1)
				{
					fprintf(stderr, "Error in line %d: '%s' is a reserved word.\n", lineCount, token);
					errorFlag = 1;
				}
				
				/* symbol was not declared by the rules, this is an error*/
				if(symbolFlag == -2)
				{
					fprintf(stderr, "Error in Line %d: '%s' is not a valid symbol decleration.\n",lineCount, token);
					errorFlag = 1;
				}
				
				/*symbol declared correctly at the beginning of the line*/
				if(symbolFlag == 1)
				{
					resetToken(symbolToken);
					strcpy(symbolToken, token);
					token = strtok(NULL, " \n\t");	
					
					if(token != NULL)
						restOfLineToken = strtok(NULL, "\n");
						
					else
					{
						errorFlag = 1;
						onlyTokenFlag = 1;	
					}
				}
			}
			
			/* checking if line is an .entry/.extern for new data decleration*/
			if(!onlyTokenFlag && (!strcmp(token, ".entry") || !strcmp(token, ".extern")))
			{
				functionErrorFlag = entryExternLine(symbolHead, symbolToken, token, restOfLineToken, lineCount, symbolFlag, reservedWords);

				if(functionErrorFlag)
					errorFlag = 1;
				
				if(!functionErrorFlag)
					if(!strcmp(token, ".extern"))
					{
						newSymbolNode = newSymbol(restOfLineToken, 0, 2);
						symbolHead = insertSymbol(symbolHead, newSymbolNode);
					}
			}/*end .entry/.extern line*/

			/* checking if the line is a .data/.string for data*/
			else if(!onlyTokenFlag && (!strcmp(token, ".data") || !strcmp(token, ".string")) && symbolFlag != -1)
			{
				functionErrorFlag = dataStringLine(symbolToken, token, restOfLineToken, symbolHead, lineCount, symbolFlag);
				
				if(functionErrorFlag)
					errorFlag = 1;
				
				/* there was no error in the line check*/
				if(!functionErrorFlag)
				{
					/* creating a new symbol with data reference by DC*/
					if(symbolFlag)
					{
						newSymbolNode = newSymbol(symbolToken, DC + 100, 0);
						symbolHead = insertSymbol(symbolHead, newSymbolNode);
					}
					
					insertFlag = 1;
					i = 1;
					
					while(insertFlag)
					{
						/* reading all the integers the data image*/
						if(!strcmp(token, ".data"))
						{
							if(i == 1)
							{
								token2 = strtok(restOfLineToken, ",");
								i++;
							}
							
							else
								token2 = strtok(NULL, ",");
							
							if(token2 == NULL)
								insertFlag = 0;	
								
							else
								newDataNode = newDataImage(atoi(token2));
						}
						
						/* reading all the characters of the string into the data image*/
						if(!strcmp(token, ".string"))
						{
							if(i < strlen(restOfLineToken))
							{
								newDataNode = newDataImage((int)(*(restOfLineToken + i)));
								i++;
							}
							
							/* adding "\0" to the end of the given string*/
							if(i == strlen(restOfLineToken))
								newDataNode = newDataImage(0);
						}
						
						if(insertFlag)
						{
							dataHead = insertImage(dataHead, newDataNode);
							
							if(!strcmp(token, ".string") && i == strlen(restOfLineToken))
								insertFlag = 0;
							
							DC++;

						}
					}
				}	
				
				else
					errorFlag = 1;		
			}/*end of data/.string line*/
			
			/*command check*/
			else if(!onlyTokenFlag && (commandFlag || checkIfCommand(token, reservedWords)))
			{
				functionErrorFlag = commandLine(symbolHead, symbolToken, token, restOfLineToken, lineCount, symbolFlag, reservedWords); 
				
				/* there was an error with the command line*/
				if(functionErrorFlag)
					errorFlag = 1;
				
				/* the command line is legit*/
				else
				{
					/* creating a new symbol with data reference by IC*/
					if(symbolFlag)
					{
						newSymbolNode = newSymbol(symbolToken, IC + 100, 1);
						symbolHead = insertSymbol(symbolHead, newSymbolNode);
					}
					
					convertInstructionToBinary(instructionArr, token, restOfLineToken, reservedWords);			
					L = 1 + findL(token, restOfLineToken, reservedWords);

					/* creating the sub list for the instruction with the allocated nodes for the data
					those nodes are temporary storing the instruction arr because their data is not relevant yet*/
					newInstructionNode = newInstructionImage(instructionArr);
					IC++;
					
					instructionNode = newInstructionNode;
					
					for(i = 1; i < L; i++)
					{
						newInstructionNode->next = newInstructionImage(instructionArr);
						newInstructionNode = newInstructionNode->next;
						IC++;
					}
					
					if(instructionHead == NULL)
						instructionHead = instructionNode;
						
					else
						instructionHead = insertImage(instructionHead, instructionNode);
				}
				
			}/*end of command line*/	

			/* not a command and not an instruction and not a symbol decleration is an error*/
			else if(onlyTokenFlag || (!commandFlag && !instructionFlag && strncmp(token + strlen(token) - 1, ":", 1) && !checkIfCommand(token, reservedWords) && !checkIfInstruction(token, reservedWords)))
			{
				fprintf(stderr, "Error in line %d: unrecogniable command or instruction.\n", lineCount);
				errorFlag = 1;
			}				
		}

		lineCount++;
	}
	
	/* updating the value of all data symbol to be DC + IC*/
	symbolNode = symbolHead;
	
	while(symbolNode != NULL)
	{
		if(!(symbolNode->dataType))
			symbolNode->symbolValue += IC;
		
		symbolNode = symbolNode->next;
	}
 	
	/*second loop on the assembly file*/
	if(!errorFlag)
	{
		fseek(filePointer, 0, SEEK_SET);
		errorFlag = secondLoop(filePointer, fileLength, symbolHead, instructionHead, reservedWords);
		
		printf("**********************************************\n");
	}
	
	if(errorFlag)
		fprintf(stderr, "\nAssembler of '%s' failed.\n\n", currentFile);
		
	else
	{
		printf("Assemble of '%s' was successfull.\n\n", currentFile);
		
		symbolNode = symbolHead;
		instructionNode = instructionHead;
		dataNode = dataHead;
	
		/* creating the object file*/
		makeObjectFile(newFileName, instructionNode, dataNode, IC, DC);
		
		symbolNode = symbolHead;
		
		/* checking to see if any symbol declared as entry. */
		while(symbolNode != NULL)
		{
			/* entry symbol encountered, can create an entry file. */
			if(!strcmp(symbolNode->type, "relocateble"))
			{
				makeEntryFile(newFileName, symbolNode);
				break;
			}
			
			else
				symbolNode = symbolNode->next;
		}
		
		symbolNode = symbolHead;
		
		/* checking to see if any symbol declared as extern. */
		while(symbolNode != NULL)
		{
			/* extern symbol encountered, can create an extern file. */
			if(!strcmp(symbolNode->type, "external"))
			{
				makeExternFile(filePointer, fileLength, newFileName, symbolHead, reservedWords);
				break;
			}
			
			else
				symbolNode = symbolNode->next;
		}
	}

	free(line);
	free(symbolToken);
	
	/* freeing the symbol table*/
	while(symbolHead != NULL)
	{
		symbolNode = symbolHead;
		
		free(symbolNode->symbolName);
		free(symbolNode->type);
		
		symbolHead = symbolHead->next;
		free(symbolNode);
	}
	
	/*freeing the data image list*/
	while(dataHead != NULL)
	{
		dataNode = dataHead;
		
		dataHead = dataHead->next;
		free(dataNode);
	}
	
	/*freeing the instruction image list*/
	while(instructionHead != NULL)
	{
		instructionNode = instructionHead;
		
		instructionHead = instructionHead->next;
		free(instructionNode);
	}
}

/* This function deals with the second loop of the assembler, finishing the images and detecting more errors*/
int secondLoop(FILE* filePointer, int fileLength, struct SymbolTable* symbolHead, struct Image* instructionHead, char* reservedWords[])
{
	char* line = (char*)malloc(MAX_LINE_LENGTH);
	char* token;
	char* token2;
	char* restOfLineToken;
	
	int L; /* the number of instruction images of the command line*/
	int errorFlag = 0; /* a flag that detects if an error has been occured in the second loop*/
	int functionErrorFlag; /* a flag that detects if an error has been occured inside a called function*/
	int binary[14]; /* the binary representation of the operand*/
	int num;	/* an integer that represents the operand*/
	int lineCount = 1; /* current line*/
	int i; 
	
	struct SymbolTable* symbolNode = NULL; /* a node of the symbol table*/
	struct Image* instructionNode = instructionHead; /* a node of the instruction table*/
	
	while(ftell(filePointer) < fileLength)
	{
		fgets(line, MAX_LINE_LENGTH, filePointer);

		token = strtok(line, " \t\n");
		
		if(!strncmp(token + strlen(line) - 1, ":", 1))
			token = strtok(NULL, " \t");
		
		if(!strcmp(token, ".entry"))
		{
			token = strtok(NULL, " \t\n");
			
			symbolNode = symbolHead;
			symbolNode = getSymbol(symbolNode, token);
			
			/* updating the symbol to entry type*/
			if(symbolNode != NULL)
			{
				/* symbol is external, this is an error*/
				if(!strcmp(symbolNode->type, "external"))
				{
					fprintf(stderr, "Error in line %d: '%s' already declared as external.\n", lineCount, token);
					errorFlag = 1;
				}
				/* the same symbol can be declared as entry more than once, but its usless*/
				else if(!strcmp(symbolNode->type, "relocateble"))
					printf("Warning in line %d: '%s' already declared as entry before.\n",lineCount, token);
				
				else
					strcpy(symbolNode->type, "relocateble");
			}
				
			else
				printf("Warning in line %d: '%s' declared as entry but is not initialized with an instruction or with data.\n",lineCount, token);
		}
		
		if(checkIfCommand(token, reservedWords))
		{
			/*command with 0 operands*/
			if(strcmp(token, "stop") && strcmp(token, "rts"))
				restOfLineToken = strtok(NULL, "\n");
					
			L = findL(token, restOfLineToken, reservedWords);
			
			instructionNode = instructionNode->next;
						
			/* command with 2 operands*/
			if(!strcmp(token, "mov") || !strcmp(token, "cmp") || !strcmp(token, "add") || !strcmp(token, "sub") || !strcmp(token, "lea")) 
			{
				token2 = strtok(restOfLineToken, ",");
			
				while(!strncmp(token2, " ", 1) || !strncmp(token2, "\t", 1))
						token2++;
						
				/* 2 instruction images nodes*/
				if(L == 2)
				{
					/* first operand update*/
					functionErrorFlag = updateInstructionNode(symbolHead, instructionNode, token2, lineCount, reservedWords);
					
					/* the given operand is an unrecognisble symbol*/
					if(functionErrorFlag)
						errorFlag = 1;
											
					instructionNode = instructionNode->next;
					
					token2 = strtok(NULL, " \t\n");
					
					/* second operand update*/
					functionErrorFlag = updateInstructionNode(symbolHead, instructionNode, token2, lineCount, reservedWords);
					
					/* the given operand is an unrecognisble symbol*/
					if(functionErrorFlag)
						errorFlag = 1;
								
					instructionNode = instructionNode->next;				
				}
				
				/* 1 instruction image node = 2 registers as operands*/
				else
				{
					num = atoi(++token2);
					num = num<<6;
					token2 = strtok(NULL, " \t\n");
					num += atoi(++token2);
					num = num<<2;
					convertToBinary(num, binary);
					
					for(i = 0; i < 14; i++)
						instructionNode->content[i] = binary[i];
						
					instructionNode = instructionNode->next;
				}
			}
			
			/*command with 1 operand*/
			else if(strcmp(token, "stop") && strcmp(token, "rts"))
			{
				token2 = strtok(restOfLineToken, " \t\n(");
				
				/* operand update*/
				functionErrorFlag = updateInstructionNode(symbolHead, instructionNode, token2, lineCount, reservedWords);
				
				/* the given operand is an unrecognisble symbol*/
				if(functionErrorFlag)
					errorFlag = 1;
					
				instructionNode = instructionNode->next;
				
				if(!strcmp(token, "jmp") || !strcmp(token, "bne") || !strcmp(token, "jsr"))
				{
					token2 = strtok(NULL, ",");
					
					/* operands inside the (...) for jumping commands*/
					if(token2 != NULL)
					{
						/* 2 operands inside the (...)*/
						if(L == 3)
						{
							functionErrorFlag = updateInstructionNode(symbolHead, instructionNode, token2, lineCount, reservedWords);
							instructionNode = instructionNode->next;
							
							/* the given operand is an unrecognisble symbol*/
							if(functionErrorFlag)
								errorFlag = 1;
							
							token2 = strtok(NULL, ")");
							
							functionErrorFlag = updateInstructionNode(symbolHead, instructionNode, token2, lineCount, reservedWords);
							instructionNode = instructionNode->next;
							
							/* the given operand is an unrecognisble symbol*/
							if(functionErrorFlag)
								errorFlag = 1;
						}
						
						/* 2 registers inside the (...)*/
						else if(L == 2)
						{
							num = atoi(++token2);
							num = num<<6;
							token2 = strtok(NULL, " \t\n");
							num += atoi(++token2);
							num = num<<2;
							convertToBinary(num, binary);
					
							for(i = 0; i < 14; i++)
								instructionNode->content[i] = binary[i];
						
							instructionNode = instructionNode->next;
						}
					}
				}	
			}			
		}
		
		lineCount++;
	}
	
	free(line);
	return errorFlag;
}

/* This function creates the object file of the current assembly file*/
void makeObjectFile(char* fileName, struct Image* instructionHead, struct Image* dataHead, int IC, int DC)
{
	FILE* objectFilePointer = NULL; /* the new object file*/
	
	int memoryIndex = 100; /* the memory index of the binary line*/
	int i;
	
	char content[14]; /* the binary line, with "spaciel binary representation*/
	char* objectFileName = (char*)malloc(MAX_FILE_NAME_LENGTH);
	
	
	strcpy(objectFileName, fileName);
	strcat(objectFileName, ".ob");
	objectFilePointer = fopen(objectFileName, "w+");
	
	fprintf(objectFilePointer, "\t  %d %d\n", IC, DC);
	
	/* writing the instruction image to the object file*/
	while(instructionHead != NULL)
	{
		for(i = 13; i>= 0; i--)
		{
			if(instructionHead->content[i] == 0)
				strncpy(content + i, ".", 1);
				
			else
				strncpy(content + i, "\\", 1);
		}
				
		instructionHead = instructionHead->next;
		
		if(memoryIndex < 1000)
			fprintf(objectFilePointer, "0");
	
		fprintf(objectFilePointer, "%d\t\t", memoryIndex);
		memoryIndex++;
			
		for(i = 13; i>= 0; i--)
			fputc(content[i], objectFilePointer);
			
		fprintf(objectFilePointer, "\n");
	}
	
	/* writing the data image to the object file*/
	while(dataHead != NULL)
	{
		for(i = 13; i>= 0; i--)
		{
			if(dataHead->content[i] == 0)
				strncpy(content + i, ".", 1);
				
			else
				strncpy(content + i, "\\", 1);
		}
				
		dataHead = dataHead->next;
		
		if(memoryIndex < 1000)
			fprintf(objectFilePointer, "0");
			
		fprintf(objectFilePointer, "%d\t\t", memoryIndex);
		memoryIndex++;
			
		for(i = 13; i>= 0; i--)
			fputc(content[i], objectFilePointer);
			
		fprintf(objectFilePointer, "\n");
	}
	
	printf("'%s' created succesfully.\n", objectFileName);
		
	free(objectFileName);	
	fclose(objectFilePointer);
}

/* This function creates an entry file*/
void makeEntryFile(char* fileName, struct SymbolTable* symbolNode)
{
	FILE* entryFilePointer = NULL; /* the new object file*/

	char* entryFileName = (char*)malloc(MAX_FILE_NAME_LENGTH);
	
	strcpy(entryFileName, fileName);
	strcat(entryFileName, ".ent");
	entryFilePointer = fopen(entryFileName, "w+");
	
	while(symbolNode != NULL)
	{
		if(!strcmp(symbolNode->type, "relocateble"))
			fprintf(entryFilePointer, "%s\t%d\n", symbolNode->symbolName, symbolNode->symbolValue);
		
		symbolNode = symbolNode->next;
	}
	
	printf("'%s' created succesfully.\n", entryFileName);
	
	free(entryFileName);
	fclose(entryFilePointer);
}

/* This function creates an extern file*/
void makeExternFile(FILE* filePointer, int fileLength, char* fileName, struct SymbolTable* symbolHead, char* reservedWords[])
{
	FILE* externFilePointer = NULL;
	
	char* line = (char*)malloc(MAX_LINE_LENGTH);
	char* externFileName = (char*)malloc(MAX_FILE_NAME_LENGTH);
	char* token;
	char* restOfLineToken;
	
	int IC = 100;
	int L;
	int registerFlag;
	
	struct SymbolTable* symbolNode = NULL;
	
	strcpy(externFileName, fileName);
	strcat(externFileName, ".ext");
	externFilePointer = fopen(externFileName, "w+");
	
	fseek(filePointer, 0, SEEK_SET);
	
	while(ftell(filePointer) < fileLength)
	{
		registerFlag = 0;
		fgets(line, MAX_LINE_LENGTH, filePointer);

		token = strtok(line, " \t\n");
		
		/* skiping symbol decleration*/
		if(!strncmp(token + strlen(token) - 1, ":", 1))
			token = strtok(NULL, " \t\n");
			
		restOfLineToken = strtok(NULL, "\n");
		
		/* external symbol search only in command lines*/
		if(checkIfCommand(token, reservedWords))
		{
			IC++;
			
			L = findL(token, restOfLineToken, reservedWords);
			
			/* command with 1 operand*/
			if(L == 1)
			{	
				restOfLineToken = strtok(restOfLineToken, " \n\t,");
				
				/* if L = 1 and first operand is register, than the second operand is also a register*/
				if(!checkIfRegister(restOfLineToken, reservedWords))
				{
					if(checkIfSymbol(restOfLineToken, reservedWords) == 1)
					{
						symbolNode = symbolHead;
						symbolNode = getSymbol(symbolNode, restOfLineToken);
					
						if(!strcmp(symbolNode->type, "external"))
							fprintf(externFilePointer, "%s\t%d\n", restOfLineToken, IC);
					}
				}
				
				IC++;
			}
			
			/* commands with more than 1 operand (2 or 3)*/
			else if(L > 1)
			{
				/* commands with 2 operands*/
				if(strcmp(token, "jmp") && strcmp(token, "bne") && strcmp(token, "jsr"))
				{
					token = strtok(restOfLineToken, ",");
					
					while(!strncmp(token, " ", 1) || !strncmp(token, "\t", 1))
						token++;
						
					while(strncmp(token, "#", 1) && (!strncmp(token +strlen(token) - 1, " ", 1) || !strncmp(token +strlen(token) - 1, "\t", 1)))
						strncpy(token + strlen(token) - 1, "\0", 1);
						
					if(checkIfSymbol(token, reservedWords) == 1)
					{
						symbolNode = symbolHead;
						symbolNode = getSymbol(symbolNode, token);
				
						if(!strcmp(symbolNode->type, "external"))
							fprintf(externFilePointer, "%s\t%d\n", token, IC);
					}
					
					IC++;
					token = strtok(NULL, " \t\n");
					
					
					if(checkIfSymbol(token, reservedWords) == 1)
					{
						symbolNode = symbolHead;
						symbolNode = getSymbol(symbolNode, token);
					
						if(!strcmp(symbolNode->type, "external"))
							fprintf(externFilePointer, "%s\t%d\n", token, IC);
					}
					
					IC++;
				}
				
				/* jump commands with 2/3 operands (including operands in (...)*/
				else
				{
					token = strtok(restOfLineToken, "(");
					
					if(checkIfRegister(token, reservedWords))
						registerFlag = 1;
					
					if(checkIfSymbol(token, reservedWords) == 1)
					{
						symbolNode = symbolHead;
						symbolNode = getSymbol(symbolNode, token);
					
						if(!strcmp(symbolNode->type, "external"))
							fprintf(externFilePointer, "%s\t%d\n", token, IC);
					}
					
					IC++;
					
					token = strtok(NULL, ",");
					
					if(checkIfSymbol(token, reservedWords) == 1)
					{
						symbolNode = symbolHead;
						symbolNode = getSymbol(symbolNode, token);
					
						if(!strcmp(symbolNode->type, "external"))
							fprintf(externFilePointer, "%s\t%d\n", token, IC);
					}
				
					IC++;
					token = strtok(NULL, ")");
					
					if(checkIfSymbol(token, reservedWords) == 1)
					{
						symbolNode = symbolHead;
						symbolNode = getSymbol(symbolNode, token);
					
						if(!strcmp(symbolNode->type, "external"))
							fprintf(externFilePointer, "%s\t%d\n", token, IC);
					}
					
					/* at least one of the operands inside (...) is not a register. If both were registers, they
					got only one image node*/
					if(!registerFlag && L == 2)
						IC++;	
				}
			}
		}
	}
	
	printf("'%s' created succesfully.\n", externFileName);
	
	free(line);
	free(externFileName);
	fclose(externFilePointer);
}

/* This function updates the instruction image of the current given token. The token is a symbol or a register or an integer.
The function returns 1 in case that the token is an unrecognisable symbol or 0 otherwise.*/
int updateInstructionNode(struct SymbolTable* symbolHead, struct Image* instructionNode, char* token, int lineCount, char* reservedWords[])
{
	int binary[14]; /* the binary representaion of the given token*/
	int ERA[2]; /* ERA representation*/
	int num; /* an integer representation of the given token*/
	int i; 
	
	struct SymbolTable* symbolNode = NULL;
	
	if(checkIfSymbol(token, reservedWords) && !checkIfRegister(token, reservedWords) && strncmp(token, "#", 1))
	{
		symbolNode = symbolHead;
		symbolNode = getSymbol(symbolNode, token);
						
		/* the symbol isn't defined, this is an error*/
		if(symbolNode == NULL)
		{
			fprintf(stderr, "Error in line %d: operand '%s' is undefined.\n", lineCount, token);
			return 1;
		}
						
		else
		{
			num = symbolNode->symbolValue;
							
			/* the symbol is external*/
			if(!strcmp(symbolNode->type, "external"))
			{
				ERA[0] = 0;
				ERA[1] = 1;
			}
							
			/* the symbol is defined in this file, relocatble*/
			else
			{
				ERA[0] = 1;
				ERA[1] = 0;
			}
		}
	}
					
	/* the operand is an integer or a register*/
	if(!strncmp(token, "#", 1) || checkIfRegister(token, reservedWords))
	{
		num = atoi(++token);
						
		/* absolute*/				
		ERA[0] = 0;
		ERA[1] = 0;
	}
					
	/* creating the binary representation of the second operand*/
	num = num<<2;
	convertToBinary(num, binary);
	binary[1] = ERA[0];
	binary[0] = ERA[1];
					
	for(i = 0; i < 14; i++)
		instructionNode->content[i] = binary[i];
		
	return 0;
}
	
/* This function checks of a given symbol is already declared in the symbol table.*/
struct SymbolTable* getSymbol(struct SymbolTable* symbolNode, char* symbol)
{
	while(symbolNode != NULL)
	{
		if(!strcmp(symbolNode->symbolName, symbol))
			return symbolNode;
		
		symbolNode = symbolNode->next;
	}
	
	return NULL;
}

/* This function gets a symbol name and a value(address. address is 0 if external) and data type(0 for data or 1 for command)
and creates a new symbol*/
struct SymbolTable* newSymbol(char* newSymbolName, int newSymbolValue, int dataType)
{
	struct SymbolTable* symbol = (struct SymbolTable*)malloc(sizeof(struct SymbolTable));
	symbol->symbolName = (char*)malloc(strlen(newSymbolName) + 1);
	strcpy(symbol->symbolName, newSymbolName);
	symbol->symbolValue = newSymbolValue;
	symbol->dataType = dataType;
	symbol->next = NULL;
	symbol->type = (char*)malloc(15);
	if(newSymbolValue == 0)
		strcpy(symbol->type, "external");
	
	/* for the second loop it will be initialized as relocatble*/
	else
		strcpy(symbol->type, "null");

	return symbol;
}

/* This function gets a number (integer between -8191 to 8191 or an ascii value of a character,
and creates a new node for the data image*/
struct Image* newDataImage(int num)
{
	struct Image* newDataImage = (struct Image*)malloc(sizeof(struct Image));
	int binary[14];
	int i;
	
	convertToBinary(num, binary);
	
	for(i = 13; i >= 0; i--)
		newDataImage->content[i] = binary[i];
	
	newDataImage->next = NULL;
	
	return newDataImage;
}

/* This function gets an instruction arr(may be null for allocation porpuses) 
and creates a new node for instruction image*/
struct Image* newInstructionImage(int* instructionArr)
{
	struct Image* newInstructionImage = (struct Image*)malloc(sizeof(struct Image));
	int i;
	
	for(i = 13; i >= 0; i--)
		newInstructionImage->content[i] = instructionArr[i];
	
	newInstructionImage->next = NULL;
	
	return newInstructionImage;
}

/* This function gets a command line and checks if its legit.
The function returns 0 if the code is legal or 1 if there was an error.*/
int commandLine(struct SymbolTable* symbolHead, char* symbolToken, char* commandToken, char* restOfLineToken, int lineCount, int symbolFlag, char* reservedWords[])
{
	int errorFlag = 0; /* a flag that checks if there was at least one error in lhe command line*/
	int intFlag = 0; /* a flag that checks if a given token is an integer*/
	int nullFlag = 0; /* a flag that checks for null*/
	int i = 0;

	char* token; /* token for strtok*/
	char* tempRestOfLineToken = (char*)malloc(MAX_LINE_LENGTH);
	
	struct SymbolTable* symbolNode = NULL; /* a SymbolTable node*/
	
	if(restOfLineToken != NULL)
		strcpy(tempRestOfLineToken, restOfLineToken);

	/* check if a symbol was encountered at the start of the line*/
	if(symbolFlag)
	{
		symbolNode = symbolHead;
		symbolNode = getSymbol(symbolNode, symbolToken);
						
		/* symbol already declared, this is an error*/
		if(symbolNode != NULL)
		{
			fprintf(stderr, "Error in line %d: %s already declared.\n", lineCount, symbolToken);
			errorFlag = 1;
		}
	}
	/* commands with no operands*/
	if(!strcmp(commandToken, "rts") || !strcmp(commandToken, "stop"))
	{
		/* more text after command, this is illegal*/
		if(restOfLineToken != NULL)
		{
			fprintf(stderr, "Error in line %d: illegal text after '%s' command.\n", lineCount, commandToken);
			errorFlag = 1;
		}
	}
	
	/*command with one operand */
	else if(!strcmp(commandToken, "not") || !strcmp(commandToken, "clr") || !strcmp(commandToken, "inc") || !strcmp(commandToken, "dec") || !strcmp(commandToken, "prn") || !strcmp(commandToken, "red") || !strcmp(commandToken, "rts") || !strcmp(commandToken, "jmp") || !strcmp(commandToken, "bne") || !strcmp(commandToken, "jsr"))
	{
		/* trimming the token from blank spaces from the end of the token*/
		while(strlen(tempRestOfLineToken) > 0 && (!strncmp(tempRestOfLineToken + strlen(tempRestOfLineToken) - 1, " ", 1) || !strncmp(tempRestOfLineToken + strlen(tempRestOfLineToken) - 1, "\t", 1)))
		strncpy(tempRestOfLineToken + strlen(tempRestOfLineToken) - 1, "\0", 1);

		/* no text after command, this is illegal*/
		if(strlen(tempRestOfLineToken) < 1)
		{
			fprintf(stderr, "Error in line %d: '%s' command encountered but no operand found.\n", lineCount, commandToken);
			errorFlag = 1;
		}
		
		else
		{
			token = strtok(tempRestOfLineToken, " \t\n");
			
			/* command or instruction as operand, this is illegal*/
			if(checkIfCommand(token, reservedWords) || checkIfInstruction(token, reservedWords))
			{
				fprintf(stderr, "Error in line %d: '%s' can't be an operand.\n", lineCount, token);
				errorFlag = 1;
			}
			
			/* the operand is not a symbol or a symbol with more text afterwards ([symbol](...)*/
			else if(checkIfSymbol(token, reservedWords) != 1)
			{
				if(!checkIfRegister(token, reservedWords) && strcmp(commandToken, "jmp") && strcmp(commandToken, "bne") && strcmp(commandToken, "jsr") && strcmp(commandToken, "prn"))
				{
					fprintf(stderr, "Error in line %d: not a valid syntax for '%s' command.\n", lineCount, commandToken);
					errorFlag = 1;
				}
					
				else if(!strncmp(token, "#", 1))
				{
					intFlag = checkIfInt(token + 1);
					
					/* the operand is an integer and the command isn't 'prn', this is illegal*/
					if(intFlag && strcmp(commandToken, "prn"))
					{
						fprintf(stderr, "Error in line %d: '%s' can't get an integer as an operand.\n", lineCount, commandToken); 
						errorFlag = 1;
					}
					
					if(intFlag && !strcmp(commandToken, "prn"))
					{
						/* the integer is not within 11 bits limit, this is an error*/
						if((atoi(token + 1) > 2047) || (atoi(token + 1) < -2047))
						{
							fprintf(stderr, "Error in line %d: integer is  not in range.\n", lineCount);
							errorFlag = 1;
						}	
					}
					
					/* # and no integer after it, this is an error*/
					else if(!intFlag)
					{
						fprintf(stderr, "Error in line %d: '#' is an integer decleration but no integer encountered.\n", lineCount);
						errorFlag = 1;
					}
				}
				
				else
				{
					/* if command is jmp/bne/jsr and the token isn't a register, check for (parameter,parameter) properties*/
					if(!checkIfRegister(token, reservedWords) && (!strcmp(commandToken, "jmp") || !strcmp(commandToken, "bne") || !strcmp(commandToken, "jsr")))
					{
						token = strtok(token, "(");
						
						/* syntax error or not a valid symbol name, this is an error*/
						if(checkIfSymbol(token, reservedWords) != 1)
						{
							fprintf(stderr, "Error in line %d: not a valid operand name for '%s' command.\n", lineCount, commandToken);
							errorFlag = 1;
						}
						
						
						token = strtok(NULL, " \t\n");

						/* syntax error*/
						if(token == NULL || strncmp(token + strlen(token) - 1, ")", 1))
						{
							fprintf(stderr, "Error in line %d: syntax error for '%s' command.\n", lineCount, commandToken);
							free(tempRestOfLineToken);
							return 1;
						}
						
						token = strtok(token, ")");
						token = strtok(token, ",");
						
						/* no operands for the command, this is an error*/
						if(token == NULL)
						{
							fprintf(stderr, "Error in line %d: no operands were found for '%s' command.\n", lineCount, commandToken);
							errorFlag = 1;
							nullFlag = 1;
						}
							
						else if(!strncmp(token, "#", 1))
						{
							token++;
							
							/*first operand:  # is integer declaratoin vut not a valid int afterwards, this is an error*/
							if(!checkIfInt(token))
							{
								fprintf(stderr, "Error in line %d: not a valid integer for '%s' command.\n", lineCount, commandToken);
								errorFlag = 1;
							}
							
							/* integer out of range, this is an error*/
							else if(atoi(token) > 2047 || atoi(token) < -2047)
							{
								fprintf(stderr, "Error in line %d: '%s' out of range.\n", lineCount, token);
								errorFlag = 1;
							}
						}
												
						/*first operand:  operand is a command or an instruction or (not a symbol and not a register) , this is an error*/
						else if(checkIfCommand(token, reservedWords) || checkIfInstruction(token, reservedWords) || (!checkIfRegister(token, reservedWords) && checkIfSymbol(token, reservedWords) != 1))
						{
							fprintf(stderr, "Error in line %d: '%s' is not a valid operand for '%s' command.\n", lineCount, token, commandToken);
							errorFlag = 1;
						}
						
						if(token != NULL)
						{
							token = strtok(NULL, " \t\n");
						
							/* second operand was not found, this is an error*/
							if(token == NULL && !nullFlag)
							{
								fprintf(stderr, "Error in line %d: no second operand  was found for '%s' command.\n", lineCount, commandToken);
								errorFlag = 1;
							}
						
							else if(!strncmp(token, "#", 1))
							{
								token++;
							
								/* second operand: # is integer declaratoin vut not a valid int afterwards, this is an error*/
								if(!checkIfInt(token))
								{
									fprintf(stderr, "Error in line %d: not a valid integer for '%s' command.\n", lineCount, commandToken);
									errorFlag = 1;
								}
							
								/* integer out of range, this is an errir*/
								else if(atoi(token) > 2047 || atoi(token) < -2047)
								{
									fprintf(stderr, "Error in line %d: '%s' out of range.\n", lineCount, token);
									errorFlag = 1;
								}
							}
						
							/*second operand:  operand is a command or an instruction or (not a symbol and not a register) , this is an error*/
							else if(checkIfCommand(token, reservedWords) || checkIfInstruction(token, reservedWords) || (!checkIfRegister(token, reservedWords) && checkIfSymbol(token, reservedWords) != 1))
							{
								fprintf(stderr, "Error in line %d: '%s' is not a valid operand for '%s' command.\n", lineCount, token, commandToken);
								errorFlag = 1;
							}
						}		
					}
					
					/* operand is illegal*/
					else if(!checkIfRegister(token, reservedWords))
					{
						fprintf(stderr, "Error in line %d: '%s' is not a valid operand.\n", lineCount, token);
						errorFlag = 1;
					}
				}
			}
			
			/* the operand is a symbol*/
			else
			{
			
				token = strtok(NULL, " \t\n");
				
				/* more text after the operand, this is illegal*/
				if(token != NULL)
				{
					fprintf(stderr, "Error in line %d: illegal text after the operand.\n", lineCount); 
					errorFlag = 1;
				}
			}
		}
	}
	
	/* commands with 2 operands*/
	else
	{	
		/* trimming the token from blank spaces from the end of the token*/
		while(strlen(tempRestOfLineToken) > 0 && (!strncmp(tempRestOfLineToken + strlen(tempRestOfLineToken) - 1, " ", 1) || !strncmp(tempRestOfLineToken + strlen(tempRestOfLineToken) - 1, "\t", 1)))
		strncpy(tempRestOfLineToken + strlen(tempRestOfLineToken) - 1, "\0", 1);
		
		/* no operands after the command, this is an error*/
		if(strlen(tempRestOfLineToken) < 1)
		{
			fprintf(stderr, "Error in line %d: no operands were found for '%s' command.\n", lineCount, commandToken);
			errorFlag = 1; 
		}
		
		else
		{
			token = strtok(tempRestOfLineToken, ",");

			/* trimming token from blank spaces at the start*/
			while(!strncmp(token, " \t", 1))
				token++;
			
			i = strlen(token) - 1;
			
			/* trimming token from blank spaces at the end*/
			while(!strncmp(token + i, " \t", 1))
			{
				strncpy(token + i, "\0", 1);
				i--;
			}
			
			/* integer decleration was encountered*/
			if(!strncmp(token, "#", 1))
			{
				token++;
				intFlag = checkIfInt(token);
				
				if(intFlag)
				{
					/* an integer can't be an operand for lea command, this is an error*/
					if(!strcmp(commandToken, "lea"))
					{
						fprintf(stderr, "Error in line %d: an integer is not a valid source operand for 'lea' command.\n", lineCount);
						errorFlag = 1;
					}
					
					/* integer out of 12 signed int range, this is an error*/
					else if((atoi(token) > 2047) || (atoi(token) < -2047))
					{
						fprintf(stderr, "Error in line %d: '%s' is out of range for source operand.\n", lineCount, token);
						errorFlag = 1;
					}
				}
				
				/* # is an integer decleration but no integer afterwards, this is an error*/
				else
				{
					fprintf(stderr, "Error in line %d: source operand syntax error.\n", lineCount);
					errorFlag = 1;
				}
				
			}
			
			/* one operand is a command/instruction or something that is not a symbol or a register, this is an error*/
			else if(checkIfCommand(token, reservedWords) || checkIfInstruction(token, reservedWords) || (checkIfSymbol(token, reservedWords) != 1 && !checkIfRegister(token, reservedWords)))
			{
				fprintf(stderr, "Error in line %d: '%s' is not a valid source operand.\n", lineCount, token);
				errorFlag = 1;
			}
			
			/* register as source operand for lea command, this is an error*/
			else if(!strcmp(commandToken, "lea") && checkIfRegister(token, reservedWords))
			{
				fprintf(stderr, "Error in line %d: a register is not a valid source operand for 'lea' command.\n", lineCount);
				errorFlag = 1;
			}
			
			token = strtok(NULL, " \t\n");
			
			/* only one operand was sent, this is an error*/
			if(token == NULL)
			{
				fprintf(stderr, "Error in line %d: '%s' encountered but no second operand was found.\n", lineCount, commandToken);
				errorFlag  = 1;
			}
			
			else
			{
				i = strlen(token) - 1;
			
				/* trimming token from blank spaces at the end*/
				while(!strncmp(token + i, " \t", 1))
				{
					strncpy(token + i, "\0", 1);
					i--;
				}
				
				/* integer decleration was encountered*/
				if(!strncmp(token, "#", 1))
				{
					token++;
					intFlag = checkIfInt(token);
					
					if(intFlag)
					{
						/* only 'cmp' command should have an integer for destination operand, this is an error*/
						if(strcmp(commandToken, "cmp"))
						{
							fprintf(stderr, "Error in line %d: an integer is not a valid destination operand for '%s' command.\n", lineCount, commandToken);
							errorFlag = 1;
						}
						
						else if((atoi(token) > 2047) || (atoi(token) < -2047))
						{
							fprintf(stderr, "Error in line %d: '%s' is out of range for destination operand.\n", lineCount, token);
							errorFlag = 1;
						}
					}
					
					/* # is an integer decleration but no integer afterwards, this is an error*/
					else
					{
						fprintf(stderr, "Error in line %d: destination operand syntax error.\n", lineCount);
						errorFlag = 1;
					}
				}
				
				/* one operand is a command/instruction or something that is not a symbol or a register, this is an error*/
				else if(checkIfCommand(token, reservedWords) || checkIfInstruction(token, reservedWords) || (checkIfSymbol(token, reservedWords) != 1 && !checkIfRegister(token, reservedWords)))
				{
					fprintf(stderr, "Error in line %d: '%s' is not a valid destination operand.\n", lineCount, token);
					errorFlag = 1;
				}
				
				token = strtok(NULL, " \t\n");
				
				/* more text after destination operand, this is an error*/
				if(token != NULL)
				{
					fprintf(stderr, "Error in line %d: illegal text after second operand.\n", lineCount);
					errorFlag = 1;
				}
			}				
		}
	}
	
	free(tempRestOfLineToken);
	
	return errorFlag;
}

/* This function gets an .entry/.extern type of line and checks if its legit.
The function returns 0 if the code is legal or 1 if there was an error.*/
int entryExternLine(struct SymbolTable* symbolHead, char* symbolToken, char* entryOrExternToken, char* restOfLineToken, int lineCount, int symbolFlag, char* reservedWords[])
{
	int errorFlag = 0;
	char* token;	
	struct SymbolTable* symbolNode = NULL;
			
	/* symbol is usless at the start of line, but it's not an error*/
	if(symbolFlag)
		printf("Warning in line %d: usless symbol declaration at the beggining of the line.\n", lineCount);
		
	token = strtok(restOfLineToken, " \n\t");
					
	/* no text after .entry/.extern is an error*/
	if(token == NULL)
	{
		fprintf(stderr, "Error in line %d: new decleration encountered but no symbol found.\n", lineCount);
		errorFlag = 1;
	}
				
	else
	{
		restOfLineToken = strtok(NULL, " \t\n");
		symbolFlag = checkIfSymbol(token, reservedWords);
	}
	
	if(restOfLineToken != NULL)
	{
		while(!strncmp(restOfLineToken, " ", 1) || !strncmp(restOfLineToken, "\t", 1))
			restOfLineToken++;
	}
	
	/* more text after token is an error*/
	if(restOfLineToken != NULL && strlen(restOfLineToken) > 1)
	{
		fprintf(stderr, "Error in line %d: more text after token decleration.\n", lineCount);
		errorFlag = 1;
	}
					
	/*symbol name is not valid. This is an error*/
	if(token != NULL && !symbolFlag)
	{
		fprintf(stderr, "Error in line %d: not a valid symbol name.\n", lineCount);
		errorFlag = 1;
	}
	
	else if(!errorFlag)
	{
		symbolNode = symbolHead;
		symbolNode = getSymbol(symbolNode, token);
				
		/* extern symbol already declared, this is an error*/
		if(symbolNode != NULL && !strcmp(entryOrExternToken, ".extern"))
		{
			fprintf(stderr, "Error in line %d: %s already declared.\n", lineCount, token);
			errorFlag = 1;
		}	
		
		/* entry symbol already declared as external symbol, this is an error*/
		else if(symbolNode != NULL && !strcmp(entryOrExternToken, ".entry") && !strcmp(symbolNode->type, "external"))
		{
			fprintf(stderr, "Error in line %d: '%s' already declared from an external source.\n", lineCount, token);
			errorFlag = 1;
		}			
	}
	
	return errorFlag;
}

/* This function gets an .data/.string type of line and checks if its legit. 
The function returns 0 if the code is legal or 1 if there was an error.*/
int dataStringLine(char* symbolToken, char* dataOrStringToken, char* restOfLineToken, struct SymbolTable* symbolHead, int lineCount, int symbolFlag)
{
	int errorFlag = 0;
	int dataErrorFlag = 0;
	char* tempRestOfLineToken = (char*)malloc(MAX_LINE_LENGTH);
	
	struct SymbolTable* symbolNode = NULL;
	
	/* no parameters after decleration, this is an error*/
	if(restOfLineToken == NULL)
	{
		fprintf(stderr, "Error in line %d: no data was found after '%s' decleration.\n", lineCount, dataOrStringToken);
		errorFlag = 1;
	}
	
	/* symbol declared at the start of the line*/
	if(symbolFlag)
	{
		symbolNode = symbolHead;
		symbolNode = getSymbol(symbolNode, symbolToken);
					
		/* symbol already exists in the symbol table, this is an error*/
		if(symbolNode != NULL)
		{
			fprintf(stderr, "Error in line %d: %s already declared.\n", lineCount, symbolToken);
			errorFlag = 1;
		}
	}
		
	if((symbolNode == NULL) && (restOfLineToken != NULL))
	{
		/* one or more integers*/
		if(!strcmp(dataOrStringToken, ".data"))
		{
			strcpy(tempRestOfLineToken, restOfLineToken);
			dataErrorFlag = intListCheck(tempRestOfLineToken);
							
			/* list of ints may contain a member thats not an integer or ',' placement is not valid. this is an error*/
			if(dataErrorFlag == 1)
			{
				fprintf(stderr, "Error in line %d: not a valid list of ints.\n", lineCount);
				errorFlag = 1;
			}
							
			if(dataErrorFlag == 2)
			{
				fprintf(stderr, "Error in line %d: one or more of the integers not in range.\n", lineCount);
				errorFlag = 1;
			}
		}
		
		/* string*/
		if(!strcmp(dataOrStringToken, ".string"))
		{
			/* the string after .string doesn't start or ends with '"', this is an error.*/
			if(strncmp(restOfLineToken, "\"", 1) || strncmp(restOfLineToken + strlen(restOfLineToken) - 1, "\"", 1))
			{
				fprintf(stderr, "Error in line %d: string decleration is not valid.\nA string should start and end with '\"'.\n",lineCount);
				errorFlag = 1;
				dataErrorFlag = 1;
			}
								
		}
	
	}
	
	free(tempRestOfLineToken);
	
	return errorFlag;
}

/* This function gets new symbol node and inserts it into the symbol table*/
struct SymbolTable* insertSymbol(struct SymbolTable* head, struct SymbolTable* node)
{
	struct SymbolTable* symbolNode = head;
	if(symbolNode == NULL)
		head = node;
		
	else
	{
		while(symbolNode->next != NULL)
			symbolNode = symbolNode->next;
		
		symbolNode->next = node;
	}
	
	return head;
}

/* This function gets new image node and inserts it into the image*/
struct Image* insertImage(struct Image* head, struct Image* node)
{
	struct Image* imageNode = head;
	if(imageNode == NULL)
		head = node;
		
	else
	{
		while(imageNode->next != NULL)
			imageNode = imageNode->next;
			
		imageNode->next = node;
	}
	
	return head;
}

/* This function gets a command line and translates it into its binary reresentation*/
void convertInstructionToBinary(int* instructionArr, char* commandToken, char* restOfLineToken, char* reservedWords[])
{
	int commandIndex = checkIfCommand(commandToken, reservedWords); /* the number of the command (0-15)*/
	char* token;
	char* tempRestOfLineToken = (char*)malloc(MAX_LINE_LENGTH);
	
	convertToBinary(commandIndex - 1, instructionArr); /* convert the command index to binary*/
	
	if(restOfLineToken != NULL)
		strcpy(tempRestOfLineToken, restOfLineToken);
	
	/*opcode and ERA*/
	instructionArr[9] = instructionArr[3];
	instructionArr[8] = instructionArr[2];
	instructionArr[7] =	instructionArr[1];
	instructionArr[6] = instructionArr[0];
	instructionArr[1] = 0;
	instructionArr[0] = 0;
	
	/*commands with no source operand and destination operand*/
	if(!strcmp(commandToken, "rts") || !strcmp(commandToken, "stop"))
	{
		instructionArr[5] = 0;
		instructionArr[4] = 0;
		instructionArr[3] = 0;
		instructionArr[2] = 0;
	}
	
	/*commands with destination operand*/
	else if(!strcmp(commandToken, "not") || !strcmp(commandToken, "clr") || !strcmp(commandToken, "inc") || !strcmp(commandToken, "dec") || !strcmp(commandToken, "prn") || !strcmp(commandToken, "red") || !strcmp(commandToken, "rts") || !strcmp(commandToken, "jmp") || !strcmp(commandToken, "bne") || !strcmp(commandToken, "jsr"))
	{
		token = strtok(tempRestOfLineToken, "( \t\n");
		
		instructionArr[5] = 0;
		instructionArr[4] = 0;
		
		if(!strncmp(token, "#", 1))
		{
			instructionArr[3] = 0;
			instructionArr[2] = 0;
		}
			
		else if(checkIfRegister(token, reservedWords))
		{
			instructionArr[3] = 1;
			instructionArr[2] = 1;
		}
		
		else
		{
			instructionArr[3] = 0;
			instructionArr[2] = 1;
		}	
	}
	
	/*commands with source operand and destination operand*/
	else
	{
		token = strtok(tempRestOfLineToken, ",");
		
		while(!strncmp(token, " ", 1) || !strncmp(token, "\t", 1))
			token++;
			
		while(!strncmp(token + strlen(token) - 1, " ", 1) || !strncmp(token + strlen(token) - 1, "\t", 1))
			strncpy(token + strlen(token) - 1, "\0", 1);
			
		if(!strncmp(token, "#", 1))
		{
			instructionArr[5] = 0;
			instructionArr[4] = 0;
		}
			
		else if(checkIfRegister(token, reservedWords))
		{
			instructionArr[5] = 1;
			instructionArr[4] = 1;
		}
		
		else
		{
			instructionArr[5] = 0;
			instructionArr[4] = 1;
		}
	
			
		token = strtok(NULL, " \t\n");
		
		if(!strncmp(token, "#", 1))
		{
			instructionArr[3] = 0;
			instructionArr[2] = 0;
		}
			
		else if(checkIfRegister(token, reservedWords))
		{
			instructionArr[3] = 1;
			instructionArr[2] = 1;
		}
		
		else
		{
			instructionArr[3] = 0;
			instructionArr[2] = 1;
		}
	}
	
	/*1'st and 2'nd parameters*/
	if(strcmp(commandToken, "jmp") && strcmp(commandToken, "bne") && strcmp(commandToken, "jsr"))
	{
		instructionArr[13] = 0;
		instructionArr[12] = 0;
		instructionArr[11] = 0;
		instructionArr[10] = 0;
	}
	
	else
	{
		token = strtok(NULL, ",");
		
		if(token != NULL)
		{
			if(!strncmp(token, "#", 1))
			{
				instructionArr[13] = 0;
				instructionArr[12] = 0;
			}
			
			else if(checkIfRegister(token, reservedWords))
			{
				instructionArr[13] = 1;
				instructionArr[12] = 1;
			}
		
			else
			{
				instructionArr[13] = 0;
				instructionArr[12] = 1;
			}
			
			token = strtok(NULL, ")");
			
			if(token != NULL)
			{
				if(!strncmp(token, "#", 1))
				{
					instructionArr[11] = 0;
					instructionArr[10] = 0;
				}
			
				else if(checkIfRegister(token, reservedWords))
				{
					instructionArr[11] = 1;
					instructionArr[10] = 1;
				}
		
				else
				{
					instructionArr[11] = 0;
					instructionArr[10] = 1;
				}
			}
			
			else
			{
				instructionArr[13] = 0;
				instructionArr[12] = 0;
				instructionArr[11] = 0;
				instructionArr[10] = 0;
			}
		}
	}
	
	free(tempRestOfLineToken);
}

