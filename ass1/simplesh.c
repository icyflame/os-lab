#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <unistd.h>

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

		if(strcmp(command, "exit") == 0) {
			return 0;
		} else if(strcmp(command, "clear") == 0) {
			printf("\033[2J\033[1;1H");
		} else if(strcmp(command, "env") == 0) {
			int i;
			for(i=0; envp[i]!=0;++i){
				printf("%s\n", envp[i]);
			}
		} else if(strcmp(command, "history") == 0) {
			print_history();
		} else if(strcmp(command, "pwd") == 0) {
			printf("%s\n", cwd);
		} else {
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

			printf("Num tokens: %d\n", num_tokens);

			if(num_tokens >= 2) {
				if(strcmp(tokens[0], "cd") == 0) {
					chdir(tokens[1]);
				} else if(strcmp(tokens[0], "history") == 0) {
					int num_commands_to_display = atoi(tokens[1]);
					read_last_few_commands(num_commands_to_display);
				}
			} else {
				perror("Unrecognized command");
			}
		}
	}

	return 0;
}
