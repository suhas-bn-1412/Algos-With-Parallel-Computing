#include<stdio.h>
#include<stdlib.h>
#include<mpi.h>

int mat_mul(int n,int** a, int** b, int** c);

int main()
{
  int n;
  scanf("%d",&n);
  int**a;
  a=(int**)malloc(sizeof(int*)*n);

  int**b;
  b=(int**)malloc(sizeof(int*)*n);

  int**c;
  c=(int**)malloc(sizeof(int*)*n);

  for(int i=0;i<n;i++)
  {
	a[i]=(int*)malloc(sizeof(int)*n);
	b[i]=(int*)malloc(sizeof(int)*n);
	c[i]=(int*)malloc(sizeof(int)*n);
  }
  
  for(int i=0;i<n;i++)
  {
   for(int j=0;j<n;j++)
   {
	scanf("%d",&a[i][j]);
   }
  }
 
  for(int i=0;i<n;i++)
  {
    for(int j=0;j<n;j++)
    {
	scanf("%d",&b[i][j]);
    }
  }

  mat_mul(n,a,b,c);
  
  for(int i=0;i<n;i++)
  {
   for(int j=0;j<n;j++)
   {
	printf("%d ",c[i][j]);
   }
   printf("\n");
  }
}

int mat_mul(int n,int** a, int** b, int** c)
{
   int i,j,k;
#pragma omp parallel shared(a,b,c) private(i,j,k) 
   {
#pragma omp for  schedule(static)
   for (i=0; i<n;i++){
      for (j=0; j<n;j++){
         c[i][j]=0;
         for (k=0; k<n; k++){
		//printf("mul %d %d\n",a[i][k],b[k][j]);
            c[i][j]=(c[i][j])+((a[i][k])*(b[k][j]));
         }
      }
   }
   }
}
