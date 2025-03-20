/* File:	Sum_MPI_v1.c
 * Purpose:	A parallel algorithm to calculate the summation of a function
 *
 * Compile:	mpicc -g -Wall -o Sum_MPI_v1 Sum_MPI_v1.c -lm 
 * Run:		mpiexec -n <number of processes> ./Sum_MPI_v1
 *
 * Algorithm:
 * 	1.	Each process calculates its local summation
 * 	2a. 	Each process != 0 sends its summation to process 0
 * 	2b. 	Process 0 sums the calculations and prints the results 
 */
#include <math.h>
#include <stdlib.h>
#include <mpi.h>
#include <stdio.h>

/* Calculate the summation term */
double Summation_term(int lower_limit, int upper_limit);

/* Get user input */
void Get_input(int my_rank, int comm_sz, int* lower_limit, 
	int* upper_limit);

int main(void) {
	int my_rank, comm_sz, local_i, local_n, i, n;
	double local_summation, total_summation;
	int source;

	/* Initialize MPI */
	MPI_Init(NULL, NULL);

	/* Get my process rank */
	MPI_Comm_rank(MPI_COMM_WORLD, &my_rank);

	/* Find out the amount of processes being used */
	MPI_Comm_size(MPI_COMM_WORLD, &comm_sz);

	/* Get input */
	Get_input(my_rank, comm_sz, &i, &n);

	/* Divide work among processes */
	local_n = n / comm_sz;
	local_i = (local_n * my_rank);

	/* Edge case for when n < p */
	if(my_rank == comm_sz - 1) {
		local_n += n % comm_sz;
	}

	/* Perform the function locally */
	local_summation = Summation_term(local_i, local_n + local_i);

	/* Add up the summation term calculated by each process*/
	if (my_rank != 0) {
		MPI_Send(&local_summation, 1, MPI_DOUBLE, 0, 0, MPI_COMM_WORLD);
	} else {
		total_summation = local_summation;
		for (source = 1; source < comm_sz; source++) {
			MPI_Recv(&local_summation, 1, MPI_DOUBLE, source, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
			total_summation += local_summation;
		}
	}

	if (my_rank == 0) {
		total_summation = total_summation * 4;	
		printf("%f\n", total_summation);
	}

	MPI_Finalize();
	return 0;
}

/*------------------------------------------------------------------
 * Function: 	Summation_term
 * Purpose: 	Calculate the summation term, which can be written
 * 				as [(-1)^i] / [2i+1]
 * Input args:	i
 */
double Summation_term(int lower_limit, int upper_limit) {
	int i = 0;
	double result = 0;

	for (i = lower_limit; i < upper_limit; i++) {
		result += pow(-1.0,(double)i) / ((2.0*i)+1);
	}

	return result;
}


/*------------------------------------------------------------------
 * Function:	Get_input
 * Purpose:		Get the user input, the lower and upper limits of
 * 				the summation
 * Input args:	my_rank:	process rank in MPI_COMM_WORLD
 * 				comm_sz: 	number of porcesses in MPI_COMM_WORLD
 * Output args:	lower_limit: 	pointer to i
 * 				upper_limit: 	pointer to n
 */
void Get_input(
	int my_rank,		/* in */ 
	int comm_sz,		/* in */ 
	int* lower_limit,	/* out */ 
	int* upper_limit	/* out */) {

MPI_Bcast(lower_limit, 1, MPI_INT, 0, MPI_COMM_WORLD);
MPI_Bcast(upper_limit, 1, MPI_INT, 0, MPI_COMM_WORLD);
	/* in this case, i is assumed to be 0 */
	*lower_limit = 0;

	int dest;

	if (my_rank == 0) { 
		printf("Enter n: ");
		fflush(stdout);	
		scanf("%d", upper_limit);
		for (dest = 1; dest < comm_sz; dest++) {
			MPI_Send(lower_limit, 1, MPI_INT, dest, 0,
					MPI_COMM_WORLD);
			MPI_Send(upper_limit, 1, MPI_INT, dest, 0,
					MPI_COMM_WORLD);
		}
	} else { /* my_rank != = */
		MPI_Recv(lower_limit, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,
				MPI_STATUS_IGNORE);
		MPI_Recv(upper_limit, 1, MPI_INT, 0, 0, MPI_COMM_WORLD,
				MPI_STATUS_IGNORE);
	}
}	/* Get_input */
