#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>

#define HISTSIZE 2
#define HISTFILESIZE 10000

extern char ** environ;
char HISTFILENAME[256];

#define TOKEN_SIZE 100
#define COMMAND_SIZE 512

char * command_history[HISTSIZE];

char cwd[256], command[COMMAND_SIZE];

void getcw() {
	getcwd(cwd, sizeof(cwd));
}

void write_cmd_to_file(char command[]) {
	FILE * filout;
	filout = fopen(HISTFILENAME, "a+");
	fputs(command, filout);
	fputs("\n", filout);
	fclose(filout);
}

void print_history() {
	FILE * filin;
	filin = fopen(HISTFILENAME, "r");
	int i;
	char * hist_command;
	hist_command = (char *) malloc(COMMAND_SIZE * sizeof(char));
	i = 0;
	while(!feof(filin)){
		fgets(hist_command, COMMAND_SIZE, filin);
		printf("%4d %s", i, hist_command);
		++i;
	}
	fclose(filin);
}

void read_last_few_commands(int num_commands_to_read) {
	char ** command_history;
}

void execute(char ** tokens, int numTokens) {
	int i, run_bg = 0;
	char * argv_val;
	if(tokens[numTokens-1][strlen(tokens[numTokens-1])-1] == '&') {
		printf("Running this command in the background.");
		run_bg = 1;
		tokens[numTokens-1][strlen(tokens[numTokens-1])-1] = '\0';
	}
	pid_t pid = getpid(), pidc;
	pidc = fork();
	if(pidc == -1) {
		perror("fork error");
	} else if(pidc == 0) {
		printf("Inside child!");
		if(execvp(tokens[0], tokens) == -1) {
			perror("Error with exec function!");
		}
	} else {
		int status;
		if(!run_bg) {
			printf("Inside parent, waiting");
			if(waitpid(pidc, &status, 0) == pidc){
				if (WIFEXITED(status)) {
					printf("exited, status=%d\n", WEXITSTATUS(status));
				}
			}
		}
	}
}

int main(int argc, char ** argv, char ** envp) {

	strcat(HISTFILENAME, getenv("HOME"));
	strcat(HISTFILENAME, "/histfile");

	printf("History file: %s\n", HISTFILENAME);

	char * tokens[100];
	int offset = 0;

	while(1) {

		getcw();
		printf("%s > ", cwd);
		fgets(command, COMMAND_SIZE, stdin);

		int n = strlen(command);

		if(n == 1) {
			continue;
		}

		if(command[n-1] == '\n'){
			command[n-1] = '\0';
			n = n - 1; // Found that n = strlen + 1.
		}

		write_cmd_to_file(command);

		printf("You entered %s %d\n", command, n);
		// split the string using the space character
		char temp_string[256];
		strcpy(temp_string, command);
		tokens[0] = (char *) malloc(TOKEN_SIZE * sizeof(char));
		strcpy(tokens[0], strtok(temp_string, " "));
		printf("Token 0: %s\n", tokens[0]);
		int num_tokens;
		for(num_tokens=1; ; ++num_tokens) {
			tokens[num_tokens] = (char *) malloc(TOKEN_SIZE * sizeof(char));
			tokens[num_tokens] = strtok(NULL, " ");
			if(tokens[num_tokens] == NULL)
				break;
		}

		if(strcmp(tokens[0], "exit") == 0) {
			return 0;
		} else if(strcmp(tokens[0], "clear") == 0) {
			printf("\033[2J\033[1;1H");
		} else if(strcmp(tokens[0], "env") == 0) {
			int i;
			for(i=0; envp[i]!=0;++i){
				printf("%s\n", envp[i]);
			}
		} else if(strcmp(tokens[0], "history") == 0) {
			print_history();
		} else if(strcmp(tokens[0], "pwd") == 0) {
			printf("%s\n", cwd);
		} else if(strcmp(tokens[0], "cd") == 0) {
			chdir(getenv("HOME"));
		} else if(strcmp(tokens[0], "ls") == 0) {
			DIR * pwd = opendir(".");
			if(pwd == NULL) {
				perror("mkdir encountered an error!");
			} else {
				struct dirent * dp;
				while((dp = readdir(pwd)) != NULL) {
					printf("%s\n", dp->d_name);
				}
			}
		}	else if(strcmp(tokens[0], "cd") == 0 && num_tokens >= 2) {
			chdir(tokens[1]);
		} else if(strcmp(tokens[0], "history") == 0) {
			int num_commands_to_display = atoi(tokens[1]);
			read_last_few_commands(num_commands_to_display);
		} else if(strcmp(tokens[0], "mkdir") == 0 && num_tokens >= 2) {
			if(mkdir(tokens[1], S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH) == -1) {
				perror("mkdir failed");
			}
		} else if(strcmp(tokens[0], "rmdir") == 0 && num_tokens >= 2) {
			if(rmdir(tokens[1]) == -1) {
				perror("rmdir failed");
			}
		} else {
			execute(tokens, num_tokens);
		}
	}
	return 0;
}
