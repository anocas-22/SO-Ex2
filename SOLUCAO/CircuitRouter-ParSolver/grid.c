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
 * PURPOSE ARE DISCLAIMED. IN NO EfilenameVENT SHALL STANFORD UNIVERSITY BE LIABLE
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
 * grid.c
 *
 * =============================================================================
 */


#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "coordinate.h"
#include "grid.h"
#include "lib/types.h"
#include "lib/vector.h"
#include <pthread.h>
#include <time.h>

const unsigned long CACHE_LINE_SIZE = 32UL;


/* =============================================================================
 * grid_alloc
 * =============================================================================
 */
grid_t* grid_alloc (long width, long height, long depth){
    grid_t* gridPtr;

    gridPtr = (grid_t*)malloc(sizeof(grid_t));
    if (gridPtr) {
        gridPtr->width  = width;
        gridPtr->height = height;
        gridPtr->depth  = depth;
        long n = width * height * depth;
        long* points_unaligned = (long*)malloc(n * sizeof(long) + CACHE_LINE_SIZE);
        assert(points_unaligned);
        gridPtr->points_unaligned = points_unaligned;
        gridPtr->points = (long*)((char*)(((unsigned long)points_unaligned
                                          & ~(CACHE_LINE_SIZE-1)))
                                  + CACHE_LINE_SIZE);
        memset(gridPtr->points, GRID_POINT_EMPTY, (n * sizeof(long)));
    }

    return gridPtr;
}

/* =============================================================================
 * grid_free
 * =============================================================================
 */
void grid_free (grid_t* gridPtr){
    free(gridPtr->points_unaligned);
    free(gridPtr);
}


/* =============================================================================
 * grid_copy
 * =============================================================================
 */
void grid_copy (grid_t* dstGridPtr, grid_t* srcGridPtr){
    assert(srcGridPtr->width  == dstGridPtr->width);
    assert(srcGridPtr->height == dstGridPtr->height);
    assert(srcGridPtr->depth  == dstGridPtr->depth);

    long n = srcGridPtr->width * srcGridPtr->height * srcGridPtr->depth;
    memcpy(dstGridPtr->points, srcGridPtr->points, (n * sizeof(long)));
}


/* =============================================================================
 * grid_isPointValid
 * =============================================================================
 */
bool_t grid_isPointValid (grid_t* gridPtr, long x, long y, long z){
    if (x < 0 || x >= gridPtr->width  ||
        y < 0 || y >= gridPtr->height ||
        z < 0 || z >= gridPtr->depth)
    {
        return FALSE;
    }

    return TRUE;
}


/* =============================================================================
 * grid_getPointRef
 * =============================================================================
 */
long* grid_getPointRef (grid_t* gridPtr, long x, long y, long z){
    return &(gridPtr->points[(z * gridPtr->height + y) * gridPtr->width + x]);
}


/* =============================================================================
 * grid_getPointIndices
 * =============================================================================
 */
void grid_getPointIndices (grid_t* gridPtr, long* gridPointPtr, long* xPtr, long* yPtr, long* zPtr){
    long height = gridPtr->height;
    long width  = gridPtr->width;
    long area = height * width;
    long index3d = (gridPointPtr - gridPtr->points);
    (*zPtr) = index3d / area;
    long index2d = index3d % area;
    (*yPtr) = index2d / width;
    (*xPtr) = index2d % width;
}


/* =============================================================================
 * grid_getPoint
 * =============================================================================
 */
long grid_getPoint (grid_t* gridPtr, long x, long y, long z){
    return *grid_getPointRef(gridPtr, x, y, z);
}


/* =============================================================================
 * grid_isPointEmpty
 * =============================================================================
 */
bool_t grid_isPointEmpty (grid_t* gridPtr, long x, long y, long z){
    long value = grid_getPoint(gridPtr, x, y, z);
    return ((value == GRID_POINT_EMPTY) ? TRUE : FALSE);
}


/* =============================================================================
 * grid_isPointFull
 * =============================================================================
 */
bool_t grid_isPointFull (grid_t* gridPtr, long x, long y, long z){
    long value = grid_getPoint(gridPtr, x, y, z);
    return ((value == GRID_POINT_FULL) ? TRUE : FALSE);
}


