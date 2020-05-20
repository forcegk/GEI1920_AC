/**
*   NOMBRE:      Alonso Rodriguez Iglesias
*   EMAIL:       alonso.rodriguez@udc.es
*   DNI:         49330318X
*   COMPILACION: makefile (CC: mpicc)
*   EJECUCION:   mpirun --oversubscribe -np numprocs ./main m k n alfa test debug time
*
*   MPI:         openmpi 4.0.3-1 [https://www.archlinux.org/packages/extra/x86_64/openmpi/]
*   ENTORNO:     Linux 5.6.13-1-ck-ivybridge #1 SMP PREEMPT x86_64 GNU/Linux
*   DISTRO:      Arch Linux Rolling
*
*   CPU:         Intel(R) Core(TM) i5-3230M CPU @ 2.60GHz
*   RAM:         7,6 GiB DDR3 @ 1600 MHz
*/

#define BCAST_PACKED
//#define BCAST_STRUCT

#define COMM_SUBTOPO
//#define COMM_SPLIT

//#define DO_SORT_OUTPUT

//#define DEBUG

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <mpi.h>
#include <math.h>

int isPerfectSquare(long double x){
  long double sr = sqrt(x); 
  return ((sr - floor(sr)) == 0);
} 

int main(int argc, char *argv[]) {

    int numprocs, sqrtnumprocs, rank, comm_fila_rank, comm_columna_rank, i, j, l, m, n, k, test, debug, time;
    int temp_int;
    float alfa;
    double t_exec;

    // Inicializamos MPI
    MPI_Init(&argc, &argv);
    // Determinar el rango del proceso invocado
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    // Determinar el numero de procesos
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    sqrtnumprocs = (int) sqrt(numprocs);

    // Proceso 0 :: LECTURA DE PARAMETROS
    // Parametro 1 -> m
    // Parametro 2 -> k
    // Parámetro 3 -> n
    // Parámetro 4 -> alfa
    // Parámetro 5 -> booleano que nos indica si se desea imprimir matrices y vectores de entrada y salida
    // Parámetro 6 -> booleano que nos indica si se desea comprobar que el resultado es correcto
    // Parámetro 7 -> booleano que nos indica si se desea medir (e imprimir) el tiempo
    if(!rank){
        if(argc>7){
            m = atoi(argv[1]);
            k = atoi(argv[2]);
            n = atoi(argv[3]);
            alfa  = atof(argv[4]);
            test  = atoi(argv[5]);
            debug = atoi(argv[6]);
            time  = atoi(argv[7]);
        }
        else{
            fprintf(stderr, "NUMERO DE PARAMETROS [argc=%d] INCORRECTO\n", argc);
            fprintf(stderr, "Usage:\n"
                            "  mpirun -np <num_procs> ./main m k n alfa test debug time\n\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }

        // Testear si soportamos el número de procesos
        if(!isPerfectSquare(numprocs)){
            fprintf(stderr, "\nNO SOPORTADO: NUMERO DE PROCESOS NO ES CUADRADO PERFECTO\n\n");
            fflush(stdout);
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }

        // Mostrar m k n y posibles errores
        const char arrow[] = "  <-- tamaño no válido";
        char invalid_mult[28]; sprintf(invalid_mult, "  <-- no es múltiplo de %d", sqrtnumprocs);
        fprintf(stderr, "-> m=%d%s%s\n-> k=%d%s%s\n-> n=%d%s%s\n",
                  m, m<=0?arrow:"",(m%sqrtnumprocs)?invalid_mult:"",
                  k, k<=0?arrow:"",(k%sqrtnumprocs)?invalid_mult:"",
                  n, n<=0?arrow:"",(n%sqrtnumprocs)?invalid_mult:""
                );
        fflush(stderr);
        
        // Testear m k n 
        if((m<=0) || (k<=0) || (n<=0)){
            fprintf(stderr, "\nTAMAÑO NO VALIDO\n\n");
            fflush(stdout);
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }

        // Testeamos si la matriz es divisible equitativamente
        if((m%sqrtnumprocs) || (k%sqrtnumprocs) || (n%sqrtnumprocs)){
            fprintf(stderr, "\nNO SOPORTADO: TAMAÑO NO MULTIPLO DE %d\n\n", sqrtnumprocs);
            fflush(stdout);
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }

    }

    /*** INICIO BROADCAST DE PARÁMETROS ***/
    #if defined BCAST_PACKED
        #undef BCAST_STRUCT

        int buffer_size, posicion = 0;
        MPI_Pack_size(4, MPI_INT, MPI_COMM_WORLD, &buffer_size);
        MPI_Pack_size(1, MPI_FLOAT, MPI_COMM_WORLD, &temp_int);
        buffer_size += temp_int;

        uint8_t buffer[buffer_size];

        if(!rank){
            MPI_Pack(&m, 1, MPI_INT, buffer, buffer_size, &posicion, MPI_COMM_WORLD);
            MPI_Pack(&k, 1, MPI_INT, buffer, buffer_size, &posicion, MPI_COMM_WORLD);
            MPI_Pack(&n, 1, MPI_INT, buffer, buffer_size, &posicion, MPI_COMM_WORLD);
            MPI_Pack(&alfa, 1, MPI_FLOAT, buffer, buffer_size, &posicion, MPI_COMM_WORLD);
            MPI_Pack(&time, 1, MPI_INT, buffer, buffer_size, &posicion, MPI_COMM_WORLD);
        }
        MPI_Bcast(buffer, buffer_size, MPI_PACKED, 0, MPI_COMM_WORLD);
        if(rank){
            MPI_Unpack(buffer, buffer_size, &posicion, &m, 1, MPI_INT, MPI_COMM_WORLD);
            MPI_Unpack(buffer, buffer_size, &posicion, &k, 1, MPI_INT, MPI_COMM_WORLD);
            MPI_Unpack(buffer, buffer_size, &posicion, &n, 1, MPI_INT, MPI_COMM_WORLD);
            MPI_Unpack(buffer, buffer_size, &posicion, &alfa, 1, MPI_FLOAT, MPI_COMM_WORLD);
            MPI_Unpack(buffer, buffer_size, &posicion, &time, 1, MPI_INT, MPI_COMM_WORLD);
        }
    #elif defined BCAST_STRUCT
        #undef BCAST_PACKED
        // TODO MPI_Type_Struct y hacer Bcast de él
        typedef struct _params {
            int m, k, n, time;
            float alfa;
        } params;

        params ps;

        int lengths[] = {4, 1};
        MPI_Aint offsets[2];
        MPI_Datatype oldtypes[] = {MPI_INT, MPI_FLOAT};

        MPI_Get_address(&(ps.m), &(offsets[0]));
        MPI_Get_address(&(ps.alfa), &(offsets[1]));

        offsets[1] = offsets[1] - offsets[0];
        offsets[0] = 0;

        MPI_Datatype type_params;
        MPI_Type_create_struct(2, lengths, offsets, oldtypes, &type_params);
        MPI_Type_commit(&type_params);

        // Escribimos los valores a la struct
        if(!rank){
            ps.m = m;
            ps.k = k;
            ps.n = n;
            ps.time = time;
            ps.alfa = alfa;
        }

        // Le hacemos broadcast
        MPI_Bcast(&ps, 1, type_params, 0, MPI_COMM_WORLD);

        // Leemos los valores en el resto de procesos
        if(rank){
            m = ps.m;
            k = ps.k;
            n = ps.n;
            time = ps.time;
            alfa = ps.alfa;
        }

        // Ya liberamos aquí el tipo, ya que no lo vamos a volver a usar
        MPI_Type_free(&type_params);
    #else
        if(!rank){
            fprintf(stderr, "\n\nNo hay mecanismo de difusión de parámetros definido!\n"
                            "Quizás olvidaste #define BCAST_PACKED o #define BCAST_STRUCT\n\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }
    #endif
    /*** FIN BROADCAST DE PARÁMETROS ***/

    /*** INICIO INICIALIZACION DE MATRICES ***/
    float      *A,      *B,      *C;
    float *localA, *localB, *localC;
    float   *bufA,   *bufB;

    if(!rank){
        // Proceso 0 inicializa las matrices A, B y C
        A = malloc(m*k*sizeof(float));
        B = malloc(k*n*sizeof(float));
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

        if(debug){
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
    /*** FIN INICIALIZACION DE MATRICES ***/


    /*** INICIO DEFINICIÓN DE COMUNICADORES ***/
    MPI_Comm comm_fila;
    MPI_Comm comm_columna;

    #if defined COMM_SUBTOPO
        #undef COMM_SPLIT
        MPI_Comm comm_malla;
        
        int ndims = 2;
        int dims[] = {sqrtnumprocs, sqrtnumprocs}; // aquí podríamos usar  MPI_Dims_create
        int periods[] = {0, 0};                    // pero por comodidad voy a hacer así.
        
        MPI_Cart_create(MPI_COMM_WORLD, ndims, dims, periods, 0, &comm_malla);

        int comm_fila_freedims[] = {0, 1};
        int comm_columna_freedims[] = {1, 0};
        MPI_Cart_sub(comm_malla, comm_fila_freedims, &comm_fila);
        MPI_Cart_sub(comm_malla, comm_columna_freedims, &comm_columna);
    #elif defined COMM_SPLIT
        #undef COMM_SUBTOPO
        int q, p;
        p = rank/sqrtnumprocs;
        q = rank%sqrtnumprocs;

        MPI_Comm_split(MPI_COMM_WORLD, p, q, &comm_fila);
        MPI_Comm_split(MPI_COMM_WORLD, q, p, &comm_columna);

    #else
        if(!rank){
            fprintf(stderr, "\n\nNo hay mecanismo de creación de topología de filas y columnas definido!\n"
                            "Quizás olvidaste #define COMM_SUBTOPO o #define COMM_SPLIT\n\n");
            MPI_Abort(MPI_COMM_WORLD, 1);
            return 1;
        }
    #endif

    //guardamos los ranks en fila y columna
    MPI_Comm_rank(comm_columna, &comm_columna_rank);
    MPI_Comm_rank(comm_fila, &comm_fila_rank);

    /*** FIN DEFINICIÓN DE COMUNICADORES ***/

    /*** INICIO BROADCAST DE MATRICES ***/
    int mpp = m/sqrtnumprocs; // mpp = m per process
    int kpp = k/sqrtnumprocs; // kpp = k per process
    int npp = n/sqrtnumprocs; // npp = n per process

    #ifdef DEBUG
        #if defined COMM_SUBTOPO
        int comm_malla_coords[ndims];
        MPI_Cart_get(comm_malla, ndims, dims, periods, comm_malla_coords);
        fprintf(stderr, "Proceso #%d => [%d,%d]\n", rank, comm_malla_coords[0], comm_malla_coords[1]);
        #endif
    #endif

    MPI_Datatype type_submatrix_A;
    MPI_Datatype type_submatrix_B;
    MPI_Datatype type_submatrix_C;
    if(!rank){
        MPI_Type_vector(mpp, kpp, k, MPI_FLOAT, &type_submatrix_A);
        MPI_Type_vector(kpp, npp, n, MPI_FLOAT, &type_submatrix_B);
        MPI_Type_vector(mpp, npp, n, MPI_FLOAT, &type_submatrix_C);
        MPI_Type_commit(&type_submatrix_A);
        MPI_Type_commit(&type_submatrix_B);
        MPI_Type_commit(&type_submatrix_C);
    }

    // Reservamos espacio para las submatrices
    localA = malloc(mpp*kpp*sizeof(float));
    localB = malloc(kpp*npp*sizeof(float));
    localC = calloc(npp*mpp,sizeof(float)); // inicializamos a 0

    // y tambien para las matrices buffer
    bufA = malloc(mpp*kpp*sizeof(float));
    bufB = malloc(kpp*npp*sizeof(float));

    /** MEDICIÓN DEL TIEMPO INICIAL **/
    if(time){
        MPI_Barrier(MPI_COMM_WORLD);
        t_exec = MPI_Wtime();
    }
    /*********************************/

    // Enviamos las matrices
    if(!rank){
        for(i = 1; i<numprocs; i++){
            MPI_Send(A+i*kpp+(i/sqrtnumprocs)*(mpp-1)*k, 1, type_submatrix_A, i, 0, MPI_COMM_WORLD);
                //   A                          -> posición inicial
                //   i*kpp                      -> padding en la misma fila
                //   (i/sqrtnumprocs)*(mpp-1)*k -> padding entre filas de bloques
            MPI_Send(B+i*npp+(i/sqrtnumprocs)*(kpp-1)*n, 1, type_submatrix_B, i, 0, MPI_COMM_WORLD);
        }

        // Y copiar su zona desde A a localA  ::  TODO IMPROVE MEMORY USAGE BY USING CUSTOM PADDING ON ACCESS FOR RANK 0
        for(i = 0; i<mpp; i++){
            memcpy(localA+i*kpp, A+i*k, kpp*sizeof(float));
        }
        // Igual con localB
        for(i = 0; i<kpp; i++){
            memcpy(localB+i*npp, B+i*n, npp*sizeof(float));
        }
    }else{
        // receive
        MPI_Recv(localA, mpp*kpp, MPI_FLOAT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        MPI_Recv(localB, kpp*npp, MPI_FLOAT, MPI_ANY_SOURCE, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
    }

    #ifdef DEBUG
    for(temp_int = 0; temp_int<numprocs; temp_int++){
        if(rank == temp_int){
            printf("\n#%d :: SubMatriz A es...\n", temp_int);
                for(i=0; i<mpp; i++){
                    for(j=0; j<kpp; j++){
                        printf("%f ", localA[i*kpp+j]);
                    }
                    printf("\n");
                }
                fflush(stdout);
            printf("\n#%d :: SubMatriz B es...\n", temp_int);
                for(i=0; i<kpp; i++){
                    for(j=0; j<npp; j++){
                        printf("%f ", localB[i*npp+j]);
                    }
                    printf("\n");
                }
                fflush(stdout);
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
    #endif

    /*** FIN BROADCAST DE MATRICES ***/


    /*** INICIO ALGORITMO ***/

    for(temp_int = 0; temp_int<sqrtnumprocs; temp_int++){

        if(comm_fila_rank == temp_int){
            // Copiamos el localA a bufA
            memcpy(bufA, localA, mpp*kpp*sizeof(float));
        }
        if(comm_columna_rank == temp_int){
            // Idem para bufB
            memcpy(bufB, localB, kpp*npp*sizeof(float));
        }

        // Difusión
        MPI_Bcast(bufA, mpp*kpp, MPI_FLOAT, temp_int, comm_fila);
        MPI_Bcast(bufB, kpp*npp, MPI_FLOAT, temp_int, comm_columna);

        // Calculamos los resultados en localC
        // IMPORTANTE C TIENE QUE ESTAR INICIALIZADA A CEROS
        for(i=0; i<mpp; i++){
            for(j=0; j<npp; j++){
                for(l=0; l<kpp; l++){
                    localC[i*npp+j] += alfa*bufA[i*kpp+l]*bufB[l*npp+j];
                }
            }
        }
        
       
        #ifdef DEBUG
        MPI_Barrier(MPI_COMM_WORLD);
        for(int asdf = 0; asdf<numprocs; asdf++){
            if(rank == asdf){
                printf("\nIt.%d, #%d :: bufA es...\n", temp_int, asdf);
                    for(i=0; i<mpp; i++){
                        for(j=0; j<kpp; j++){
                            printf("%f ", bufA[i*kpp+j]);
                        }
                        printf("\n");
                    }
                    fflush(stdout);
                printf("\nIt.%d, #%d :: bufB es...\n", temp_int, asdf);
                    for(i=0; i<kpp; i++){
                        for(j=0; j<npp; j++){
                            printf("%f ", bufB[i*npp+j]);
                        }
                        printf("\n");
                    }
                    fflush(stdout);
            }
            MPI_Barrier(MPI_COMM_WORLD);
        }
        #endif

    }

    #ifdef DEBUG
    MPI_Barrier(MPI_COMM_WORLD);
    for(int asdf = 0; asdf<numprocs; asdf++){
        if(rank == asdf){
            printf("\nIt.%d, #%d :: SubMatriz C es...\n", temp_int, asdf);
                for(i=0; i<mpp; i++){
                    for(j=0; j<npp; j++){
                        printf("%f ", localC[i*npp+j]);
                    }
                    printf("\n");
                }
                fflush(stdout);
        }
        MPI_Barrier(MPI_COMM_WORLD);
    }
    #endif

    // Y recogemos y combiamos las matrices
    if(!rank){
        // Y copiar su zona desde localC a C
        for(i = 0; i<mpp; i++){
            memcpy(C+i*n, localC+i*npp, npp*sizeof(float));
        }

        for(i = 1; i<numprocs; i++){
            // recibimos y colocamos en su sitio
            MPI_Recv(C+i*npp+(i/sqrtnumprocs)*(mpp-1)*n, 1, type_submatrix_C, i, 0, MPI_COMM_WORLD, MPI_STATUS_IGNORE);
        }
    }else{
        MPI_Send(localC, mpp*npp, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
    }

    // Aquí recogemos la segunda medición del tiempo
    if(time){
        t_exec = MPI_Wtime() - t_exec;
    }

    /*** TEST POST CALCULO ***/
    if(!rank){
        if(debug){
            printf("\nMatriz C es...\n");
            for(i=0; i<m; i++){
                for(j=0; j<n; j++){
                    printf("%f ", C[i*n+j]);
                }
                printf("\n");
            }
            fflush(stdout);
        }

        printf("\n");
        
        if(test){
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
    }

    // Para que no se superponga la impresión de las matrices con los tiempos
    if(time){
        #ifdef DO_SORT_OUTPUT
        for(i=0; i<numprocs; i++){
        #endif
            MPI_Barrier(MPI_COMM_WORLD);

            #ifdef DO_SORT_OUTPUT
            if(rank == i){
            #endif
                fprintf(stderr, "Tiempo de ejecución del proceso #%d: %lf\n", rank, t_exec);
                fflush(stdout);
            #ifdef DO_SORT_OUTPUT  
            }    
        }
        #endif
    }

    /*** FIN TEST ***/

    // WELCOME TO THE FREE ZONE //

    // Global
    free(localA);
    free(localB);
    free(localC);
    free(bufA);
    free(bufB);

    // Free MPI_Type and MPI_Comm
    MPI_Comm_free(&comm_fila);
    MPI_Comm_free(&comm_columna);

    #if defined BCAST_PACKED
    #elif defined BCAST_STRUCT
    // Mejor lo liberamos tras usarlo, así no lo llevamos hasta
    //  el final del programa
    // MPI_Type_free(&type_params);
    #endif
    
    #if defined COMM_SUBTOPO
    MPI_Comm_free(&comm_malla);
    #elif defined COMM_SPLIT
    #endif

    //Free rank 0 only
    if(!rank){
        free(A);
        free(B);
        free(C);

        MPI_Type_free(&type_submatrix_A);
        MPI_Type_free(&type_submatrix_B);
        MPI_Type_free(&type_submatrix_C);
    }

    MPI_Finalize();
    return 0;
}