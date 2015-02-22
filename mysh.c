#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include "mysh.h"
#define _GNU_SOURCE

// ssize_t getline(char **lineptr, size_t *n, FILE *stream);
// char *strtok(char *str, const char *delim)

//   printf("here \n");

int main (void)
{
	char* command;
	size_t comLength = (1024*sizeof(char));
	ssize_t readNum;
	char* args[1024];
	char* delim = " \n";
	bool exit = false;
	
	// error messaging
	//const char * const sys_errlist[10];
     //int sys_nerr;
     int errno;
     //char* errorMes;
	
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
			//errorMes = "getline";
			perror("getline");
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
		
		// separate args printout
		printf("\n\n");
		
		/* Built-in Commands */
		
		// exit command
		//TODO: does not account for >1 args
		if (((strcmp("exit", args[0]) == 0)) || 
		     (strcmp("exit\n", args[0]) == 0))
		{
			exit = true;
		}
	
		// change directory
		else if ((strcmp("cd", args[0]) == 0))
		{
			if (args[1] == NULL)
			{
				char* home = getenv("HOME");
				chdir(home);
			}

			// change to directory desired
			else if(chdir(args[1]) == -1)
			{
				perror("cd");
			}
		}
		
		// print current working directory
		else if (strcmp("pwd", args[0]) == 0)
		{
			char* cwd = NULL;
			cwd = get_current_dir_name();
			printf("%s\n", cwd);
		}
		
			
		
		// if not a command above, is it a prog??
		else {
				
			// Running Programs
			// returns -1 if error
			// fork to split 
			int pid;
			pid = fork();
		
			// if pid == 0, this is the child
			if (pid == 0)
			{
				//printf("child\n");
				//char* prog = getenv(args[0]);
				//printf("%s\n", prog);
				//prog = get_current_dir_name();
				execvp(args[0], args);
				
				//should not return after exec
				printf("should not get here, ERROR");
				kill(pid, SIGKILL);
			}
			// this is the parent 
			else 
			{
				//printf("parent\n");
				// wait until child process is terminated
				if (wait(NULL) == -1)
				{
					perror("wait");
				}
			
			}
			
		} //end else
		
		
	} // end while

	free(command);
	
	// return 0 if sucessful exit
	return 0;
}




