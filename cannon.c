#include <mpi.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <string.h>

int main(int argc,char *argv[]){
  if(argc < 2){
    printf("Usage error: size of matrix!\n");
    return 0;
  }
  int i,j,k;
  int N = atoi(argv[1]);
  int a[N][N],b[N][N],c[N][N];
  int numP,rank;
  double timeCost;

  MPI_Init(&argc,&argv);
  MPI_Comm_size(MPI_COMM_WORLD, &numP);
  MPI_Comm_rank(MPI_COMM_WORLD, &rank);
  if(rank == 0)  timeCost = MPI_Wtime();
  int procGrid = sqrt(numP);
  int blockLength = N/procGrid;

  int localA[blockLength][blockLength],localB[blockLength][blockLength],localResult[blockLength][blockLength];

  for(i=0;i<blockLength;i++){
    for(j=0;j<blockLength;j++){
      localResult[i][j] = 0;
    }
  }

  if(rank == 0){
    int count = 0;
    for (i=0; i<N; i++) {
      for (j=0; j<N; j++){
	a[i][j] = count;
	b[i][j] = count;
	count++;
      }
    }
  }

  int sizes[2]    = {N, N};
  int subsizes[2] = {blockLength, blockLength}; 
  int starts[2]   = {0,0};
  MPI_Datatype type, subArrayType;
  MPI_Type_create_subarray(2, sizes, subsizes, starts, MPI_ORDER_C, MPI_INT, &type);
  MPI_Type_create_resized(type, 0, blockLength*sizeof(int), &subArrayType);
  MPI_Type_commit(&subArrayType);

  if(rank == 0){
    //Copy initial local matrix for rank 0
    for(i = 0;i<blockLength;i++){
      memcpy(&localA[i][0],&a[i][0],sizeof(int)*blockLength);
      memcpy(&localB[i][0],&b[i][0],sizeof(int)*blockLength);
    }

    for(i=0;i<numP;i++){
      //pI indicate row number in sense of block
      //pJ indicate column number in sense of block
      int pAi = floor(blockLength*i/N);
      int pAj = i - pAi*procGrid;
      int pBi = floor(blockLength*i/N);
      int pBj = i - pBi*procGrid;

      //Shift blocks
      //Shift A to left pI unit
      int startAx = 0, startAy = 0;
      int startBx = 0, startBy = 0;
      startAx = ((pAj-pAi)<0)?(procGrid+pAj-pAi)*blockLength:(pAj-pAi)*blockLength;
      startAy = pAi*blockLength;
      //MPI_Isend(&a[startAy][startAx],1,subArrayType,i,0,MPI_COMM_WORLD,&sendReqs[0]);

      //Shift B to above pJ unit
      startBy = ((pBi-pBj)<0)?(procGrid + pBi-pBj)*blockLength:(pBi-pBj)*blockLength;
      startBx = pBj*blockLength;

      MPI_Request sendReqs[2];
      MPI_Status sendSta[2];
      int numCompleted,finishedReqs[2];
      MPI_Isend(&a[startAy][startAx],1,subArrayType,i,0,MPI_COMM_WORLD,&sendReqs[0]);
      MPI_Isend(&b[startBy][startBx],1,subArrayType,i,1,MPI_COMM_WORLD,&sendReqs[1]);
    }
  }


  //Shift calculating
  int calCount = 0;
  while(calCount < procGrid){
    //Non-block recv block a and b
    MPI_Request recvReqs[2];
    MPI_Status recvSta[2];
    if(calCount == 0){
      MPI_Irecv(&localA[0][0],blockLength*blockLength,MPI_INT,0,0,MPI_COMM_WORLD,&recvReqs[0]);
      MPI_Irecv(&localB[0][0],blockLength*blockLength,MPI_INT,0,1,MPI_COMM_WORLD,&recvReqs[1]);
    }else{
      int sourceA = ((rank+1)%procGrid == 0)?rank+1-procGrid:rank+1;
      int sourceB = (floor(rank/procGrid) != (procGrid-1))?rank+procGrid:rank-procGrid*(procGrid-1);
      MPI_Irecv(&localA[0][0],blockLength*blockLength,MPI_INT,sourceA,0,MPI_COMM_WORLD,&recvReqs[0]);
      MPI_Irecv(&localB[0][0],blockLength*blockLength,MPI_INT,sourceB,1,MPI_COMM_WORLD,&recvReqs[1]);
    }
    
    //Wait for receiving finish
    MPI_Wait(&recvReqs[0],&recvSta[0]);
    MPI_Wait(&recvReqs[1],&recvSta[1]);

    int localTempA[blockLength][blockLength],localTempB[blockLength][blockLength];
    //Copy to local temp
    memcpy(&localTempA[0][0],&localA[0][0],sizeof(int)*blockLength*blockLength);
    memcpy(&localTempB[0][0],&localB[0][0],sizeof(int)*blockLength*blockLength);

    //Non blocking send out
    MPI_Request sendReqs[2];
    MPI_Status sendSta[2];
    int nextAid = ((rank%procGrid) != 0)?rank-1:rank-1+N/blockLength;
    MPI_Isend(&localA[0][0],blockLength*blockLength,MPI_INT,nextAid,0,MPI_COMM_WORLD,&sendReqs[0]);
    int nextBid = (floor(rank/procGrid) != 0)?rank-procGrid:rank+procGrid*(procGrid-1);
    MPI_Isend(&localB[0][0],blockLength*blockLength,MPI_INT,nextBid,1,MPI_COMM_WORLD,&sendReqs[1]);

    //Do local calculate
    for(i=0;i<blockLength;i++){
      for(j=0;j<blockLength;j++){
	int sum = 0;
	for(k=0;k<blockLength;k++){
	  sum += localTempA[i][k]*localTempB[k][j];
	}
	localResult[i][j] += sum;
      }
    }

    calCount ++;
  }
    
  //Gather local result into c
  int *startPtrC = (rank == 0)?&(c[0][0]):NULL;
  /* scatter initial block to all processors */
  int sendCounts[numP];
  int displs[numP];

  //Calculate the space between block
  if (rank == 0) {
    for (i=0; i<numP; i++)
      sendCounts[i] = 1;
    int disp = 0;
    for (i=0; i<procGrid; i++) {
      for (j=0; j<procGrid; j++) {
	displs[i*procGrid+j] = disp;
	disp += 1;
      }
      disp += (blockLength-1)*procGrid;
    }
  }

  printf("Matrix 1\n");
  for(int var_i=0;var_i<blockLength;var_i++)
  {
    for(int var_j=0;var_j<blockLength;var_j++)
    {
	printf("%d ",localA[var_i][var_j]);
    }
    printf("\n");
  }

  printf("\nMatrix 2\n");
  for(int var_i=0;var_i<blockLength;var_i++)
  {
    for(int var_j=0;var_j<blockLength;var_j++)
    {
	printf("%d ",localB[var_i][var_j]);
    }
    printf("\n");
  }

  printf("\nFinal Answer\n");
  for(int var_i=0;var_i<blockLength;var_i++)
  {
    for(int var_j=0;var_j<blockLength;var_j++)
    {
	printf("%d ",localResult[var_i][var_j]);
    }
    printf("\n");
  }

  MPI_Gatherv(&(localResult[0][0]), blockLength*blockLength,MPI_INT,startPtrC,sendCounts,displs,subArrayType,0,MPI_COMM_WORLD);

  if(rank == 0) printf("Cannon time cost for size %d is %lf\n",N,MPI_Wtime()-timeCost);

  MPI_Type_free(&subArrayType);
  MPI_Finalize();
}

