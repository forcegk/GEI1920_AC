DEBUG = -O0 -g
RELEASE = -O3

CFLAGS = ${RELEASE}

default:
	@echo "Usage:"
	@echo "   p1c:     Practica 1 sin optimizaciones"
	@echo "   autovec: Practica 1 con vectorizacion automatica"
	@echo "   p1SSE_hadd:   Practica 1 con vectorizacion manual y uso de hadd"
	@echo "   p1SSE_no_hadd:   Practica 1 con vectorizacion manual sin uso de hadd"

all: p1c autovec p1SSE_hadd p1SSE_no_hadd

p1c: matrizVectorP1.c
	gcc ${CFLAGS} -o matrizVectorP1 matrizVectorP1.c -lm

autovec: matrizVectorP1.c
	gcc ${CFLAGS} -march=nocona -msse3 -ftree-vectorize -ftree-vectorizer-verbose=2 -o matrizVectorP1Vec matrizVectorP1.c -lm

p1SSE_hadd: matrizVectorP1SSE_hadd.c
	gcc ${CFLAGS} -march=nocona -msse3 -o matrizVectorP1SSE_hadd matrizVectorP1SSE_hadd.c -lm

p1SSE_no_hadd: matrizVectorP1SSE_no_hadd.c
	gcc ${CFLAGS} -march=nocona -msse3 -o matrizVectorP1SSE_no_hadd matrizVectorP1SSE_no_hadd.c -lm

c: clean

clean:
	-rm matrizVectorP1 matrizVectorP1Vec matrizVectorP1SSE_hadd matrizVectorP1SSE_no_hadd

