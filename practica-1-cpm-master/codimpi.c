#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <mpi.h>

#define NN 384000000
#define MAX_INT ((int)((unsigned int)(-1)>>1))

int* vin;
int* vout;

void qs(int *val, int ne)
 {
  int i,f,j;
  int pivot,vtmp,vfi;

  pivot = val[0];
  i = 1;
  f = ne-1; 
  vtmp = val[i];

  while (i <= f)
   {
    if (vtmp < pivot) {
	    val[i-1] = vtmp;
	    i ++;
	    vtmp = val[i];
	}
    else {
            vfi = val[f];
	    val[f] = vtmp;
	    f --;
	    vtmp = vfi;
	}
   }
  val[i-1] = pivot;
 
  if (f>1) qs(val,f);
  if (i < ne-1) qs(&val[i],ne-f-1);
 }

void merge2(int* val, int n,int *vo)
{
  int vtmp;
  int i,j,posi,posj; 

   posi = 0;
   posj = (n/2);
 
   for (i=0;i<n;i++)
      if (((posi < n/2) && (val[posi] <= val[posj])) || (posj >= n))
	  vo[i] = val[posi++];
	else if (posj < n)
	  vo[i] = val[posj++];
}

int main(int nargs,char* args[])
{
  int ndades,i,m,parts,porcio;
  int *vtmp;
  long long sum=0;

  int el_meu_rank,size;
  MPI_Init(&nargs, &args);
  MPI_Comm_rank(MPI_COMM_WORLD, &el_meu_rank);
  MPI_Comm_size(MPI_COMM_WORLD, &size);

  assert(nargs == 3);
  
  ndades = atoi(args[1]);
  assert(ndades <= NN);

  parts = atoi(args[2]);
  if (parts < 2) assert("Han d'haver dues parts com a minim" == 0);
  if (ndades % parts) assert("N ha de ser divisible per parts" == 0);
  porcio = ndades/parts;
  vin = (int*)malloc(porcio * sizeof(int));
  vout = (int*)malloc(porcio * sizeof(int));

  for(i=0;i<porcio*el_meu_rank;i++) rand()%MAX_INT;
  for(i=0;i<porcio;i++) vin[i]=rand()%MAX_INT;

// Quicksort a parts
  qs(vin,porcio);

// Merge en arbre

  int t = 2;
  int y=1, enviat=0;

  for (m = 2*porcio; m <= ndades; m *= 2)	//Mergesort en arbre
   { 
    if (el_meu_rank%t == 0)
    {  
		vin = (int*)realloc(vin, m * sizeof(int));
		vout = (int*)realloc(vout, m * sizeof(int));
      	MPI_Recv(&vin[m/2], m/2, MPI_INT, el_meu_rank+y, 0, MPI_COMM_WORLD,MPI_STATUS_IGNORE);
		merge2(vin,m,vout);
		t *= 2;
		vtmp=vin;
		vin=vout;
		vout=vtmp;
    } 
	else if (enviat == 0)
	{
		MPI_Send(vin, m/2, MPI_INT, el_meu_rank-y, 0, MPI_COMM_WORLD);       
		enviat = 1;
		free(vin);
		free(vout);
	}
	y*=2;
   }
// Validacio
    if(el_meu_rank==0)
	{  
		// Validacio
		for (i=1;i<ndades;i++) assert(vin[i-1]<=vin[i]);
		for (i=0;i<ndades;i+=100)
		sum += vin[i];
        printf("validacio %lld \n",sum);
		free(vin);
		free(vout);
	}

  MPI_Finalize();

  return(0);
}

