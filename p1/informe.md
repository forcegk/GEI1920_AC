# INFORME P1 AC :: 2019-20
## Alonso Rodríguez Iglesias

### Especificaciones del equipo
    Compilador: gcc (Arch Linux 9.3.0-1) 9.3.0
    Kernel: Linux 5.5.16-1-ck-ivybridge #1 SMP PREEMPT x86_64 GNU/Linux
    CPU: Intel(R) Core(TM) i5-3230M CPU @ 2.60GHz
    RAM: 7.60 GiB DDR3 @ 1600 MHz

### Metodología
    Se han recogido los mejores tiempos de cada una de las mediciones, con
    una excepción que comentaremos en las conclusiones, por la que también
    recogemos los resultados de la primera medición.

    Tamaños:
       - Pequeño  6000 x  5000
       - Mediano 12000 x 15000
       - Grande  40000 x 35000 [Min. 5.6 GB de RAM libres]

       - Todas con alpha = 1 y sin imprimir resultados

### Resultados
    Autovectorización:
       - Pequeño: Tiempo = 0:028 (seg:mseg)
       - Mediano: Tiempo = 0:175 (seg:mseg)
       - Grande:  Tiempo = 1:380 (seg:mseg)

    Con uso de hadd:
       - Pequeño: Tiempo = 0:022 (seg:mseg)
       - Mediano: Tiempo = 0:130 (seg:mseg)
       - Grande:  Tiempo = 1:026 (seg:mseg) [Primera ejecución 1:109(seg:mseg)]

    Sin uso de hadd:
       - Pequeño: Tiempo = 0:022 (seg:mseg)
       - Mediano: Tiempo = 0:134 (seg:mseg)
       - Grande:  Tiempo = 1:033 (seg:mseg) [Primera ejecución 1:058(seg:mseg)]


### Conclusiones
      Como vemos, el compilador realiza un muy buen trabajo en la
    auto-vectorización. Asimismo, el no usar instrucciones horizontales
    no parece beneficiar nunca, excepto en un caso que me ha parecido bastante
    curioso, y es que en la primera ejecución, de manera consistente, el
    código que no emplea operaciones horizonales tiene un ligeramente mejor
    desempeño que el que sí las utiliza.
      Esto podría deberse a algún tipo de prefetch o comportamientos con la
    caché, del procesador, o del sistema operativo.
      Al repetir la prueba, desde la segunda y posteriores, la que emplea
    operaciones horizonales suele mejorar el desempeño de la que no las emplea.