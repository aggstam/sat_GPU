// -----------------------------------------------------------------------
//
// This program solves the Propositional (Boolean) Satisfiability problem
// using Depth-First Search algorithm.Problems are read from an input file, 
// while solution is written to screen and an output file.
//
// Author: Aggelos Stamatiou, April 2017
//
// -----------------------------------------------------------------------

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <string.h>

int N;			// Number of propositions.
int K;			// Number of clauses.
int M;			// Number of propositions per clause.
int *Problem;		// This is a table to keep all the clauses of the problem.

				// CPU timers.
clock_t t1, t2;

// Constant for errors while allocating memory. If mem_error -1 programm exhausted all available memory and terminates.
int mem_error;

// Frontier's node structure.
struct frontier_node {
	int *vector;			// Node's vector.
	struct frontier_node *previous; // Pointer to the previous frontier node.
	struct frontier_node *next;	// Pointer to the next frontier node.
};

struct frontier_node *head = NULL;  // The one end of the frontier.
struct frontier_node *tail = NULL;  // The other end of the frontier.

void syntax_error(char **argv) {
	printf("Wrong syntax. Use the following:\n\n");
	printf("%s <inputfile>\n\n", argv[0]);
	printf("where:\n");
	printf("<inputfile> = name of the file with the problem description\n");
	printf("Program terminates.\n");
}

