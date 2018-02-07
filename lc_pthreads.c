#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define MAX_THREADS	16

int dimensions;
int numbers;
int iterations; 
int num_threads;

int curr_dim;

double XX[MAX_THREADS];
double XXw[MAX_THREADS];
double Xy[MAX_THREADS];

double** X;
double* w;
double* Y;

double* Xw;
//double* index;
double* Xisquare;
double* Xiy;

int threads[MAX_THREADS];

//void computeXw();
//void *computeDifferential(void *);
//void *getValue(void *);

void computew();
void computeXw();
double getLoss();

static inline double monotonic_seconds();

int main(int argc, char *argv[]) 
{
	
	/*read in data and store in files*/
	printf("Input file - %s\n", argv[1]);
	FILE* finput_file, *flabels_file;
	if((finput_file = fopen(argv[1], "r")) == NULL) 
	{
		printf("Sorry! Cannot open file\n");
	}
	else 
	{
		printf("here we go\n");
	}
	
	if((flabels_file = fopen(argv[2], "r")) == NULL) 
	{
		printf("Sorry! Cannot open labels file\n");
	}
	else 
	{
		printf("labels file opened\n");
	}
		
	fscanf(finput_file, "%d %d\n", &numbers, &dimensions);
	fscanf(flabels_file, "%d", &numbers);

	iterations = atoi(argv[3]);
	num_threads = atoi(argv[4]);

	printf("Numbers = %d\n", numbers);
	printf("Dimensions = %d\n", dimensions);
	
	//numbers = 1;

	printf("iterations = %d, num_threads = %d\n", iterations, num_threads);

	int i, j;

	X = (double**) malloc (numbers * sizeof(double*));
       	Y = (double*) malloc (numbers * sizeof(double));
	w = (double*) calloc (dimensions, sizeof(double));

	Xisquare = (double*) calloc (dimensions, sizeof(double));
	Xiy = (double*) calloc (dimensions, sizeof(double));
	
	i = j = 0;
	
	Xw = (double*) calloc (numbers, sizeof(double));
	//index = (double*) malloc (numbers * sizeof(double));
	
	for(i = 0; i < numbers; i++) 
	{
		X[i] = (double*) malloc (dimensions * sizeof(double));
		fscanf(flabels_file, "%lf", &Y[i]);
		for(j = 0; j < dimensions; j++) 
		{
			fscanf(finput_file, "%lf", &X[i][j]);
			Xisquare[j] += (X[i][j] * X[i][j]);
			//printf("Xi = %lf\n", Xisquare[j]);
			Xiy[j] += X[i][j] * Y[j];
		}
	}
	
	computeXw();
	double start_time = monotonic_seconds();
	//computeXw();	
	computew();
	double end_time = monotonic_seconds();
	printf("Time required = %lf\n", end_time - start_time);
	return 0;
}

void computew() 
{
	
	double *yXwdiff = (double *) calloc (numbers, sizeof(double));
	double *Xwtemp = (double *) calloc (numbers, sizeof(double));
	double numerator; 
	for(int i = 0; i < iterations; i++) 
	{
		for (int j = 0; j < dimensions; j++) 
		{
			numerator = 0;
			//yXwdiff[numbers] = { 0 };
			//Xwtemp[numbers] = { 0 };
			yXwdiff = (double *) calloc (numbers, sizeof(double));
			Xwtemp = (double *)calloc (numbers, sizeof(double));
			for(int k = 0; k < numbers; k++) {
				Xwtemp[k] = Xw[k] - (X[k][j] * w[j]);
				yXwdiff[k] += Y[k] - Xwtemp[k];
				numerator += X[k][j] * yXwdiff[k];
			}
			//printf("numerator = %lf\n", numerator);
			w[j] = numerator / (Xisquare[j] + 1e-12);

			for (int k = 0; k < numbers; k++) {
				Xw[k] = Xwtemp[k] + (X[k][j] * w[j]);
			}
		}

		double loss = getLoss(); 
		printf("Iteration = %d Loss = %lf\n", i, loss);
	}
	
}

double getLoss() 
{
	double loss = 0;
	for(int i = 0; i < numbers; i++) {
		loss += ((Xw[i] - Y[i]) * (Xw[i] - Y[i]));
	}

	return loss;
}

void computeXw() 
{
	for(int i = 0; i < numbers; i++) {
		for (int j = 0; j < dimensions; j++) {
			Xw[i] += X[i][j] * w[j];		
		}
	}
}

static inline double monotonic_seconds()
{
	struct timespec ts;	
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec + ts.tv_nsec * 1e-9;
}