/* =============================================================================
 * grid_setPoint
 * =============================================================================
 */
void grid_setPoint (grid_t* gridPtr, long x, long y, long z, long value){
    (*grid_getPointRef(gridPtr, x, y, z)) = value;
}


/* =============================================================================
 * grid_addPath
 * =============================================================================
 */
void grid_addPath (grid_t* gridPtr, vector_t* pointVectorPtr){
    long i;
    long n = vector_getSize(pointVectorPtr);

    for (i = 0; i < n; i++) {
        coordinate_t* coordinatePtr = (coordinate_t*)vector_at(pointVectorPtr, i);
        long x = coordinatePtr->x;
        long y = coordinatePtr->y;
        long z = coordinatePtr->z;
        grid_setPoint(gridPtr, x, y, z, GRID_POINT_FULL);
    }
}


/* =============================================================================
 * grid_addPath_Ptr
 * =============================================================================
 * Percorre o caminho em pointVectorPtr e tenta fazer lock das posicoes e
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
bool_t grid_addPath_Ptr (grid_t* gridPtr, vector_t* pointVectorPtr, vector_t* lockVector){
    long i, j, tries = 0;
    long n = vector_getSize(pointVectorPtr);
    bool_t returnVal = TRUE;

    for (i = 1; i < (n-1); i++) {
      long* gridPointPtr = (long*)vector_at(pointVectorPtr, i);
      if(lock_point(gridPtr, gridPointPtr, lockVector, i)) {
        if (*gridPointPtr == GRID_POINT_FULL) {
          returnVal = FALSE;
          n = i + 1;
          break;
        }
      } else {
        tries++;
        if (tries <= 3) {
          nanosleep((const struct timespec[]){{0, random() % 100}}, NULL);
          for (j = 0; j < i; j++) {
            long* gridPointPtr = (long*)vector_at(pointVectorPtr, j);
            unlock_point(gridPtr, gridPointPtr, lockVector, j);
          }
          i = 0;
        } else {
          returnVal = FALSE;
          n = i + 1;
          break;
        }
      }
    }

    for (i = 1; i < (n-1); i++) {
      long* gridPointPtr = (long*)vector_at(pointVectorPtr, i);
      if (returnVal) {
        *gridPointPtr = GRID_POINT_FULL;
      }
      unlock_point(gridPtr, gridPointPtr, lockVector, i);
    }

    return returnVal;
}

/* =============================================================================
 * get_index
 * =============================================================================
 */
long get_index(grid_t* gridPtr, long x, long y, long z) {
  return (z * gridPtr->height + y) * gridPtr->width + x;
}


/* =============================================================================
 * lock_point
 * =============================================================================
 */
bool_t lock_point(grid_t* gridPtr, long* gridPointPtr, vector_t* lockVector, long i) {
  long x, y, z;
  grid_getPointIndices(gridPtr, gridPointPtr, &x, &y, &z);
  return (pthread_mutex_trylock((pthread_mutex_t*) vector_at(lockVector, get_index(gridPtr, x,y,z))) == 0);
}


 /* =============================================================================
  * unlock_point
  * =============================================================================
  */
  void unlock_point(grid_t* gridPtr, long* gridPointPtr, vector_t* lockVector, long i) {
    long x, y, z;
    grid_getPointIndices(gridPtr, gridPointPtr, &x, &y, &z);
    pthread_mutex_unlock((pthread_mutex_t*) vector_at(lockVector, get_index(gridPtr, x,y,z)));
  }

/* =============================================================================
 * grid_print
 * =============================================================================
 */
void grid_print (grid_t* gridPtr, FILE* file){
    long width  = gridPtr->width;
    long height = gridPtr->height;
    long depth  = gridPtr->depth;
    long z;
    for (z = 0; z < depth; z++) {
        fprintf(file, "[z = %li]\n", z);
        long x;
        for (x = 0; x < width; x++) {
            long y;
            for (y = 0; y < height; y++) {
                fprintf(file, "%4li", *grid_getPointRef(gridPtr, x, y, z));
            }
            fputs("", file);
        }
        fputs("", file);
    }
}


/* =============================================================================
 *
 * End of grid.c
 *
 * =============================================================================
 */
