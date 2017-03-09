/*
 * bipartiteMatchingMatrix.c
 *
 *  Created on: Feb 5, 2016
 *      Author: pascal
 */

#include <stdio.h>
#include <stdlib.h>
#include <malloc.h>
#include <string.h>

// A DFS based recursive function that returns true if a
// matching for vertex u is possible
char bpm(char** bpGraph, size_t N, size_t M, int u, char* seen, int* matchR)
{
	// Try every job one by one
	for (size_t v = 0; v < N; v++)
	{
		// If applicant u is interested in job v and v is
		// not visited
		if (bpGraph[u][v] && !seen[v])
		{
			seen[v] = 1; // Mark v as visited

			// If job 'v' is not assigned to an applicant OR
			// previously assigned applicant for job v (which is matchR[v])
			// has an alternate job available.
			// Since v is marked as visited in the above line, matchR[v]
			// in the following recursive call will not get job 'v' again
			if (matchR[v] < 0 || bpm(bpGraph, N, M, matchR[v], seen, matchR))
			{
				matchR[v] = u;
				return 1;
			}
		}
	}
	return 0;
}

// Returns maximum size of matching from M to N
int maxBPM(char** bpGraph, size_t N, size_t M)
{
	// An array to keep track of the applicants assigned to
	// jobs. The value of matchR[i] is the applicant number
	// assigned to job i, the value -1 indicates nobody is
	// assigned.
	int* matchR = malloc(N * sizeof(int));

	// Initially all jobs are available
	memset(matchR, -1, N * sizeof(int));
	//    for (size_t i=0; i<N; ++i) {
	//    	matchR[i] = -1;
	//    }

	int result = 0; // Count of jobs assigned to applicants
	for (size_t u = 0; u < M; u++)
	{
		// Mark all jobs as not seen for next applicant.
		char* seen = malloc(N * sizeof(char));
		memset(seen, 0, N * sizeof(char));

		// Find if the applicant 'u' can get a job
		if (bpm(bpGraph, N, M, u, seen, matchR))
			result++;

		free(seen);
	}

	free(matchR);
	return result;
}

//// Driver program to test above functions
//int main()
//{
//	const size_t N = 6;
//	const size_t M = 7;
//	// Let us create a bpGraph shown in the above example
//	char a[] = {0, 1, 1, 0, 0, 0};
//	char b[] = {1, 0, 0, 1, 0, 0};
//	char c[] = {0, 0, 1, 0, 0, 0};
//	char d[] = {0, 0, 1, 1, 0, 0};
//	char e[] = {0, 0, 0, 0, 0, 0};
//	char f[] = {0, 0, 0, 0, 0, 1};
//	char g[] = {1, 1, 0, 0, 1, 1};
//
//	char* bpGraph[M];
//	bpGraph[0] = a;
//	bpGraph[1] = b;
//	bpGraph[2] = c;
//	bpGraph[3] = d;
//	bpGraph[4] = e;
//	bpGraph[5] = f;
//	bpGraph[6] = g;
//
//	printf("Maximum number of applicants that can get job is %i\n", maxBPM(bpGraph, N, M));
//
//
//	return 0;
//}

