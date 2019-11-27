/* 
 *  File: header.h
 *  
 *  Author:
 *
 *  Date:
 *
 *  Description: Header file for compb.c, inputseed.c, learn.c, and weights.c.
 *
 */

#include "XButil.h"
#include <stdio.h>


typedef struct
    {
    int input_from;
    int input_to;
    float threshold;
    float output;
    float error;
    float *weight;
    float *old_weight[2];
    }NODE_STRUC;


typedef struct
    {
    float *desired;
    float *input;
    }PATT_STRUC;


/* global variables */
NODE_STRUC *node;
PATT_STRUC *pattern;

int patt_num, iterations, number_of_outputs, number_of_nodes, last_node,
    middle_layer_start, third_layer_start, number_of_inputs, net_array[10000];
float *weights_array;
FILE *nptr, *pptr, *wptr, *w2ptr;
int number_of_patterns;
float *desiredtest;
float *inputtest;
int number_of_weights;
int number_of_thresholds;
int number_of_input_thresholds;
long randseed;


/* function prototypes */
int compute();
void write_weights();
void learn();
void read_network_file(FILE *nptr);
Boolean read_weight_file(FILE *wptr);
Boolean read_pattern_file(FILE *pptr);
void make_weights();
