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
double loss;

double** X;
double* w;
double* Y;

double* Xw;
double* Xisquare;
double* Xiy;

double *Xwtemp;
double *yXwdiff;
int threads[MAX_THREADS];
double numerator[MAX_THREADS];
//double loss[MAX_THREADS];

pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void computew();
void computeXw();
double getLoss();

void *computeNumerator_parallel(void *);

void *computeXw_parallel(void *);

void *getLoss_parallel(void *);

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
	
	for(i = 0; i < numbers; i++) 
	{
		X[i] = (double*) malloc (dimensions * sizeof(double));
		fscanf(flabels_file, "%lf", &Y[i]);
		for(j = 0; j < dimensions; j++) 
		{
			fscanf(finput_file, "%lf", &X[i][j]);
			Xisquare[j] += (X[i][j] * X[i][j]);
			Xiy[j] += X[i][j] * Y[j];
		}
	}

	printf("Data read in\n");
	
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
	
	Xwtemp = (double *) calloc (numbers, sizeof(double));
	
	pthread_t p_threads[MAX_THREADS];
	pthread_attr_t attr;
	pthread_attr_init(&attr);
	pthread_attr_setscope(&attr, PTHREAD_SCOPE_SYSTEM);

	
	//double numerator; 
	for(int i = 0; i < iterations; i++) {
		for (int j = 0; j < dimensions; j++) {
			//numerator = 0;
			yXwdiff = (double *) calloc (numbers, sizeof(double));
			
			curr_dim = j;
			
			for(int iter_thread = 0; iter_thread < num_threads; iter_thread++) {
				threads[iter_thread] = iter_thread;
				pthread_create(&p_threads[iter_thread], &attr, computeNumerator_parallel, (void *) &threads[iter_thread]);
			}
			
			double the_numerator = 0;
			for(int iter_thread = 0; iter_thread < num_threads; iter_thread++) {
				//the_numerator += numerator[iter_thread];
				pthread_join(p_threads[iter_thread], NULL);
				the_numerator += numerator[iter_thread];
			}
			
			/*for(int k = 0; k < numbers; k++) {
				Xwtemp[k] = Xw[k] - (X[k][j] * w[j]);
				yXwdiff[k] += Y[k] - Xwtemp[k];
				numerator += X[k][j] * yXwdiff[k];
			}*/
			//double the_numerator = 0;
			/*for(int iter_thread = 0; iter_thread < num_threads; iter_thread++) {
				the_numerator += numerator[iter_thread];
			}*/	
			w[j] = the_numerator / (Xisquare[j] + 1e-12);
			
			free(yXwdiff);
			int num_per_thread = (int) numbers/num_threads;		
			
			for(int iter_thread = 0; iter_thread < num_threads; iter_thread++) {
				threads[iter_thread] = iter_thread;
				pthread_create(&p_threads[iter_thread], &attr, computeXw_parallel, (void *) &threads[iter_thread]);
			}
				 
			for(int iter_thread = 0; iter_thread < num_threads; iter_thread++) {
				pthread_join(p_threads[iter_thread], NULL);
			}
			
			/*for (int k = 0; k < numbers; k++) {
				Xw[k] = Xwtemp[k] + (X[k][j] * w[j]);
				//printf("k = %d Xw[k] = %lf\n", k, Xw[k]);
			}*/
		}
		
		//double loss = getLoss(); 
		//double total_loss = 0;
		loss = 0;
		
		/*if(pthread_mutex_init(&lock, NULL) != 0) {
			perror("mutex init failed\n");
		}*/
					
		for(int iter_thread = 0; iter_thread < num_threads; iter_thread++) {
			threads[iter_thread] = iter_thread;
			pthread_create(&p_threads[iter_thread], &attr, getLoss_parallel, (void *) &threads[iter_thread]);
		}
		for(int iter_thread = 0; iter_thread < num_threads; iter_thread++) {
			pthread_join(p_threads[iter_thread], NULL);
			//total_loss += loss[iter_thread];
		}
		
		
		printf("Iteration = %d Loss = %lf\n", i, loss);
	}
	
}

void *computeNumerator_parallel (void *ptr) {
	
	int *p = ptr;
	int start = *p;
	int thread_no = start;
	int end = 0;
	int num_per_thread = numbers/num_threads;
	
	int j = curr_dim;
	start = thread_no * num_per_thread;

	//printf("Thread no - %d\n", thread_no);
	
	if(thread_no == num_threads - 1) {
		end = numbers;
	}
	else {
		end = start + num_per_thread;
	}
	
	//double *yXwdiff = (double *) calloc (num_per_thread, sizeof(double));
	numerator[thread_no] = 0;	
	
	//pthread_mutex_lock(&mutex);	
	for(int i = start; i < end; i++) {
		Xwtemp[i] = Xw[i] - (X[i][j] * w[j]);
		yXwdiff[i] += Y[i] - Xwtemp[i];
		//pthread_mutex_lock(&mutex);
		numerator[thread_no] += X[i][j] * yXwdiff[i];
		//pthread_mutex_unlock(&mutex);
	}
	//pthread_mutex_unlock(&mutex);

	//free(yXwdiff);

}

void *computeXw_parallel(void *ptr) {
	int *p = ptr;
	int start = *p;
	int thread_no = start;
	int end = 0;
		
	int num_per_thread = numbers/num_threads;
	int j = curr_dim;

	start = thread_no * num_per_thread;

	if(thread_no == num_threads - 1) {
		end = numbers;
	}
	else {
		end = start + num_per_thread;
	}

	//printf("start = %d end = %d\n", start, end);	
	for(int i = start; i < end; i++) {
		Xw[i] = Xwtemp[i] + (X[i][j] * w[j]);
	}
		
}

void *getLoss_parallel(void *ptr) {
	int *p = ptr;
	int start = *p;
	int thread_no = start;
	int end = 0;
	
	int num_per_thread = numbers/num_threads;
	
	start = thread_no * num_per_thread;
	
	if(thread_no == num_threads - 1) {
		end = numbers;
	}
	else {
		end = start + num_per_thread;
	}

	pthread_mutex_lock(&mutex);
	for(int i = start; i < end; i++) {
		//pthread_mutex_lock(&mutex);
		loss += ((Xw[i] - Y[i]) * (Xw[i] - Y[i]));
		//pthread_mutex_unlock(&mutex);
	}
	pthread_mutex_unlock(&mutex);
}


double getLoss() {
	double loss = 0;
	for(int i = 0; i < numbers; i++) {
		loss += ((Xw[i] - Y[i]) * (Xw[i] - Y[i]));
	}

	return loss;
}

void computeXw() {
	for(int i = 0; i < numbers; i++) {
		for (int j = 0; j < dimensions; j++) {
			Xw[i] += X[i][j] * w[j];		
		}
	}
}

static inline double monotonic_seconds() {
	struct timespec ts;	
	clock_gettime(CLOCK_MONOTONIC, &ts);
	return ts.tv_sec + ts.tv_nsec * 1e-9;
}
