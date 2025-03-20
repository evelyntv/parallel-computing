/* File:	Sum_MPI_v2.c
 * Purpose:	A parallel algorithm to calculate the summation of a function
 *
 * Compile:	mpicc -g -Wall -o Sum_MPI_v2 Sum_MPI_v2.c -lm 
 * Run:		mpiexec -n <number of processes> ./Sum_MPI_v2
 *
 * Algorithm:
 * 	1.		Each process calculates its local summation
 * 	2a. 	Each process != 0 sends its summation to process 0
 * 	2b. 	Process 0 sums the calculations and prints the results 
 *
 * Assume n = 1000000000
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <mpi.h>

/* Calculate the summation term */
double Summation_term(int lower_limit, int upper_limit);

/* Get user input */
void Get_input(int my_rank, int comm_sz, int* lower_limit, int* upper_limit);

int main(void) {
	int my_rank, comm_sz, local_i, local_n, i, n;
	double local_summation, total_summation, start, finish, loc_elapsed, elapsed;

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

	MPI_Barrier(MPI_COMM_WORLD);
	start = MPI_Wtime();
	/* Perform the function locally */
	local_summation = Summation_term(local_i, local_n + local_i);
	finish = MPI_Wtime();
	loc_elapsed = finish-start;
	MPI_Reduce(&local_summation, &total_summation, 1, MPI_DOUBLE, MPI_SUM, 0, 
			MPI_COMM_WORLD);
	MPI_Reduce(&loc_elapsed, &elapsed, 1, MPI_DOUBLE, MPI_MAX, 0, MPI_COMM_WORLD);
	

	if (my_rank == 0) {
		total_summation = total_summation * 4;
		printf("%f\n", total_summation);
		printf("elapsed time: %f seconds\n", finish-start);
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
 * Function: 	Build_mpi_type	
 * Purpose: 	Build a derived datatype so that the three input
 * 		values can be sent in a single message
 * Input args:	lower_limit: pointer to the lower limit (i)
 * 		upper_limit: pointer to the upper limit (n)
 * Output args: input_mpi_t_p: the new MPI datatype
 */
void Build_mpi_type(
		int* 		lower_limit,	/* in */
		int* 		upper_limit,	/* in */
		MPI_Datatype* 	input_mpi_t_p	/* out */) {
	
	int array_of_blocklengths[2] = {1, 1};
	MPI_Datatype array_of_types[2] = {MPI_INT, MPI_INT};
	MPI_Aint lower_limit_addr, upper_limit_addr;
	MPI_Aint array_of_displacements[2] = {0};

	MPI_Get_address(lower_limit, &lower_limit_addr);
	MPI_Get_address(upper_limit, &upper_limit_addr);
	array_of_displacements[1] = upper_limit_addr - lower_limit_addr;

	MPI_Type_create_struct(2, array_of_blocklengths,
			array_of_displacements, array_of_types,
			input_mpi_t_p);

	MPI_Type_commit(input_mpi_t_p);
}	/* Build_mpi_type */

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
	
	/* in this case, i is assumed to be 0 */
	*lower_limit = 0;
	
	if (my_rank == 0) {
		printf("Enter n: ");
		fflush(stdout);	
		scanf("%d", upper_limit);
	}

	MPI_Bcast(lower_limit, 1, MPI_INT, 0, MPI_COMM_WORLD);
	MPI_Bcast(upper_limit, 1, MPI_INT, 0, MPI_COMM_WORLD);
}	/* Get_input */
