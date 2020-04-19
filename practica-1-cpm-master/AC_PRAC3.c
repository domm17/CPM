#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>  

#define NN 64000000
#define MAX_INT ((int)((unsigned int)(-1)>>1))

int valors[NN+1];
int valors2[NN+1];
int tdades, tthreads, nrep;

pthread_t threads[128];

pthread_barrier_t barrierqs;		//Barreres de sincronisme

typedef struct data {		//Dades a passar a cada fil d'execució
  int* val;
  int ne;
  int nthread;
} data;

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

void *sort(void *threadargs)	
 {
  data *tdata;
  tdata = (data *)threadargs;
  int t = 2, i, *vin, *vout, *vtmp, m, n, nthread = tdata->nthread, j = nthread + 1;
  
  qs(tdata->val, tdata->ne);		//Quicksort amb els fils especificats per paràmetre
  pthread_barrier_wait(&barrierqs);	//Barrera per a sincronitzar-los
  
  vin = valors;
  vout = valors2;
  i = nthread*tdata->ne;
  for (m = 2*tdata->ne; m < tdades; m *= 2)	//Mergesort en arbre
   { 
    if (nthread%t == 0)
    {  
     pthread_join(threads[j], NULL);		//En cada iteració del merge ens sicronitzem amb el 
     j = nthread+t;				//fil que tractava les dades que ara utilitzarem
     merge2(&vin[i],m,&vout[i]);
     t *= 2;
     vtmp=vin;
     vin=vout;
     vout=vtmp;
    } else pthread_exit(NULL);			//Si ja no hem de tractar més dades, finalitzem
    if (nthread == 0)
     {
      nrep++;
     }
   }
 }

main(int nargs,char* args[])
{
  int ndades,i,m,parts, result;
  int *vin,*vout,*vtmp;
  long long sum=0;

  assert(nargs == 3);
  
  ndades = atoi(args[1]);
  assert(ndades <= NN);

  tdades = ndades;
  
  parts = atoi(args[2]);
  if (parts < 2) assert("Han d'haver dues parts com a minim" == 0);
  if (ndades % parts) assert("N ha de ser divisible per parts" == 0);

  tthreads = parts;

  data data_array[parts];

  pthread_barrier_init(&barrierqs, NULL, parts);
 
  for(i=0;i<ndades;i++) valors[i]=rand()%MAX_INT;

  for (i=0;i<parts;i++)
  {
    data_array[i].val = &valors[i*ndades/parts];
    data_array[i].ne = ndades/parts;
    data_array[i].nthread = i;
    result = pthread_create(&threads[i], NULL, sort, (void *)&data_array[i]);
    assert(!result);
  }

  for(i=0; i<parts; i+=parts/2) {
       result = pthread_join(threads[i], NULL);
  }  
  pthread_barrier_destroy(&barrierqs);
 
   if (nrep%2 == 1)
   {
    vin = valors;
    vout = valors2;
   } else 
    {
     vin = valors2;
     vout = valors;
    }

  merge2(vout,ndades,vin);		//Fem l'últim merge

  for (i=1;i<ndades;i++) assert(vin[i-1]<=vin[i]);
  for (i=0;i<ndades;i+=100)
    sum += vin[i];
  printf("validacio %lld \n",sum);
}

