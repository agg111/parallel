lc_pthreads:
	gcc lc_pthreads.c -o lc_pthreads -pthread

lc_openmp:
	gcc lc_openmp.c -o lc_openmp -fopenmp

clean:
	-rm lc_pthreads
	-rm lc_openmp
