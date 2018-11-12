/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * This code is an adaptation of the Lee algorithm's implementation originally included in the STAMP Benchmark
 * by Stanford University.
 *
 * The original copyright notice is included below.
 *
  * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 * Copyright (C) Stanford University, 2006.  All Rights Reserved.
 * Author: Chi Cao Minh
 *
 * =============================================================================
 *
 * Unless otherwise noted, the following license applies to STAMP files:
 *
 * Copyright (c) 2007, Stanford University
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are
 * met:
 *
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in
 *       the documentation and/or other materials provided with the
 *       distribution.
 *
 *     * Neither the name of Stanford University nor the names of its
 *       contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY STANFORD UNIVERSITY ``AS IS'' AND ANY
 * EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
 * PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL STANFORD UNIVERSITY BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF
 * THE POSSIBILITY OF SUCH DAMAGE.
 *
 * =============================================================================
 *
 * CircuitRouter-SeqSolver.c
 *
 * =============================================================================
 */


#include <assert.h>
#include <getopt.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include "lib/list.h"
#include "maze.h"
#include "router.h"
#include "lib/timer.h"
#include "lib/types.h"
#include "lib/vector.h"

enum param_types {
    PARAM_BENDCOST   = (unsigned char)'b',
    PARAM_XCOST      = (unsigned char)'x',
    PARAM_YCOST      = (unsigned char)'y',
    PARAM_ZCOST      = (unsigned char)'z',
    PARAM_NUMTAREFAS = (unsigned char)'t',
};

enum param_defaults {
    PARAM_DEFAULT_BENDCOST   = 1,
    PARAM_DEFAULT_XCOST      = 1,
    PARAM_DEFAULT_YCOST      = 1,
    PARAM_DEFAULT_ZCOST      = 2,
    PARAM_DEFAULT_NUMTAREFAS = 1,
};

bool_t global_doPrint = TRUE;
char* global_inputFile = NULL;
long global_params[256]; /* 256 = ascii limit */


/* =============================================================================
 * displayUsage
 * =============================================================================
 */
static void displayUsage (const char* appName){
    printf("Usage: %s [options]\n", appName);
    puts("\nOptions:                            (defaults)\n");
    printf("    b <INT>    [b]end cost          (%i)\n", PARAM_DEFAULT_BENDCOST);
    printf("    x <UINT>   [x] movement cost    (%i)\n", PARAM_DEFAULT_XCOST);
    printf("    y <UINT>   [y] movement cost    (%i)\n", PARAM_DEFAULT_YCOST);
    printf("    z <UINT>   [z] movement cost    (%i)\n", PARAM_DEFAULT_ZCOST);
    printf("    h          [h]elp message       (false)\n");
    exit(1);
}


/* =============================================================================
 * setDefaultParams
 * =============================================================================
 */
static void setDefaultParams (){
    global_params[PARAM_BENDCOST] = PARAM_DEFAULT_BENDCOST;
    global_params[PARAM_XCOST]       = PARAM_DEFAULT_XCOST;
    global_params[PARAM_YCOST]       = PARAM_DEFAULT_YCOST;
    global_params[PARAM_ZCOST]       = PARAM_DEFAULT_ZCOST;
    global_params[PARAM_NUMTAREFAS]  = PARAM_DEFAULT_NUMTAREFAS;
}


/* =============================================================================
 * parseArgs
 * =============================================================================
 */
static char* parseArgs (long argc, char* const argv[]){
    long i;
    long opt;
    char* filename;

    opterr = 0;

    setDefaultParams();

    while ((opt = getopt(argc, argv, "hb:x:y:z:t:")) != -1) {
        switch (opt) {
            case 'b':
            case 'x':
            case 'y':
            case 't':
            case 'z':
                global_params[(unsigned char)opt] = atol(optarg);
                break;
            case '?':
            case 'h':
            default:
                opterr++;
                break;
        }
    }

    filename = argv[optind];
    optind++;

    for (i = optind; i < argc; i++) {
        fprintf(stderr, "Non-option argument: %s\n", argv[i]);
        opterr++;
    }

    if (opterr) {
        displayUsage(argv[0]);
    }

    return filename;
}


/* =============================================================================
 * main
 * =============================================================================
 */
