#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#define _GNU_SOURCE

// ssize_t getline(char **lineptr, size_t *n, FILE *stream);
// char *strtok(char *str, const char *delim)

//   printf("here \n");

int
runcmd(char* args[], int numArgs)
{
	// exit command
	if (((strcmp("exit", args[0]) == 0)) || 
	     (strcmp("exit\n", args[0]) == 0))
	{
		// return a 1 and set exit = true
		return 1;
	}
	
	// change directory
	
		

	return 0;
}


int main (void)
{
	char* command;
	size_t comLength = (1024*sizeof(char));
	ssize_t readNum;
	char* args[1024];
	char* delim = " ";
	bool exit = false;
	
	// error messaging
	//const char * const sys_errlist[10];
     //int sys_nerr;
     int errno;
     char* errorMes;
	
	command = (char*) malloc(1024*sizeof(char));

	// loop continually as input comes in
	while(!exit)
	{
		// prompt user for input
		printf("mysh>");
	
		// read input to string, readNum = num chars up to \n, -1 if error
		readNum = getline(&command, &comLength, stdin);
		
		if (readNum == -1)
		{
			errorMes = "getline";
			perror(errorMes);
		}
		
		// parse commands and add all arguments to an arary
		// get first token
		args[0] = strtok(command, delim);
		int argsNum = 0;
		while (args[argsNum] != NULL)
		{
			printf("arg %d: %s\n", argsNum, args[argsNum]);
			argsNum++;
			args[argsNum] = strtok(NULL, delim);
		}
		
		//TODO: account for either '\n' or ' ' as last arg
		//if (args[argsNum-1] == 
		
		//run built-in command
		if (argsNum == 1)
		{
			if(runcmd(args, argsNum) == 1)
			{
				exit = true;
			}
			
		}
			
			
			
	} // end while

	free(command);
	
	// return 0 if sucessful exit
	return 0;
}




