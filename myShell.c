#include<stdio.h>
#include<string.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/wait.h>
#include<sys/types.h>
#include<readline/readline.h>
#include<readline/history.h>
#include<fcntl.h>


void GetPrompt();


void SplitInput(char input[1000], char args[100][100], int *argSize)
{
	char buffer[100];
	int bufferLen = 0;

	*argSize = 0;
	int len = strlen(input);
	for(int i = 0; i < len; i++)
	{
		if(input[i] == ' ')
		{
			if(bufferLen == 0)
			{
				continue;
			}
			else
			{
				buffer[bufferLen] = '\0';
				strcpy(args[*argSize],buffer);
				(*argSize)++;
				bufferLen = 0;
			}
		}
		else
		{
			buffer[bufferLen] = input[i];
			bufferLen++;
		}
	}
	if(bufferLen > 0)
	{
		buffer[bufferLen] = '\0';
		strcpy(args[*argSize],buffer);
		(*argSize)++;
		bufferLen = 0;
	}
}

void CommandHandler(char args[100][100], int argSize)
{
	pid_t myPid;
	int status;
	for(myPid = waitpid(-1, &status, WNOHANG); myPid != -1 && myPid != 0; myPid = waitpid(-1, &status, WNOHANG))
	{
		//kill zombie child process
	}
	

	if(strcmp(args[0],"export") == 0)
	{
		if(argSize == 2)//if this command have only two arguments 
		{
			char varName[100]; //variable name
			int varNameLen = 0; //variable name length
			int len = strlen(args[1]);
			int haveEqual = 0; //check '=' is in the string
			int pos; //start posiotn of the value
			for(int i = 0; i < len; i++)
			{
				if(args[1][i] == '=')
				{
					haveEqual = 1;
					pos = i+1;
					break;
				}
				varName[varNameLen] = args[1][i];
				varNameLen++;
			}
			varName[varNameLen] = '\0';
			if(haveEqual == 0)//no '=' in string
			{
				printf("not a valid command!\n");
			}
			else if(varNameLen > 0)
			{
				char varVal[100];
				int varValLen = 0;
				for(int i = pos; i < len; i++)
				{
					varVal[varValLen] = args[1][i];
					varValLen++;
				}
				varVal[varValLen] = '\0';
				setenv(varName,varVal,1); //set the envirment variable
				printf("set variable sucessfully!\n");
			}
			else//varNameLen is 0
			{
				printf("not a valid command!\n");
			}
			
		}
		else
		{
			printf("not a valid command!\n");
		}
	}
	else if(strcmp(args[0],"pwd") == 0)
	{
		if(argSize == 1)
		{
			char buffer[100];
			printf("%s\n",getcwd(buffer,sizeof(buffer)));
		}
		else
		{
			printf("not a valid command!\n");
		}
	}
	else if(strcmp(args[0],"cd") == 0)
	{
		if(argSize == 1)
		{
			//do nothing
		}
		else if(argSize == 2)
		{
			chdir(args[1]);
		}
		else
		{
			printf("cd: too many arguments!\n");
		}
	}
	else if(strcmp(args[0],"echo") == 0)
	{
		for(int i = 1; i < argSize; i++)
		{
			if(args[i][0] == '$')
			{
				char envName[100];
				int envNameLen = 0;
				int len = strlen(args[i]);
				for(int j = 1; j < len; j++)
				{
					envName[envNameLen] = args[i][j];
					envNameLen++;
				}
				envName[envNameLen] = '\0';
				printf("%s ",getenv(envName));
			}
			else
			{
				printf("%s ",args[i]);
			}
			
		}
		printf("\n");
	}
	else if(strcmp(args[0],"exit") == 0)
	{
		if(argSize == 1)
		{
			exit(0);
		}
		else
		{
			printf("not a valid command!\n");
		}
	}
	else//other command
	{
		int outputRedirection = 0;
		int redirectPosition;
		for(int i = 0; i < argSize; i++)//to check if > is in arguments
		{
			if(strcmp(args[i],">") == 0)
			{
				outputRedirection = 1;
				redirectPosition = i;
				break;
			}
		}


		if(strcmp("&",args[argSize-1]) == 0)
		{
			pid_t pid = fork();
			if(pid == 0)//child process
			{
				if(outputRedirection == 1)
				{
					if(redirectPosition == 0)// no command before >
					{
						printf("no command before '>'!\n");
						exit(0);
					}
					else if(argSize != redirectPosition+3)//too many file after >
					{
						printf("too many file after '>'!\n");
						exit(0);
					}
					else
					{
						char *myCommand[redirectPosition+1];
						myCommand[redirectPosition] = NULL;
						for(int i = 0; i < redirectPosition; i++)
						{
							myCommand[i] = malloc(strlen(args[i])+1);
							strcpy(myCommand[i],args[i]);
						}
						char *fName = args[redirectPosition+1];
						int fd; 
						if((fd = creat(fName,0644)) == -1)
						{
							printf("error!\n");
						}
						dup2(fd,STDOUT_FILENO);
						close(fd);
						if(execvp(myCommand[0],myCommand) < 0)
						{
							printf("not a valid command!\n");
						}
						exit(0);
					}
				}
				else
				{
					if(argSize >= 2)
					{
						char *myCommand[argSize];
						myCommand[argSize-1] = NULL;
						for(int i = 0; i < argSize-1; i++)
						{
							myCommand[i] = malloc(strlen(args[i])+1);
							strcpy(myCommand[i],args[i]);
						}

						if(execvp(myCommand[0],myCommand) < 0)
						{
							printf("not a valid command!\n");
						}
						exit(0);
					}
					else//only have "&" in command
					{
						printf("not a valid command!\n");
						exit(0);
					}
				}
			} 
			//parent don't need to wait
		}
		else
		{
			pid_t pid = fork();
			if(pid == 0)//child process
			{
				if(outputRedirection == 1)
				{
					if(redirectPosition == 0)// no command before >
					{
						printf("no command before '>'!\n");
						exit(0);
					}
					else if(argSize != redirectPosition+2)//too many file after >
					{
						printf("too many file after '>'!\n");
						exit(0);
					}
					else
					{
						char *myCommand[redirectPosition+1];
						myCommand[redirectPosition] = NULL;
						for(int i = 0; i < redirectPosition; i++)
						{
							myCommand[i] = malloc(strlen(args[i])+1);
							strcpy(myCommand[i],args[i]);
						}

						char *fName = args[redirectPosition+1];
						int fd; 
						if((fd = creat(fName,0644)) == -1)
						{
							printf("error!\n");
						}
						dup2(fd,STDOUT_FILENO);
						close(fd);
						
						if(execvp(myCommand[0],myCommand) < 0)
						{
							printf("not a valid command!\n");
						}
						
						exit(0);
					}
					
				}
				else
				{
					char *myCommand[argSize+1];
					myCommand[argSize] = NULL;
					for(int i = 0; i < argSize; i++)
					{
						myCommand[i] = malloc(strlen(args[i])+1);
						strcpy(myCommand[i],args[i]);
					}
					
					if(execvp(myCommand[0],myCommand) < 0)
					{
						printf("not a valid command!\n");
					}
					
					exit(0);
				}
			} 
			else//parent process
			{
				wait(NULL);
			}
		}
	}
	
}

int main()
{
	char input[1000]; //input max length is 1000
	char args[100][100]; //max arguments is 100,each arguments can have length 100
	int argSize;
	printf("\033[H\033[J"); //clear the screen
	while(1)
	{
		char prefixPrompt[50] = "\033[1;34m";
		GetPrompt(prefixPrompt); //get current directory path
		strcat(prefixPrompt,"\033[0m$ ");
		char *input = readline(prefixPrompt); //read command
		if(strlen(input) > 0)//if input is not empty
		{
			SplitInput(input,args,&argSize); //split the input string
			if(argSize > 0)
			{
				add_history(input);
				CommandHandler(args,argSize);
			}
		}
	}
}



void GetPrompt(char *prefixPrompt)//get current directory path
{
	char buffer[1000];
	getcwd(buffer,sizeof(buffer));
	strcat(prefixPrompt,buffer);
}