// Reading the input file.
int readfile(char *filename) {
	int i, j;

	FILE *infile;
	int err;

	// Opening the input file.
	infile = fopen(filename, "r");
	if (infile == NULL) {
		printf("Cannot open input file. Program terminates.\n");
		return -1;
	}

	// Reading the number of propositions.
	err = fscanf(infile, "%d", &N);
	if (err<1) {
		printf("Cannot read the number of propositions. Program terminates.\n");
		fclose(infile);
		return -1;
	}

	if (N<1) {
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

	if (K<1) {
		printf("Low number of clauses. Program terminates.\n");
		fclose(infile);
		return -1;
	}

	// Reading the number of propositions per clause.
	err = fscanf(infile, "%d", &M);
	if (err<1) {
		printf("Cannot read the number of propositions per clause. Program terminates.\n");
		fclose(infile);
		return -1;
	}

	if (M<2) {
		printf("Low number of propositions per clause. Program terminates.\n");
		fclose(infile);
		return -1;
	}

	// Allocating memory for the clauses...
	Problem = (int*)malloc(K*M * sizeof(int));

	// ...and read them
	for (i = 0; i < K; i++) {
		for (j = 0; j < M; j++) {
			err = fscanf(infile, "%d", Problem + i*M + j);
			if (err < 1) {
				printf("Cannot read the #%d proposition of the #%d clause. Program terminates.\n", j + 1, i + 1);
				fclose(infile);
				return -1;
			}
			if (Problem[i*M + j] == 0 || Problem[i*M + j] > N || Problem[i*M + j] < -N) {
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
	int i, j;

	printf("The current problem:\n");
	printf("====================\n");
	for (i = 0; i<K; i++) {
		for (j = 0; j<M; j++) {
			if (j>0)
				printf(" or ");
			if (Problem[i*M + j]>0)
				printf("P%d", Problem[i*M + j]);
			else
				printf("not P%d", -Problem[i*M + j]);
		}
		printf("\n");
	}
}

// Auxiliary function that displays the current assignment of truth values to the propositions.
void display(int *vector) {
	int i;

	for (i = 0; i < N; i++) {
		if (vector[i] == 1) {
			printf("P%d=true  ", i + 1);
		}
		else {
			printf("P%d=false  ", i + 1);
		}
	}
}

// This function adds a pointer to a new leaf search-tree node at the front of the frontier.
int add_to_frontier(struct frontier_node *node) {

	if (node == NULL) {
		return -1;
	}

	node->previous = NULL;
	node->next = head;

	if (head == NULL) {
		head = node;
		tail = node;
	}
	else {
		head->previous = node;
		head = node;
	}

	return 0;
}

// This function checks whether a current partial assignment is already invalid. 
// In order for a partial assignment to be invalid, there should exist a clause such that
// all propositions in the clause have already value and their values are such that 
// the clause is false. We validate the vector by counting how many clauses are valid.
// In order for the vector to be invalid, count is less than K (number of clauses).
int valid(struct frontier_node *node) {
	int sum = 0;
	for (int i = 0; i<K; ++i) {
		int valid = 0;
		for (int j = 0; j<M; ++j) {
			valid += ((Problem[i*M + j]>0) &&
				(node->vector[Problem[i*M + j] - 1] >= 0)) ||
				((Problem[i*M + j]<0) &&
				(node->vector[-Problem[i*M + j] - 1] <= 0));
		}
		sum += (valid>0); // if valid = 0, the clause is invalid.
	}

	// Check validation.

	if (sum < K) {
		return 0;
	}

	return 1;
}

// Check whether a vector is a complete assignment and it is also valid.
int solution(struct frontier_node *node) {

	for (int i = 0; i < N; i++) {
		if (node->vector[i] == 0) {
			return 0;
		}
	}

	return valid(node);
}

// Auxiliary function that copies the values of one vector to another.
void copy(int *vector1, int *vector2) {

	for (int i = 0; i < N; i++) {
		vector2[i] = vector1[i];
	}
}

// Given a partial assignment vector, for which a subset of the first propositions have values, 
// this function pushes up to two new vectors to the frontier, which concern giving to the first unassigned 
// proposition the values true=1 and false=-1, after checking that the new vectors are valid.
void generate_children(struct frontier_node *node) {
	int i;
	int *vector = node->vector;

	// Find the first proposition with no assigned value.
	for (i = 0; i<N && vector[i] != 0; i++);

	vector[i] = -1;
	struct frontier_node *negative = (struct frontier_node*) malloc(sizeof(struct frontier_node));
	negative->vector = (int*)malloc(N * sizeof(int));
	if (negative == NULL || negative->vector == NULL) {
		mem_error = -1;
		return;
	}
	copy(vector, negative->vector);
	// Check whether a "false" assignment is acceptable...
	if (valid(negative)) {
		// ...and pushes it to the frontier.
		add_to_frontier(negative);
	}

	vector[i] = 1;
	struct frontier_node *positive = (struct frontier_node*) malloc(sizeof(struct frontier_node));
	positive->vector = (int*)malloc(N * sizeof(int));
	if (positive == NULL || positive->vector == NULL) {
		mem_error = -1;
		return;
	}
	copy(vector, positive->vector);
	// Check whether a "true" assignment is acceptable...
	if (valid(positive)) {
		// ...and pushes it to the frontier.
		add_to_frontier(positive);
	}

}

// This function implements the searching algorithm we've used,
// checking the frontier head if it's a solution, otherwise creating its
// children and pushes them to the frontier.
struct frontier_node *search() {
	struct frontier_node *current_node;
	struct frontier_node *temp_node;

	// Initializing the frontier.
	struct frontier_node *root = (struct frontier_node*) malloc(sizeof(struct frontier_node));
	root->vector = (int*)malloc(N * sizeof(int));
	if (root == NULL || root->vector == NULL) {
		mem_error = -1;
		return NULL;
	}
	for (int i = 0; i < N; i++) {
		root->vector[i] = 0;
	}

	generate_children(root);

	t1 = clock();

	// While the frontier is not empty...
	while (head != NULL) {
		// Extract the first node from the frontier.
		current_node = head;

		// If it is a solution return it.
		if (solution(current_node)) {
			t2 = clock();
			return current_node;
		}

		// Generate its children.
		generate_children(current_node);

		// Delete the first node of the frontier.
		temp_node = head;
		head = head->next;
		free(temp_node);
		if (head == NULL) {
			tail = NULL;
		}
		else {
			head->previous = NULL;
		}
	}

	t2 = clock();

	return NULL;

}

int main(int argc, char **argv) {
	int err;

	srand((unsigned)time(NULL));

	if (argc != 2) {
		syntax_error(argv);
		exit(-1);
	}

	err = readfile(argv[1]);
	if (err < 0) {
		exit(-1);
	}

	printf("\nThis programm solves the Propositional (Boolean) Satisfiability Problem written\n");
	printf("in file %s, using Depth First Search Algorithm.\n", argv[1]);

	//display_problem();

	struct frontier_node *solution_node = search(); // The main call.

	if (solution_node != NULL) {

		printf("\nSolution found with depth-first!\n");
		printf("\nSolution vector propositions values:\n");
		display(solution_node->vector);

	}
	else {
		if (mem_error == -1) {
			printf("Memory exhausted. Program terminates.\n");
		}
		else {
			printf("\nNO SOLUTION EXISTS. Proved by depth-first!");
		}
	}

	printf("\n\nTime spent: %0.3f secs\n", ((float)t2 - t1) / CLOCKS_PER_SEC);

	return 0;
}
