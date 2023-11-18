#!/bin/bash

   source /home/software/psxe_2020/parallel_studio_xe_2020.0.088/psxevars.sh

   mpicc knn-mpi.c chrono.c -o knn-mpi -lm

echo "  **** COMPILACAO COMPLETA ****  " 
