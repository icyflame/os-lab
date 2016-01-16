/*
 * K Siddharth Kannan - 13ME30050
 */

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>

int linear_search_in_arr(int * arr, int size, int target) {
	int i;
	for(i=0; i<size; ++i){
		if(arr[i] == target){
			return i;
		}
	}
	return -1;
}

void printPid(char * details) {
	printf("This is the %s pid %d\n", details, getpid());
}

void found_handler(int sig){
	printf("The integer was found!");
}

int main(int argc, char ** argv) {
	if(argc < 3) {
		printf("First argument: File to search in\nSecond argument: Number to search for");
		return 1;
	}

	FILE * filin;
	int temp;
	int numbers[1000];
	int counter = 0;
	int search = atoi(argv[2]);

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

	signal(SIGUSR1, found_handler);
	pid_t main = getpid();

	int remaining_arr;

	// create two process from the main process
	pid_t c1, c2;
	// start and end indexes
	// main process starts with the whole array
	int start = 0, end = counter - 1;
	remaining_arr = end - start + 1;

	char in;

	while(1) {
		c1 = fork();
		if(c1 == 0){
			// inside child process 1
			end = start + (end - start) / 2;
			remaining_arr = end - start + 1;
			printf("[%d] %d - %d\n", getpid(), start, end);
			if(remaining_arr <= 5){
				// start searching in this part of the array
				// TODO
				printf("search whatever part of the array this is\n");
				break;
			}
		} else {
			/*printf("Still inside the main parent process %d\n", getpid());*/
			c2 = fork();

			if(c2 == 0) {
				// inside child process 2
				start = start + (end - start) / 2 + 1;
				remaining_arr = end - start + 1;
				printf("[%d] %d - %d\n", getpid(), start, end);
				if(remaining_arr <= 5){
					// start searching in this part of the array
					// TODO
					printf("have to search whatever this part of the array is\n");
					break;
				}
			} else {
				printf("Main parent process, break now!\n");
				break;
			}
		}
	}

	return 0;
}
