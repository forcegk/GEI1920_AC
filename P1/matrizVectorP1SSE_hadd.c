#include <assert.h>
#include <limits.h>
#include <math.h>
#include <stdlib.h>
#include <stdio.h>
#include <sys/time.h>

#include <pmmintrin.h>

int main( int argc, char *argv[] ) {
    int m, n, test, i, j;
    float alfa;
    struct timeval t0, t1, t;

    // Parámetro 1 -> m
    // Parámetro 2 -> n
    // Parámetro 3 -> alfa
    // Parámetro 4 -> booleano que nos indica si se desea imprimir matrices y vectores de entrada y salida
    if(argc>3){
        m = atoi(argv[1]);
        n = atoi(argv[2]);
        alfa = atof(argv[3]);
        test = atoi(argv[4]);
    }
    else{
        printf("NUMERO DE PARAMETROS INCORRECTO\n");
        exit(0);
    }

    int n_high = (((int)n/4)*4)+4;

    float *x = (float *) _mm_malloc(n_high*sizeof(float),   16);
    float *A = (float *) _mm_malloc(m*n_high*sizeof(float), 16);
    float *y = (float *) _mm_malloc(m*sizeof(float),        16);

    // Se inicializan la matriz y los vectores

    // dejamos sin inicializar el rango entre n y n_high
    for(i=0; i<m; i++){
        for(j=0; j<n; j++){
            A[i*n_high+j] = 1+i+j;
        }
    }

    // aqui si que ponemos a cero la diferencia
    for(i=0; i<n_high; i++){
        x[i] = i<n? (1+i):0;
    }

    for(i=0; i<m; i++){
        y[i] = (1-i);
    }

    if(test){
        printf("\nMatriz A es...\n");
        for(i=0; i<m; i++){
            for(j=0; j<n; j++){
                printf("%f ", A[i*n_high+j]);
            }
            printf("\n");
        }

        printf("\nVector x es...\n");
        for(i=0; i<n; i++){
            printf("%f \n", x[i]);
        }
        printf("\n");

        printf("\nVector y al principio es...\n");
        for(i=0; i<m; i++){
            printf("%f \n", y[i]);
        }
        printf("\n");
    }


    /* Declaración de variables para SSE */

    __m128 y_i;
    __m128 x_j;
    float * temp = (float *) _mm_malloc(4*sizeof(float), 16);
    __m128 alfa_v = _mm_set_ps(alfa, alfa, alfa, alfa);

    /* end variables SSE*/

    // Parte fundamental del programa
    assert (gettimeofday (&t0, NULL) == 0);


    /* OPTIMIZAMOS AQUÍ */

    /*
    for (i=0; i<m; i++) {
        for (j=0; j<n; j++) {
            y[i] += alfa*A[i*n+j]*x[j];
        }
    }
    */
    
    for (i=0; i<m; i++){

        for (j=0; j<n; j+=4) {

            // Cargamos los datos
            y_i = _mm_load_ps(&(A[(i*n_high)+j]));
            x_j = _mm_load_ps(&(x[j]));
            

            // Multiplicamos las partes de la matriz
            y_i = _mm_mul_ps(y_i, x_j);

            // Multiplicamos el resultado por el alfa
            y_i = _mm_mul_ps(y_i, alfa_v);

            // Aqui tenemos ya los cuatro elementos


            // Reducimos, usando hadd
            // [1 2 3 4]  [x y z t]
            //          \/ hadd
            //   [1+2 3+4 x+y z+t]
            //   [ 3   7   ~   ~]

            // repetimos
            // [3 7 ~ ~]  [~ ~ ~ ~]
            //          \/ hadd
            //   [3+7  ~   ~   ~ ]
            //   [ 10  ~   ~   ~ ]
            
            // escribimos a un array de 4 floats
            // y sumamos el primer campo a y[i]

            //                                     usamos alfa_v por poner algo
            _mm_store_ps(temp, _mm_hadd_ps(_mm_hadd_ps(y_i, alfa_v), alfa_v));
            y[i] += temp[0];

        }
        
    }

	
    /* HASTA AQUÍ */

    assert (gettimeofday (&t1, NULL) == 0);

    timersub(&t1, &t0, &t);

    if(test){
        printf("\nAl final vector y es...\n");
        for(i=0; i<m; i++){
            printf("%f \n", y[i]);
        }
        printf("\n");

        float *tasty = (float *) malloc(m*sizeof(float));
        for(i=0; i<m; i++){
            tasty[i] = 1-i;
        }

        // Se calcula el producto sin ninguna vectorización
        for (i=0; i<m; i++) {
            for (j=0; j<n; j++) {
                tasty[i] += alfa*A[i*n_high+j]*x[j];
            }
        }

        int errores = 0;
        for(i=0; i<m; i++){
            if(tasty[i] != *(y+i)){
                errores++;
                printf("\n Error en la posicion %d porque %f != %f", i, *(y+i), tasty[i]);
            }
        }
        printf("\n%d errores en el producto matriz vector con dimensiones %dx%d\n", errores, m, n);
        free(tasty);
    }

    printf ("Tiempo      = %ld:%ld(seg:mseg)\n", t.tv_sec, t.tv_usec/1000);

    _mm_free(x);
    _mm_free(y);
    _mm_free(A);
    
    _mm_free(temp);

    return 0;
}