/* 
 *  File: weights.c
 *  
 *  Author:
 *
 *  Date:
 *
 *  Description: Writes the weights to a file.
 *
 */

#include "header.h" 

void write_weights()
{
int i,j;

for (i = middle_layer_start;i < number_of_nodes;i++)
   {
   for (j = (node + i)->input_from;j < ((node + i)->input_to + 1);j++)
      {
      fprintf(w2ptr,"%f\n",(*((node + i)->weight + j - 
      (node + i)->input_from)));
      }
   }

for (i = 0;i < number_of_inputs;i++)
   {
   fprintf(w2ptr,"%f\n",0.0);
   }

for (i = number_of_inputs;i < number_of_nodes;i++)
   {
   fprintf(w2ptr,"%f\n",(node + i)->threshold);
   }
}

