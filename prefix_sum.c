#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<mpi.h>
#include <time.h>

#define ARRAY_SIZE 15

int main(int argc, char *argv[])
{
int rank;
int size;

  double total_time;
	clock_t start, end;
	start = clock();

if(MPI_Init(&argc, &argv) != MPI_SUCCESS)
{
    fprintf(stderr, "Unable to initialize MPI!\n");
    return -1;
}
MPI_Comm_rank(MPI_COMM_WORLD, &rank);
MPI_Comm_size(MPI_COMM_WORLD, &size);
if(ARRAY_SIZE % size != 0 && rank == 0)
{
    fprintf(stderr,"Array size must be multiple of mpi job size.\n");
    return -1;
}
MPI_Status status;
int * array = (int *) malloc(sizeof(int) * ARRAY_SIZE);
int * chunk = (int *) malloc(sizeof(int) * ARRAY_SIZE/size);
int i = 0;

int total_sum = 0;
for(i = 0; i < ARRAY_SIZE; i++)
{
    array[i] = rand() % 1024;
    printf("%d ",array[i]);
    total_sum+=array[i];
}
printf("\n");
MPI_Barrier(MPI_COMM_WORLD);
MPI_Scatter(array,ARRAY_SIZE/size,MPI_INT,chunk,ARRAY_SIZE/size,MPI_INT,0,MPI_COMM_WORLD);
int sum = 0;
int temp = 0;
int key = 1;
for(i = 0; i < ARRAY_SIZE/size; i++)
    sum += chunk[i];

while(key <= size/2)
{
    if((rank+1) % key == 0)
        if(rank/key % 2 == 0)
        {
            MPI_Send(&sum, 1, MPI_INT, rank+key,0,MPI_COMM_WORLD);
        }
        else
        {
            MPI_Recv(&temp, 1, MPI_INT, rank-key,0,MPI_COMM_WORLD,&status);
            sum += temp;
        }
          key = 2 * key;
        MPI_Barrier(MPI_COMM_WORLD);
            
}
if(rank == size-1)
{
    fprintf(stderr,"Total: %d\n",sum);
    fprintf(stderr,"Correct Sum: %d\n",total_sum);
}
free(array);
free(chunk);
MPI_Finalize();

	end = clock();
	//time count stops 
	total_time = ((double) (end - start))/CLOCKS_PER_SEC;
	printf("\nTime taken is: %fs\n", total_time);

return 0;

}
