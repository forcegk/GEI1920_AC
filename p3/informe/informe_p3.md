# INFORME AC P3 :: 2019-20
## Alonso Rodriguez Iglesias

### Datos
    NOMBRE:      Alonso Rodriguez Iglesias
    EMAIL:       alonso.rodriguez@udc.es
    DNI:         49330318X
    COMPILACION: makefile (CC: mpicc)
    EJECUCION:   mpirun --oversubscribe -np numprocs ./main m k n alfa test debug time

    MPI:         openmpi 4.0.3-1 [https://www.archlinux.org/packages/extra/x86_64/openmpi/]
    ENTORNO:     Linux 5.6.13-1-ck-ivybridge #1 SMP PREEMPT x86_64 GNU/Linux
    DISTRO:      Arch Linux Rolling

    CPU:         Intel(R) Core(TM) i5-3230M CPU @ 2.60GHz
    RAM:         7,6 GiB DDR3 @ 1600 MHz

### Cambios realizados
Esta práctica se apoya en la base de la anterior para implementar el algoritmo SUMMA.
Con respecto a la práctica anterior los cambios son los siguientes:
1. Ahora contamos con el parámetro debug, que nos permite indicar al programa si queremos mostrar las matrices A, B y C. Esta función antes la desempeñaba el parámetro test, que ahora ha pasado a tener un nombre con más sentido, y nos permite activar o desactivar si se realiza un test con respecto a la versión single-thread de la matriz C, ya que dependiendo del tamaño de la matriz, podía llevar una cantidad importante de tiempo.
2. Cambios generales en la estructura del programa, especialmente en la difusión de parámetros, matrices, y algoritmo empleado, que se exponen a continuación.

### Funcionamiento de la práctica
El funcionamiento de la práctica es el siguiente:
1. Leemos los parámetros, que se encuentran documentados en el propio código.
2. Verificamos que los parámetros son válidos. En caso de no serlo, abortamos el programa y mostramos un mensaje de error.
3. El proceso #0 distribuye los datos leidos por línea de comandos.
4. El proceso #0 inicializa las matrices.
5. El proceso #0 define los comunicadores fila y columna.
6. Cada proceso calcula mpp, kpp y npp (*pp = * por proceso // * per process) y reserva memoria para localA, localB, y localC, así como para bufA y bufB.
7. Cada proceso comienza su medición de tiempo.
8. El proceso #0 envía las submatrices correspondientes a cada proceso haciendo uso de un tipo vector de MPI, y copia su parte en local, sin hacerse un send a si mismo.
9. Se realiza el algoritmo. Para ello las difusiones se realizan empleando únicamente comunicadores fila y columna.
10. El proceso #0 recoge de vuelta los resultados de cada uno de los procesos.
11. Medimos el final de los tiempos de ejecución.
12. Si el flag test está a 1, comprobamos que el resultado de la multiplicación sea correcto.
13. Mostramos los tiempos de ejecución (comunicaciones y computaciones) por cada proceso.
14. Liberamos la memoria, tipos, comunicadores, etc reservados, restantes.

Las matrices se imprimen por stdout, y el resto de mensajes por stderr, así podemos ejecutar comandos del estilo

    mpirun -np 4 ./main m k n alfa test debug time > matrices.txt
y guardar los resultados a parte, obteniendo los tiempos y los errores en la salida stderr de terminal.
Para que se imprima alguna matriz a matrices.txt, debug tiene que valer 1.

### Parámetros
| Parámetro | Descripción |
|-----------|-------------|
| m         | Valor m de la matriz |
| k         | Valor k de la matriz |
| n         | Valor n de la matriz |
| alfa      | Factor de escalado alfa |
| test      | Indica si queremos comprobar errores en la matriz de resultado |
| debug     | Indica si queremos mostrar las matrices por stdout |
| time      | Indica si queremos imprimir los tiempos de cada proceso |

Cualquier valor menor que 0, o que no sea múltiplo de la raíz cuadrada del número de procesos para m, k, n, hará que el programa aborte forma controlada, mostrará donde se encuentra el fallo, y el correspondiente mensaje de error.

### Alterar el funcionamiento con \#define
Para cumplir con algunos requisitos, como por ejemplo implementar los comunicadores con MPI_Comm_Split, y con MPI_Cart_sub, ciertos bloques del código son activables o desactivables de forma transparente, para realizar la división de procesos en comunicadores de formas diferentes.

Podemos alterar el funcionamiento de dos apartados importantes de la práctica:
* Difusión de parámetros

|   \#define   | Descripción |
|--------------|-------------|
| BCAST_PACKED | La difusión de parámetros se realiza mediante una serie de MPI_Pack's en el proceso #0, y se desempaqueta en el resto con los MPI_Unpack's correspondientes |
| BCAST_STRUCT | La difusión de parámetros se realiza mediante la definición de un tipo struct de MPI, se rellenan los campos de la struct en el proceso #0, se difunden al resto de procesos, donde se asignan los valores de las variables locales a los que trae rellenados la struct |

* Definición de comunicadores fila y columna

|   \#define   | Descripción |
|--------------|-------------|
| COMM_SUBTOPO | Se crea un comunicador cartesiano entre todos los procesos, y sobre él se definen los comunicadores fila y columna mediante subtopologías |
|  COMM_SPLIT  | Se crean los comunicadores fila y columna en base al rank con MPI_Comm_split |

* Otros

|   \#define   | Descripción |
|--------------|-------------|
| DO_SORT_OUTPUT | Imprime los tiempos en orden según el número de proceso. Implementación muy poco eficiente, hecha unicamente para presentar los resultados de forma bonita. Mejor no usar. Se podría implementar un anillo de tokens para hacerlo más rápido, pero la verdad es que para tres veces que lo voy a usar, que es para sacar el output de esta memoria, no compensa |
|  DEBUG  | Se muestran por stderr las coordenadas para cada proceso, las submatrices A, B y C que maneja cada proceso, los valores de bufA y bufB en cada iteración del algoritmo para cada proceso. Todo esto de forma muy poco eficiente, pero de nuevo, para los casos de uso que esto tiene, que son por ejemplo una matriz de entrada 2x2 o 4x4, y una de salida de dimensiones similares, con un bajo número de procesos, es una implementación más que suficiente |

### Compilación
    [alonso@anarchy-alonso:AC/p3]$ make
    mpicc -Wall -g   -c -o main.o main.c
    mpicc -Wall -g  -o main main.o -lm


### Ejecución
Los resultados de la ejecución son con los flags BCAST_PACKED y COMM_SUBTOPO, así como DO_SORT_OUTPUT definidos.
Resultados con BCAST_STRUCT y/o COMM_SPLIT son exactamente iguales así que se han omitido.

La ejecución es del formato

    mpirun --oversubscribe -np \<num_procesos\> ./main 60 30 90 1 1 1 1
 
#### 4 Procesos

#### 9 Procesos

#### 25 Procesos
