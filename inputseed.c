/* 
 *  File: inputseed.c
 *  
 *  Author: 
 *
 *  Date:
 *
 *  Description: Reads in network, pattern, and weights files and initializes
 *               the data structures for the neural network functions.
 *
 */


#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include "header.h"

#include "XButil.h"

extern Boolean wts_changed;

void read_network_file(FILE *nptr)
{
   int  i = 0, flag = 0, k, j, start_position;
   char text_buff[25];
   
   rewind(nptr);
   fscanf(nptr,"%*s");
   fscanf(nptr,"%*s %i", &number_of_nodes);
   fscanf(nptr,"%*s %i", &number_of_inputs);
   fscanf(nptr,"%*s %i", &number_of_outputs);
   fscanf(nptr,"%*s");
   fscanf(nptr,"%*s");
   fscanf(nptr,"%s", text_buff);
   number_of_weights = 0;
   number_of_input_thresholds = number_of_inputs;
   number_of_thresholds = number_of_nodes - number_of_inputs;


   while(strcmp(text_buff, "end"))
      {
 
         fscanf(nptr,"%i %i %i %i",&net_array[i],&net_array[i+1],
                   &net_array[i+2],&net_array[i+3]);

          if (net_array[i + 2] >= number_of_inputs && !flag)
          {
              third_layer_start = net_array[i];
              flag = 1;
          }
         number_of_weights = number_of_weights + net_array[i+1] * net_array[i+3];

          i = i + 4;
          fscanf(nptr,"%s", text_buff);
 
      }

   net_array[i] = '\n';
   
   middle_layer_start = net_array[0];
   node = (NODE_STRUC*) malloc(sizeof(NODE_STRUC) * number_of_nodes);

   i = 0;
   start_position = 0;

   /* While we still have more layers */

   while (net_array[start_position] != '\n')
   {

      /* While we still have more nodes in a layer */
      for(k = 0; k < net_array[start_position + 1]; k++)
      {
        (node + (net_array[start_position] + k))->input_from = 
           net_array[start_position + 2];

        (node + (net_array[start_position] + k))->input_to = 
           (net_array[start_position + 3] - 1) + net_array[start_position + 2];
      }

      i= i + 1;
      start_position = 4 * i;

   }

/* for (i=0;i<number_of_nodes;i++)
    printf("i: %d\ninput_from: %d\ninput_to: %d\n\n", i, node[i].input_from, node[i].input_to); */
}


 /* *************************************************** */

 int  random1()
      {
      randseed =  15625L *  randseed + 22221L ;
      return((randseed >> 16) & 0x7fff);
      }      


 /* *************************************************** */


void make_weights()
{
   int i = 0, k, j, start_position, x;
  

   x = 0;
   i = 0;
   start_position = 0;

   /* set random seed */
   srand((int) time(NULL));
   randseed = (long)rand();

/* Make the weight values for each node. */

   /* While we still have more layers */
   
while (net_array[start_position] != '\n')
   {
      /* While we still have more nodes in a layer */
      for(k = 0; k < net_array[start_position + 1]; k++)
      {
              /* From each node above this layer */

      (node + (net_array[start_position] + k)) ->weight = 
      (float*) malloc(sizeof(float) * net_array[start_position + 3]);

      (node + (net_array[start_position] + k)) ->old_weight[0] = 
      (float*) malloc(sizeof(float) * net_array[start_position + 3]);

      (node + (net_array[start_position] + k)) ->old_weight[1] = 
      (float*) malloc(sizeof(float) * net_array[start_position + 3]);

       for(j = 0; j < net_array[start_position + 3]; j++)
          {
          (node + (net_array[start_position ] + k))->weight[j] =
          ((random1()/pow(2.0,15.0) - 0.5));
          (node + (net_array[start_position ] + k))->old_weight[0][j] = 0.0;
          (node + (net_array[start_position ] + k))->old_weight[1][j] = 0.0;
          }
      }
      
      i = i+1;
      start_position = 4 * i;

   }

/* Make the threshhold values and store in the proper node structure */

   /* move past the zero threshholds for the inputs */
   x = x + (number_of_inputs); 
   
   i = 0;
   start_position = 0;

   /* While we still have more layers */
   while (net_array[start_position] != '\n')
   {

      /* While we still have more nodes in a layer */
      for(k = 0; k < net_array[start_position + 1]; k++)
        {
       (node + (net_array[start_position] + k))->threshold =
        ((random1()/pow(2.0,15.0) - 0.5));

        }

      i = i+1;
      start_position = 4 * i;

   }

wts_changed=True;
}
/* ****************************************************** */

