#include <stdio.h>
#include <mpi.h>
#include <math.h>
     
double calc_area(double left_size, double right_size, double function_between_left_and_right) {
    double dx = (right_size - left_size);
    double area = dx * function_between_left_and_right;
    return area;
}     

double func(double x) {
    return exp(x);
}

void main(int argc, char **argv)
{
    int ierr, num_procs, id;
    double result, result_total, left_size = 0.0, right_size = 4.0;
    
    ierr = MPI_Init(&argc, &argv);

    ierr = MPI_Comm_rank(MPI_COMM_WORLD, &id);
    ierr = MPI_Comm_size(MPI_COMM_WORLD, &num_procs);
    
    double dx = (left_size + right_size) / num_procs;
    double a = left_size + (dx * id);
    double b = left_size + (dx * (id + 1));
    //printf("a = %f b = %f\n", a, b);
    result = calc_area(a, b, func((a + b) / 2.0));
    
    printf("%d %f\n", id, result);
    
    MPI_Reduce(&result, &result_total, 1, MPI_DOUBLE, MPI_SUM, 0, MPI_COMM_WORLD);
    if (id == 0) {
        printf("%f\n", result_total);
    }
    
    ierr = MPI_Finalize();   
}
