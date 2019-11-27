/* 
 *  File: compb.c
 *  
 *  Author:
 *
 *  Date:
 *
 *  Description: Trains the neural network.
 *               Modified to add momentum.  EWH 8/92
 *
 */

#include <stdio.h>
#include <math.h>
#include <Xm/Text.h>
#include "header.h"

extern Widget tss_text;
extern float momentum, gain;

void learn()
{
int i, j, k, counter = 0, patt_num = 0;
float exponent, sum, total_sum_squares = 0, cal_sum = 1;
char tss_string[20];
int cnt = 0;
int alt = 0;    /* to use old_weights[2], alternates between old_weights[0] and [1] */


while (iterations > 0)
{     

/* Calculate outputs for first middle layer */
for (i = middle_layer_start;i < third_layer_start;i++)
   {
   exponent = 0.0;
   for (j = (node + i)->input_from;j < ((node + i)->input_to + 1);j++)
      {
      exponent = exponent + *((node + i)->weight + j - (node +i)->input_from)  *
      *((pattern + patt_num)->input + j - (node + i)->input_from);
      }
   (node + i)->output = 1.0 / (1.0 + exp(-(exponent + (node + i)->threshold)));
   }

/* Calculate outputs for rest of middle layers and output nodes */
for (i = third_layer_start;i < number_of_nodes;i++)
   {
   exponent = 0.0;
   for (j = (node + i)->input_from;j < ((node+i)->input_to + 1);j++)
      {
      exponent = exponent + *((node + i)->weight + j - (node + i)->input_from) *
      (node + j)->output;
      }
   (node + i)->output = 1.0 / (1.0 + exp(-(exponent + (node + i)->threshold)));   
   }

/* If cal_sum flag is set calculate total sum of squares*/
if (cal_sum)
   {
   for (i = (number_of_nodes - number_of_outputs);i < number_of_nodes;i++)
      {
      k = i - (number_of_nodes - number_of_outputs);
      total_sum_squares += (*((pattern + patt_num)->desired + k) - (node + i)->output)*
         (*((pattern + patt_num)->desired + k) - (node + i)->output);
      }
   }

/* Calculate output error(s)  */
for (i = (number_of_nodes - number_of_outputs);i < number_of_nodes;i++)
   {
   k = i - (number_of_nodes - number_of_outputs);
   (node + i)->error = (node + i)->output * (1 - (node + i)->output) *
   (*((pattern + patt_num)->desired + k) - (node + i)->output);
   }

/* Zero out previous errors */
for (i = (number_of_nodes - number_of_outputs - 1);i > (number_of_inputs - 1);
     i--)
   {
   (node + i)->error = 0.0;
   }

/* Calculate middle layer errors */
for (i = (number_of_nodes - 1);i > (third_layer_start - 1);i--)
   {
   for (j = (node + i)->input_from;j < ((node + i)->input_to + 1);j++)
      {
      (node + j)->error = (node + j)->error + (node + j)->output * 
      (1 - (node + j)->output) * (node + i)->error *
      (*((node +i)->weight + j -(node + i)->input_from));
      }
   }

/* Calculate new weights for third through output nodes */
for (i = (number_of_nodes - 1);i > (third_layer_start - 1);i--)
   {
   for (j = (node + i)->input_from;j < ((node + i)->input_to + 1);j++)
      {
      *((node + i)->old_weight[alt] + j - (node + i)->input_from)=
         *((node + i)->weight + j - (node + i)->input_from);
 
      *((node + i)->weight + j - (node + i)->input_from) = *((node + i)->weight + j
      - (node +i)->input_from) + gain * (node + i)->error * (node + j)->output
      + momentum * (*((node + i)->weight + j - (node + i)->input_from) -
      (*((node + i)->old_weight[!alt] + j - (node + i)->input_from)));  
      }
   }

/* Calculate new weights for first middle layer */
for (i = (third_layer_start - 1);i > (number_of_inputs - 1);i--)
   {
   for (j = (node + i)->input_from;j < ((node + i)->input_to + 1);j++)
      {
      *((node + i)->old_weight[alt] + j - (node + i)->input_from)=
         *((node + i)->weight + j - (node + i)->input_from);

      *((node + i)->weight + j - (node + i)->input_from) = *((node +i)->weight
      + j - (node + i)->input_from) + gain * (node + i)->error *
      (*((pattern + patt_num)->input + j - (node + i)->input_from))
      + momentum * (*((node + i)->weight + j - (node + i)->input_from) -
      (*((node + i)->old_weight[!alt] + j - (node + i)->input_from)));
      } 
   }

/* next */
alt=!alt;

/* Calculate new thresholds */
for (i = (number_of_nodes - 1);i > (middle_layer_start - 1);i--)
   (node + i)->threshold = (node + i)->threshold + (gain * (node + i)->error);

patt_num++;
if (patt_num == number_of_patterns)
   {
   patt_num = 0;
   iterations--;
   counter++;

   /* If cal_sum flag is set print out total sum of squares and reset flag 
      and counter. */ 
   if (cal_sum)
      {
      sprintf(tss_string,"%f",total_sum_squares);
      XmTextSetString(tss_text, tss_string);

      if (cnt == 2)
          {
          XmUpdateDisplay(tss_text);
          cnt = 0;
          }
      else
          cnt++;

      counter = 0;
      cal_sum = 0;
      }

   /* If 20 iterations have passed since last total sum of squares
      calculation, set cal_sum flag and set total_sum_squares to 0.0.   */
   if (counter == 20)
      {
      total_sum_squares = 0.0;
      cal_sum = 1;
      }
   }
}


/* initial tss */
patt_num = 0;
total_sum_squares = 0.0;

while (patt_num != number_of_patterns)
    {
    /* Calculate outputs for first middle layer */
    for (i = middle_layer_start;i < third_layer_start;i++)
        {
        exponent = 0.0;
        for (j = (node + i)->input_from;j < ((node + i)->input_to + 1);j++)
            exponent = exponent + *((node + i)->weight + j - (node +i)->input_from)  *
                *((pattern + patt_num)->input + j - (node + i)->input_from);

        (node + i)->output = 1.0 / (1.0 + exp(-(exponent + (node + i)->threshold)));
        }
 
    /* Calculate outputs for rest of middle layers and output nodes */
    for (i = third_layer_start;i < number_of_nodes;i++)
        {
        exponent = 0.0;
        for (j = (node + i)->input_from;j < ((node+i)->input_to + 1);j++)
            exponent = exponent + *((node + i)->weight + j - (node + i)->input_from) *
                (node + j)->output;

        (node + i)->output = 1.0 / (1.0 + exp(-(exponent + (node + i)->threshold)));   
        }

    /* Calculate total sum of squares*/
    for (i = (number_of_nodes - number_of_outputs);i < number_of_nodes;i++)
        {
        k = i - (number_of_nodes - number_of_outputs);
        total_sum_squares += (*((pattern + patt_num)->desired + k) - (node + i)->output)*
            (*((pattern + patt_num)->desired + k) - (node + i)->output);
        }

    patt_num++;
    if (patt_num == number_of_patterns)
        {
        sprintf(tss_string,"%f",total_sum_squares);
        XmTextSetString(tss_text, tss_string);
        }
    }

}
