// -----------------------------------------------------------------------
//
// This program solves the Propositional (Boolean) Satisfiability problem
// using Depth-First Search algorithm.Problems are read from an input file, 
// while solution is written to screen and an output file.
//
// Author: Aggelos Stamatiou, April 2017
//
// -----------------------------------------------------------------------

// Common code file
#include "core.c"

// Auxiliary function that copies the values of one vector to another.
void syntax_error(char **argv)
{
    printf("Wrong syntax. Use the following:\n\n");
    printf("%s <inputfile>\n\n", argv[0]);
    printf("where:\n");
    printf("<inputfile> = name of the file with the problem description\n");
    printf("Program terminates.\n");
}

// This function checks whether a current partial assignment is already invalid. 
// In order for a partial assignment to be invalid, there should exist a clause such that
// all propositions in the clause have already value and their values are such that 
// the clause is false. We validate the vector by counting how many clauses are valid.
// In order for the vector to be invalid, count is less than K (number of clauses).
int valid(struct frontier_node *node)
{
    int sum = 0;
    for (int i = 0; i < K; ++i) {
        int valid = 0;
        for (int j = 0; j < M; ++j) {
            valid += ((Problem[(i * M) + j] > 0) &&
                (node->vector[Problem[(i * M) + j] - 1] >= 0)) ||
                ((Problem[(i * M) + j] < 0) &&
                (node->vector[-Problem[(i * M) + j] - 1] <= 0));
        }
        sum += (valid > 0); // if valid = 0, the clause is invalid.
    }

    // Check validation.
    if (sum < K) {
        return 0;
    }

    return 1;
}

// Depth-First Search functions
#include "dfs.c"

int main(int argc, char **argv)
{
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
    } else {
        if (mem_error == -1) {
            printf("Memory exhausted. Program terminates.\n");
        } else {
            printf("\nNO SOLUTION EXISTS. Proved by depth-first!");
        }
    }

    printf("\n\nTime spent: %0.3f secs\n", ((float)t2 - t1) / CLOCKS_PER_SEC);

    return 0;
}
