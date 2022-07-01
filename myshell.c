/********************************
This is a template for assignment on writing a custom Shell. 

Students may change the return types and arguments of the functions given in this template,
but do not change the names of these functions.

Though use of any extra functions is not recommended, students may use new functions if they need to, 
but that should not make code unnecessorily complex to read.

Students should keep names of declared variable (and any new functions) self explanatory,
and add proper comments for every logical step.

Students need to be careful while forking a new process (no unnecessory process creations) 
or while inserting the single handler code (should be added at the correct places).

Finally, keep your filename as myshell.c, do not change this name (not even myshell.cpp, 
as you not need to use any features for this assignment that are supported by C++ but not by C).
*******************************/

#include <stdio.h>
#include <string.h>
#include <stdlib.h>			// exit()
#include <unistd.h>			// fork(), getpid(), exec()
#include <sys/wait.h>		// wait()
#include <signal.h>			// signal()
#include <fcntl.h>			// close(), open()

#define MAXSIZE 100			// max number of commands to be supported

void sighandler(int sig) 
{
    signal(SIGTSTP, sighandler); 
}

void parseInput(char *string,char **parsed,char *delimiter)
{
	// This function will parse the input string into multiple commands or a single command with arguments depending on the delimiter (&&, ##, >, or spaces).
	
	int i;
	char* inputCopy=strdup(string); 
	for (i = 0; i < MAXSIZE; i++) {
		parsed[i] = strsep(&inputCopy, delimiter);

		if (parsed[i] == NULL)
			break;
		if (strlen(parsed[i]) == 0)
			i--;
	}
	free(inputCopy);
	
}

void executeCommand(char **parsed)
{
	// This function will fork a new process to execute a command
	if(strcmp(parsed[0],"cd")==0)
	{
		chdir(parsed[1]);
	}
	else 
	{
		int rc = fork();
		if (rc < 0){			// fork failed; exit
		exit(0);
		}
		else if (rc == 0) {		// child (new) process 
			signal(SIGINT, SIG_DFL);			// Restore the default behavior for SIGINT signal
			execvp(parsed[0], parsed);
			printf("Shell: Incorrect command\n");		// if execvp fails
			exit(0);					//for proper termination of child process
		} 
		else {              
			int rc_wait = wait(NULL);
		}
	}

}

void executeParallelCommands(char **parsedParallel)
{
	// This function will run multiple commands in parallel
	int i=0;
	while(parsedParallel[i]!=NULL)
	{
		char *parsed[MAXSIZE];
		parseInput(parsedParallel[i],parsed," ");
		

		if(strcmp(parsed[0],"cd")==0)
		{
			chdir(parsed[1]);
		}
		else 
		{
			int rc = fork();
			if (rc < 0){			// fork failed; exit
			exit(0);
			}
			else if (rc == 0) {		// child (new) process 
				signal(SIGINT, SIG_DFL);			// Restore the default behavior for SIGINT signal
				execvp(parsed[0], parsed);
				printf("Shell: Incorrect command\n");		// if execvp fails
				exit(0);					//for proper termination of child process
			} 
		}
		i++;
	}
	int check_status,wait_pid;
	while ((wait_pid = wait(&check_status)) > 0);
}

void executeSequentialCommands(char **parsedSequential)
{	
	// This function will run multiple commands in parallel
	int i=0;
	while(parsedSequential[i]!=NULL)
	{
		char *parsed[MAXSIZE];
		parseInput(parsedSequential[i],parsed," ");
		
		
		executeCommand(parsed);
		i++;
	}
	
}

void executeCommandRedirection(char **parsedRedirection)
{
	// This function will run a single command with output redirected to an output file specificed by user
	
	char *parsed1[MAXSIZE],*parsed2[MAXSIZE];
	parseInput(parsedRedirection[0],parsed1," ");
	parseInput(parsedRedirection[1],parsed2," ");
	int rc = fork();
	
	if (rc < 0){			// fork failed; exit
		exit(0);
	}
	else if (rc == 0) {		// child (new) process

		//Redirecting STDOUT
		
		signal(SIGINT, SIG_DFL);			// Restore the default behavior for SIGINT signal
		
		close(STDOUT_FILENO);				//closing standard ouptu file
		open(parsed2[0], O_CREAT | O_WRONLY | O_APPEND);	//opening a file in which the output should be redirected
		
		
		execvp(parsed1[0], parsed1);
		printf("Shell: Incorrect command\n");	// if execvp fails
		exit(0);				//for proper termination of child process

	} 
	else {              // parent process
		int rc_wait = wait(NULL);
		
	}
	
}

int main()
{
	// Initial declarations
	signal(SIGINT, SIG_IGN);	// Ignore SIGINT signal
	signal(SIGTSTP, sighandler);

	char *parsedArguments[MAXSIZE],*parsedSequential[MAXSIZE],*parsedParallel[MAXSIZE],*parsedRedirection[MAXSIZE];
	
	while(1)	// This loop will keep your shell running until user exits.
	{
		// Print the prompt in format - currentWorkingDirectory$
		
		char currentWorkingDirectory[1024];
		getcwd(currentWorkingDirectory, sizeof(currentWorkingDirectory));
		printf("%s$",currentWorkingDirectory);
		
		// accept input with 'getline()'
		
		size_t size = 1024;

		char *string;

		string = (char *) malloc (size);
		
		getline (&string, &size, stdin);

        	int n=strlen(string);
        	string[n-1]='\0';
        	
		// Parse input with 'strsep()' for different symbols (&&, ##, >) and for spaces.
 		
 		parseInput(string,parsedArguments," "); 
 		parseInput(string,parsedParallel,"&&"); 
 		parseInput(string,parsedSequential,"##"); 
 		parseInput(string,parsedRedirection,">"); 		
		
		if(parsedArguments[0]==NULL)continue;
		
		
		if(strcmp(parsedArguments[0],"exit")==0)	// When user uses exit command.
		{
			printf("Exiting shell...\n");
			break;
		}


		if(parsedParallel[1]!=NULL)
			executeParallelCommands(parsedParallel);		// This function is invoked when user wants to run multiple commands in parallel (commands separated by &&)
		else if(parsedSequential[1]!=NULL)
			executeSequentialCommands(parsedSequential);	// This function is invoked when user wants to run multiple commands sequentially (commands separated by ##)
		else if(parsedRedirection[1]!=NULL)
			executeCommandRedirection(parsedRedirection);	// This function is invoked when user wants redirect output of a single command to and output file specificed by user
		else
			executeCommand(parsedArguments);		// This function is invoked when user wants to run a single commands
				
	}
	
	return 0;
}
