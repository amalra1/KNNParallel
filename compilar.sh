#!/bin/bash

   source /home/software/psxe_2020/parallel_studio_xe_2020.0.088/psxevars.sh

   mpicc teste01.c -o teste01
   mpicc teste02.c -o teste02
   mpicc teste03.c -o teste03
   mpicc teste04.c -o teste04
   mpicc teste05.c -o teste05
   mpicc teste06.c -o teste06
   mpicc teste07.c -o teste07
   mpicc teste08.c -o teste08
   mpicc teste09.c -o teste09  
   mpicc teste10.c -o teste10
   mpicc teste11.c -o teste11
   mpicc calc_pi_MPI.c -o calc_pi_MPI

echo "  **** COMPILACAO COMPLETA ****  " 
