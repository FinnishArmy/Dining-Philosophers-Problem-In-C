/***********************************************************************
name: Ronny Z Valtonen
	Dining Philosophers Problem
description:
	See CS 360 Lecture for details.
***********************************************************************/

// Includes
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <signal.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/sem.h>
#include <math.h>
#include <errno.h>

// Ignore useless warnings
_Pragma("GCC diagnostic push")
_Pragma("GCC diagnostic ignored \"-Wunused-but-set-variable\"")
_Pragma("GCC diagnostic ignored \"-Wimplicit-function-declaration\"")


// Defines
#define PHILOSOPHERS 5

// Globals
int eatingTime = 0;
long setUpSemaphore;
long processID;
int chopstickID;
int i = 0;
int j = 0;
int k;
int  k2;


// Structs
struct sembuf semWait[1] = {{1, -1, 0}};
struct sembuf semSignal[1] = {{1, 1, 0}};

/* Definitions */

/*********************************************************************
Parameters:
int mean = eating will be 9 and thinking will be 11 seconds.
int stddev = eating will be 3 and thinking will be 7 seconds.

General Run:
Successive calls to randomGaussian produce integer return values
having a gaussian distribution with the given mean and standard
deviation.  Return values may be negative.

// Writen by Ben Mccamish at Washington State University Washington.
********************************************************************/
int randomGaussian(int mean, int stddev);

/*********************************************************************
General Goal:
The point of this is to have multiple processes running at the same time.
To do this, we must have a parenta and child function communicate with
one another in which we also much keep checks on when one is checking
for another to finish.

More info: https://en.wikipedia.org/wiki/Dining_philosophers_problem

Parameters:
Nothing here is passed into the function through the terminal.
We have globals that define the eating time and thinking time using
the given randomGaussian function. We also define a global philospher
count number so that this will work with any number of philosphers
without changing the code from the current state, except for line
43.

General Run:
We use semop for incrementing and decrementing or to test if the
Semaphore elements for zer values. We must also error check this.
Semget creates a semaphore set and initializes each element to 0.
Semctl will preform the various operations needed to pass
the sem values to the 'philosphers'. We use a switch case which
will do different operations depening on the chopstickID of the current
process.
********************************************************************/
int main(int argc, char *argv[]);


// Start of program //
int randomGaussian(int mean, int stddev) {
	double mu = 0.5 + (double) mean;
	double sigma = fabs((double) stddev);
	double f1 = sqrt(-2.0 * log((double) rand() / (double) RAND_MAX));
	double f2 = 2.0 * 3.14159265359 * (double) rand() / (double) RAND_MAX;

	if (rand() & (1 << 5)) {
    return (int) floor(mu + sigma * cos(f2) * f1);
  }
	else {
    return (int) floor(mu + sigma * sin(f2) * f1);
  }
}
													// Main //
//** README **//
/*
To run:
>		gcc -o assignment5 assignment5.c -lm
>		./assignment5
>		ctrl+C || ctrl+Z to stop
*/


// Assignment 5, fork 5 (or more) child process, each representing
// a philospher
int main(int argc, char *argv[]) {
	// My waits and signals for some reason, just doesn't work..
	// I feel like my logic is mostly correct, and it's pretty close..
	printf("Program doesn't fully work...\n\n");
	// If the semaphore is -1 then there was an error in the creation.
  if (setUpSemaphore == -1) {
		fprintf(stderr, "%d -- %d \n", errno, strerror (errno));
		return (-errno);
  }

	// Loop through all the philosphers and print the ID of each one.
  while (i < PHILOSOPHERS) {
		// Init to 0.
		k = semctl(setUpSemaphore, i, SETVAL, 0);
		if (i < PHILOSOPHERS) {
			// Print the ID's.
			printf("Semaphore %d initial value = %d \n", i, k+1);
		}
    i++;
  }
	// Loops through each of the ID's of the philosphers.
  while (processID < PHILOSOPHERS) {
			// Fork each philospher.
  		chopstickID = fork();
  		if (chopstickID == 0) {
        break;
      }
			// Print each fork and the chopstickID of the fork.
			printf("Forked Philosopher %ld (%d)\n", processID, chopstickID);
      processID++;
  }
	// Setup the chopstickID.
  if (chopstickID < 0) {
    chopstickID = -1;
  }
	// Switch case according to the current chopstickID.
  switch(chopstickID) {
		// If the chopstickID is 0, then we check for chopsticks and think and eat.
    case 0:
				// While 1 and no errors.
				// https://mccamish.encs.vancouver.wsu.edu/classes/current/cs360/lectures/week7/ipc1.pdf
				while(1 && (errno != EINTR)){
					/* critical section */
					// https://mccamish.encs.vancouver.wsu.edu/classes/current/cs360/lectures/week8/ipc2.pdf //
					printf("Philosopher %ld is thinking for %d seconds (%d)\n", processID, randomGaussian(11, 7), eatingTime);
					// We wait according to the randomGaussian thinking time.
					sleep(randomGaussian(11, 7));
					// Init the sem.
					semop(PHILOSOPHERS, semWait, 2);
					printf("Philosopher %ld wants chopsticks\n", processID);
					processID = 0;
					// Set a flag for if the chopstick is free or not free.
					int leftChopstickFlag = 1;
					int rightChopstickFlag = 1;

					// https://pages.mtu.edu/~shene/NSF-3/e-Book/MUTEX/TM-example-left-right.html
					// This solution used pthreading or something, I used POSIX, this was
					// of no help, it was used to figure out how to get the left and right chopsticks
					// and the math for it. Scrolling down to the 'Program' section.

					// Use math to get the left chopstick of the current philospher.
					if ((processID+(PHILOSOPHERS-1))%PHILOSOPHERS != 1) leftChopstickFlag = 0;
					// Use math to get the right chpostick of the current philospher.
					if ((processID+1)%PHILOSOPHERS != 1) rightChopstickFlag = 0;

					// Check our process ID.
					// Just general info to figure out how to get both chopsticks..
					// https://stackoverflow.com/questions/29253893/mixture-of-left-handed-and-right-handed-philosophers-a-tricky-questions
					if (processID == 0) {

						// If the left chopstick is free...
						if (leftChopstickFlag == 0) {
							printf("Left chopstick is free.");
							// AND the right chopstick is free...
							if (rightChopstickFlag == 0) {
								printf("Right chopstick is free."); processID = 2;
								// Init the sem since both chopsticks are fre.
								semop(processID, semSignal, 2);
							}
						}
						// Otherwise, if both if statements don't pass, then we can't use both.
						printf("No chopsticks are free.");
					}
					// Setup the sems according to the wait and signal processes.
					semop(PHILOSOPHERS, semSignal, 2);
					semop(processID, semWait, 2);
					eatingTime += randomGaussian(9, 3);
					// Sleep according the the randomGaussian eating time.
					sleep(randomGaussian(9, 3));

					// Once the philosphers have eated for 100 seconds, we are done.
					if (eatingTime >= 100) {
						printf("All philosphers have eaten for 100 seconds!\n");
						exit(0);
					}
					break;
				}


		// If the chopstickID is -1, then we cannot create the child process and print and error.
    case -1:
        fprintf(stderr, "%d -- %d \n", errno, strerror (errno));
        return (-errno);
        break;


	  // We wait for all child processes to finish.
		// Used this post to find out how to wait for all the processes to finish.
		// https://stackoverflow.com/questions/19461744/how-to-make-parent-wait-for-all-child-processes-to-finish
    default:
        waitpid(-1, NULL, 0);

  }
	// Return
  return 0;
}

// -- END OF PROGRAM -- //
