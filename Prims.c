#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#define DIM 1000
void init(void); 
void delete(int);

struct prims { int edge[DIM][DIM];
                int dim; int U[DIM];
                int total_minDist;
                int counts;
            };

struct prims prim;

int main()
{
    int ch,j,t,p_c,p_j,k,serial=1,i; 
    
    //variable that holds the current maximum distance
    int minDist;
	int newElem;
    //variable that holds the next node in MST int newElem;
    
    prim.total_minDist = 0;
    prim.counts = 0;
    minDist = 1000;
    
    
    printf("Enter the number of nodes:\n");
    scanf( "%d", &prim.dim);
    printf("Enter the cost of edges: \n");
    
    for (i = 0; i < prim.dim; ++i) {
        for (j = 0; j < prim.dim; j++) {
            scanf("%d",&(prim.edge[i][j]));
        }
    }

    init();
    
    //calculating for all the nodes
    for(k = 0; k < prim.dim -1; k++)
    {
        minDist = 1000;
        //for every node in minimum spanning tree
        for(i = 0; i < prim.counts; i++)
        {
            //declaring OpenMP's derective with the appropriate scheduling...
#pragma omp parallel for
            for(j = 0; j < prim.dim; j++)
            {
                //find the minimum weight
                if(prim.edge[prim.U[i]][j] > minDist || prim.edge[prim.U[i]][j]==0)
                {
                    continue;
                }
                else
                {
#pragma omp critical
                    {
                        minDist = prim.edge[prim.U[i]][j];
                        newElem = j;
						printf("%d --> %d\n",i+1,j+1);
                    }
                }
            }
        }
        //Adding the local minDist to the total_minDist
        prim.total_minDist += minDist;
        //Adding the next node in the U set
        prim.U[i] = newElem;
        //Substructing the elements of the column in which  the new node is assosiated with
        delete( newElem );
        //Increasing the nodes that they are in the MST
        prim.counts++;
    }
    
    printf("\n");
    //Print all the nodes in MST in the way that they stored in the U set
    for(i = 0 ; i < prim.dim; i++) {
        printf("%d ",prim.U[i] + 1);
        if( i < prim.dim - 1 ) printf("-> ");
    }
    
    printf("\n\n");
    printf("Total minimun distance: %d\n\n", prim.total_minDist);
    //printf("\nProgram terminates now..\n");
    return 0;
}

void init(void)
{    
    int i,j;
    
    prim.total_minDist = 0;
    prim.counts = 0;
    
    //initializing the U set
    for(i = 0; i < prim.dim; i++) prim.U[i] = -1;
    
    //storing the first node into the U set
    prim.U[0] = 0; 
    //deleting the first node 
    delete( prim.U[0] ); 
    //incrementing by one the number of node that are inside the U set 
    prim.counts++;
}

void delete(int next_element)
{    
    int k;
    for(k = 0; k < prim.dim; k++)
    {
        prim.edge[k][next_element] = 0;
    }
}

/*
Size of Matrix:3
total minimum distance:182
Time Taken : 0.002319s
Number of elements:3

*/