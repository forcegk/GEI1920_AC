#include <stdlib.h>
#include <stdio.h>
#include <mpi.h>

int main(int argc, char *argv[]) {

    int numprocs, rank, i, j, l, m, n, k, test, time;
    float alfa;
    double t_exec;

    // Inicializamos MPI
    MPI_Init(&argc, &argv);
    // Determinar el rango del proceso invocado
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // Determinar el numero de procesos
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);


    // Proceso 0 :: LECTURA DE PARAMETROS
    // Parametro 1 -> m
    // Parametro 2 -> k
    // Parámetro 3 -> n
    // Parámetro 4 -> alfa
    // Parámetro 5 -> booleano que nos indica si se desea imprimir matrices y vectores de entrada y salida
    // Parámetro 6 -> booleano que nos indica si se desea medir el tiempo
    if(!rank){
        if(argc>6){
            m = atoi(argv[1]);
            k = atoi(argv[2]);
            n = atoi(argv[3]);
            alfa = atof(argv[4]);
            test = atoi(argv[5]);
            time = atoi(argv[6]);
        }
        else{
            fprintf(stderr, "NUMERO DE PARAMETROS [argc=%d] INCORRECTO\n", argc);
            fprintf(stderr, "Usage:\n"
                            "  mpirun -np <num_procs> ./main m k n alfa test time\n\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 0;
        }

        char arrow[] = "  <-- error";
        fprintf(stderr, "-> m=%d%s\n-> k=%d%s\n-> n=%d%s\n", m, m<=0?arrow:"", k, k<=0?arrow:"", n, n<=0?arrow:"");
        fflush(stderr);
        
        if((m<=0) || (k<=0) || (n<=0)){
            fprintf(stderr, "\nTAMAÑO NO VALIDO\n\n");
            fflush(stdout);
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 0;
        }

    }


    // ENVIAMOS LOS DATOS AL RESTO DE PROCESOS
    MPI_Bcast(&m, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&k, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&n, 1, MPI_INT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&alfa, 1, MPI_FLOAT, 0, MPI_COMM_WORLD);
    MPI_Bcast(&time, 1, MPI_INT, 0, MPI_COMM_WORLD);


    float *B = malloc(k*n*sizeof(float));
    float *A, *C;

    if(!rank){
        // Proceso 0 inicializa las matrices A, B y C
        A = malloc(m*k*sizeof(float));
        C = malloc(n*m*sizeof(float));

        // Valores de A (m x k)
        for(i=0; i<m; i++){
            for(j=0; j<k; j++){
                A[i*k+j] = 1+i+j;
            }
        }

        // Valores de B (k x n)
        for(i=0; i<k; i++){
            for(j=0; j<n; j++){
                B[i*n+j] = 1+i+j;
            }
        }

        if(test){
            printf("\nMatriz A es...\n");
            for(i=0; i<m; i++){
                for(j=0; j<k; j++){
                    printf("%f ", A[i*k+j]);
                }
                printf("\n");
            }

            printf("\nMatriz B es...\n");
            for(i=0; i<k; i++){
                for(j=0; j<n; j++){
                    printf("%f ", B[i*n+j]);
                }
                printf("\n");
            }
            printf("\n");
        }
    }

    // Broadcast de B al resto de los procesos
    MPI_Bcast(B, k*n, MPI_FLOAT, 0, MPI_COMM_WORLD);

    int *rowarray, *send_countarray, *recv_countarray,
        *send_displarray, *recv_displarray, local_rows, local_send_count, local_recv_count;

    // Calculamos en proceso 0 las filas que corresponden a cada proceso
    if(!rank){
        int rows_over_numprocs = m/numprocs; //para evitar repetir esta operación
        int resto = m%numprocs;

        rowarray        = malloc(sizeof(int)*numprocs);
        send_countarray = malloc(sizeof(int)*numprocs);
        recv_countarray = malloc(sizeof(int)*numprocs);
        send_displarray = malloc(sizeof(int)*numprocs);
        recv_displarray = malloc(sizeof(int)*numprocs);

        for(i=0; i<numprocs; i++){
            // Suma rows_over_numprocs mas el 1 si el resto es mayor que 0
            rowarray[i] = rows_over_numprocs + (resto-->0);

            // Calcular el numero de items entre todas las filas
            send_countarray[i] = rowarray[i] * k;
            recv_countarray[i] = rowarray[i] * n;
        }

        // El desplazamiento base es 0
        send_displarray[0] = 0;
        recv_displarray[0] = 0;
        for(i=1; i<numprocs; i++){
            //Calculamos el desplazamiento relativo a base en cada proceso
            send_displarray[i] = send_displarray[i-1] + send_countarray[i-1];
            recv_displarray[i] = recv_displarray[i-1] + recv_countarray[i-1];
        }


        local_rows  = rowarray[0];
        local_send_count = send_countarray[0];
        local_recv_count = recv_countarray[0];

    } else {
        local_rows  = (m/numprocs) + (m%numprocs>rank);
        local_send_count = local_rows * k;
        local_recv_count = local_rows * n;
    }

    // Se ejecuta en todos los procesadores
    float *A_partial = malloc(local_rows*k*sizeof(float));

    // OJO MUY IMPORTANTE, HE PERDIDO UNA HORA Y MEDIA DE MI VIDA POR ESTO
    //  ¡¡¡¡¡¡¡INICIALIZAR LA ZONA A CEROS!!!!!!!
    float *C_partial = calloc(local_rows*n, sizeof(float));

    if(time){
        MPI_Barrier(MPI_COMM_WORLD);
        t_exec = MPI_Wtime();
    }

    MPI_Scatterv(A, send_countarray, send_displarray,
                 MPI_FLOAT, A_partial, local_send_count,
                 MPI_FLOAT, 0, MPI_COMM_WORLD);

    // Calculamos los resultados parciales
    for(i=0; i<local_rows; i++){
        for(j=0; j<n; j++){
            for(l=0; l<k; l++){
                C_partial[i*n+j] += alfa*A_partial[i*k+l]*B[l*n+j];
            }
        }
    }

    // Y recuperamos
    MPI_Gatherv(C_partial, local_recv_count, MPI_FLOAT,
                C, recv_countarray, recv_displarray,
                MPI_FLOAT, 0, MPI_COMM_WORLD);

    if(time){
        t_exec = MPI_Wtime() - t_exec;
    }


    /* TEST POST CALCULO */

    if(!rank){
        if(test){
            printf("\nMatriz C es...\n");
            for(i=0; i<m; i++){
                for(j=0; j<n; j++){
                    printf("%f ", C[i*n+j]);
                }
                printf("\n");
            }
            printf("\n");
        }

        int errores = 0;
        float temp_err;
        for (i=0; i<m; i++) {
            for(j=0; j<n; j++){
                temp_err = 0;

                for (l=0; l<k; l++) {
                    temp_err += alfa*A[i*k+l]*B[l*n+j];
                }

                if(temp_err != C[i*n+j]){
                    fprintf(stderr, "Error en posición [%d,%d]. Value=%f != %f=Expected\n", i, j, C[i*n+j], temp_err);
                    errores++;
                }
            }
        }
        fprintf(stderr, "\nErrores: %d\n\n", errores);
    }

    // Para que no se superponga la impresión de las matrices con los tiempos
    MPI_Barrier(MPI_COMM_WORLD);
    if(time){
        fprintf(stderr, "Tiempo de ejecución del proceso #%d: %lf\n", rank, t_exec);
        fflush(stdout);
    }

    // FREE ZONE //

    // Global
    free(B);
    free(A_partial);
    free(C_partial);

    if(!rank){
        free(A);
        free(C);
        free(rowarray);
        free(send_countarray);
        free(recv_countarray);
        free(send_displarray);
        free(recv_displarray);
    }

    MPI_Finalize();
    return 0;
}
