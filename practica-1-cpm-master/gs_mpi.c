#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <mpi.h>

#define NN 36
#define MAX_INT 10

int valors[NN+1];
int valors2[NN+1];

int main(int num_args, char* args[ ]) {
    int el_meu_rank;
    int p;
    int i;

    MPI_Init(&num_args, &args);
    MPI_Comm_rank(MPI_COMM_WORLD, &el_meu_rank);
    MPI_Comm_size(MPI_COMM_WORLD, &p);
    int elems = NN / p;
                     
    if (el_meu_rank == 0) {
       for(i=0;i<NN;i++) valors[i]=rand()%MAX_INT;
       printf("Vector: ");
       for(i=0;i<NN;i++) printf("%d ", valors[i]);
       printf("\n");
    }
    MPI_Barrier(MPI_COMM_WORLD);
    int *values = malloc(sizeof(int) * elems);

    /* Scatter/Gather sense tractament */
    MPI_Scatter(valors, elems, MPI_INT, values,
           elems, MPI_INT, 0, MPI_COMM_WORLD);
    printf("Vector proces %d: ", el_meu_rank);
    for(i=0;i<elems;i++) printf("%d ", values[i]);
    printf("\n");
    MPI_Gather(values, elems, MPI_INT, valors2,
            elems, MPI_INT, 0, MPI_COMM_WORLD);
    if (el_meu_rank == 0) {
       printf("Vector retornat: ");
       for(i=0;i<NN;i++) printf("%d ", valors2[i]);
       printf("\n");
    }
    MPI_Barrier(MPI_COMM_WORLD);
    /* Els valors que es troben en les NN % p posicions es perden */ 
    free(values);
    values = malloc(sizeof(int) * (elems + NN % p));
    int* counts = malloc(sizeof(int)*p);
    int* displs = malloc(sizeof(int)*p);
    int d = 0;

    /* Scatterv/Gatherv amb els valors restants al node 0 */
	counts[0] = elems + NN % p;
    displs[0] = d;

    for(i=1;i<p;i++)
    {
        counts[i] = elems;
        d += counts[i-1];
        displs[i] = d;
    }
    MPI_Scatterv(valors, counts, displs, MPI_INT, values, 100, MPI_INT, 0, MPI_COMM_WORLD);
    printf("Vector proces %d: ", el_meu_rank);
    if (el_meu_rank == 0)
    {
        for(i=0;i<elems + NN % p;i++) printf("%d ", values[i]);
    }
    else
    {
        for(i=0;i<elems;i++) printf("%d ", values[i]);
    }
    printf("\n");
    MPI_Gatherv(values, elems, MPI_INT, valors2, counts, displs, MPI_INT, 0, MPI_COMM_WORLD);
    if (el_meu_rank == 0)
	{
       printf("Vector retornat: ");
       for(i=0;i<NN;i++) printf("%d ", valors2[i]);
       printf("\n");
    }
    /* Els valors es retornen tots correctament */
    MPI_Barrier(MPI_COMM_WORLD);
    /* Scatterv/Gatherv amb els valors restants repartits entre tots els nodes */
	d = 0;
	for(i=0;i<p;i++)
	{
		counts[i] = elems;
		displs[i] = d;
		d += elems;
	}
	for(i=0;i<NN%p;i++)
	{
		counts[i]++;
		displs[i+1]++;
	}
	for(i=p-1;i>=NN%p;i--) displs[i] += NN%p;
	MPI_Scatterv(valors, counts, displs, MPI_INT, values, 100, MPI_INT, 0, MPI_COMM_WORLD);
	printf("Vector proces %d: ", el_meu_rank);
	if (el_meu_rank < NN%p)
	{
		for(i=0;i<elems+1;i++) printf("%d ", values[i]);
	}
	else
	{
		for(i=0;i<elems;i++) printf("%d ", values[i]);
	}
	printf("\n");
	MPI_Gatherv(values, elems, MPI_INT, valors2, counts, displs, MPI_INT, 0, MPI_COMM_WORLD);
	if (el_meu_rank == 0)
	{
		printf("Vector retornat: ");
		for(i=0;i<NN;i++) printf("%d ", valors2[i]);
		printf("\n");
	}
    /* Els valors es retornen tots correctament */

    MPI_Finalize();
    return (0);
}

