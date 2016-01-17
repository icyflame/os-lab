/*
 * K Siddharth Kannan - 13ME30050
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>

int linear_search_in_arr(int * arr, int size, int start, int end, int target) {
	// search the Integer array arr for the element target
	// between the indexes start and end, both included.

	if(start > end || end > size-1) {
		return -1;
	}	 
	int i;
	for(i=start; i<=end; ++i){
		if(arr[i] == target){
			return i;
		}
	}
	return -1;
}

void printPid(char * details) {
	printf("This is the %s pid %d\n", details, getpid());
}

int found = 0;
void found_handler(int sig){
	printf("[%d] Entered found handler!\n", getpid());
	printf("[%d] %d %d %d\n", getpid(), sig == SIGUSR1, sig == SIGCHLD, sig);
	if(sig == SIGUSR1){
		printf("The integer was found!\n");
		found = 1;
	} 
}


int main(int argc, char ** argv) {
	if(argc < 3) {
		printf("First argument: File to search in\nSecond argument: Number to search for");
		return 1;
	}

	FILE * filin;
	int temp;
	int index = -1;
	int numbers[1000];
	int counter = 0;
	int search = atoi(argv[2]);

	int pipefd[2];
	pipe(pipefd);

	filin = fopen(argv[1], "r");

	while (1) {
		if(!feof(filin)) {
			fscanf(filin, "%d ", &temp);
			numbers[counter] = temp;
			/*printf("%d", temp);*/
			counter += 1;
		} else {
			printf("File read!\n");
			printf("%d numbers read from the file\n", counter);
			break;
		}				
	}

	fclose(filin);

	if (signal(SIGUSR1, found_handler) == SIG_ERR) {
		printf("SIGUSR1 install error\n");
		exit(1);
	}
	signal(SIGCHLD, found_handler);
	pid_t main = getpid();

	int remaining_arr;

	// create two process from the main process
	pid_t c1, c2;
	// start and end indexes
	// main process starts with the whole array
	int start = 0, end = counter - 1;
	remaining_arr = end - start + 1;

	int t;

	pid_t pid;
	int status;

	while(1) {
		c1 = fork();
		if(c1 == 0){
			// inside child process 1
			end = start + (end - start) / 2;
			remaining_arr = end - start + 1;
			printf("[%d] %d - %d\n", getpid(), start, end);
			if(remaining_arr <= 5){
				// start searching in this part of the array
				if((index = linear_search_in_arr(numbers, counter, start, end, search)) != -1){
					printf("[FOUND] Element found at %d\n", index);
					kill(main, SIGUSR1);
					/*write(pipefd[1], index, sizeof(index));*/
					/*close(pipefd[1]);*/
					kill(getpid(), SIGINT);
				} else {
					printf("This segment does not have the element, killing this process now!\n");
					kill(getpid(), SIGKILL);
				}
				break;
			}
		} else {
			c2 = fork();

			if(c2 == 0) {
				// inside child process 2
				start = start + (end - start) / 2 + 1;
				remaining_arr = end - start + 1;
				printf("[%d] %d - %d\n", getpid(), start, end);
				if(remaining_arr <= 5){
					// start searching in this part of the array
					if((index = linear_search_in_arr(numbers, counter, start, end, search)) != -1){
						printf("[FOUND] Element found at %d\n", index);
						kill(main, SIGUSR1);
						/*write(pipefd[1], index, sizeof(index));*/
						/*close(pipefd[1]);*/
						kill(getpid(), SIGINT);
					} else {
						printf("This segment does not have the element, killing this process now!\n");
						kill(getpid(), SIGINT);
					}
					break;
				}
			} else {
				printf("[%d] Main parent process, wait for children and then break!\n", getpid());
				int status1, status2;
				pid_t tpid1, tpid2;
				tpid1 = wait(&status1);
				tpid2 = wait(&status2);
				printf("[%d] Statuses: %d %d", getpid(), status1, status2);
				printf("[%d] PIDs: %d %d", getpid(), tpid1, tpid2);
				break;
			}
		}
	}

	if(main == getpid() && !found) {
		printf("[NOT FOUND] Not found!\n");
	} else {
		printf("Found at %d\n", index);
		/*int index;*/
		/*read(pipefd[0], &index, sizeof(index));*/
	}
	return 0;
}