Boolean read_weight_file(FILE *wptr)
   {
   int i = 0, k, j, start_position, x = 0;
   int counter = -1;

   fseek(wptr, 0, SEEK_SET);
   while (!feof(wptr))
      {
      fscanf(wptr,"%*f");
      counter++;
      }

   if (counter != number_of_weights + number_of_input_thresholds 
                  + number_of_thresholds)
      {
      XbError("Incorrect number of weights or thresholds in weights file. \nFile not loaded.");
      return(False);
      }

   start_position = 0;
   fseek(wptr, 0, SEEK_SET);

   /* Read in the weight values for each node. */
   /* While we still have more layers */
   while (net_array[start_position] != '\n')
      {
      /* While we still have more nodes in a layer */
      for(k = 0; k < net_array[start_position + 1]; k++)
         {
         /* From each node above this layer */

         (node + (net_array[start_position] + k)) ->weight = 
         (float*) malloc(sizeof(float) * net_array[start_position + 3]);
         
         (node + (net_array[start_position] + k)) ->old_weight[0] = 
         (float*) malloc(sizeof(float) * net_array[start_position + 3]);

         (node + (net_array[start_position] + k)) ->old_weight[1] = 
         (float*) malloc(sizeof(float) * net_array[start_position + 3]);

         for(j = 0; j < net_array[start_position + 3]; j++)
            { 
            if (!feof(wptr));
               {
               fscanf(wptr,"%f", &((node + (net_array[start_position ] +k))->weight[j]));                                  }    
               (node + (net_array[start_position ] + k))->old_weight[0][j] = 0.0;
               (node + (net_array[start_position ] + k))->old_weight[1][j] = 0.0;
               }
         }
      
      i = i+1;
      start_position = 4 * i;
      }

   /* Read in the threshhold values and store in the proper node structure */

   /* move past the zero threshholds for the inputs */
 
   for (x = 0; x < number_of_inputs; x++)
      {
      if (!feof(wptr))
      fscanf(wptr,"%*f");
      }

   
   i = 0;
   start_position = 0;

   /* While we still have more layers */
   while (net_array[start_position] != '\n')
      {
      /* While we still have more nodes in a layer */
      for(k = 0; k < net_array[start_position + 1]; k++)
        {
          if (!feof(wptr))
             {
             fscanf(wptr,"%f",&((node + (net_array[start_position] + k))->threshold));
             }
        }

      i = i+1;
      start_position = 4 * i;
      }

   clearerr(wptr);
   return(True);
   }


/* ****************************************************** */
Boolean read_pattern_file(FILE *pptr)
{
   float num = 0.0;
   int j, i, k,c;
   int pattern_counter = 0;
   char ch, s[10];

   while(fscanf(pptr,"%c",&ch)!=EOF)
       {
       if (ch == 'p')
          {
          pattern_counter++;
          while((ch != '\n') && (fscanf(pptr,"%c",&ch) != EOF));
          }
       else if (ch == ';')
          while((ch != '\n') && (fscanf(pptr,"%c",&ch) != EOF));
       }
             
   number_of_patterns = pattern_counter;

   rewind(pptr);     
   clearerr(pptr);

   pattern = (PATT_STRUC*) malloc (sizeof(PATT_STRUC)* pattern_counter);

   /* for each pattern, set up enough memory for input and desired  */
   for (k = 0; k< pattern_counter; k++)
      {
      (pattern + k)->input = (float*) malloc (sizeof(float) * (number_of_inputs));
      (pattern + k)->desired = (float*) malloc (sizeof(float) * (number_of_outputs));
      }
 
   for(j=0; j<pattern_counter; j++)
   {
       while(fscanf(pptr,"%c",&ch)!=EOF)
          if (ch == ';')
              while((ch != '\n') && (fscanf(pptr,"%c",&ch) != EOF));
          else if (ch == 'p')
              {
              fseek(pptr, -1L, SEEK_CUR);
              break;
              }

       fscanf(pptr,"%s",s);

       for(i=0; i<number_of_inputs; i++)
       {
           if((fscanf(pptr,"%f",&num))==EOF)
               {
               XbError("Pattern file is incorrect.\nPattern file not loaded.");
               return(False);
               }          
           (pattern + j) ->input[i] = num;
       }
       
       for ( k=0; k< number_of_outputs; k++)
       {
           if((fscanf(pptr,"%f",&num))==EOF)
               {
               XbError("Pattern file is incorrect.\nPattern file not loaded.");
               return(False);
               } 
           (pattern + j) ->desired[k] = num;
       } 
   }
return(True);
}    
