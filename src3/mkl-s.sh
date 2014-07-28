g++ -O3 -I"$MKLROOT/include" -DBLAS_INTEL_MKL src3/*.cpp src3/*/*.cpp src3/*/*.c -lmkl_intel_lp64 -lmkl_intel_thread -lmkl_core -liomp5 -lpthread -lboost_regex -lboost_program_options  -o eval-mkl
