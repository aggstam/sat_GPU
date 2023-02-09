// ------------------------------------------------------- 
// Kernel file named cl_valid.cl.
// This Kernel checks wether a current partial assignment is invalid.
// In order for a partial assignment to be invalid, there should exist a clause such that
// all propositions in the clause have already value and their values are such that 
// the clause is false.
//
// Author: Aggelos Stamatiou, March 2017
//
// -------------------------------------------------------

__kernel void clvalid(
__global int *Problem,
__global int *vector,
__global int *finish,
__global int *partial_sums,
const int step,
const int M)
{
    int idx = get_global_id(0); // The ID of the thread in execution.
    int valid;                  // Count of valid propositions in each clause.
    int sum;                    // Sum of valid clauses.
    int i,j;
    int start = idx * step;

    sum = 0;
    for(i = start; i < finish[idx]; ++i){
        valid = 0;
        for(j = 0; j < M; ++j){
            valid+=((Problem[(i * M) +j] > 0) &&
            (vector[Problem[(i * M) +j] - 1] >= 0)) ||
            ((Problem[(i * M) + j] < 0) &&
            (vector[-Problem[(i * M) + j] - 1] <= 0));
        }
        sum += (valid > 0); // if valid = 0, the clause is invalid.
    }

    // Write the local sum to global memory, so the CPU can reduce the table
    // partial_sums, which has size the number of threads.
    partial_sums[idx] = sum;
}
