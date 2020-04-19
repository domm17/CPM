#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <omp.h>
#include <string.h>

#define CERT 1
#define FALS 0

// Ha de ser inicialment correcta !!
int taula[81] = \
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


int puc_posar(int x, int y, int z, int sudoku[81])
{
int i,j,pi,pj;

 for (i=0;i<9;i++) if (sudoku[x * 9 + i] == z || sudoku[i * 9 + y] == z) return(FALS); // Files i Columnes
 // Quadrat
 pi = (x/3)*3; //truncament
 pj = y-y%3; //truncament
 for (i=0;i<3;i++) 
  for (j=0;j<3;j++) 
	if (sudoku[(pi+i) * 9 + pj+j] == z) return(FALS);

 return(CERT);
}

////////////////////////////////////////////////////////////////////
int recorrer_ser(int i, int j, int sudoku[81])
{
int k;
long int s=0;

 if (sudoku[i * 9 + j]) //Valor fixe no s'ha d'iterar
  {
     if (j<8) return(recorrer_ser(i,j+1, sudoku));
     else if (i<8) return(recorrer_ser(i+1,0,sudoku));
     else return(1); // Final de la taula
  }
 else // hi ha un 0 hem de provar
  {
   for (k=1;k<10;k++)
     if (puc_posar(i,j,k,sudoku)) 
      {
       sudoku[i * 9 + j]= k; 
	 if (j<8) s += recorrer_ser(i,j+1,sudoku);
	 else if (i<8) s += recorrer_ser(i+1,0,sudoku);
	 else s++;
       sudoku[i * 9 + j]= 0;
      }
  }
return(s);
}


int recorrer(int i, int j, int sudoku[81], int d)
{
int k;
long int s=0;

 if (sudoku[i * 9 + j]) //Valor fixe no s'ha d'iterar
  {
     if (j<8) return(recorrer(i,j+1,sudoku,d));
     else if (i<8) return(recorrer(i+1,0,sudoku,d));
     else return(1); // Final de la taula
  }
 else // hi ha un 0 hem de provar
  {
   for (k=1;k<10;k++) 
   {
     #pragma omp task firstprivate(i, j) shared(s,d)
     {
     if (puc_posar(i,j,k,sudoku)) 
      {
       int* nSudoku;
       int t = 0;
       nSudoku = (int*)malloc(81 * sizeof(int));
       memcpy(nSudoku, sudoku, 81*sizeof(int));
       nSudoku[i * 9 +j]= k; 
       if (d < 8)
       {
	 if (j<8) t += recorrer(i,j+1,nSudoku,d+1);
	 else if (i<8) t += recorrer(i+1,0,nSudoku,d+1);
	 else t++;
       }
       else
       {
         if (j<8) t += recorrer_ser(i,j+1,nSudoku);
	 else if (i<8) t += recorrer_ser(i+1,0,nSudoku);
	 else t++;
       }
       free(nSudoku);
       #pragma omp atomic
       s += t;
       }
      }
    }
   #pragma omp taskwait
  }
return(s);
}

////////////////////////////////////////////////////////////////////
int main(int nargs,char* args[])
{
long int nsol;
int th;
th = atoi(args[1]);
omp_set_num_threads(th);
#pragma omp parallel
{
#pragma omp master
nsol = recorrer(3,4,taula,0);
}
printf("numero solucions : %ld\n",nsol);
exit(0);
}

