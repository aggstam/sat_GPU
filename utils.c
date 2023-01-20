#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

// Execution parameters
int N;			// Number of propositions.
int K;			// Number of clauses.
int M;			// Number of propositions per clause.
int *Problem;   // This is a table to keep all the clauses of the problem.	
clock_t t1, t2; // CPU timers.
int mem_error;  // Constant for errors while allocating memory. If mem_error -1 programm exhausted all available memory and terminates.

// Frontier's node structure.
struct frontier_node {
	int* vector;			        // Node's vector.
	struct frontier_node* previous; // Pointer to the previous frontier node.
	struct frontier_node* next;	    // Pointer to the next frontier node.
};

struct frontier_node* head = NULL;  // The one end of the frontier.
struct frontier_node* tail = NULL;  // The other end of the frontier.

// Reading the input file.
int readfile(char* filename) {
	int i, j;
	FILE* infile;
	int err;

	// Opening the input file.
	infile = fopen(filename, "r");
	if (infile == NULL) {
		printf("Cannot open input file. Program terminates.\n");
		return -1;
	}

	// Reading the number of propositions.
	err = fscanf(infile, "%d", &N);
	if (err < 1) {
		printf("Cannot read the number of propositions. Program terminates.\n");
		fclose(infile);
		return -1;
	}

	if (N < 1) {
		printf("Small number of propositions. Program terminates.\n");
		fclose(infile);
		return -1;
	}

	// Reading the number of clauses.
	err = fscanf(infile, "%d", &K);
	if (err<1) {
		printf("Cannot read the number of clauses. Program terminates.\n");
		fclose(infile);
		return -1;
	}

	if (K < 1) {
		printf("Low number of clauses. Program terminates.\n");
		fclose(infile);
		return -1;
	}

	// Reading the number of propositions per clause.
	err = fscanf(infile, "%d", &M);
	if (err < 1) {
		printf("Cannot read the number of propositions per clause. Program terminates.\n");
		fclose(infile);
		return -1;
	}

	if (M < 2) {
		printf("Low number of propositions per clause. Program terminates.\n");
		fclose(infile);
		return -1;
	}

	// Allocating memory for the clauses...
	Problem = (int*)malloc(K * M * sizeof(int));

	// ...and read them
	for (i = 0; i < K; i++) {
		for (j = 0; j < M; j++) {
			err = fscanf(infile, "%d", Problem + (i * M) + j);
			if (err < 1) {
				printf("Cannot read the #%d proposition of the #%d clause. Program terminates.\n", j + 1, i + 1);
				fclose(infile);
				return -1;
			}
			if (Problem[i*M + j] == 0 || Problem[(i * M) + j] > N || Problem[(i * M) + j] < -N) {
				printf("Wrong value for the #%d proposition of the #%d clause. Program terminates.\n", j + 1, i + 1);
				fclose(infile);
				return -1;
			}
		}
	}

	fclose(infile);

	return 0;
}

// Auxiliary function that displays all the clauses of the problem.
void display_problem() {
	printf("The current problem:\n");
	printf("====================\n");
	for (int i = 0; i < K; i++) {
		for (int j = 0; j < M; j++) {
			if (j > 0) {
				printf(" or ");
		    }
			if (Problem[(i * M) + j] > 0) {
				printf("P%d", Problem[(i * M) + j]);
		    } else {
				printf("not P%d", -Problem[(i * M) + j]);
		    }
		}
		printf("\n");
	}
}

// Auxiliary function that displays the current assignment of truth values to the propositions.
void display(int* vector) {
	for (int i = 0; i < N; i++) {
		if (vector[i] == 1) {
			printf("P%d=true  ", i + 1);
		} else {
			printf("P%d=false  ", i + 1);
		}
	}
}

// Auxiliary function that copies the values of one vector to another.
void copy(int* vector1, int* vector2) {
	for (int i = 0; i < N; i++) {
		vector2[i] = vector1[i];
	}
}
