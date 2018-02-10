#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <omp.h>

#define MAX_THREADS	16

int dimensions;
int numbers;
int iterations; 
int num_threads;

int curr_dim;

double** X;
double* w;
double* Y;

double* Xw;
double* Xisquare;
double* Xiy;

double *Xwtemp;
double *yXwdiff;

//omp_lock_t lock;

int threads[MAX_THREADS];
double numerator[MAX_THREADS];
//double loss[MAX_THREADS];
//double loss;

void computew();
void computeXw();

double getLoss(int);

double getNumerator(int);

void computeXw_parallel(int);

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
	
	//numbers = 1000;

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

	printf("Data read\n");	
	
	double start_time = monotonic_seconds();
		
	computeXw();
	
	//double start_time = monotonic_seconds();
	//computeXw();	
	computew();
	double end_time = monotonic_seconds();
	printf("Time required = %lf\n", end_time - start_time);
	return 0;
}

void computew() 
{
	
	//double *yXwdiff;
	
	Xwtemp = (double *) calloc (numbers, sizeof(double));
	//yXwdiff = (double *) calloc (numbers, sizeof(double));
	int thread_no;
	double numerator;
 
	for(int i = 0; i < iterations; i++) 
	{
		for (int j = 0; j < dimensions; j++) 
		{
			numerator = 0;
			
			curr_dim = j;
		
			yXwdiff = (double *) calloc (numbers, sizeof(double));
			
			/*for(int k = 0; k < numbers; k++) {
				Xwtemp[k] = Xw[k] - (X[k][j] * w[j]);
				yXwdiff[k] += Y[k] - Xwtemp[k];
				numerator += X[k][j] * yXwdiff[k];
			}*/

			//int thread_no;
			omp_set_dynamic(0);
			omp_set_num_threads(num_threads);
			//omp_lock_t lock;
			//omp_init_lock(&lock);
			double numerator = 0;
			#pragma omp parallel for \
			reduction(+ : numerator)
			for (int iter_thread = 0; iter_thread < num_threads; iter_thread++) {
				thread_no = omp_get_thread_num();	
				double num = getNumerator(thread_no);
				numerator += num;
			}
			
			w[j] = numerator / (Xisquare[j] + 1e-12);
			
			free(yXwdiff);
			int num_per_thread = (int) numbers/ num_threads;
			
			//curr_dim = j;		
	
			omp_set_dynamic(0);
			omp_set_num_threads(num_threads);
				
			//int thread_no;		
			#pragma omp parallel for
			for(int iter_thread = 0; iter_thread < num_threads; iter_thread++) {
				thread_no = omp_get_thread_num();
				computeXw_parallel(thread_no);
			}

		}
		
		thread_no = 0; 
		double total_loss = 0;
		
		omp_set_dynamic(0);
		omp_set_num_threads(num_threads);
		
		double loss = 0;
		#pragma omp parallel for \
		reduction(+ : loss)
		for(int i = 0; i < num_threads; i++) {
			thread_no = omp_get_thread_num();
			double l = getLoss(thread_no);
			//omp_set_lock(&lock);	
			loss += l;
			//omp_unset_lock(&lock);
		}
		
		/*for(int iter_num = 0; iter_num < num_threads; iter_num++) {
			total_loss += loss[thread_no];
		}*/
		 
		printf("Iteration = %d Loss = %lf\n", i, loss);
	}
	
}

double getNumerator(int thread_no) {
	int num_per_thread = numbers/num_threads;
	int j = curr_dim;
	
	int start = thread_no * num_per_thread;
	int end = start + num_per_thread;
	
	if(thread_no == num_threads - 1) {
		end = numbers;
	}
	
	//numerator[thread_no] = 0;
	int i;
	double numerator = 0;
	for(i = start; i < end; i++) {
		Xwtemp[i] = Xw[i] - (X[i][j] * w[j]);
		yXwdiff[i] += Y[i] - Xwtemp[i];
		numerator += X[i][j] * yXwdiff[i];
	}
	
	return numerator;
}

void computeXw_parallel(int thread_no) {
		
	int num_per_thread = numbers/num_threads;
	int j = curr_dim;
	
	int start = thread_no * num_per_thread;
	int end = start + num_per_thread;
	
	if(thread_no == num_threads - 1) {
		end = numbers;
	}

	//printf("start = %d end = %d\n", start, end);	
	for(int i = start; i < end; i++) {
		Xw[i] = Xwtemp[i] + (X[i][j] * w[j]);
	}
		
}

double getLoss(int thread_no) 
{
	int num_per_thread = numbers/num_threads;
	int start = thread_no * num_per_thread;
	int end = start + num_per_thread;
	
	if(thread_no == num_threads - 1) {
		end = numbers;
	}
	
	//loss[thread_no] = 0;
	//omp_lock_t lock;
	//omp_init_lock(&lock);
	double loss;
	//omp_set_lock(&lock);
	for(int i = start; i < end; i++) {
		loss += ((Xw[i] - Y[i]) * (Xw[i] - Y[i]));
	}
	//omp_unset_lock(&lock);
	return loss;
	//return loss[thread_no];
	//printf("Loss = %lf\n", loss[thread_no]);
}

void computeXw() {
	
	//int num_per_thread = numbers/num_threads;
	//int start = thread_no * num_per_thread;
	//int end = start + num_per_thread;
	//printf("thread no = %d, start = %d end = %d\n", thread_no, start, end);
	
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
