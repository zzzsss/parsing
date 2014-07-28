gcc -c src3/cslm/Blas.c
g++ -O3 -DEVAL_MAXIMUM -DBLAS_ATLAS ./Blas.o src3/*/*.cpp src3/*.cpp -L/usr/lib/atlas-base -lboost_regex -lboost_program_options -lf77blas -o eval-atlas-maximux
