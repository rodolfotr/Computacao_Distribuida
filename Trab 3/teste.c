/*
Universidade Federal da Fronteira Sul
Curso: Ciencia da Computação
Disciplina: Computação Distribuida
Trabalho: Implementação de Quick Sort em lista dupla com ordenação em multithread e na GPU.
Aluno: Rodolfo Trevisol, Murilo Olveira

Instalação do OpenMP:
sudo apt-get install gcc-multilib
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <ctype.h>

//OpenMP Include
#include <omp.h>


#define ARRAY_MAX_SIZE 50000

int arr1[ARRAY_MAX_SIZE];
int arr2[ARRAY_MAX_SIZE];
int arr3[ARRAY_MAX_SIZE];

void swap(int* a, int* b)
{
	int t = *a;
	*a = *b;
	*b = t;
}

int partition (int arr[], int low, int high)
{
	int pivot = arr[high];
	int i = (low - 1);
	for (int j = low; j <= high- 1; j++)
	{
		if (arr[j] <= pivot)
		{
			i++;
			swap(&arr[i], &arr[j]);
		}
	}
	swap(&arr[i + 1], &arr[high]);
	return (i + 1);
}

void quickSort(int arr[], int low, int high)
{
	if (low < high)
	{
		int pi = partition(arr, low, high);

		#pragma omp task firstprivate(arr,low,pi)
		{
			quickSort(arr,low, pi - 1);
			quickSort(arr, pi + 1, high);
		}


	}
}


int partitionTHR(int arr[], int low, int high)
{
	int pivot = arr[high];
	int i = (low - 1);
    #pragma omp for
	for (int j = low; j <= high- 1; j++)
	{
		if (arr[j] <= pivot)
		{
			i++;
			swap(&arr[i], &arr[j]);
		}
	}
	swap(&arr[i + 1], &arr[high]);
	return (i + 1);
}

void quickSortTHR(int arr[], int low, int high)
{
	if (low < high)
	{
		int pi = partitionTHR(arr, low, high);

		#pragma omp task firstprivate(arr,low,pi)
		{
			quickSortTHR(arr,low, pi - 1);
			quickSortTHR(arr, pi + 1, high);
		}


	}
}

int partitionGPU(int arr[], int low, int high)
{
	int pivot = arr[high];
	int i = (low - 1);
    #pragma omp parallel for{
        for (int j = low; j <= high- 1; j++)
        {
            if (arr[j] <= pivot)
            {
                i++;
                swap(&arr[i], &arr[j]);
            }
        }
        swap(&arr[i + 1], &arr[high]);
    }
	return (i + 1);
}

void quickSortGPU(int arr[], int low, int high)
{
	if (low < high)
	{
		int pi = partitionGPU(arr, low, high);

		#pragma omp parallel sections
		{
            #pragma omp section
			quickSortGPU(arr,low, pi - 1);

            #pragma omp section
			quickSortGPU(arr, pi + 1, high);
		}


	}
}

void printArray(int arr[], int size)
{
	int i;
	for (i=0; i < size; i++)
		printf("%d ", arr[i]);
	printf("\n");
}

int main()
{
    srand ( time(0) ^ getpid() );
	double start_time, run_time;
	printf("\nPreenchendo arays");
	for( int i = 0; i < ARRAY_MAX_SIZE-1; i++ )
    {
       arr1[i] = rand() % 50 +1;
       arr2[i] = arr1[i];
       arr3[i] = arr1[i];

    }
	printf("\nIniciando QuickSort");
	int n = sizeof(arr1)/sizeof(arr1[0]);
    quickSort(arr1, 0, n-1);

	run_time = omp_get_wtime() - start_time;
	printf("\nExecução single thread: %lf segundos\nPrimeiro: %d - Ultimo: %d\n\n", run_time, arr1[0], arr1[ARRAY_MAX_SIZE-1]);
    sleep(2);




    //Execução Threads
    n = sizeof(arr2)/sizeof(arr2[0]);
    omp_set_num_threads(4);
    start_time = omp_get_wtime();
    
    #pragma omp parallel
    {

        int id = omp_get_thread_num();
        int nthrds = omp_get_num_threads();
            
        //#pragma omp single nowait
        quickSort(arr2, 0, n-1);


    }
	run_time = omp_get_wtime() - start_time;
	printf("\nExecução Threads: %lf segundos\n\tPrimeiro: %d - Ultimo: %d", run_time, arr2[0], arr2[ARRAY_MAX_SIZE-1]);
    sleep(2);
	






    //Execução Acelerador
    n = sizeof(arr3)/sizeof(arr3[0]);
    start_time = omp_get_wtime();
    

    int id = omp_get_thread_num();
    int nthrds = omp_get_num_threads();
    #pragma omp parallel private(arr3)
    {
        quickSortGPU(arr3, 0, n-1);
    }
        
	run_time = omp_get_wtime() - start_time;
	printf("\nExecução Acelerador: %lf segundos\n\tPrimeiro: %d - Ultimo: %d", run_time, arr3[0], arr3[ARRAY_MAX_SIZE-1]);
	
    
    
    
    
    printf("\n");
	return 0;
}
