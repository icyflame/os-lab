/*
 * K Siddharth Kannan - 13ME30050
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>
#include <math.h>
#include <sys/wait.h>
#include <sys/types.h>

#define BUFSIZE 100

int n, k, prime_counter, self_process_counter, *first_time_avail, *primearr, **pipes;
pid_t *process_id_arr;

int is_prime(int number) {
	int i;
	for(i=2; i <= sqrt(number); ++i)
	{
		if(number % i == 0)
		{
			return 0;
		}
	}
	return 1;
}

void print(char * str) {
	printf("[%d] %s\n", getpid(), str);
}

static void handle_available(int sig, siginfo_t *siginfo, void *context) {
	printf("[%d] Child %d sent available\n", getpid(), siginfo->si_pid);

	// find the process number that sent this signal
	int pcounter=-1, i;
	for(i=0; i<k; ++i) {
		if(process_id_arr[i] == siginfo->si_pid) {
			pcounter = i;
			break;
		}
	}

	if(pcounter == -1) {
		perror("this process is not a part of our program.");
	}

	// if not the first time, read this process's pipe
	// and see if any primes were generated
	
	if(!first_time_avail[pcounter]) {
		char buf[BUFSIZE];
		int numread = read(pipes[pcounter+k][0], buf, BUFSIZE);
		printf("[%d] Process pid %d sent this: %s\n", getpid(), siginfo->si_pid, buf);
		print("Enter an integer to coninute");
		int temp;
		scanf("%d", &temp);
	} else {
		first_time_avail[pcounter] = 0;
	}

	// generate k numbers and
	// write to the pcounter-th pipe

	int * numbers_to_send;
	numbers_to_send = (int *) malloc(sizeof(int) * k);

	char temp[30], *final_string;
	// each number can have a maximum size of 5 digits
	// with one space after each number
	final_string = (char *) malloc(sizeof(char) * 6 * k);

	sprintf(final_string, "%d ", rand() % 30000 + 1);

	for(i=1; i<k; ++i) {
		numbers_to_send[i] = rand() % 30000 + 1;
		sprintf(temp, "%d ", numbers_to_send[i]);
		strcat(final_string, temp);
	}

	printf("[%d] Sending numbers to process %d with pid %d\n", getpid(), pcounter, siginfo->si_pid);
	printf("[%d] String (%d bytes) to be written to pipe: %s\n", getpid(), (int)strlen(final_string), final_string);
	size_t nwrote;
	if((nwrote = write(pipes[pcounter][1], final_string, strlen(final_string))) == -1) {
		perror("write function didn't work");
	}

	if((int)nwrote < strlen(final_string)) {
		printf("[%d] Wrote %d instead of the expected %d characters\n", getpid(), (int)nwrote, (int)strlen(final_string));
	} else {
		printf("[%d] Wrote %d == expected %d characters\n", getpid(), (int)nwrote, (int)strlen(final_string));
	}

}

static void handle_busy(int sig, siginfo_t *siginfo, void *context) {
	printf("[%d] Child %d sent busy\n", getpid(), siginfo->si_pid);
}

int main(int argc, char ** argv) {

	if(argc < 3) {
		perror("You must provide N (number of prime numbers to search for) and K (number of processes).");
		return 1;
	}

	int i, j;
	pid_t parent_pid = getpid(), pidc;
	prime_counter = 0;
	self_process_counter = -1; // parent is -1, children start indexing from 0
	const char * hello_world = "Hello, world";
	char buf[BUFSIZE];

	// treat SIGUSR1 as available
	// and SIGUSR2 as busy
	// setup the callbacks here

	struct sigaction act;

	act.sa_sigaction = &handle_available;
	act.sa_flags = SA_SIGINFO;

	if (sigaction(SIGUSR1, &act, NULL) < 0) {
		perror ("sigaction");
		return 1;
	}

	struct sigaction act2;

	act2.sa_sigaction = &handle_busy;
	act2.sa_flags = SA_SIGINFO;

	if (sigaction(SIGUSR2, &act2, NULL) < 0) {
		perror ("sigaction");
		return 1;
	}

	n              = atoi(argv[1]);
	k              = atoi(argv[2]);
	primearr       = (int *) malloc(sizeof(int) * n);

	pipes          = (int **) malloc(sizeof(int *) * 2 * k);
	for(i = 0; i < 2 * k; ++i) {
		pipes[i] = (int *) malloc(sizeof(int) * 2);
	}

	process_id_arr = (pid_t *) malloc(sizeof(pid_t) * k);

	first_time_avail = (int *) malloc(sizeof(int) * k);

	for(i = 0; i < k; ++i) {

		first_time_avail[i] = 1;

		// call pipe function here
		if(pipe(pipes[i]) == -1 || pipe(pipes[i+k]) == -1) {
			perror("pipe function did not work!");
		}

		pidc = fork();

		if(pidc == 0) {
			// inside child process

			// setup and test the pipes here
	/*		
				 size_t nbyte;
				 if((nbyte = write(pipes[i][1], hello_world, strlen(hello_world))) == -1) {
				 perror("write function didn't work");
				 }

				 if(nbyte != sizeof(hello_world)) {
				 printf("[%d] Wrote %d chars\n", getpid(), (int)nbyte);
				 perror("write function only succeeded partially");
				 } else {
				 printf("[%d] wrote %d bytes\n\n", getpid(), (int)nbyte);
				 }
		*/		 

			self_process_counter = i;

			if(kill(parent_pid, SIGUSR1) == -1) {
				perror("kill could not send the signal");
			}

			break;

		} else {
			// inside parent process
			// put pidc into an array of child process IDs

			process_id_arr[i] = pidc;
		}
	}

	// all processes have now been created	

	if(getpid() != parent_pid) {
		// inside the child processes
		print("This is inside the child process");

		// get the numbers from the pipe

		while(1){
			printf("[%d] Listening on the pipe number %d for this PID\n", getpid(), self_process_counter);
			char pipe_numbers[BUFSIZE];
			int numread =  read(pipes[self_process_counter][0], pipe_numbers, BUFSIZE);
			/*pipe_numbers[numread] = '\0';*/
			kill(parent_pid, SIGUSR2);
			printf("[%d] Recieved (%d chars) from the pipe: %s\n", getpid(), (int)strlen(pipe_numbers), pipe_numbers);

			// use strtok to get all the numbers read from the pipe
			char tokens[100][20]; // TODO - change to dynamic allocation

			int * numbers;
			numbers = (int *) malloc(sizeof(int) * k);

			numbers[0] = atoi(strtok(pipe_numbers, " "));

			int counter = 1;

			while(numbers[counter-1] != 0) {
				numbers[counter++] = atoi(strtok(NULL, " "));
			}

			if(counter != k+1) {
				printf("[%d] Read %d integers instead of expected %d\n", getpid(), counter, k);
				perror("Numbers read didn't match number of expected integers");
			} else {
				printf("[%d] Read %d numbers sent from the parent\n", getpid(), counter);
			}
			for(i=0; i<counter; ++i) {
				printf("[%d] Number: %d\n", getpid(), numbers[i]);
			}

			// only take the first k integers
			//
			// send back the number of primes
			// and the primes themselves: 2 59 61
			int temp_prime_count = 0;
			char tfinal[BUFSIZE], final_string[BUFSIZE], temp[10]; // TODO - dynamic final_string
			
			for(i=0; i<k; ++i) {
				if(is_prime(numbers[i]) == 1) {
					printf("[%d] Determind %d to be a prime\n", getpid(), numbers[i]);
					sprintf(temp, "%d ", numbers[i]);
					strcat(tfinal, temp);
					temp_prime_count++;
				}
			}
			printf("[%d] Found a total of %d prime numbers\n", getpid(), temp_prime_count);
			sprintf(final_string, "%d", temp_prime_count);
			if(temp_prime_count > 0) {
				strcat(final_string, " ");
				strcat(final_string, tfinal);
			}

			write(pipes[self_process_counter+k][1], final_string, strlen(final_string));

			kill(parent_pid, SIGUSR1);
		}
	}

	if(getpid() == parent_pid) {
		for(j=0; j<k; ++j) {
			printf("[%d] Child %d : %d\n", getpid(), j+1, process_id_arr[j]);
		}
	}

	if(getpid() == parent_pid) {
		print("waiting for children to execute!");
		while(1) {
			continue;
		}
		int status;
		for(i=0; i<k; ++i) {
			if(waitpid(process_id_arr[i], &status, 0) == -1) {
				perror("waitpid encountered an error");
			}
		}
		printf("[%d] Closing all the pipes\n", getpid());
		for(i=0; i<k; ++i) {
			close(pipes[i][0]);
			close(pipes[i][0]);
		}
	}

	print("This process is exiting!");

	return 0;
}
