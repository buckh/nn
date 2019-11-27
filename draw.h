/*
 *  NeuralNet - draw.h
 *     Version 2.0
 *     by Eugene Hodges, IV (ewhodges@eos.ncsu.edu)
 *
 *     Copyright 1992, 1993 by Eugene Hodges
 *     All Rights Reserved
 *
 *     Permission to use, copy, and distribute this software and text for
 *     non-commercial purposes and without fee is hereby granted, provided
 *     that this notice appears in all copies.
 *
 *     The author disclaims all warranties with regard to the software or
 *     text including all implied warranties of merchantability and fitness.
 *
 *     In no event shall the author or NCSU be liable for any special,
 *     indirect or cosequential damages or any damages whatsoever
 *     resulting from loss of use, data or profits, whether in an
 *     action of contract, negligence or other tortious action,
 *     arising out of or in connection with the use or performance
 *     of this software or text.
 *
 */

#include <stdio.h>

/* necessary X11 include files */
#include <X11/Intrinsic.h>
#include <X11/Xlib.h>
#include <X11/Xutil.h>

/* necessary Motif include files */
#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/DrawingA.h>
#include <Xm/ScrollBar.h>
#include <Xm/FileSB.h>
#include <Xm/Text.h>

/* necessary include file for Xb functions */
#include "XButil.h"

#define BlockHeight  60      /* height of node circle plus border area (20 + 20 + 20) */
#define TopMargin    25      /* margin where the title (Values, etc) is written */
#define NodeRadius   10      /* the radius of a node circle */
#define SideMargin   80      /* margin where the input and output values are shown */


/* structure used to hold information about each node in the network;
   for each node:
       node's center coordinates on the drawing area
       whether it's sending/receiving input to/from other nodes  */
typedef struct {
               int center_x, center_y;
               Boolean in_connected, out_connected;
               } Node;


/* structure used to form a single linked list with each node being one line in
   *.net (netlist) format; the difference is instead of number of nodes including
   the beginning one used in the connection as in the .net file format, this 
   structure holds the beginning and ending nodes for the in and out (look at
   a .net file and you will see the difference); 
   for each net file line:
       the beginning and end nodes of the nodes sending the output
       the beginning and end nodes of the nodes receiving that output
   
   It represents the 'network:' section only from the .net file format.
   also:  begin_out always < end_out, begin_in always < end_in */
typedef struct net_list{
               struct net_list *next;
               int begin_out, end_out,
                   begin_in, end_in;
               } NetList;


/* Creates and initializes the graphics contexts (gc's) used by the program */
void setup_gcs();


/* Handles the callback from the scrollbar (valueChanged, drag) */
void scrollCB(Widget w, XtPointer client_data, XmScrollBarCallbackStruct *call_data);


/* Gets the following information about the display:
       foreground and background colors
       root window of screen
       depth of screen  */
void get_display_info();


/* Handles the resizing of the drawing area widget (drawing_area); reconstructs
   the pixmap for the new window size and redraws all the information on it */
void da_resizeCB(Widget w, caddr_t client_data, XmDrawingAreaCallbackStruct *call_data);


/* Handles the expose event for the drawing area widget (drawing_area) by 
   copying from the pixmap the exposed area to the window */
void da_exposeCB(Widget w, caddr_t client_data, XmDrawingAreaCallbackStruct *call_data);


/* Constructs an unconnected layout of nodes on the pixmap from the following parameters
   (obtained from the 'Network -> New' menu option (dialog)):
       number of input nodes (num_in_nodes)
       number of output nodes (num_out_nodes)
       number of hidden layers (num_hid_layers)
       number of nodes in each hidden layer (num_hid_nodes[3])

   The layout of nodes is centered according to the layer with the highest number of
   nodes and the following visual characteristics of the layout:
       BlockHeight  60     -- height of node circle plus border area (20 + 20 +20)
       TopMargin    25     -- margin where the title (Values, etc) is written
       NodeRadius   10     -- the radius of a node circle
       SideMargin   80     -- margin where the input and output values are shown 

   Also, the array node_info (of type Node) is initialized with the info about each
   node (x and y coordinates of node center, in_connected and out_connected set to
   False except for input layer nodes which have in_connected initialized to True and
   output layer nodes which have out_connected to True for the obvious reasons). */
void set_up_net();


/* Monitors the button clicks and makes the appropriate action.  For the first 
   button it allows the nodes to be connected whenever a network is being created.
   For the second button, it allows a node to be unselected (if it was the previous
   choice through the first button being clicked).  In both cases it determines the
   number of the node clicked on and checks the node_info array for information to 
   exclude all but valid nodes for clicking upon.  It also decides if the click is
   to select (highlight) or unselect (unhighlight) a node.  This function is the
   heart of network creation and modification.  To understand more, read the
   comments in the function. */
void node_monitor(Widget w, caddr_t data, XEvent *event);


/* Takes the single linked list net_list and draws the links from node to node on
   the pixmap and does not copy to the screen.  Used when reconstructing network
   (resize) or reading net in from file. */
void draw_links();


/* Deletes all the elements in the single linked list net_list, free the memory.   
   Each element in the list represents a line in the .net file format.*/
void delete_sll();


/* Adds an element to the single linked list net_list.  Each element in the list
   represents a line in the .net file format.  This function also inserts the 
   element in the list in order according to begin_out (from smallest to largest)
   so that storing the list to a .net file format is very simple. */
void add_elem(NetList *new);


/* Prints the single linked list to the stdout.  Useful only for debugging. */
void print_sll();


/* Takes the net_list single linked list and does the simple conversion to .net
   file format as it writes it to the file.  Saves the network in other words. */
Boolean save_net_list();


/* Reads in a .net file and does the simple conversion to the net_list single 
   linked list format.  */
Boolean read_net_list(FILE *file);


/* Determines from the information in the single linked list net_list the number
   of hidden layers in the network.  Used when reading in a .net file since this
   information is not explicitly stated in it. */
int layers();


/* Disconnects the nodes selected through the use of the 'Network -> Disconnect' 
   menu option.  The nodes disconnected are all the nodes in a line of the .net
   file format (which is the equivalent of one element in the single linked list
   net_list.  */
void disconnect_nodes(int i, int choice);


/* Deletes an element from the single linked list net_list.  This action is the
   equivalent of removing a line from the .net file format. */
void delete_node(NetList **del);


/* Fully connects the network and is called from the 'Network -> Fully Connect'
   menu option and answering OK to the dialog.  It deletes any existing net_list
   and then creates a net_list single linked list that specifies a fully connected
   network.  It then calls draw_links to draw all the lines. */
void fully_connect();


/* Sets up net_array for actual neural net functions instead of from a file.  It
   essentially does a .net conversion as it creates net_array.  This is called
   every time the network is completed. */
void read_network_from_net_list();
