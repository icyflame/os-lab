/*
 * K Siddharth Kannan - 13ME30050
 */
#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <string.h>

#define BUFSIZE 100

int n, k, prime_counter, self_process_counter, *primearr, **pipes;
pid_t *process_id_arr;

void print(char * str) {
	printf("[%d] %s\n", getpid(), str);
}

static void handle_available(int sig, siginfo_t *siginfo, void *context) {
	printf("[%d] Child %d sent available\n", getpid(), siginfo->si_pid);
}

static void handle_busy(int sig, siginfo_t *siginfo, void *context) {
	printf("[%d] Child %d sent busy\n", getpid(), siginfo->si_pid);
}

int main(int argc, char ** argv) {

	if(argc < 3) {
		perror("You must provide N (number of prime numbers to search for) and K (number of processes).");
		return 1;
	}

	int i;
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

	pipes          = (int **) malloc(sizeof(int *) * k);
	for(i = 0; i < k; ++i) {
		pipes[i] = (int *) malloc(sizeof(int) * 2);
	}

	process_id_arr = (pid_t *) malloc(sizeof(pid_t) * k);

	for(i = 0; i < k; ++i) {

		// call pipe function here
		if(pipe(pipes[i]) == -1) {
			perror("pipe function did not work!");
		}

		pidc = fork();

		if(pidc == 0) {
			// inside child process

			// setup the pipes here
			size_t nbyte;
			if((nbyte = write(pipes[i][1], hello_world, strlen(hello_world))) == -1) {
				perror("write function didn't work");
			}

			if(nbyte != 12) {
				printf("[%d] Wrote %d chars", getpid(), (int)nbyte);
				perror("write function only succeeded partially");
			} else {
				printf("[%d] wrote %d bytes\n", getpid(), (int)nbyte);
			}

			self_process_counter = i;

			if(kill(parent_pid, SIGUSR1) == -1) {
				perror("kill could not send the signal");
			}

			print("Hey, I was created!");
			break;

		} else {
			// inside parent process
			// put pidc into an array of child process IDs

			process_id_arr[i] = pidc;
		}
	}

	// all processes have now been created	
	// print all pids in the parent process only.

	if(getpid() == parent_pid) {
		for(i=0; i<k; ++i) {
			printf("[%d] Child %d : %d\n", getpid(), i+1, process_id_arr[i]);
		}

		// read the hello world message from all children, to ensure pipe is working
		for(i=0; i<k; ++i) {
			read(pipes[i][0], buf, BUFSIZE);
			printf("[%d] Recieved from child %d: %s\n", getpid(), i, buf);
		}

		// send k random numbers to each child - TODO
	}

	if(getpid() != parent_pid) {
		// inside the child processes
		print("This is inside the child process");
	}

	if(getpid() == parent_pid) {
		printf("[%d] Closing all the pipes\n", getpid());
		for(i=0; i<k; ++i) {
			close(pipes[i][0]);
			close(pipes[i][0]);
		}
	}

	return 0;
}
