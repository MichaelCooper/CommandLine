/*
James Erickson & Michael Cooper
DLM: 10/2/2014
CS 433 Program 2
This program is to simulate a simple unix command line with history 
 functionality. It takes user input, stores if for the history, and 
 then executes the given commands using fork() and execvp ()
This program assumes a linux or UNIX environment and a g++ compiler.

Exit commands are still a known bug. each child must be killed for the
 program to exit.
*/ 
#include <sys/types.h>
#include <sys/wait.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h> 

#define MAX_LINE 80

int main(void)
{
  char *args[MAX_LINE/2 + 1];
  bool should_run = true;
  char blank[MAX_LINE] = { 0 }; //initially blank array buffer to be pointed to by args 

  char history[10][MAX_LINE/2 + 1] = {'\0'};

  pid_t pid;

  bool backg = false; //everytime we read in a command, start by assuming it's not in the background
  int i = 0;
  int j = 0;
  int k = 0;
  int nextSlot = 0;
enter:	//loop control overrides <-- entry point
  while(should_run)
    {
      i = 0;
      j = 0;
      k = 0;
      backg = false; // run process in background

      printf("\nosh> ");
      fflush(stdout);

      fgets(blank,MAX_LINE,stdin); // pull in the raw command line
      
execute:
    do
	{ 
		if (i == 0) 
		{
			args[i] = strtok(blank," \n"); // on the first entry, different call to strtok
			if (args[i] != NULL) 
			{
				if (strncmp(args[i], "exit",4) == 0)
				{
					should_run = false; //on exit, turn off loop condition
					goto enter; //recheck the loop for exit
				}
			}
			
		}
		else if (i == MAX_LINE/2 - 1)//if we're on the second to last possible entry
	    {
	      args[i] = strtok (NULL," \n");
	      args[i+1] = 0; // be sure to end with null character
	    }
	    
	  else args[i] = strtok (NULL," \n"); 

	  if (args[i] != NULL) //prevent segmentation fault on strncmp
	    { 
			if (strncmp(args[i], "&",1) == 0) //if we're trying to start a background process
			{
				backg = true;
				args[i] = '\0'; // be sure to end with null character; can't run a command with this sign      
			}
	    }
		i++;
	} while (args[i-1] != NULL && (i) < MAX_LINE/2 ); //if the last entry is NULL or we're one from the end, leave the final NULL

    if (args[0] != NULL) 
	{
		if ((strncmp(args[0], "exit",4) != 0) || 
			(strncmp(args[0], "history",7) != 0 ) || 
			(strrchr(args[0], '!') != NULL)) //on exit or history, don't create processes      
			{
			    //add args to history.
				strcpy(&history[nextSlot][0], args[j]);			
				nextSlot = (nextSlot +1) % 10;
				
				pid = fork();

				if (pid < 0) //error
				{
					fprintf(stderr, "Fork Failed");
					return 1;
				}

				else if (pid == 0) //child process
				{
					execvp(args[0], args);
				}
 
				else //parent process
				{
					if (!backg)    
					{
						wait(NULL);
						printf("Child Complete \n");
						fflush(stdout);
						if(!should_run)
							_exit(0);
					}
				}
			}
			//if history listing selected, show a count of the history
			//	and then the current commands stored in them.
			if (strncmp(args[0], "history", 7) == 0) 
		    {
				int count = 0;
				j = 0;
				for(i = 0; i < 10; i++)
				{
					printf("\n%d ", count);
					while(history[i][j] != '\n' && history[i][j] != '\0')
					{
						printf("%c", history[i][j]);
						j++;
					}
					printf("\n");
					fflush(stdout);
					j = 0;
					count = (count +1) % 10;				
				}
		    }
			//if a bang is found, find it's coefficient, and acquire the 
			//correct historical command
			else if(strncmp(args[0], "!", 1) == 0){
				char blank[MAX_LINE/2+1];
				strcpy(&blank[0], args[0]);
				strcpy(args[0], history[(nextSlot-atoi(&blank[1])%10)]);
				if(args[0] == NULL)
				{
					printf("\nNo Such Process Exists");
					goto enter;
				}
				goto execute;
			}				    
		}
	}
exit:
	return 0;
}