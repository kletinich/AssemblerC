#include "data.h"

void resetToken(char*);
int checkIfSymbol(char*, char*[]);
int intListCheck(char*);
int checkIfInt(char*);
void convertToBinary(int, int*);
int checkIfCommand(char*, char*[]);
int checkIfInstruction(char*, char*[]);
int checkIfmacroOrRegister(char*, char*[]);
int findL(char*, char*, char*[]);

/* this function gets a token and resets it from garbage text*/
void resetToken(char *token)
{
	int i;
	
	for(i = strlen(token) - 1; i >= 0; i--)
	{
		strncpy(token + i, "\0", 1);
	}
}

/* This function gets a token and checks if its a valid symbol syntax*/
int checkIfSymbol(char* token, char* reservedWords[])
{
	int i;
	
	/* check if the symbols length is more than 0*/
	if(strlen(token) == 0)
		return 0;
	
	/* check if the symbol is a reserved word (command/instruction/mcr/endmcr/register)*/ 	
	for(i = 0; i < 30; i++)
	{
		if(!strcmp(token, reservedWords[i]))
			return -1;
	}
	
	/* check the first letter of the symbol (should be one of a-z or A-Z*/
	if(token[0] < 65 || (token[0] > 90 && token[0] < 97) || token[0] > 122)
		return -2;
	
	/* check the rest of the letters  (should be a-z or A-Z or 0-9)*/	
	for(i = 1; i < strlen(token); i++)
		if(token[i] < 48 || (token[i] > 57 && token[i] < 65) || (token[i] > 90 && token[i] < 97) || token[i] > 122)
			return -2;
	
	return 1;
}


/* This function gets a string and checks if its a valid int list (for .data porpuses).
return 0 if the list is legal (ints seperated by ',') or 1 if not or 2 if in not in 13 signed bits range.*/
int intListCheck(char* intList)
{
	char* token;
	int flag = 1;
	int intRepresentation;
	
	/* the list starts with ',' , this is illegal*/
	if(!strncmp(intList, ",", 1))
		return 1;
		
	while(!strncmp(intList + strlen(intList) - 1, " \t", 1))
		strncpy(intList + strlen(intList) - 1, "\0", 1);
	
	/* the list ends with ',' , this is illegal*/	
	if(!strncmp(intList + strlen(intList) - 1, ",", 1))
		return 1;
	
	token = strtok(intList, ",");
	
	/* scanning the list*/
	while(token != NULL)
	{
		while(!strncmp(token, " ", 1) || !strncmp(token, "\t", 1))
			token++;
			
		flag = checkIfInt(token);
		
		if(!flag)
			return 1;
			
		intRepresentation = atoi(token);
		
		if((intRepresentation > 8191) || (intRepresentation < -8191))
			return 2;
			
		token = strtok(NULL, ",");
	}
	
	return 0;
}

/* This function gets a string and checks if its a valid integer
The function returns 0 if it's not valid or 1 otherwise*/
int checkIfInt(char* str)
{
	int i;
	int endOfIntFlag = 0;
	
	/*empty str*/
	if(str == NULL)
		return 0;
		
	if((!strncmp(str, "+", 1) || !strncmp(str, "-", 1)) && (strlen(str) == 1))
		return 0;
	
	/*check if the first letter of str is either '-' or '+' or a value between 0 - 9*/
	if(strncmp(str, "+", 1) && strncmp(str, "-", 1) && (str[0] < 48 || str[0] > 57))
		return 0;
	
	for(i = 1; i < strlen(str); i++)
	{
		/* there may be blank spaces after the last int char*/
		if(str[i] == 32 || str[i] == 9)
			endOfIntFlag = 1;
			
		/* check if the chars are between 0 and 9*/
		if((str[i] < 48 || str[i] > 57) && !endOfIntFlag)
			return 0;
			
		/* if a blank space was encountered, there shouldn't be any more non blank characters.*/
		if(str[i] != 32 && str[i] != 9 && endOfIntFlag)
			return 0;
	}
	
	return 1;
}

