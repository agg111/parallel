Parallelizing linear classifier using POSIX and OpenMP

To compile the linear classifier code with pthreads - 
make lc_pthreads

The program takes data files, number of iterations and number of threads as command line arguments as follows - 
./lc_pthreads data_large.csv data_large.label no_of_iterations no_of_threads

For example, to run the lc_pthreads for 10 iterations and 1 thread, the command is as follow - 
./lc_pthreads /export/scratch/CSCI5451_S18/assignment-1/data_large.csv /export/scratch/CSCI5451_S18/assignment-1/data_large.label 10 1

For 10 iterations, 16 threads - 
./lc_pthreads /export/scratch/CSCI5451_S18/assignment-1/data_large.csv /export/scratch/CSCI5451_S18/assignment-1/data_large.label 10 16

To compile the linear claasifier code with openMP - 
make lc_openmp

The openmp code takes data file labels file, number of iterations and number of threads as commandline arguments as follows- 
./lc_openmp data_large.csv data_large.label no_of_iterations no_of_threads

For example, to run openmp code for 10 iterations and 1 thread, command should be as follows-
./lc_openmp /export/scratch/CSCI5451_S18/assignment-1/data_large.csv /export/scratch/CSCI5451_S18/assignment-1/data_large.label 10 1

For 10 iterations and 16 threads-
./lc_openmp /export/scratch/CSCI5451_S18/assignment-1/data_large.csv /export/scratch/CSCI5451_S18/assignment-1/data_large.label 10 16