int main(int argc, char** argv){
    /*
     * Initialization
     */

    char* filename = parseArgs(argc, (char** const)argv);
    maze_t* mazePtr = maze_alloc();
    assert(mazePtr);

    long numPathToRoute = maze_read(mazePtr, filename);
    router_t* routerPtr = router_alloc(global_params[PARAM_XCOST],
                                       global_params[PARAM_YCOST],
                                       global_params[PARAM_ZCOST],
                                       global_params[PARAM_BENDCOST]);
    assert(routerPtr);
    list_t* pathVectorListPtr = list_alloc(NULL);
    assert(pathVectorListPtr);

    //Inicializa o lock global para a interacao com workQueuePtr e pathVectorListPtr
    pthread_mutex_t lock;
    if (pthread_mutex_init(&lock, NULL) != 0) {
      perror("Failure initializing mutex: ");
      exit(EXIT_FAILURE);
    }

    //Cria um vetor de mutexes do mesmo tamanho que a grid
    long gridSize = mazePtr->gridPtr->width * mazePtr->gridPtr->height * mazePtr->gridPtr->depth;
    vector_t* lockVector = vector_alloc(gridSize);
    if (lockVector == NULL) {
      fprintf(stderr, "Failed to initialize the vector lockVector.\n");
      exit(EXIT_FAILURE);
    }

    //Inicializa todos os mutexes de lockVector
    for (long i = 0; i < gridSize; i++) {
      pthread_mutex_t* pointLock = (pthread_mutex_t*) malloc(sizeof(pthread_mutex_t));
      if (pointLock == NULL) {
        perror("Failed malloc: ");
        exit(EXIT_FAILURE);
      }
      if (pthread_mutex_init(pointLock, NULL) != 0) {
        perror("Failed malloc: ");
        exit(EXIT_FAILURE);
      }
      if(!vector_pushBack(lockVector, (void*) pointLock)) {
        fprintf(stderr, "Failed to add thread to vector.\n");
        exit(EXIT_FAILURE);
      }
    }

    //Passa o lock global e o vetor de mutexes para o router_solve
    router_solve_arg_t routerArg = {routerPtr, mazePtr, pathVectorListPtr, &lock, lockVector};
    TIMER_T startTime;
    TIMER_READ(startTime);

    //Cria um vetor de threads com o tamanho passado na execucao
    vector_t* threads = vector_alloc(global_params[PARAM_NUMTAREFAS]);
    if (threads == NULL) {
      fprintf(stderr, "Failed to initialize the vector threads.\n");
      exit(EXIT_FAILURE);
    }

    //Inicializa todas as threads no vetor threads executando router_solve com routerArg como argumento
    for (long i = 0; i < global_params[PARAM_NUMTAREFAS]; i++) {
      pthread_t* thread = (pthread_t*) malloc(sizeof(pthread_t));
      if (thread == NULL) {
        perror("Failed malloc: ");
        exit(EXIT_FAILURE);
      }
      if (pthread_create(thread, 0, (void*) router_solve, (void *)&routerArg) != 0) {
        fprintf(stderr, "Failed to create thread.\n");
        exit(EXIT_FAILURE);
      }
      if (!vector_pushBack(threads, (void*) thread)) {
        fprintf(stderr, "Failed to add thread to vector.\n");
        exit(EXIT_FAILURE);
      }
    }

    //Espera que todas as threads acabem a execucao e liberta-as
    for (long i = 0; i < global_params[PARAM_NUMTAREFAS]; i++) {
      if (pthread_join(*((pthread_t*)vector_at(threads, i)), NULL) != 0) {
        fprintf(stderr, "Failed to join thread.\n");
        exit(EXIT_FAILURE);
      }
      free((pthread_t*)vector_at(threads, i));
    }

    //Liberta o vetor e destroi todos os mutexes e o vetor correspondente
    vector_free(threads);

    if (pthread_mutex_destroy(&lock) != 0) {
      fprintf(stderr, "Failed to destroy mutex.\n");
      exit(EXIT_FAILURE);
    }

    for (long i = 0; i < gridSize; i++) {
      if (pthread_mutex_destroy((pthread_mutex_t*)vector_at(lockVector, i)) != 0) {
        fprintf(stderr, "Failed to destroy mutex.\n");
        exit(EXIT_FAILURE);
      }
      free((pthread_mutex_t*)vector_at(lockVector, i));
    }
    vector_free(lockVector);

    TIMER_T stopTime;
    TIMER_READ(stopTime);

    long numPathRouted = 0;
    list_iter_t it;
    list_iter_reset(&it, pathVectorListPtr);
    while (list_iter_hasNext(&it, pathVectorListPtr)) {
        vector_t* pathVectorPtr = (vector_t*)list_iter_next(&it, pathVectorListPtr);
        numPathRouted += vector_getSize(pathVectorPtr);
	}

    FILE* file;
    file = fopen(filename, "a");
    if (file == NULL)
        exit(-1);
    fprintf(file, "Paths routed    = %li\n", numPathRouted);
    fprintf(file, "Elapsed time    = %f seconds\n", TIMER_DIFF_SECONDS(startTime, stopTime));
    fclose(file);

    /*
     * Check solution and clean up
     */
    assert(numPathRouted <= numPathToRoute);
    bool_t status = maze_checkPaths(mazePtr, pathVectorListPtr, global_doPrint, filename);
    assert(status == TRUE);
    file = fopen(filename, "a");
    if (file == NULL)
        exit(-1);
    fputs("Verification passed.", file);
    fclose(file);

    maze_free(mazePtr);
    router_free(routerPtr);

    list_iter_reset(&it, pathVectorListPtr);
    while (list_iter_hasNext(&it, pathVectorListPtr)) {
        vector_t* pathVectorPtr = (vector_t*)list_iter_next(&it, pathVectorListPtr);
        vector_t* v;
        while((v = vector_popBack(pathVectorPtr))) {
            // v stores pointers to longs stored elsewhere; no need to free them here
            vector_free(v);
        }
        vector_free(pathVectorPtr);
    }
    list_free(pathVectorListPtr);


    exit(0);
}


/* =============================================================================
 *
 * End of CircuitRouter-SeqSolver.c
 *
 * =============================================================================
 */
