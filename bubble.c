#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <mpi.h>

void swap(int *xp, int *yp) 
{ 
    int temp = *xp; 
    *xp = *yp; 
    *yp = temp; 
} 

void bubbleSort(int arr[], int n) 
{ 
   int i, j; 
   for (i = 0; i < n-1; i++)       
  
       // Last i elements are already in place    
       for (j = 0; j < n-i-1; j++)  
           if (arr[j] > arr[j+1]) 
              swap(&arr[j], &arr[j+1]); 
} 

int ProcNum = 0;
int ProcRank = -1;
int main(int argc , char* argv[]) 
{
double total_time;
clock_t start, end;
start = clock();
	int n = atoi(argv[1]);
	int *original_array = malloc(n * sizeof(int));
	
	int c;
	srand(time(NULL));
	printf("This is the unsorted array: ");
	for(c = 0; c < n; c++) {
		
		original_array[c] = rand() % n;
		printf("%d ", original_array[c]);
		
		}
double *pData = 0;
double *pProcData = 0;
int DataSize = 0;
int BlockSize = 0;
MPI_Init(&argc, &argv);
bubbleSort(original_array,n);
MPI_Comm_size(MPI_COMM_WORLD, &ProcNum);
MPI_Comm_rank(MPI_COMM_WORLD, &ProcRank);
printf("\nParallel bubble sort program output\n");
for(int i=0;i<n;i++)
{
printf("%d ",original_array[i]);
}
printf("\n");
MPI_Finalize();
//bubbleSort(original_array,n);
end = clock();
total_time = ((double) (end - start))/CLOCKS_PER_SEC;
printf("\nTime taken is: %fs\n", total_time);
return 0;
}





