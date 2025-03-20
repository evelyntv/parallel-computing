/* File:	Sum_Serial.c
 * Purpose:	A serial algorithm to calculation the summation of a function
 *
 * Input: 	The lower limit i, and the upper limit n'
 * Output:	The summation from i to n of 4*[(-1)^i / 2i+4]
 *
 * Compile:	gcc Sum_Serial.c -o Sum_Serial -lm 
 * Run:		./Sum_Serial 
 *
 * n = 1000000000
 */

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include "timer.h"

/* Calculate the summation */
double Summation(int i, int n);

/* Get the input value */
void Get_input(int* n);

int main(void) {
	int i = 0, n;	
	double result = 0, start, finish;

	Get_input(&n);
	
	/* Start timer */
	GET_TIME(start);

	/* Execute functions */
	result = Summation(i, n);

	/* End timer */
	GET_TIME(finish);

	/* Output result and time */
	printf("%f\n", result);
	printf("elapsed time: %e seconds\n", finish-start);
	
	return 0;
} /* main */

/*--------------------------------------------------------------------
 * Function:	Summation
 * Purpose:	Calculate and return the sum of all summands.
 * Input args:	The lower limit i, and the upper limit n
 * Output:	The sum of all summands times 4
 */
double Summation(int i, int n) {
	double sum = 0.0;

	for(i; i < n; i++) {
		sum += (pow(-1.0,(double)i) / ((2.0*i)+1));
	}	
	
	return 4*sum;
} /* Summation */

/*--------------------------------------------------------------------
 * Function:	Get_input
 * Purpose:	Get the user input, in this case we assume i = 0, and
 * 		we want to know n.
 * Output args:	n: 	pointer to upper limit n
 */
void Get_input(int* n) {
	printf("enter n: "); scanf("%d", n);
} /* Get_input */
