#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <mpi.h>

#define CERT 1
#define FALS 0

int el_meu_rank,size,taulers=0;

// Ha de ser inicialment correcta !!
int taula[9][9] = \
        {1,2,3, 4,5,6,  7,8,9,  \
         9,8,7, 3,2,1,  6,5,4,  \
         6,5,4, 7,8,9,  1,2,3,  \
\
         7,9,8, 1,0,0,  0,0,0,  \
         0,0,0, 0,0,0,  0,0,0,  \
         0,0,0, 0,0,0,  0,0,0,  \
\
         0,0,0, 0,0,0,  0,0,0,  \
         0,0,0, 0,0,0,  0,0,0,  \
         0,0,0, 0,0,0,  0,0,0};


int puc_posar(int x, int y, int z)
{
int i,j,pi,pj;

 for (i=0;i<9;i++) if (taula[x][i] == z) return(FALS); // Files
 for (i=0;i<9;i++) if (taula[i][y] == z) return(FALS); // Columnes
 // Quadrat
 pi = (x/3)*3; //truncament
 pj = y-y%3; //truncament
 for (i=0;i<3;i++) 
  for (j=0;j<3;j++) 
	if (taula[pi+i][pj+j] == z) return(FALS);

 return(CERT);
}

////////////////////////////////////////////////////////////////////
int recorrer_ser(int i, int j)
{
int k;
long int s=0;

 if (taula[i][j]) //Valor fixe no s'ha d'iterar
  {
     if (j<8) return(recorrer_ser(i,j+1));
     else if (i<8) return(recorrer_ser(i+1,0));
     else return(1); // Final de la taula
  }
 else // hi ha un 0 hem de provar
  {
   for (k=1;k<10;k++)
     if (puc_posar(i,j,k)) 
      {
       taula[i][j]= k; 
	 if (j<8) s += recorrer_ser(i,j+1);
	 else if (i<8) s += recorrer_ser(i+1,0);
	 else s++;
       taula[i][j]= 0;
      }
  }
return(s);
}


int recorrer(int i, int j, int d)
{
int k;
long int s=0;

 if (taula[i][j]) //Valor fixe no s'ha d'iterar
  {
     if (j<8) return(recorrer(i,j+1,d));
     else if (i<8) return(recorrer(i+1,0,d));
     else return(1); // Final de la taula
  }
 else // hi ha un 0 hem de provar
  {
   for (k=1;k<10;k++)
     if (puc_posar(i,j,k)) 
      {
        taula[i][j]= k; 
	if (d < 6)
	{
	 if (j<8) s += recorrer(i,j+1,d+1);
	 else if (i<8) s += recorrer(i+1,0,d+1);
	 else s++;
	}
	else
	{
	 if (el_meu_rank == taulers % size)
	 {
	   if (j<8) s+= recorrer_ser(i,j+1);
	   else if (i<8) s += recorrer_ser(i+1,0);
	   else s++;
	 }
	 taulers++;
	}
	taula[i][j]= 0;
      }
  }
return(s);
}

////////////////////////////////////////////////////////////////////
int main(int nargs, char* args[])
{
int i,j,k;
long int nsol, total;

MPI_Init(&nargs, &args);
MPI_Comm_rank(MPI_COMM_WORLD, &el_meu_rank);
MPI_Comm_size(MPI_COMM_WORLD, &size);

nsol = recorrer(0,0,0);

MPI_Reduce(&nsol, &total, 1, MPI_LONG, MPI_SUM, 0, MPI_COMM_WORLD);

if (el_meu_rank == 0)
{
  printf("numero solucions : %ld\n",total);
}
MPI_Finalize();
exit(0);
}