/* This function gets a number between -8191 to 8191 and translates them to their binary form*/
void convertToBinary(int num, int* binary)
{
	int i;
	int negFlag = 0; /* a flag that checks if the given number is negative. */
	
	/* 2's complement required*/
	if(num < 0)
	{
		negFlag = 1;
		num = -num;
	}
	
	/* initialize the binary array*/
	for(i = 0; i < 14; i++)
		binary[i] = 0;
			
	i = 0;
	
	/* turn the number into a positive binary form*/
	while(num > 0)
	{
		binary[i++] = num % 2;
		num = num / 2;
	}
	
	/* 2's complement initializing*/
	if(negFlag)
	{
		/* turn all 0's to 1's and all 1's to 0's*/
		for(i = 0; i < 14; i++)
		{
			if(binary[i] == 0)
				binary[i] = 1;
			
			else
				binary[i] = 0;
		}
		
		/* adding 1 to the number*/
		for(i = 0; i < 14; i++)
		{
			if(binary[i] == 0)
			{
				binary[i] = 1;
				break;
			}
				
			else
			{
				binary[i] = 0;
			}
		}
	} 
}

/* This function checks if token is a command name.
The function returns the command index + 1 if it is or 0 otherwise*/
int checkIfCommand(char* token, char* reservedWords[])
{
	int i;
	
	/*checking for command words*/
	for(i = 0; i < 16; i++)
		if(!strcmp(token, reservedWords[i]))
			return i + 1;
	
	return 0;
}

/* This function checks if token is an instruction.
The function returns 1 or 2 if it is or 0 otherwise*/
int checkIfInstruction(char* token, char* reservedWords[])
{
	if(!strcmp(token, reservedWords[18]) || !strcmp(token, reservedWords[19]))
		return 1;
	
	if(!strcmp(token, reservedWords[20]) || !strcmp(token, reservedWords[21]))
		return 2;
	
	return 0;
}

/* This function checks if token is a register name.
The function returns 1 if it is or 0 otherwise*/
int checkIfRegister(char *token, char* reservedWords[])
{
	int i;
	
	/*checking for reserved words*/
	for(i = 22; i < 30; i++)	
		if(!strcmp(token, reservedWords[i]))
			return 1;
	
	return 0;
}

/* This function gets a command line and returns the number of arguments (L) needed for the instruction image*/
int findL(char* commandToken, char* restOfLineToken, char* reservedWords[])
{		
	char* token;
	char* tempRestOfLineToken = (char*)malloc(MAX_LINE_LENGTH);
	
	if(restOfLineToken != NULL)
		strcpy(tempRestOfLineToken, restOfLineToken);

	/*command with no operands*/
	if(!strcmp(commandToken, "rts") || !strcmp(commandToken, "stop"))
		return 0;
		
	/*command with one operand and not a jumping command*/
	else if(!strcmp(commandToken, "not") || !strcmp(commandToken, "clr") || !strcmp(commandToken, "inc") || !strcmp(commandToken, "dec") || !strcmp(commandToken, "prn") || !strcmp(commandToken, "red") || !strcmp(commandToken, "rts"))
	{
		free(tempRestOfLineToken);
		return 1;
	}
		
	/*jumping command */
	else if(!strcmp(commandToken, "jmp") || !strcmp(commandToken, "bne") || !strcmp(commandToken, "jsr"))
	{
		token = strtok(tempRestOfLineToken, "(");
		token = strtok(NULL, ",");
		free(tempRestOfLineToken);
		
		if(token == NULL)
			return 1;
			
		if(!checkIfRegister(token, reservedWords))
			return 3;
			
		token = strtok(NULL, ")");
		
		/* two registers as operands take one image node*/
		if(checkIfRegister(token, reservedWords))
			return 2;
		
		return 3;
	}
	
	/* command with 2 operands*/
	token = strtok(tempRestOfLineToken, ",");
	free(tempRestOfLineToken);
		
	while(!strncmp(token, " \t", 1))
		token++;
			
	while(!strncmp(token + strlen(token) - 1, " \t", 1))
		strncpy(token+strlen(token) - 1, "\0", 1);
			
	if(!checkIfRegister(token, reservedWords))
		return 2;
			
	token = strtok(NULL, "\n");
		
	while(!strncmp(token, " \t", 1))
		token++;
			
	while(!strncmp(token + strlen(token) - 1, " \t", 1))
		strncpy(token + strlen(token) - 1, "\0", 1);
		
	/* two registers as operands take one image node*/	
	if(checkIfRegister(token, reservedWords))
		return 1;
			
	return 2;	
}
