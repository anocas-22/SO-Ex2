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
 * grid.h
 *
 * =============================================================================
 */


#ifndef GRID_H
#define GRID_H 1

#include <stdio.h>
#include "lib/types.h"
#include "lib/vector.h"


typedef struct grid {
    long width;
    long height;
    long depth;
    long* points;
    long* points_unaligned;
} grid_t;

enum {
    GRID_POINT_FULL  = -2L,
    GRID_POINT_EMPTY = -1L
};


/* =============================================================================
 * grid_alloc
 * =============================================================================
 */
grid_t* grid_alloc (long width, long height, long depth);


/* =============================================================================
 * grid_free
 * =============================================================================
 */
void grid_free (grid_t* gridPtr);


/* =============================================================================
 * grid_copy
 * =============================================================================
 */
void grid_copy (grid_t* dstGridPtr, grid_t* srcGridPtr);


/* =============================================================================
 * grid_isPointValid
 * =============================================================================
 */
bool_t grid_isPointValid (grid_t* gridPtr, long x, long y, long z);


/* =============================================================================
 * grid_getPointRef
 * =============================================================================
 */
long* grid_getPointRef (grid_t* gridPtr, long x, long y, long z);


/* =============================================================================
 * grid_getPointIndices
 * =============================================================================
 */
void grid_getPointIndices (grid_t* gridPtr, long* gridPointPtr, long* xPtr, long* yPtr, long* zPtr);


/* =============================================================================
 * grid_getPoint
 * =============================================================================
 */
long grid_getPoint (grid_t* gridPtr, long x, long y, long z);


/* =============================================================================
 * grid_isPointEmpty
 * =============================================================================
 */
bool_t grid_isPointEmpty (grid_t* gridPtr, long x, long y, long z);


/* =============================================================================
 * grid_isPointFull
 * =============================================================================
 */
bool_t grid_isPointFull (grid_t* gridPtr, long x, long y, long z);


/* =============================================================================
 * grid_setPoint
 * =============================================================================
 */
void grid_setPoint (grid_t* gridPtr, long x, long y, long z, long value);


/* =============================================================================
 * grid_addPath
 * =============================================================================
 */
void grid_addPath (grid_t* gridPtr, vector_t* pointVectorPtr);


/* =============================================================================
 * grid_addPath_Ptr
 * =============================================================================
 * Percorre o caminho em pointVectorPtr, tenta fazer lock das posicoes e
 * verifica se estas estao preenchidas.
 * Se nao conseguir fazer lock vai tentar outra vez apos esperar algum tempo
 * aleatorio de forma a nao encontrar o mesmo obstaculo e mesmo assim apenas vai
 * tentar 3 vezes para impedir que, caso seja impossivel resolver o impasse,
 * se gaste demasiado tempo
 * Caso nao consiga fazer lock das posicoes ou se uma delas estiver preenchida
 * vai fazer unlock de todas as posicoes e devolver FALSE
 * Caso consiga fazer todos os locks sem problemas vai escrever o caminho na
 * grid global, fazer unlock das posicoes e devolver TRUE
 */
bool_t grid_addPath_Ptr (grid_t* gridPtr, vector_t* pointVectorPtr, vector_t* lockVector);

/* =============================================================================
 * get_index
 * Descobre a posicao de um ponto na grid apartir das suas coordenadas
 * =============================================================================
 */
long get_index(grid_t* gridPtr, long x, long y, long z);

/* =============================================================================
 * lock_point
 * Descobre a posicao de um ponto e bloqueia o mutex equivalente
* =============================================================================
 */
bool_t lock_point(grid_t* gridPtr, long* gridPointPtr, vector_t* lockVector, long i);

/* =============================================================================
 * unlock_point
 * Descobre a posicao de um ponto e desbloqueia o mutex equivalente
 * =============================================================================
 */
void unlock_point(grid_t* gridPtr, long* gridPointPtr, vector_t* lockVector, long i);

/* =============================================================================
 * grid_print
 * =============================================================================
 */
void grid_print (grid_t* gridPtr, FILE* file);


#endif /* GRID_H */


/* =============================================================================
 *
 * End of grid.c
 *
 * =============================================================================
 */
