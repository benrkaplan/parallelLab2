#include <stdio.h>
#include <stdlib.h>
#include <omp.h>
#include <math.h>





int ** allMatrix;
int cityCount;
int pathCount;
int * shortestPath;
int shortestDistance;
int shortestThread;
int threadCount;

int * citiesToCalculate;
int pathsPerThread;
int tidCity;

void getShortestPath();

int main(int argc, char *argv[]){

	//begin reading input
	FILE * filePointer;
	int i, j;

	filePointer = fopen(argv[3], "r");

	//get city count and thread count from command line 
	if( filePointer ) {
		cityCount = atoi(argv[2]);
		threadCount = atoi(argv[1]);
		if( threadCount >= cityCount ){
			threadCount = cityCount - 1;
		}
	} else {
		printf("Error, can not open file... exiting...\n" );
		exit(1);
	}
	//begin creating matrix
	allMatrix = (int**)malloc(cityCount * sizeof(int *));
	//can't be allocated, we must exit
	if( !allMatrix ){
		printf("Cannot allocate matrix... exiting...\n");
		exit(1);
	} 
	for( i = 0; i < cityCount; i++ ){
		allMatrix[i] = (int *)malloc(cityCount * sizeof(int));
		if( !allMatrix[i] ){
			printf("Error... exiting...\n");
			exit(1);
		}
	}
	//end creating matrix
	//populate matrix
	for( i = 0; i < cityCount; i++ ){
		for ( j = 0; j < cityCount; j++ ) {
			fscanf(filePointer, "%d ", &allMatrix[i][j]);
		}
	}
	fclose(filePointer);
	//end reading input

	int temp;
	pathCount = 1;
	//calculating number of paths
	for( temp = cityCount - 1; temp > 0; temp-- ){
		pathCount *= temp;
	}
	//calculate paths per thread by
	//dividing the number of paths by the number of threads	
	pathsPerThread = pathCount / threadCount;
	//shortest path set 
	shortestPath = (int *) malloc(cityCount * sizeof(int));
	//arbitrarily large number to begin with
	shortestDistance = 999999999;

	if( cityCount == 2 ) {
		printf("Best Path: 0 -> 1");
	
		printf("\nDistance: %d.\n\n", allMatrix[0][1]);
	} else {
		//function call to parallel calculation 
		getShortestPath();
	}

	return 0;
}

void getShortestPath() {

	//begin parallel
	#pragma omp parallel num_threads(threadCount) private(citiesToCalculate, tidCity)
	{
		//allcating memory
		citiesToCalculate = (int *)malloc((cityCount - 2) * sizeof(int));
		int i;

		//computing cities to calculate in every thread
		int index = 0;
		tidCity = omp_get_thread_num()+1;
		for( i = 1; i < cityCount; i ++ ){
			if( i != tidCity ){
				citiesToCalculate[index++] = i;
			}
		}

		int startDistance = allMatrix[0][tidCity];
		int thisDistance;

		for( i = 0; i < pathsPerThread; i++ ){

			thisDistance = startDistance;
			thisDistance += allMatrix[tidCity][citiesToCalculate[0]];

			for( i = 1; i < cityCount - 2; i++) {
				thisDistance += allMatrix[citiesToCalculate[i-1]][citiesToCalculate[i]];
				if( thisDistance < shortestDistance ) {
					continue;
				} else {
					break;
				}
			}

			if( thisDistance < shortestDistance ) {

				#pragma omp critical 
				{
					if( thisDistance < shortestDistance ) {
						shortestDistance = thisDistance;
						shortestPath[0] = 0;
						shortestPath[1] = tidCity;
						for (i=0;i<cityCount-2;i++){
              				shortestPath[i+2] = citiesToCalculate[i];
						}
						shortestThread = omp_get_thread_num();
					}
				}
			}

			int a, b, c;

			a = cityCount - 3;

			while (a>0 && citiesToCalculate[a-1] >= citiesToCalculate[a]) {
				a--;
			}

			if( a == 0 ){
				break;
			}

			b = cityCount - 3; 

			while(citiesToCalculate[b] <= citiesToCalculate[a-1]) {
				b--;
			}

			c = citiesToCalculate[a-1];
			citiesToCalculate[a-1] = citiesToCalculate[b];
			citiesToCalculate[b] = c;

			b = cityCount-3;

			while( a < b ) {

				c = citiesToCalculate[a];
				citiesToCalculate[a] = citiesToCalculate[b];
				citiesToCalculate[b] = c;
				a++;
				b--;
			}

		}
		//make sure all calculations are done before printing anything
		#pragma omp barrier
	} //end parallelism 

	//handling all printing of output
	printf("Best Path: ");
	int i;
	for( i = 0; i < cityCount; i++ ){
		if( i + 1 != cityCount ){
			printf("%d -> ", shortestPath[i] );
		} else {
			printf("%d ", shortestPath[i] );
		}	
	}
	printf("\nDistance: %d.\n\n", shortestDistance);
	//end of function 
}

