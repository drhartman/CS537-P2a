#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <sys/stat.h>
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
	char* args2[1024];
	char* args3[1024];
	char* delim = " \n";
	//bool exitWhile = false;
	bool overwriteRed;
	bool appendRed;
	bool pipeBool;
	bool pipeBool2;
	bool valid;
	int indexRed = -1;
	int indexAppend = -1;
	int indexPipe = -1;
	int indexPipe2 = -1;
	int args2Index = -1;
	
	int pipefd[2];
	int pipefd1[2];
	int pipefd2[2];
	
	// error messaging
	//const char * const sys_errlist[10];
     //int sys_nerr;
     int errno;
     //char* errorMes;
	
	command = (char*) malloc(1024*sizeof(char));

	// loop continually as input comes in
	while(1)
	{
		// prompt user for input
		printf("mysh> ");
		
		// reset
		overwriteRed = false;
		appendRed = false;
		pipeBool = false;
		pipeBool2 = false;
		valid = true;
	
		// read input to string, readNum = num chars up to \n, -1 if error
		readNum = getline(&command, &comLength, stdin);
		
		if (readNum == -1)
		{
			//errorMes = "getline";
			perror("getline");
			fprintf(stderr, "Error!\n");
			
		}
		
		// validate input
		int i;
		for (i = 0; i < readNum; i++)
		{
			// c is the char to be validated
			char c = command[i];
			
			
			if (c - ' ' < 0 || c -'~' > 0)
			{
				valid = false;
			}
		
		}
		
		// no input or valid
		if (readNum > 1 || valid)
		{
			// parse commands and add all arguments to an arary
			// get first token
			args[0] = strtok(command, delim);
			
			int argsNum = 0;
			while (args[argsNum] != NULL)
			{
				// check for overwrite redirection
				if (strcmp(">", args[argsNum]) == 0)
				{
					overwriteRed = true;
					indexRed = argsNum +1;
				}
			
				// check for append redirection
				if (strcmp(">>", args[argsNum]) == 0)
				{
					appendRed = true;
					indexAppend = argsNum +1;
				}
			
				// check for pipes
				if (strcmp("|", args[argsNum]) == 0)
				{
					if (pipeBool == true)
					{
						pipeBool2 = true;
						indexPipe2 = argsNum +1;
					}
					else
					{
					pipeBool = true;
					indexPipe = argsNum +1;
					}
				}
			
				//TODO: delete this
				//printf("arg %d: %s\n", argsNum, args[argsNum]);
			
				argsNum++;
				args[argsNum] = strtok(NULL, delim);
			}
		
			// if no tokens, break 
			if (args[0] != NULL)
			{
				
				// separate args printout
				//TODO: delete this
				//printf("\n\n");
		
				/* Built-in Commands */
				// exit command
				//TODO: does not account for >1 args
				if (((strcmp("exit", args[0]) == 0)) || 
					(strcmp("exit\n", args[0]) == 0))
				{
					if (argsNum > 1)
					{
						//TODO: uhhh don't print this?
						perror("exit");
						fprintf(stderr, "Error!\n");
					}
					else
					{
						exit(0);
						//exit = true;
					}
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
						fprintf(stderr, "Error!\n");
					}
				}
		
				// print current working directory
				else if (strcmp("pwd", args[0]) == 0)
				{
					char* cwd = NULL;
					cwd = get_current_dir_name();
					printf("%s\n", cwd);



				// if not a command above, is it a prog??
				} else if (argsNum > 0)  // || (strcmp("ls", args[0]) == 0))
					{
					int pid;

					// for pipes, create each proc from mysh
					if (pipeBool)
					{
						if (pipe(pipefd) == -1)
						{
							perror("pipe");
							fprintf(stderr, "Error!\n");
						}
				
						//fork for first prog
						pid = fork();
				
						// this is the 1st child (pipe)
						if (pid == 0)
						{
							// close read pipe?
							close(pipefd[0]);
							// set output to write to pipe
							if (dup2(pipefd[1], 1) < 0)
							{
								perror("pipe dup");
								fprintf(stderr, "Error!\n");
							}
					
							// remove '|' from args
							//args[indexPipe] = NULL;
					
					
							// remove all args after and including '|'
							while (args[indexPipe] != NULL)
							{
								args[indexPipe] = NULL;
								indexPipe++;
							}
					
							// execute prog, should not return
							execvp(args[0], args);
				
							//should not return after exec
							printf("should not get here, ERROR");
							kill(pid, SIGKILL);
					
					
						}
						else
						// this is the parent (pipe)
						{
							// wait until child process is terminated
							if (wait(NULL) == -1)
							{
								perror("wait");
								fprintf(stderr, "Error!\n");
							}
					
							//fork for 2nd prog
							pid = fork();
					
							// this is the 2nd child
							if (pid == 0)
							{
								// close write pipe?
								close(pipefd[1]);
								// set output to write to pipe
								if (dup2(pipefd[0], 0) < 0)
								{
									perror("pipe dup");
									fprintf(stderr, "Error!\n");
								}
					
								int args2Index = 0;
								indexPipe++;
								// remove all args after and including '|'
								while (args[indexPipe] != NULL)
								{
									args2[args2Index] = args[indexPipe];
						
									indexPipe++;
									args2Index++;
								}
					
								// execute prog, should not return
								execvp(args2[0], args2);
				
								//should not return after exec
								printf("should not get here, ERROR");
								kill(pid, SIGKILL);
							}
							else 
							{
								// parent must close both ends of pipe
								close(pipefd[0]);
								close(pipefd[1]);
								
								//printf("parent\n");
								// wait until child process is terminated
								if (wait(NULL) == -1)
								{
									perror("wait");
									fprintf(stderr, "Error!\n");
								}
							}	
						} // end parent pipe
					} // end pipeBool
					else if (pipeBool2)
					{
					
					


if (pipe(pipefd1) == -1)
{
	perror("pipe1");
	fprintf(stderr, "Error!\n");
}

if (pipe(pipefd2) == -1)
{
	perror("pipe2");
	fprintf(stderr, "Error!\n");
}

//fork for first prog
pid = fork();

// this is the 1st child (pipe)
if (pid == 0)
{
	// close read pipe1?
	close(pipefd1[0]);
	// set output to write to pipe1
	if (dup2(pipefd1[1], 1) < 0)
	{
		perror("pipe dup 1");
		fprintf(stderr, "Error!\n");
	}

	// remove all args after and including '|'
	while (args[indexPipe] != NULL)
	{
		args[indexPipe] = NULL;
		indexPipe++;
	}

	// execute prog, should not return
	execvp(args[0], args);

	//should not return after exec
	printf("should not get here, ERROR");
	kill(pid, SIGKILL);
}
else
// this is the parent (pipe)
{
	// wait until child process is terminated
	if (wait(NULL) == -1)
	{
		perror("wait");
		fprintf(stderr, "Error!\n");
	}

	//fork for 2nd prog
	pid = fork();

	// this is the 2nd child
	if (pid == 0)
	{
		// set read to read to pipe1
		if (dup2(pipefd1[0], 0) < 0)
		{
			perror("pipe dup");
			fprintf(stderr, "Error!\n");
		}
	
			
		// close write pipe1?
		//close(pipefd1[1]);
		// set output to write to pipe2
		if (dup2(pipefd2[1], 1) < 0)
		{
			perror("pipe dup");
			fprintf(stderr, "Error!\n");
		}
		
		args2Index = 0;
		//indexPipe++;
		// remove all args after and including '|'
		while (strcmp("|", args[indexPipe]) != 0)
		//while (*args[indexPipe] != '|')
		{
			args2[args2Index] = args[indexPipe];

			indexPipe++;
			args2Index++;
		}

		// execute prog, should not return
		execvp(args2[0], args2);

		//should not return after exec
		printf("should not get here, ERROR");
		kill(pid, SIGKILL);
	}
	// this is the parent again
	else 
	{
	// wait until child process is terminated
		if (wait(NULL) == -1)
		{
		perror("wait");
		fprintf(stderr, "Error!\n");
		}

		//fork for 3rd prog
		pid = fork();

		// this is the 3rd child
		if (pid == 0)
		{
		// close write pipe?
		close(pipefd2[1]);
		// set output to read from pipe
		if (dup2(pipefd2[0], 0) < 0)
		{
			perror("pipe dup");
			fprintf(stderr, "Error!\n");
		}

		args2Index = 0;
		//indexPipe2++;
		// remove all args after and including '|'
		while (args[indexPipe2] != NULL)
		{
			args3[args2Index] = args[indexPipe2];

			indexPipe2++;
			args2Index++;
		}
		
		
		
		// execute prog, should not return
		execvp(args3[0], args3);

		//should not return after exec
		printf("should not get here, ERROR");
		kill(pid, SIGKILL);


	}
	// this is the final parent
	else
	{
		// parent must close both ends of pipe
		close(pipefd1[0]);
		close(pipefd1[1]);
		close(pipefd2[0]);
		close(pipefd2[1]);
		
		//printf("parent\n");
		// wait until child process is terminated
		if (wait(NULL) == -1)
		{
			perror("wait");
			fprintf(stderr, "Error!\n");
		}
	}	
} // end parent pipe
					
	}				
					
					
					
					
				
					
					
					
					
					
					
					
					
					
					
					
					
					
					
					
					
					
					
					
					
					
					
					
					} // end pipeBool2
					// not pipe
					else
					{	
						// Running Programs
						// returns -1 if error
						// fork to split 
						pid = fork();
			
			
						// if pid == 0, this is the child
						if (pid == 0)
						{
							// overwrite redirection
							if (overwriteRed)
							{
								//printf("overwriteRed\n");
					
								//printf("%s\n", args[indexRed]);
					
								int ovrRedFile = open(args[indexRed], 
								O_TRUNC|O_CREAT|O_RDWR, S_IRWXU);
					
								//printf("%d\n", ovrRedFile);
					
								//int dup = 
								if (dup2(ovrRedFile, 1) < 0)
								{
									perror("dup ovr");
									fprintf(stderr, "Error!\n");
								}
								//close(1);
					
								//printf("does this work?");
					
								// remove '>' and 'filename' from args
								args[indexRed - 1] = NULL;
								args[indexRed] = NULL;
							}
				
							// append redirection
							if (appendRed)
							{
								//printf("appendRed\n");
				
								//printf("%s\n", args[indexAppend]);
					
								int ovrAppFile = open(args[indexAppend], 
								O_APPEND|O_CREAT|O_RDWR, S_IRWXU);
					
								//printf("%d\n", ovrAppFile);
					
								//int dup = 
								if (dup2(ovrAppFile, 1) < 0)
								{
									perror("dup app");
									fprintf(stderr, "Error!\n");
								}
								//close(1);
					
								//printf("does this work?");
					
								// remove '>>' and 'filename' from args
								args[indexAppend - 1] = NULL;
								args[indexAppend] = NULL;
							}
				
				
				
							// execute prog, should not return
							if (execvp(args[0], args) == -1)
							{
								perror("exec");
								fprintf(stderr, "Error!\n");
								//should not return after exec
								//printf("should not get here, ERROR");
								kill(getpid(), SIGKILL);
							}
				
						
						}
						// this is the parent 
						else 
						{
							//printf("parent\n");
							// wait until child process is terminated
							if (wait(NULL) == -1)
							{
								perror("wait");
								fprintf(stderr, "Error!\n");
							}
							
						}
			
					} // end not pipe
			
				} //end not exit else / is prog
				else
				// if not a prog, then ERROR
				{
				//TODO: uhhh don't print this?
				perror("exit");
				fprintf(stderr, "Error!\n");

				}
		
			} // end if no tokens
		
		} // end if readNum != 1
		
	} // end while

	free(command);
	
	// return 0 if sucessful exit
	return 0;
}




