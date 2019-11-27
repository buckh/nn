/* 
 *  File: compb.c
 *  
 *  Author:
 *
 *  Date:
 *
 *  Description: Computes the output of each node for a given input pattern.
 *
 */

#include <stdio.h>
#include <math.h>
#include <strings.h>
#include "header.h"

#include "draw.h"

extern char *motif_pattern_entered;
extern int da_width, da_height, slider_value, pix_height;
extern Node *node_info;
extern Pixmap net_pix;
extern Widget drawing_area;
extern GC draw_gc, eraser_gc, copy_gc;

int compute()
{
int i,j;
float exponent;
float *patt;
char *value,result[25];
XGCValues vals;
XFontStruct *font;
int x,y;

XFillRectangle(XtDisplay(drawing_area), net_pix, eraser_gc, 0, TopMargin, 50,
    pix_height);
XFillRectangle(XtDisplay(drawing_area), net_pix, eraser_gc,
    da_width-SideMargin/2-10, TopMargin, SideMargin/2+10, pix_height);

XGetGCValues(XtDisplay(drawing_area), draw_gc, GCFont, &vals);
font=XQueryFont(XtDisplay(drawing_area),vals.font);

patt = (float *) malloc(sizeof(float) * number_of_inputs);
value=strtok(motif_pattern_entered," ");
sscanf(value,"%f",patt);
x=10;
y=node_info[0].center_y+(font->ascent)/2;
XDrawString(XtDisplay(drawing_area), net_pix, draw_gc, x, y, value, strlen(value));

for (i = 1;i < number_of_inputs;i++)
   {
   value=strtok(NULL," ");
   if (!value)
       {
       XbError("Too few numbers entered in pattern.\nThe number of numbers entered must be equal to the number of input nodes.");
       return(0);
       }
   sscanf(value,"%f",(patt + i));
   y=node_info[i].center_y+(font->ascent)/2;

   XDrawString(XtDisplay(drawing_area), net_pix, draw_gc, x, y, value, strlen(value));
   }

/* Calculate outputs for first middle layer */
for (i = middle_layer_start;i < third_layer_start;i++)
   {
   exponent = 0.0;
   for (j = (node + i)->input_from;j < ((node + i)->input_to + 1);j++)
      {
      exponent = exponent + *((node + i)->weight + j - (node +i)->input_from)  *
      *((patt + j - (node + i)->input_from));
      }
   (node + i)->output = 1.0 / (1.0 + exp(-(exponent + (node + i)->threshold)));   
}

/* Calculate outputs for rest of middle layers and output nodes */
for (i = third_layer_start;i < number_of_nodes;i++)
   {
   exponent = 0.0;
   for (j = (node + i)->input_from;j < ((node + i)->input_to + 1);j++)
      {
      exponent = exponent + *((node + i)->weight + j - (node + i)->input_from) *
      (node + j)->output;
      }
   (node + i)->output = 1 / (1 + exp(-(exponent + (node + i)->threshold)));
   }

/* Display outputs */
x=da_width-SideMargin/2-10;
for (i = (number_of_nodes - number_of_outputs);i < number_of_nodes;i++)
   {
   sprintf(result,"%f",(node + i)->output);

   y=node_info[i].center_y;
   y+=(font->ascent)/2;

   XDrawString(XtDisplay(drawing_area), net_pix, draw_gc, x, y, result, strlen(result));
   }

XCopyPlane(XtDisplay(drawing_area), net_pix, XtWindow(drawing_area), copy_gc, 0,
    TopMargin+slider_value, da_width, da_height-TopMargin, 0, TopMargin, 1);    

XCopyPlane(XtDisplay(drawing_area), net_pix, XtWindow(drawing_area), copy_gc, 0,
    0, da_width, TopMargin, 0, 0, 1);

} 
