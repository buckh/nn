/*
 *  NeuralNet - draw.c
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

/* header file for this file */
#include "draw.h"

/* header file for for ai files */
#include "header.h"

/* include bitmaps (cursor and mask) for custom cursors--Not used.  Left for the future?
#include "one.cursor"
#include "two.cursor"
#include "three.cursor"
#include "four.cursor"
#include "one_mask"
#include "two_mask"
#include "three_mask"
#include "four_mask"    */

/* classifications of nodes */
#define INPUT        1
#define HID1         2
#define HID2         3
#define HID3         4
#define OUTPUT       5

/* the numbers aren't important, just that they're distinct for use in callbacks */
#define OK           1
#define CANCEL       2

/* the max num of links from one layer to the next that will be drawn before a 
   trapezoid is drawn instead */
#define MAX_DRAW_LINES  1000

/* Min and Max macros */
#define Min(x,y)     ((x < y) ? (x) : (y))
#define Max(x,y)     ((x > y) ? (x) : (y))

/* external functions and variables (in interface.c) */
extern void create_net_save_dialog();
extern void wts_menu_settings();

extern Widget toplevel,
              main_form,
              scroll, 
              net_save_dialog,
              weights_save_option, 
              wts_file_text,
              total_iter_text,
              tss_text,
              network_save_option,
              network_disconnect_option, 
              pattern_test_option;

extern int num_hid_layers, 
           num_in_nodes, 
           num_out_nodes, 
           num_hid_nodes[3], 
           total_iterations;

extern XmStringCharSet char_set;

extern Boolean disconnect,
               net_file_open,
               wts_file_open, 
               net_changed;


/* Global and file declarations. */
Widget drawing_area, 
       ques_dialog;

GC draw_gc,                     /* regular drawing gc */
   title_gc,                    /* gc for drawing title */
   highlight_gc,                /* xor gc for selecting/unselecting nodes */
   eraser_gc,                   /* erases lines in disconnection */
   copy_gc,                     /* for copying from net_pix to drawing_area (has depth
                                   of screen) */
   da_highlight_gc;             /* for doing highlighting in drawing area (has depth of
                                   screen) */

int da_height,                  /* height of the drawing area (from its resource) */
    da_width,                   /* width of the drawing area (from its resource) */
    slider_value,               /* value of the scroll bar */
    pix_height;                 /* height of the pixmap net_pix */

static int foreground,          /* foreground color of the drawing area */
           background,          /* background color of the drawing area */
           depth,               /* depth of the display */
           largest,             /* largest number of nodes in any layer */
           total_circles;       /* total number of nodes in the network */ 

static Window root_window;      /* root window id */

/* Not used.  Future? */
/* static Cursor one_cursor, two_cursor, three_cursor, four_cursor;  */

Pixmap net_pix;                 /* pixmap of network with drawing, etc. */

static XFontStruct *title_font, /* drawing title font */
                   *draw_font;  /* font for numbers in node circles */

Boolean draw=False,             /* true when the user is creating a network and
                                   when the modification is allowed (i.e. not 
                                   while other things are being calculated, etc.) */
        reset=False,            /* true when the node_monitor function is to be reset
                                   (when network closed) */
        net_complete=False;     /* true only when all nodes are connected and
                                   signifies the net is complete and can be
                                   trained and/or saved */

Node *node_info;                /* array of Node in which an index is a node
                                   number; nodes are numbered consecutively
                                   beginning at 0 for the first input node */
static NetList *net_list=NULL,  /* single linked list in which each list element
                                   represents a line from the .net file format's
                                   'network:' section */
               *temp=NULL;      /* temporary variable used to go into linked list */

char new_net_filename[200];     /* current network file name (.net) */


/* callback for disconnect question dialog. */
static void quesCB(Widget w, int client_data, XmAnyCallbackStruct *call_data)
    {
    switch(client_data)
        {
        case OK:
            /* go ahead and disconnect highlighted nodes */
            disconnect_nodes(NULL, 2);
            break;
        case CANCEL:
            /* cancel disconnect operation */
            disconnect_nodes(NULL, 3);
            break;
        }

    XtUnmanageChild(w);
    }


/* question dialog which asks user to if it is ok to disconnect highlighted nodes */
void create_question_dialog()
    {
    Arg al[4];
    int ac;

    ac=0;
    XtSetArg(al[ac], XmNdialogTitle,XmStringCreateLtoR(
        "NeuralNet: Question Dialog", XmSTRING_DEFAULT_CHARSET)); ac++;
    XtSetArg(al[ac], XmNmessageAlignment, XmALIGNMENT_CENTER); ac++;
    ques_dialog = XmCreateQuestionDialog(toplevel,"ques_dialog", al, ac);

    XtAddCallback(ques_dialog, XmNokCallback, quesCB, OK);
    XtAddCallback(ques_dialog, XmNcancelCallback, quesCB, CANCEL);

    XtUnmanageChild(XmMessageBoxGetChild(ques_dialog, XmDIALOG_HELP_BUTTON));
    }


/* sets the messageString resource of the disconnect question dialog and manages it */
void question (char *s)
    {
    Arg al[2];
    int ac;
    XmString question;

    question=XmStringCreateLtoR(s, char_set);

    ac = 0;
    XtSetArg(al[ac], XmNmessageString, question);  ac++;
    XtSetValues(ques_dialog,al,ac);

    XmStringFree(question);

    XtManageChild(ques_dialog);
    }


/* Gets the following information about the display:
       foreground and background colors
       root window of screen
       depth of screen  */
void get_display_info()
    {
    Arg al[10];
    int ac;

    /* get the current fg and bg colors. */
    ac=0;
    XtSetArg(al[ac], XmNforeground, &foreground); ac++;
    XtSetArg(al[ac], XmNbackground, &background); ac++;
    XtGetValues(drawing_area, al, ac);

    root_window=RootWindowOfScreen(XtScreen(drawing_area));
    depth=DefaultDepthOfScreen(XtScreen(drawing_area));
    }


/* Creates and initializes the graphics contexts (gc's) used by the program */
void setup_gcs()
/* set up the graphics contexts */
    {
    XGCValues vals;
    Pixmap p;

    p=XCreatePixmap(XtDisplay(drawing_area), root_window, 1, 1, 1);

    vals.plane_mask = 0x1L;

    /* set the gc values and create the drawing gc */
    vals.foreground = foreground;
    vals.background = background;
    draw_font=XLoadQueryFont(XtDisplay(drawing_area),
        "-Adobe-Helvetica-Medium-r-Normal--10-100-*");
    vals.font = draw_font->fid;
    draw_gc=XCreateGC(XtDisplay(drawing_area), p, GCFont | GCForeground | GCBackground |
        GCPlaneMask, &vals);
    copy_gc=XCreateGC(XtDisplay(drawing_area), XtWindow(drawing_area), GCFont | GCForeground
        | GCBackground, &vals); 

    /* set the gc values and create the eraser (fg=bg) gc */
    vals.foreground = 0;
    vals.background = 0;
    eraser_gc=XCreateGC(XtDisplay(drawing_area), p, GCForeground | GCBackground |
        GCPlaneMask, &vals);

    /* set the gc values and create the highlight (xor) gc */
    vals.foreground = 1;
    vals.background = 0;
    vals.font = draw_font->fid;
    vals.function = GXxor;
    highlight_gc=XCreateGC(XtDisplay(drawing_area), p, GCFunction | GCFont | GCForeground |
        GCBackground | GCPlaneMask, &vals);

    vals.foreground = foreground ^ background;
    vals.background = background;
    da_highlight_gc=XCreateGC(XtDisplay(drawing_area), XtWindow(drawing_area), GCFunction | 
        GCFont | GCForeground | GCBackground, &vals);

    /* set the gc values and create the title gc (only to draw title text) */
    vals.foreground = 1;
    vals.background = 0;
    title_font=XLoadQueryFont(XtDisplay(drawing_area),
        "-Adobe-Helvetica-Bold-r-Normal--12-120-*");
    vals.font = title_font->fid;
    title_gc=XCreateGC(XtDisplay(drawing_area), p, GCFont | GCForeground | GCBackground |
        GCPlaneMask, &vals);
    }


/* Handles the resizing of the drawing area widget (drawing_area); reconstructs
   the pixmap for the new window size and redraws all the information on it */
void da_resizeCB(Widget w, caddr_t client_data, XmDrawingAreaCallbackStruct *call_data)
    {
    Arg al[10];
    int ac,i;
    Dimension temp_width, temp_height;
    Pixmap temp1,temp2;
    Node *temp_node_info;

    if (XtIsRealized && draw)
        {

        /* get new height, width */
        temp_height=da_height;
        ac=0;
        XtSetArg(al[ac], XmNheight, &da_height); ac++;
        XtSetArg(al[ac], XmNwidth, &temp_width); ac++;
        XtGetValues(drawing_area, al, ac);

/*        if (((temp_value + da_height)>pix_height) && (pix_height>da_height))
            slider_value=pix_height-da_height;
        else
            slider_value=temp_value;
        XCopyPlane(XtDisplay(drawing_area), net_pix, XtWindow(drawing_area), copy_gc, 0,
            slider_value+TopMargin, da_width, da_height-TopMargin, 0, TopMargin, 1);    
*/
        /* make appropriate changes to scroll bar */
        ac=0;
        XtSetArg(al[ac], XmNsliderSize, da_height); ac++;
        XtSetArg(al[ac], XmNpageIncrement, da_height); ac++;
        XtSetValues(scroll, al, ac);

        /* if the width has changed, copy the input and output values, recreate
           the network on a new pixmap, and copy the input and output values to the
           new pixmap */
        if (temp_width != da_width)
            {
            /* copy the input and output values */
            temp1 = XCreatePixmap (XtDisplay(drawing_area), root_window, SideMargin,
                pix_height, 1);
            temp2 = XCreatePixmap (XtDisplay(drawing_area), root_window, SideMargin,
                pix_height, 1);
            XCopyPlane(XtDisplay(drawing_area), net_pix, temp1, draw_gc, 0, 0, SideMargin,
                pix_height, 0, 0, 1);
            XCopyPlane(XtDisplay(drawing_area), net_pix, temp2, draw_gc, da_width-SideMargin,
                0, SideMargin, pix_height, 0, 0, 1);

            /* recreate network pixmap, freeing old one, and don't draw to screen until 
               completely finished */
            draw=False;
            temp_node_info=node_info;
            XFreePixmap(XtDisplay(drawing_area), net_pix);
            set_up_net();
            draw_links();

            /* put connection info back into node_info and free old node info 
               (temp_node_info) */
            for(i=0; i < total_circles; i++)
                {
                node_info[i].in_connected=temp_node_info[i].in_connected;
                node_info[i].out_connected=temp_node_info[i].out_connected;
                }
            free(temp_node_info);

            /* redraw input and output values to the pixmap here */
            XCopyPlane(XtDisplay(drawing_area), temp1, net_pix, draw_gc, 0, 0, SideMargin,
                pix_height, 0, 0, 1);
            XCopyPlane(XtDisplay(drawing_area), temp2, net_pix, draw_gc, 0, 0, SideMargin,
                pix_height, da_width-SideMargin, 0, 1);
            XFreePixmap(XtDisplay(drawing_area), temp1);
            XFreePixmap(XtDisplay(drawing_area), temp2);
            }
        }
    }


/* Handles the expose event for the drawing area widget (drawing_area) by 
   copying from the pixmap the exposed area to the window */
void da_exposeCB(Widget w, caddr_t client_data, XmDrawingAreaCallbackStruct *call_data)
    {
    XExposeEvent *event = (XExposeEvent *) call_data->event;

    if (XtIsRealized && draw)
        {
        XCopyPlane(XtDisplay(drawing_area), net_pix, XtWindow(drawing_area), copy_gc, event->x,
            event->y+slider_value, event->width, event->height, event->x, event->y, 1);
        XCopyPlane(XtDisplay(drawing_area), net_pix, XtWindow(drawing_area), copy_gc, 0,
            0, da_width, TopMargin, 0, 0, 1);    
        }
    }


/*  Not used.
void create_cursors()
    {
    one_cursor=XbCreateCursor(drawing_area, one_bits, one_mask_bits, one_width, 
        one_height, one_x_hot, one_y_hot);
    two_cursor=XbCreateCursor(drawing_area, two_bits, two_mask_bits, two_width, 
        two_height, two_x_hot, two_y_hot);
    three_cursor=XbCreateCursor(drawing_area, three_bits, three_mask_bits, three_width, 
        three_height, three_x_hot, three_y_hot);
    four_cursor=XbCreateCursor(drawing_area, four_bits, four_mask_bits, four_width, 
        four_height, four_x_hot, four_y_hot);
    }
*/


/* Handles the callback from the scrollbar (valueChanged, drag) */
void scrollCB(Widget w, XtPointer client_data, XmScrollBarCallbackStruct *call_data)
    {
    slider_value=call_data->value;
    if (draw)
        XCopyPlane(XtDisplay(drawing_area), net_pix, XtWindow(drawing_area), copy_gc, 0,
            slider_value+TopMargin, da_width, da_height-TopMargin, 0, TopMargin, 1);    
    }


/* draws a string centered about the coordinates x, y */
void draw_string_centered(Widget w, Window drawable, GC gc, int x, int y, char *str, 
    XFontStruct *font)
    {
    int str_width;

    /* get the font characteristics and compute offset */
    str_width=XTextWidth(font, str, strlen(str));
    x-=str_width/2;
    y+=(font->ascent)/2;

    XDrawString(XtDisplay(w), drawable, gc, x, y, str, strlen(str));
    }


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
void set_up_net()
    {
    Arg al[10];
    int ac, i, start_y, cnt=0;
    Dimension orig_width, width, height;
    char temp[10];
    XArc *circles;
    XGCValues vals;
    XFontStruct *font;
    int str_width;

    XbWatchCursor(toplevel);

    /* find the largest number of nodes in any layer (for centering purposes) */
    if (num_in_nodes>num_out_nodes)
        largest=num_in_nodes;
    else
        largest=num_out_nodes;

    for (i=0;i<3;i++)
        if (largest<num_hid_nodes[i])
            largest=num_hid_nodes[i];

    /* compute total number of nodes and allocate a XArc structure to store the
       characteristics of each circle */
    total_circles=num_in_nodes + num_out_nodes + num_hid_nodes[0] + 
        num_hid_nodes[1] + num_hid_nodes[2];
    circles=(XArc *)malloc(total_circles * sizeof(XArc));

    /* allocate the node_info array with one index per node in network */
    node_info=(Node *)malloc(total_circles * sizeof(Node));

    /* get the current height and width of the drawing area */
    ac=0;
    XtSetArg(al[ac], XmNwidth, &orig_width); ac++;
    XtSetArg(al[ac], XmNheight, &height); ac++;
    XtGetValues(drawing_area, al, ac);
    da_width=width=orig_width;
    da_height=height;

    /* compute the height of the pixmap */
    pix_height=largest*BlockHeight+TopMargin;

    /* set the appropriate resources of the scroll bar to properly reflect drawing area
       and pixmap heights */
    ac=0;
    XtSetArg(al[ac], XmNmaximum, (pix_height < height) ? height : pix_height); ac++;
    XtSetArg(al[ac], XmNsliderSize, height ); ac++;
    XtSetArg(al[ac], XmNpageIncrement, height); ac++;
    XtSetValues(scroll, al, ac);

    /* allocate the pixmap net_pix and clear it */
    net_pix = XCreatePixmap(XtDisplay(drawing_area), root_window, width, pix_height, 1);
    XFillRectangle(XtDisplay(drawing_area), net_pix, eraser_gc, 0, 0, width, pix_height);
    draw=True;

    /* draw the layer titles in the title */
    XGetGCValues(XtDisplay(drawing_area), title_gc, GCFont, &vals);
    font=XQueryFont(XtDisplay(drawing_area),vals.font);
    draw_string_centered(drawing_area, net_pix, title_gc, SideMargin, 10, "Input", font);
    draw_string_centered(drawing_area, net_pix, title_gc, width/2, 10, "Hidden", font);
    draw_string_centered(drawing_area, net_pix, title_gc, width-SideMargin, 10, "Output",
        font);

    /* center the first Values in the left side margin and the second Values in right
       side margin */
    str_width=XTextWidth(font, "Values", strlen("Values"));
    XDrawString(XtDisplay(drawing_area), net_pix, title_gc, 5, 10+(font->ascent)/2, "Values",
        strlen("Values"));
    XDrawString(XtDisplay(drawing_area), net_pix, title_gc, width-str_width-5, 
        10+(font->ascent)/2, "Values", strlen("Values"));

    /* draw line under title */
    XDrawLine(XtDisplay(drawing_area), net_pix, draw_gc, 0, 15+(font->ascent)/2, width,
        15+(font->ascent)/2);

    XFreeFontInfo(NULL, font, 1);

    XGetGCValues(XtDisplay(drawing_area), draw_gc, GCFont, &vals);
    font=XQueryFont(XtDisplay(drawing_area),vals.font);

    /* compute the starting y coordinate for the first input node and then draw the input
       nodes in the first column of nodes; initialize the node_info array appropriately;
       number the nodes */
    start_y=(pix_height - TopMargin - (num_in_nodes * BlockHeight))/2 + TopMargin + BlockHeight/2;
    for (i=0;i<num_in_nodes;i++,cnt++)
        {
        circles[cnt].x=(short )(SideMargin-NodeRadius);
        circles[cnt].y=(short )(start_y+i*BlockHeight-NodeRadius);
        circles[cnt].width=circles[cnt].height=(unsigned short )(NodeRadius*2);
        circles[cnt].angle1=(short )0;
        circles[cnt].angle2=(short )(360*64);

        node_info[cnt].center_x=SideMargin;
        node_info[cnt].center_y=start_y + i*BlockHeight;
        node_info[cnt].in_connected=True;
        node_info[cnt].out_connected=False;
        
        sprintf(temp,"%d",cnt);
        draw_string_centered(drawing_area, net_pix, draw_gc, SideMargin, start_y+i*BlockHeight, temp, font);
        }

    if (num_hid_layers==1)
        {
        /* adjust width so that round off error will not affect appearance (width now a 
           multiple of 2) */
        width=(orig_width/2)*2;

        /* compute the starting y coordinate for the first node in the only middle layer and 
           then draw the nodes in the middle column of nodes; initialize the node_info
           array appropriately; number the nodes */
        start_y=(pix_height - TopMargin - (num_hid_nodes[0] * BlockHeight))/2 + TopMargin + BlockHeight/2;
        for (i=0;i<num_hid_nodes[0];i++,cnt++)
            {
            circles[cnt].x=(short )((width/2)-NodeRadius);
            circles[cnt].y=(short )(start_y+i*BlockHeight-NodeRadius);
            circles[cnt].width=circles[cnt].height=(unsigned short )(NodeRadius*2);
            circles[cnt].angle1=(short)0;
            circles[cnt].angle2=(short )(360*64);

            node_info[cnt].center_x=width/2;
            node_info[cnt].center_y=start_y + i*BlockHeight;
            node_info[cnt].in_connected=node_info[cnt].out_connected=False;
        
            sprintf(temp,"%d",cnt);
            draw_string_centered(drawing_area, net_pix, draw_gc, width/2, start_y+i*BlockHeight, temp, font);
            }
        }

    if (num_hid_layers==2)
        {
        /* adjust width so that round off error will not affect appearance (width now a 
           multiple of 3) */
        width=(orig_width/3)*3;

        /* compute the starting y coordinate for the first node in the first middle layer and 
           then draw the nodes in the second column of nodes; initialize the node_info
           array appropriately; number the nodes */
        start_y=(pix_height - TopMargin - (num_hid_nodes[0] * BlockHeight))/2 + TopMargin + BlockHeight/2;
        for (i=0;i<num_hid_nodes[0];i++,cnt++)
            {
            circles[cnt].x=(short )(((width-150)/3)-NodeRadius+SideMargin);
            circles[cnt].y=(short )(start_y+i*BlockHeight-NodeRadius);
            circles[cnt].width=circles[cnt].height=(unsigned short )(NodeRadius*2);
            circles[cnt].angle1=(short)0;
            circles[cnt].angle2=(short )(360*64);

            node_info[cnt].center_x=(width-150)/3 + SideMargin;
            node_info[cnt].center_y=start_y + i*BlockHeight;
            node_info[cnt].in_connected=node_info[cnt].out_connected=False;
        
            sprintf(temp,"%d",cnt);
            draw_string_centered(drawing_area, net_pix, draw_gc, (width-150)/3+SideMargin, start_y+i*BlockHeight, temp, font);
            }

        /* compute the starting y coordinate for the first node in the second middle layer and 
           then draw the nodes in the third column of nodes; initialize the node_info
           array appropriately; number the nodes */
        start_y=(pix_height - TopMargin - (num_hid_nodes[1] * BlockHeight))/2 + TopMargin + BlockHeight/2;
        for (i=0;i<num_hid_nodes[1];i++,cnt++)
            {
            circles[cnt].x=(short )((2*(width-150)/3)-NodeRadius+SideMargin);
            circles[cnt].y=(short )(start_y+i*BlockHeight-NodeRadius);
            circles[cnt].width=circles[cnt].height=(unsigned short )(NodeRadius*2);
            circles[cnt].angle1=(short)0;
            circles[cnt].angle2=(short )(360*64);

            node_info[cnt].center_x=2*(width-150)/3 + SideMargin;
            node_info[cnt].center_y=start_y + i*BlockHeight;
            node_info[cnt].in_connected=node_info[cnt].out_connected=False;
        
            sprintf(temp,"%d",cnt);
            draw_string_centered(drawing_area, net_pix, draw_gc, 2*(width-150)/3+SideMargin, start_y+i*BlockHeight, temp, font);
            }
        }

    if (num_hid_layers==3)
        {
        /* adjust width so that round off error will not affect appearance (width now a 
           multiple of 4) */
        width=(orig_width/4)*4;

        /* compute the starting y coordinate for the first node in the first middle layer and 
           then draw the nodes in the second column of nodes; initialize the node_info
           array appropriately; number the nodes */
        start_y=(pix_height - TopMargin - (num_hid_nodes[0] * BlockHeight))/2 + TopMargin + BlockHeight/2;
        for (i=0;i<num_hid_nodes[0];i++,cnt++)
            {
            circles[cnt].x=(short )((width-150)/4-NodeRadius+SideMargin);
            circles[cnt].y=(short )(start_y+i*BlockHeight-NodeRadius);
            circles[cnt].width=circles[cnt].height=(unsigned short )(NodeRadius*2);
            circles[cnt].angle1=(short)0;
            circles[cnt].angle2=(short )(360*64);

            node_info[cnt].center_x=(width-150)/4 + SideMargin;
            node_info[cnt].center_y=start_y + i*BlockHeight;
            node_info[cnt].in_connected=node_info[cnt].out_connected=False;
        
            sprintf(temp,"%d",cnt);
            draw_string_centered(drawing_area, net_pix, draw_gc, (width-150)/4+SideMargin, start_y+i*BlockHeight, temp, font);
            }

        /* compute the starting y coordinate for the first node in the second middle layer and 
           then draw the nodes in the third column of nodes; initialize the node_info
           array appropriately; number the nodes */
        start_y=(pix_height - TopMargin - (num_hid_nodes[1] * BlockHeight))/2 + TopMargin + BlockHeight/2;
        for (i=0;i<num_hid_nodes[1];i++,cnt++)
            {
            circles[cnt].x=(short )((2*(width-150)/4)-NodeRadius+SideMargin);
            circles[cnt].y=(short )(start_y+i*BlockHeight-NodeRadius);
            circles[cnt].width=circles[cnt].height=(unsigned short )(NodeRadius*2);
            circles[cnt].angle1=(short)0;
            circles[cnt].angle2=(short )(360*64);

            node_info[cnt].center_x=2*(width-150)/4 + SideMargin;
            node_info[cnt].center_y=start_y + i*BlockHeight;
            node_info[cnt].in_connected=node_info[cnt].out_connected=False;
        
            sprintf(temp,"%d",cnt);
            draw_string_centered(drawing_area, net_pix, draw_gc, 2*(width-150)/4+SideMargin, start_y+i*BlockHeight, temp, font);
            }

        /* compute the starting y coordinate for the first node in the third middle layer and 
           then draw the nodes in the fourth column of nodes; initialize the node_info
           array appropriately; number the nodes */
        start_y=(pix_height - TopMargin - (num_hid_nodes[2] * BlockHeight))/2 + TopMargin + BlockHeight/2;
        for (i=0;i<num_hid_nodes[2];i++,cnt++)
            {
            circles[cnt].x=(short )((3*(width-150)/4)-NodeRadius+SideMargin);
            circles[cnt].y=(short )(start_y+i*BlockHeight-NodeRadius);
            circles[cnt].width=circles[cnt].height=(unsigned short )(NodeRadius*2);
            circles[cnt].angle1=(short)0;
            circles[cnt].angle2=(short )(360*64);

            node_info[cnt].center_x=3*(width-150)/4 + SideMargin;
            node_info[cnt].center_y=start_y + i*BlockHeight;
            node_info[cnt].in_connected=node_info[cnt].out_connected=False;
        
            sprintf(temp,"%d",cnt);
            draw_string_centered(drawing_area, net_pix, draw_gc, 3*(width-150)/4+SideMargin, start_y+i*BlockHeight, temp, font);
            }
        }


    /* compute the starting y coordinate for the first node in the output layer and 
       then draw the nodes in the last column of nodes; initialize the node_info
       array appropriately; number the nodes */
    start_y=(pix_height - TopMargin - (num_out_nodes * BlockHeight))/2 + TopMargin + BlockHeight/2;
    for (i=0;i<num_out_nodes;i++,cnt++)
        {
        circles[cnt].x=(short )(orig_width-SideMargin-NodeRadius);
        circles[cnt].y=(short )(start_y+i*BlockHeight-NodeRadius);
        circles[cnt].width=circles[cnt].height=(unsigned short )(NodeRadius*2);
        circles[cnt].angle1=(short)0;
        circles[cnt].angle2=(short )(360*64);

        node_info[cnt].center_x=orig_width - SideMargin;
        node_info[cnt].center_y=start_y + i*BlockHeight;
        node_info[cnt].in_connected=False;
        node_info[cnt].out_connected=True;
        
        sprintf(temp,"%d",cnt);
        draw_string_centered(drawing_area, net_pix, draw_gc, orig_width-SideMargin, start_y+i*BlockHeight, temp, font);
        }

    XFreeFontInfo(NULL, font, 1);

    /* draw the circles (nodes) onto the pixmap */
    XDrawArcs(XtDisplay(drawing_area), net_pix, draw_gc, circles, total_circles);

    free(circles);

    /* clear the drawing area and generate an expose event to display network */
    XClearArea(XtDisplay(drawing_area), XtWindow(drawing_area), 0, 0, 0, 0, TRUE);
    
    XbNormalCursor(toplevel);   
    }

/*  Not used
void cursor_monitor(Widget w, caddr_t data, XEvent *event)
    {
    printf("x:  %d, y:  %d\n", event->xmotion.x, event->xmotion.y);
    }
*/


/* returns the layer of a node given the node number; used in checking to make sure layers
   are connected from left to right */
int layer(int node_num)
    {

    if (num_hid_nodes[1]==0)
        return( (node_num < num_in_nodes) ? INPUT :
            (node_num < num_in_nodes + num_hid_nodes[0]) ? HID1 : 3);

    if (num_hid_nodes[2]==0)
        return( (node_num < num_in_nodes) ? INPUT :
            (node_num < num_in_nodes + num_hid_nodes[0]) ? HID1 :
            (node_num < num_in_nodes + num_hid_nodes[0] + num_hid_nodes[1]) ? HID2 : 4 );

    else
        return( (node_num < num_in_nodes) ? INPUT :
            (node_num < num_in_nodes + num_hid_nodes[0]) ? HID1 :
            (node_num < num_in_nodes + num_hid_nodes[0] + num_hid_nodes[1]) ? HID2 :
            (node_num < num_in_nodes + num_hid_nodes[0] + num_hid_nodes[1] + num_hid_nodes[2]) ? HID3 : OUTPUT);

    }


/* Disconnects the nodes selected through the use of the 'Network -> Disconnect' 
   menu option.  The nodes disconnected are all the nodes in a line of the .net
   file format (which is the equivalent of one element in the single linked list
   net_list.  */
void disconnect_nodes(int i, int choice)
    {
    static NetList *curr=NULL;
    static int j,k,cnt,num_circles;
    static XArc *circles;
    XSegment *lines;
    XPoint vertices[4];

    switch(choice)
        {
        case(1):
        /* 'Network -> Disconnect just chosen */

            /* display error message and return if no nodes connected */
            if (!net_list)
                {
                XbNormalCursor(drawing_area);
                XbError("There are no connected nodes.");
                disconnect=False;
                return;
                }
    
            /* find the appropriate element in single linked list (represents a
               line in the 'network:' section of the .net file) */
            for(curr=net_list;curr;curr=curr->next)
                if ((i<=curr->end_out) && (i>=curr->begin_out))
                break;

            /* allocate space for all the circles to be highlighted */
            circles=(XArc *)malloc(sizeof(XArc)*((curr->end_out-curr->begin_out+1)+
                (curr->end_in-curr->begin_in+1)));

            /* store all of the nodes sending output that are to be highlighted */
            for(k=0,j=curr->begin_out;j<=curr->end_out;j++,k++)
                {
                circles[k].x=(short )(node_info[j].center_x-NodeRadius);
                circles[k].y=(short )(node_info[j].center_y-NodeRadius);
                circles[k].width=circles[k].height=(unsigned short )(2*NodeRadius);
                circles[k].angle1=(short )0;
                circles[k].angle2=(short )360*64;
                }

            /* store all of the nodes receiving input that are to be highlighted */
            for(j=curr->begin_in;j<=curr->end_in;j++,k++)
                {
                circles[k].x=(short )(node_info[j].center_x-NodeRadius);
                circles[k].y=(short )(node_info[j].center_y-NodeRadius);
                circles[k].width=circles[k].height=(unsigned short )(2*NodeRadius);
                circles[k].angle1=(short )0;
                circles[k].angle2=(short )360*64;
                }

            /* record the number of circles highlighted */
            num_circles=k;

            /* highlight the circles on the pixmap and then copy it to the display */
            XFillArcs(XtDisplay(drawing_area), net_pix, highlight_gc, circles, num_circles);

            XCopyPlane(XtDisplay(drawing_area), net_pix, XtWindow(drawing_area), copy_gc, 0,
                TopMargin+slider_value, da_width, da_height-TopMargin, 0, TopMargin, 1);    

            XCopyPlane(XtDisplay(drawing_area), net_pix, XtWindow(drawing_area), copy_gc, 0,
                0, da_width, TopMargin, 0, 0, 1);

            XbNormalCursor(drawing_area);

            /* give the user a chance to change mind */
            question("Okay to disconnect selected nodes?");

            break;

        case(2):
        /* go ahead with disconnection */

           /* unhighlight the hightlighted nodes */
           XFillArcs(XtDisplay(drawing_area), net_pix, highlight_gc, circles, num_circles);
           free(circles);
            
           if (((curr->end_out - curr->begin_out + 1) *
                   (curr->end_in - curr->begin_in + 1)) <= MAX_DRAW_LINES)
               {
               /* allocate storage for the links to be erased */
               lines=(XSegment *)malloc(((curr->end_out)-(curr->begin_out)+1)*
                   ((curr->end_in)-(curr->begin_in)+1)*sizeof(XSegment));

               /* store the info for the lines to be erased */
               for(cnt=0,j=curr->begin_out;j<=(curr->end_out);j++)
                   for(k=curr->begin_in;k<=(curr->end_in);k++,cnt++)
                       {
                       lines[cnt].x1=(short )(node_info[j].center_x + NodeRadius + 1);
                       lines[cnt].y1=(short )(node_info[j].center_y);
                       lines[cnt].x2=(short )(node_info[k].center_x - NodeRadius - 1);
                       lines[cnt].y2=(short )(node_info[k].center_y);
                       }

               XDrawSegments(XtDisplay(drawing_area), net_pix, eraser_gc, lines,
                   ((curr->end_out)-(curr->begin_out)+1)*(curr->end_in-curr->begin_in+1));
               free(lines);
               }
           else
               {
               vertices[0].x=(short )(node_info[curr->end_out].center_x + NodeRadius + 1);
               vertices[0].y=(short )(node_info[curr->end_out].center_y);

               vertices[1].x=(short )(node_info[curr->begin_out].center_x + NodeRadius + 1);
               vertices[1].y=(short )(node_info[curr->begin_out].center_y);

               vertices[2].x=(short )(node_info[curr->begin_in].center_x - NodeRadius - 1);
               vertices[2].y=(short )(node_info[curr->begin_in].center_y);

               vertices[3].x=(short )(node_info[curr->end_in].center_x - NodeRadius - 1);
               vertices[3].y=(short )(node_info[curr->end_in].center_y);

               XFillPolygon(XtDisplay(drawing_area), net_pix, eraser_gc, vertices, 4,
                   Convex, CoordModeOrigin);
               }

            /* update data:  unconnect the output of the nodes that were sending output 
               and unconnect the input of the nodes that were receiving input */
            for(k=curr->begin_out;k<=curr->end_out;k++)
                node_info[k].out_connected=False;
            for(k=curr->begin_in;k<=curr->end_in;k++)
                node_info[k].in_connected=False;

            /* redraw the nodes in the operation to restore pixel lost in the circle; free 
               the memory allocations */

            /* delete nodes from net_list */
            delete_node(&curr);

            /* signify disconnect operation complete; set other status variables
               appropriately */
            disconnect=False;
            net_complete=False;
            net_changed=True;
            XtSetSensitive(network_save_option,False);
            XtSetSensitive(pattern_test_option,False);
            wts_file_open=False;
            wts_menu_settings();
            XmTextSetString(wts_file_text,"No file open.");

            /* copy changes to the screen */
            XCopyPlane(XtDisplay(drawing_area), net_pix, XtWindow(drawing_area), copy_gc, 0,
                TopMargin+slider_value, da_width, da_height-TopMargin, 0, TopMargin, 1);    

            XCopyPlane(XtDisplay(drawing_area), net_pix, XtWindow(drawing_area), copy_gc, 0,
                0, da_width, TopMargin, 0, 0, 1);
            break;

        case(3):
        /* cancel disconnect operation */

            /* unhighlight the highlighted nodes */
            XFillArcs(XtDisplay(drawing_area), net_pix, highlight_gc, circles, num_circles);
            free(circles);

            /* copy changes to the screen */
            XCopyPlane(XtDisplay(drawing_area), net_pix, XtWindow(drawing_area), copy_gc, 0,
                TopMargin+slider_value, da_width, da_height-TopMargin, 0, TopMargin, 1);    

            XCopyPlane(XtDisplay(drawing_area), net_pix, XtWindow(drawing_area), copy_gc, 0,
                0, da_width, TopMargin, 0, 0, 1);

            /* signify disconnect operation complete (cancelled in this case) */
            disconnect=False;
            break;
        }

    }


/* Monitors the button clicks and makes the appropriate action.  For the first 
   button it allows the nodes to be connected whenever a network is being created.
   For the second button, it allows a node to be unselected (if it was the previous
   choice through the first button being clicked).  In both cases it determines the
   number of the node clicked on and checks the node_info array for information to 
   exclude all but valid nodes for clicking upon.  It also decides if the click is
   to select (highlight) or unselect (unhighlight) a node.  This function is the
   heart of network creation and modification.  To understand more, read the
   comments in the function. */
void node_monitor(Widget w, caddr_t data, XEvent *event)
    {
    int x, y, i, j, k, cnt, swap;
    static int num_click=1, first_layer_sel, second_layer_sel, prev_sel=(-1), num_connected=0,
               first_node_selected;
    static XArc circles[3], circle;
    XSegment *lines;
    XPoint vertices[4];
    static Boolean single_node_disc=True;

    /* reset the status of node_monitor() for the 'Network -> Close' option */
    if (reset)
        {
        num_click=1;
        num_connected=0;
        delete_sll();
        net_complete=False;
        reset=False;
        return;
        }

    /* return if no drawing allowed or click was in a margin */
    if (!draw || (event->xbutton.y<=TopMargin) || (event->xbutton.x<=(SideMargin-NodeRadius))
              || (event->xbutton.x>=(da_width-SideMargin+NodeRadius)))
        return;

    /* simplify references to click coordinates */
    x=event->xbutton.x;
    y=event->xbutton.y;

    /* check to see if a node is selected */
    for(i=0; i < total_circles; i++)
        {
        if ((x < node_info[i].center_x + NodeRadius) && 
                (x > node_info[i].center_x - NodeRadius) &&
                ((y + slider_value) < node_info[i].center_y + NodeRadius) && 
                ((y + slider_value) > node_info[i].center_y - NodeRadius))
            break;
        }

    /* no node clicked upon */
    if (i==total_circles)
        return;

    /* for a disconnect operation */
    if (disconnect)
        {
        if (node_info[i].out_connected && (i<(total_circles-num_out_nodes)))
            {
            /* show nodes selected by the click (highlight) */
            disconnect_nodes(i,1);
            return;
            }
        else
            {
            /* nothing to disconnect */
            XbError("This node is not sending output to any other nodes.");
            XbNormalCursor(drawing_area);
            disconnect=False;
            return;
            }
        }

    /* allow unselecting of node highlighted on previous click through the use of second
       mouse button; single_node_disc ensures that operation is only on previous node
       selected */
    if ((num_click!=1) && (Button2==(event->xbutton.button)) && (single_node_disc))
        {
        if (prev_sel==i)   /* redundant check */
            {
            /* this make sure all cases properly taken care of; too hard to explain here,
               but if you run the program and play around, it will become clear */
            if (temp->begin_out!=i)
                {
                circle.x=(short )(node_info[i].center_x-NodeRadius);
                circle.y=(short )(node_info[i].center_y-NodeRadius);
                circle.width=circle.height=(unsigned short )(2*NodeRadius);
                circle.angle1=(short )0;
                circle.angle2=(short )360*64;
  
                XFillArc(XtDisplay(drawing_area), net_pix, highlight_gc,
                    node_info[i].center_x - NodeRadius, node_info[i].center_y - NodeRadius,
                    2*NodeRadius, 2*NodeRadius, 0, 360*64);

                XFillArc(XtDisplay(drawing_area), XtWindow(drawing_area), da_highlight_gc, 
                    node_info[i].center_x - NodeRadius, 
                    node_info[i].center_y - NodeRadius - slider_value,
                    2*NodeRadius, 2*NodeRadius, 0, 360*64);
                }

            switch(num_click)
                {
                case 2:
                    circle.x=(short )(node_info[i].center_x-NodeRadius);
                    circle.y=(short )(node_info[i].center_y-NodeRadius);
                    circle.width=circle.height=(unsigned short )(2*NodeRadius);
                    circle.angle1=(short )0;
                    circle.angle2=(short )360*64;
  
                    XFillArc(XtDisplay(drawing_area), net_pix, highlight_gc,
                        node_info[i].center_x - NodeRadius, node_info[i].center_y - NodeRadius,
                        2*NodeRadius, 2*NodeRadius, 0, 360*64);

                    XFillArc(XtDisplay(drawing_area), XtWindow(drawing_area), da_highlight_gc, 
                        node_info[i].center_x - NodeRadius, 
                        node_info[i].center_y - NodeRadius - slider_value,
                        2*NodeRadius, 2*NodeRadius, 0, 360*64);

                    node_info[i].out_connected=False;

                    /* don't allow disconnect from menu ('Network -> Disconnect') because
                       disconnecting a node that is in the process of being connected will
                       crash the program -- Reset this here. */
                    XtSetSensitive(network_disconnect_option, True);
                    break;
                case 3:
                    node_info[i].out_connected=False;
                    break;
                case 4:
                    node_info[i].in_connected=False;
                    break;
                }

            /* decrement the click counter and set the prev_sel to the node just unselected */
            num_click--;
            prev_sel=i;

            /* disallow unselect until another node is selected */
            single_node_disc=False;
            }

        return;
        }

    /* filter out third button clicks */
    if (Button1!=event->xbutton.button)
        return;

    /* actual selection of a node */
    switch(num_click)
        {
        case 1:
        /* first click */

            /* check to see if output already connected */
            if (node_info[i].out_connected)
                {
                XbError("This node is already sending output to other node(s).");
                return;
                }

            /* output now connected, remember layer of first node chosen */
            node_info[i].out_connected=True;
            first_layer_sel=layer(i);

            first_node_selected=i;

            /* allocate an element of NetList if appropriate */
            if ((prev_sel==(-1)) || (!temp))
                temp=(NetList *)malloc(sizeof(NetList));
            temp->begin_out=i;

            /* store drawing info in XArc array */
            circles[0].x=(short )(node_info[i].center_x-NodeRadius);
            circles[0].y=(short )(node_info[i].center_y-NodeRadius);
            circles[0].width=circles[0].height=(unsigned short )(2*NodeRadius);
            circles[0].angle1=(short )0;
            circles[0].angle2=(short )360*64;

            /* draw to pixmap and screen */
            XFillArc(XtDisplay(drawing_area), net_pix, highlight_gc,
                node_info[i].center_x - NodeRadius, node_info[i].center_y - NodeRadius,
                2*NodeRadius, 2*NodeRadius, 0, 360*64);

            XFillArc(XtDisplay(drawing_area), XtWindow(drawing_area), da_highlight_gc, 
                node_info[i].center_x - NodeRadius, 
                node_info[i].center_y - NodeRadius - slider_value,
                2*NodeRadius, 2*NodeRadius, 0, 360*64);

            /* don't allow disconnect from menu ('Network -> Disconnect') because
               disconnecting a node that is in the process of being connected will
               crash the program */
            XtSetSensitive(network_disconnect_option, False);

            break;
        case 2:
        /* second click */

            /* check for errors */
            if (layer(i) != first_layer_sel)
                {
                XbError("Second choice must be in the same column (layer)\nas your first choice.");
                return;
                }
            if (node_info[i].out_connected && (i!=prev_sel))
                {
                XbError("This node is already sending output to other node(s).");
                return;
                }
            if ((i!=(first_node_selected-1)) && (i!=(first_node_selected+1)) && 
                (i!=first_node_selected))
                {
                if (i>first_node_selected)
                    for (j=first_node_selected+1; j<i; j++)
                        if (node_info[j].out_connected)
                            {
                            XbError("A node between the first and second choices\nhas its output already connected.\nPlease choose a different node for the second choice.");
                            return;
                            }
                else
                    for (j=i; j<first_node_selected; j++)
                        if (node_info[j].out_connected)
                            {
                            XbError("A node between the first and second choices\nhas its output already connected.\nPlease choose a different node for the second choice.");
                            return;
                            }
                }

            /* output of current node selected is now connected */
            node_info[i].out_connected=True;

            /* show node selected; do not highlight again (which would unhighlight) if
               click on starting node */
            if (i!=temp->begin_out)
                {
                circles[1].x=(short )(node_info[i].center_x-NodeRadius);
                circles[1].y=(short )(node_info[i].center_y-NodeRadius);
                circles[1].width=circles[1].height=(unsigned short )(2*NodeRadius);
                circles[1].angle1=(short )0;
                circles[1].angle2=(short )360*64;

                XFillArc(XtDisplay(drawing_area), net_pix, highlight_gc,
                    node_info[i].center_x - NodeRadius, node_info[i].center_y - NodeRadius,
                    2*NodeRadius, 2*NodeRadius, 0, 360*64);

                XFillArc(XtDisplay(drawing_area), XtWindow(drawing_area), da_highlight_gc, 
                    node_info[i].center_x - NodeRadius, 
                    node_info[i].center_y - NodeRadius - slider_value,
                    2*NodeRadius, 2*NodeRadius, 0, 360*64);
                }
            else
                {
                circles[1].x=(short )0;
                circles[1].y=(short )0;
                circles[1].width=circles[1].height=(unsigned short )0;
                circles[1].angle1=(short )0;
                circles[1].angle2=(short )0;
                }

            /* set the end of the output equal to the current selection */
            temp->end_out=i;

            break;
        case 3:
        /* third click */

            /* check for errors */
            if (node_info[i].in_connected)
                {
                XbError("This node is already receiving input from other node(s).");
                return;
                }
            if (layer(i) != first_layer_sel + 1)
                {
                XbError("Third choice must be in the next column to the right of\nthe column of the first choice.");
                return;
                }
        
            /* make sure begin_out <= end_out; set here to ensure proper operation of 
               deselect (unhighlight) function */
            if (temp->begin_out > temp->end_out)
                {
                swap=temp->begin_out;
                temp->begin_out=temp->end_out;
                temp->end_out=swap;
                }

            /* input of the current node selected is now connected */
            node_info[i].in_connected=True;

            /* remember the second layer selected */
            second_layer_sel=layer(i);

            /* set the beginning of the input equal to the current selection */
            temp->begin_in=i;

            /* store the drawing information */
            circles[2].x=(short )(node_info[i].center_x-NodeRadius);
            circles[2].y=(short )(node_info[i].center_y-NodeRadius);
            circles[2].width=circles[2].height=(unsigned short )(2*NodeRadius);
            circles[2].angle1=(short )0;
            circles[2].angle2=(short )360*64;

            /* draw to pixmap and screen */
            XFillArc(XtDisplay(drawing_area), net_pix, highlight_gc,
                node_info[i].center_x - NodeRadius, node_info[i].center_y - NodeRadius,
                2*NodeRadius, 2*NodeRadius, 0, 360*64);

            XFillArc(XtDisplay(drawing_area), XtWindow(drawing_area), da_highlight_gc, 
                node_info[i].center_x - NodeRadius, 
                node_info[i].center_y - NodeRadius - slider_value,
                2*NodeRadius, 2*NodeRadius, 0, 360*64);

            break;
        case 4:
        /* fourth--the final--click */

             /* check for errors */
             if (layer(i) != second_layer_sel)
                {
                XbError("Fourth choice must be in the same column (layer)\nas your third choice.");
                return;
                }
            if (node_info[i].in_connected && (i!=prev_sel))
                {
                XbError("This node is already receiving input from other node(s).");
                return;
                }
            if ((i!=(prev_sel-1)) && (i!=(prev_sel+1)) && (i!=prev_sel))
                {
                if (i>prev_sel)
                    for (j=prev_sel+1; j<i; j++)
                        if (node_info[j].in_connected)
                            {
                            XbError("A node between the third and fourth choices\nhas its input already connected.\nPlease choose a different node for the fourth choice.");
                            return;
                            }
                else
                    for (j=i; j<prev_sel; j++)
                        if (node_info[j].in_connected)
                            {
                            XbError("A node between the third and fourth choices\nhas its input already connected.\nPlease choose a different node for the fourth choice.");
                            return;
                            }
                }

            XbWatchCursor(drawing_area);

            /* input of the current node selected is now connected */
            node_info[i].in_connected=True;

            /* store so begin_in always >= end_in */
            if (temp->begin_in > i)
                {
                temp->end_in=temp->begin_in;
                temp->begin_in=i;
                }
            else
                temp->end_in=i;

            /* add the element (line in .net file format's 'network:' section) to the
               single linked list net_list */
            add_elem(temp);

            /* unhighlight previous selections (connection now complete) */
            XFillArcs(XtDisplay(drawing_area), net_pix, highlight_gc, circles, 3);

            /* connect the outputs of the nodes selected to send output and the inputs
               of the nodes selected to receive input */
            for(j=temp->begin_out; j<(temp->end_out) ; j++)
                node_info[j].out_connected=True;
            for(k=temp->begin_in; k<(temp->end_in) ; k++)
                node_info[k].in_connected=True;


            if (((temp->end_out - temp->begin_out + 1) *
                     (temp->end_in - temp->begin_in + 1)) <= MAX_DRAW_LINES)
                {
                /* allocate space to draw links for connection just specified */
                lines=(XSegment *)malloc(((temp->end_out)-(temp->begin_out)+1)*
                      ((temp->end_in)-(temp->begin_in)+1)*sizeof(XSegment));

                /* store drawing information in XSegment structure */
                for(cnt=0,j=temp->begin_out;j<=(temp->end_out);j++)
                    for(k=temp->begin_in;k<=(temp->end_in);k++,cnt++)
                        {
                        lines[cnt].x1=(short )(node_info[j].center_x + NodeRadius + 1);
                        lines[cnt].y1=(short )(node_info[j].center_y);
                        lines[cnt].x2=(short )(node_info[k].center_x - NodeRadius - 1);
                        lines[cnt].y2=(short )(node_info[k].center_y);
                        }

                /* draw the links */
                XDrawSegments(XtDisplay(drawing_area), net_pix, draw_gc, lines,
                    ((temp->end_out)-(temp->begin_out)+1)*(temp->end_in-temp->begin_in+1));
                free(lines);
                }
            else
                {
                vertices[0].x=(short )(node_info[temp->end_out].center_x + NodeRadius + 1);
                vertices[0].y=(short )(node_info[temp->end_out].center_y);

                vertices[1].x=(short )(node_info[temp->begin_out].center_x + NodeRadius + 1);
                vertices[1].y=(short )(node_info[temp->begin_out].center_y);

                vertices[2].x=(short )(node_info[temp->begin_in].center_x - NodeRadius - 1);
                vertices[2].y=(short )(node_info[temp->begin_in].center_y);

                vertices[3].x=(short )(node_info[temp->end_in].center_x - NodeRadius - 1);
                vertices[3].y=(short )(node_info[temp->end_in].center_y);

                XFillPolygon(XtDisplay(drawing_area), net_pix, draw_gc, vertices, 4,
                    Convex, CoordModeOrigin);
                }

            /* draw to display */
            XCopyPlane(XtDisplay(drawing_area), net_pix, XtWindow(drawing_area), copy_gc, 0,
                TopMargin+slider_value, da_width, da_height-TopMargin, 0, TopMargin, 1);    

            XCopyPlane(XtDisplay(drawing_area), net_pix, XtWindow(drawing_area), copy_gc, 0,
                0, da_width, TopMargin, 0, 0, 1);    

            /* reset the temp variable for next use */
            temp=NULL;

            /* count number of nodes with input connected (to see if network complete) */
            for(j=0, num_connected=0;j<total_circles;j++)
                 {
                 if(node_info[j].in_connected && node_info[j].out_connected)
                     num_connected++;
                 }

            /* save network if complete */
            if (num_connected==(num_in_nodes+num_out_nodes+num_hid_nodes[0]+num_hid_nodes[1]+
                    num_hid_nodes[2]))
                {
                /* build net_array so that net can be trained/tested */
                read_network_from_net_list();

                /* set status variables, menu option sensitivities */
                net_complete=True;
                net_changed=True;
                XtSetSensitive(pattern_test_option,True);
                XmTextSetString(total_iter_text,"0");
                XmTextSetString(tss_text,"");
                total_iterations=0;
                wts_file_open=True;
                wts_menu_settings();  
                XmTextSetString(wts_file_text,"Generated weights.");
                XtSetSensitive(weights_save_option, False);
                if (strlen(new_net_filename))
		    XtSetSensitive(network_save_option, True);

                /* generate random weights for the links */
                make_weights();

                /* initial total sum of squares */
                iterations = (-1);
                learn();
                }

            XbNormalCursor(drawing_area);

            /* don't allow disconnect from menu ('Network -> Disconnect') because
               disconnecting a node that is in the process of being connected will
               crash the program -- Reset this here. */
            XtSetSensitive(network_disconnect_option, True);
            break;
        }

    /* allow deselect (unhighlight) now that a selection has been made */
    single_node_disc=True;

    /* increment num_click so that it always goes from 1 to 4 */
    num_click%=4;
    num_click++;

    /* make sure prev_sel handled correctly (probably redundant somehow) */
    if (num_click==1)
        prev_sel=(-1);
    else
        prev_sel=i;
    }


/* Takes the single linked list net_list and draws the links from node to node on
   the pixmap and does not copy to screen.  Used when reconstructing network
   (resize) or reading net in from file. */
void draw_links()
    {
    NetList *curr=NULL;
    XSegment *lines;
    XPoint vertices[4];
    int cnt, j, k;

    if (!net_list)
        return;

    XbWatchCursor(toplevel);
    
    for(curr=net_list;curr;curr=curr->next)
        {
        if (((curr->end_out - curr->begin_out + 1) *
                (curr->end_in - curr->begin_in + 1)) <= MAX_DRAW_LINES)
            {
            lines=(XSegment *)malloc(((curr->end_out)-(curr->begin_out)+1)*
                ((curr->end_in)-(curr->begin_in)+1)*sizeof(XSegment));

            for(cnt=0,j=curr->begin_out;j<=(curr->end_out);j++)
                for(k=curr->begin_in;k<=(curr->end_in);k++,cnt++)
                    {
                    lines[cnt].x1=(short )(node_info[j].center_x + NodeRadius + 1);
                    lines[cnt].y1=(short )(node_info[j].center_y);
                    lines[cnt].x2=(short )(node_info[k].center_x - NodeRadius - 1);
                    lines[cnt].y2=(short )(node_info[k].center_y);
                    }

                XDrawSegments(XtDisplay(drawing_area), net_pix, draw_gc, lines,
                    ((curr->end_out)-(curr->begin_out)+1)*(curr->end_in-curr->begin_in+1));
        
             free(lines);
             }
         else
             {
             vertices[0].x=(short )(node_info[curr->end_out].center_x + NodeRadius + 1);
             vertices[0].y=(short )(node_info[curr->end_out].center_y);

             vertices[1].x=(short )(node_info[curr->begin_out].center_x + NodeRadius + 1);
             vertices[1].y=(short )(node_info[curr->begin_out].center_y);

             vertices[2].x=(short )(node_info[curr->begin_in].center_x - NodeRadius - 1);
             vertices[2].y=(short )(node_info[curr->begin_in].center_y);

             vertices[3].x=(short )(node_info[curr->end_in].center_x - NodeRadius - 1);
             vertices[3].y=(short )(node_info[curr->end_in].center_y);

             XFillPolygon(XtDisplay(drawing_area), net_pix, draw_gc, vertices, 4,
                 Convex, CoordModeOrigin);
             }
         }

    XbNormalCursor(toplevel);
    }


/* Fully connects the network and is called from the 'Network -> Fully Connect'
   menu option and answering OK to the dialog.  It deletes any existing net_list
   and then creates a net_list single linked list that specifies a fully connected
   network.  It then calls draw_links to draw all the lines. */
void fully_connect()
    {
    NetList *curr;
    int i, begin_in_x, begin_in_y, begin_out_x, begin_out_y, end_in_y, end_out_y;

    XbWatchCursor(drawing_area);

    /* delete single linked list if it exists */
    delete_sll();

    curr=(NetList *)malloc(sizeof(NetList));
    curr->begin_out=0;
    curr->end_out=num_in_nodes-1;
    curr->begin_in=num_in_nodes;
    curr->end_in=num_in_nodes+num_hid_nodes[0]-1;
    add_elem(curr);

    /* clear area between layers */
    begin_in_x = node_info[curr->begin_in].center_x - NodeRadius - 1;
    begin_in_y = node_info[curr->begin_in].center_y;
    end_in_y = node_info[curr->end_in].center_y;
    begin_out_x = node_info[curr->begin_out].center_x + NodeRadius + 1;
    begin_out_y = node_info[curr->begin_out].center_y;
    end_out_y = node_info[curr->end_out].center_y;
    curr=NULL;

    XFillRectangle(XtDisplay(drawing_area), net_pix, eraser_gc,
        begin_out_x,
        begin_in_y < begin_out_y ? begin_in_y : begin_out_y, 
        begin_in_x - begin_out_x,
        begin_in_y < begin_out_y ? end_in_y - begin_in_y : end_out_y - begin_out_y);

    if (num_hid_layers==1)
        {
        curr=(NetList *)malloc(sizeof(NetList));
        curr->begin_out=num_in_nodes;
        curr->end_out=num_in_nodes+num_hid_nodes[0]-1;
        curr->begin_in=num_in_nodes+num_hid_nodes[0];
        curr->end_in=num_in_nodes+num_hid_nodes[0]+num_out_nodes-1;
        add_elem(curr);

        /* clear area between layers */
        begin_in_x = node_info[curr->begin_in].center_x - NodeRadius - 1;
        begin_in_y = node_info[curr->begin_in].center_y;
        end_in_y = node_info[curr->end_in].center_y;
        begin_out_x = node_info[curr->begin_out].center_x + NodeRadius + 1;
        begin_out_y = node_info[curr->begin_out].center_y;
        end_out_y = node_info[curr->end_out].center_y;
        curr=NULL;

        XFillRectangle(XtDisplay(drawing_area), net_pix, eraser_gc,
            begin_out_x,
            begin_in_y < begin_out_y ? begin_in_y : begin_out_y, 
            begin_in_x - begin_out_x,
            begin_in_y < begin_out_y ? end_in_y - begin_in_y : end_out_y - begin_out_y);
        }        

    if (num_hid_layers==2)
        {
        curr=(NetList *)malloc(sizeof(NetList));
        curr->begin_out=num_in_nodes;
        curr->end_out=num_in_nodes+num_hid_nodes[0]-1;
        curr->begin_in=num_in_nodes+num_hid_nodes[0];
        curr->end_in=num_in_nodes+num_hid_nodes[0]+num_hid_nodes[1]-1;
        add_elem(curr);

        /* clear area between layers */
        begin_in_x = node_info[curr->begin_in].center_x - NodeRadius - 1;
        begin_in_y = node_info[curr->begin_in].center_y;
        end_in_y = node_info[curr->end_in].center_y;
        begin_out_x = node_info[curr->begin_out].center_x + NodeRadius + 1;
        begin_out_y = node_info[curr->begin_out].center_y;
        end_out_y = node_info[curr->end_out].center_y;
        curr=NULL;

        XFillRectangle(XtDisplay(drawing_area), net_pix, eraser_gc,
            begin_out_x,
            begin_in_y < begin_out_y ? begin_in_y : begin_out_y, 
            begin_in_x - begin_out_x,
            begin_in_y < begin_out_y ? end_in_y - begin_in_y : end_out_y - begin_out_y);


        curr=(NetList *)malloc(sizeof(NetList));
        curr->begin_out=num_in_nodes+num_hid_nodes[0];
        curr->end_out=num_in_nodes+num_hid_nodes[0]+num_hid_nodes[1]-1;
        curr->begin_in=total_circles-num_out_nodes;
        curr->end_in=total_circles-1;
        add_elem(curr);

        /* clear area between layers */
        begin_in_x = node_info[curr->begin_in].center_x - NodeRadius - 1;
        begin_in_y = node_info[curr->begin_in].center_y;
        end_in_y = node_info[curr->end_in].center_y;
        begin_out_x = node_info[curr->begin_out].center_x + NodeRadius + 1;
        begin_out_y = node_info[curr->begin_out].center_y;
        end_out_y = node_info[curr->end_out].center_y;
        curr=NULL;

        XFillRectangle(XtDisplay(drawing_area), net_pix, eraser_gc,
            begin_out_x,
            begin_in_y < begin_out_y ? begin_in_y : begin_out_y, 
            begin_in_x - begin_out_x,
            begin_in_y < begin_out_y ? end_in_y - begin_in_y : end_out_y - begin_out_y);
        }

    if (num_hid_layers==3)
        {
        curr=(NetList *)malloc(sizeof(NetList));
        curr->begin_out=num_in_nodes;
        curr->end_out=num_in_nodes+num_hid_nodes[0]-1;
        curr->begin_in=num_in_nodes+num_hid_nodes[0];
        curr->end_in=num_in_nodes+num_hid_nodes[0]+num_hid_nodes[1]-1;
        add_elem(curr);

        /* clear area between layers */
        begin_in_x = node_info[curr->begin_in].center_x - NodeRadius - 1;
        begin_in_y = node_info[curr->begin_in].center_y;
        end_in_y = node_info[curr->end_in].center_y;
        begin_out_x = node_info[curr->begin_out].center_x + NodeRadius + 1;
        begin_out_y = node_info[curr->begin_out].center_y;
        end_out_y = node_info[curr->end_out].center_y;
        curr=NULL;

        XFillRectangle(XtDisplay(drawing_area), net_pix, eraser_gc,
            begin_out_x,
            begin_in_y < begin_out_y ? begin_in_y : begin_out_y, 
            begin_in_x - begin_out_x,
            begin_in_y < begin_out_y ? end_in_y - begin_in_y : end_out_y - begin_out_y);


        curr=(NetList *)malloc(sizeof(NetList));
        curr->begin_out=num_in_nodes+num_hid_nodes[0];
        curr->end_out=num_in_nodes+num_hid_nodes[0]+num_hid_nodes[1]-1;
        curr->begin_in=total_circles-num_out_nodes-num_hid_nodes[2];
        curr->end_in=total_circles-num_out_nodes-1;
        add_elem(curr);

        /* clear area between layers */
        begin_in_x = node_info[curr->begin_in].center_x - NodeRadius - 1;
        begin_in_y = node_info[curr->begin_in].center_y;
        end_in_y = node_info[curr->end_in].center_y;
        begin_out_x = node_info[curr->begin_out].center_x + NodeRadius + 1;
        begin_out_y = node_info[curr->begin_out].center_y;
        end_out_y = node_info[curr->end_out].center_y;
        curr=NULL;

        XFillRectangle(XtDisplay(drawing_area), net_pix, eraser_gc,
            begin_out_x,
            begin_in_y < begin_out_y ? begin_in_y : begin_out_y, 
            begin_in_x - begin_out_x,
            begin_in_y < begin_out_y ? end_in_y - begin_in_y : end_out_y - begin_out_y);


        curr=(NetList *)malloc(sizeof(NetList));
        curr->begin_out=total_circles-num_out_nodes-num_hid_nodes[2];
        curr->end_out=total_circles-num_out_nodes-1;
        curr->begin_in=total_circles-num_out_nodes;
        curr->end_in=total_circles-1;
        add_elem(curr);

        /* clear area between layers */
        begin_in_x = node_info[curr->begin_in].center_x - NodeRadius - 1;
        begin_in_y = node_info[curr->begin_in].center_y;
        end_in_y = node_info[curr->end_in].center_y;
        begin_out_x = node_info[curr->begin_out].center_x + NodeRadius + 1;
        begin_out_y = node_info[curr->begin_out].center_y;
        end_out_y = node_info[curr->end_out].center_y;
        curr=NULL;

        XFillRectangle(XtDisplay(drawing_area), net_pix, eraser_gc,
            begin_out_x,
            begin_in_y < begin_out_y ? begin_in_y : begin_out_y, 
            begin_in_x - begin_out_x,
            begin_in_y < begin_out_y ? end_in_y - begin_in_y : end_out_y - begin_out_y);
        }

    /* update node_info to reflect connections */
    for(i=0;i<total_circles;i++)
        {
        node_info[i].in_connected=True;
        node_info[i].out_connected=True;
        }

    /* draw the links; update status variables */
    draw_links();
    net_complete=True;
    net_changed=True;

    /* copy the proper portion from the pixmap to the screen */
    XCopyPlane(XtDisplay(drawing_area), net_pix, XtWindow(drawing_area), copy_gc, 0,
        TopMargin+slider_value, da_width, da_height-TopMargin, 0, TopMargin, 1);    
    XCopyPlane(XtDisplay(drawing_area), net_pix, XtWindow(drawing_area), copy_gc, 0,
        0, da_width, TopMargin, 0, 0, 1);
    
    XbNormalCursor(drawing_area);
    }


/* Deletes all the elements in the single linked list net_list, free the memory.   
   Each element in the list represents a line in the .net file format.*/
void delete_sll()
    {
    NetList *temp1, *temp2;

    if(!net_list)
        return;
    for(temp1=net_list;temp1;)
        {
        temp2=temp1;
        temp1=temp1->next;
        free(temp2);
        }
    net_list=NULL;
    }


/* Adds an element to the single linked list net_list.  Each element in the list
   represents a line in the .net file format.  This function also inserts the 
   element in the list in order according to begin_out (from smallest to largest)
   so that storing the list to a .net file format is very simple. */
void add_elem(NetList *new)
    {
    NetList *curr=NULL, *prev=NULL;

    if (net_list)
        /* find correct location in the single linked list */
        for(prev=curr=net_list; curr->begin_out < new->begin_out;)
            {
            prev=curr;
            curr=curr->next;
            if (!curr)
                break;
            }
    else
        {
        /* add the element as the head of the list */
        net_list=new;
        net_list->next=NULL;
        return;
        }

    if (curr==net_list)
        {
        /* since curr==net_list, net_list is not NULL (already checked) and just add
           new element to the head of the list */
        new->next=net_list;
        net_list=new;
        }
    else
        {
        /* insert element into list */
        new->next=curr;
        prev->next=new;
        }

    /* clear the temporary pointers */
    prev=curr=NULL;
    }


/* Deletes an element from the single linked list net_list.  This action is the
   equivalent of removing a line from the .net file format. */
void delete_node(NetList **del)
    {
    NetList *curr=NULL, *prev=NULL;

    if (net_list)
        /* find node in single linked list */
        for(prev=curr=net_list; curr->begin_out < (*del)->begin_out;)
            {
            prev=curr;
            curr=curr->next;
            }
    else
        /* in case net_list==NULL */
        return;


    if (curr==net_list)
        {
        /* if beginning of single linked list, slide net_list down one */
        net_list=net_list->next;
        free(curr);
        }
    else
        {
        /* delete by removing reference */
        prev->next=curr->next;
        free(curr);
        }

    /* clear all pointers */
    prev=curr=(*del)=NULL;
    }


/* Determines from the information in the single linked list net_list the number
   of hidden layers in the network.  Used when reading in a .net file since this
   information is not explicitly stated in it. */
int layers()
    {
    NetList *curr;
    int in_range[2], cnt=0;

    /* in case net_list==NULL */
    if (!net_list)
        return(0);

    /* initialize number of hidden nodes in each hidden layer */
    num_hid_nodes[0]=0;
    num_hid_nodes[1]=0;
    num_hid_nodes[2]=0;

    /* determines number of hidden layers by looking to see when nodes that were receiving
       input are now sending output; every time a previous receiver becomes a sender, there
       is another middle layer; can be verified by looking at .net file and using the same
       algorithm (modeled after the way you would naturally think about it) */
    curr=net_list;
    in_range[0]=curr->begin_in;
    in_range[1]=curr->end_in;
    while(curr=curr->next)
        {
        if((curr->begin_out<=in_range[1])&&(curr->begin_out>=in_range[0]))
            {
            num_hid_nodes[cnt]=in_range[1]-in_range[0]+1;
            cnt++;
            in_range[0]=curr->begin_in;
            in_range[1]=curr->end_in;
            }
        else
            {
            in_range[0]=Min(in_range[0],curr->begin_in);
            in_range[1]=Max(in_range[1],curr->end_in);
            }
        }

    /* return number of hidden layers */
    return(cnt);
    }


/* Prints the single linked list to the stdout.  Useful only for debugging. */
void print_sll()
    {
    NetList *temp;

    for(temp=net_list;temp;temp=temp->next)
        printf("bout:  %d, eout:  %d, bin:  %d, ein:  %d\n", temp->begin_out, temp->end_out,
            temp->begin_in, temp->end_in);

    printf("\n");
    }


/* Takes the net_list single linked list and does the simple conversion to .net
   file format as it writes it to the file.  Saves the network in other words. */
Boolean save_net_list()
    {
    FILE *netfile;

    if (!(netfile=fopen(new_net_filename,"w")))
        {
        XbError("File cannot be written at this time to this directory.");
        return(False);
        }

    /* write all of the appropriate beginning information */
    fprintf(netfile,"definitions:\n");
    fprintf(netfile,"nunits %d\n",total_circles);
    fprintf(netfile,"ninputs %d\n",num_in_nodes);
    fprintf(netfile,"noutputs %d\n",num_out_nodes);
    fprintf(netfile,"end\n");
    fprintf(netfile,"network:\n");

    /* convert net_list and write it out */
    for(temp=net_list;temp;temp=temp->next)
        fprintf(netfile,"%%r %d %d %d %d\n", temp->begin_in, temp->end_in-temp->begin_in + 1,
            temp->begin_out, temp->end_out-temp->begin_out + 1);

    /* finish the file */
    fprintf(netfile,"end\n");
    fprintf(netfile,"biases:\n");
    fprintf(netfile,"%%r %d %d\n", num_in_nodes, total_circles-num_in_nodes);
    fprintf(netfile,"end\n");
    fclose(netfile);

    /* successful */
    return(True);
    }


/* Reads in a .net file and does the simple conversion to the net_list single 
   linked list format.  */
Boolean read_net_list(FILE *file)
    {
    char *str[100];
    int in_offset, out_offset, counter, i;

    /* rewind in case already used */
    rewind(file);

    /* read in the information, checking to make sure everything is there as it should be */
    fscanf(file, "%s", str);
    if (!strstr(str,"definitions:"))
        return(False);
    fscanf(file,"%s %d", str, &total_circles);
    if (!strstr(str,"nunits"))
        return(False);
    fscanf(file,"%s %d", str, &num_in_nodes);
    if (!strstr(str,"ninputs"))
        return(False);
    fscanf(file,"%s %d", str, &num_out_nodes);
    if (!strstr(str,"noutputs"))
        return(False);
    fscanf(file,"%s", str);
    if (!strstr(str,"end"))
        return(False);
    fscanf(file,"%s", str);
    if (!strstr(str,"network:"))
        return(False);
    
    /* read in the 'network:' section and convert to single linked list net_list */
    for(counter=0; counter < (total_circles-num_in_nodes); )
        {
        temp=NULL;
        temp=(NetList *)malloc(sizeof(NetList));
        fscanf(file,"%s %d %d %d %d", str, &(temp->begin_in), &in_offset, &(temp->begin_out),
              &out_offset);
        counter+=in_offset;
        temp->end_in=in_offset-1+temp->begin_in;
        temp->end_out=out_offset-1+temp->begin_out;
        add_elem(temp);
        }

    /* determine the number of hidden layers; no more than 3 allowed */
    if ((num_hid_layers=layers()) > 3)
        return(False);

    /* make sure didn't get off track somewhere */
    fscanf(file,"%s",str);
    if (!strstr(str,"end"))
        return(False);

    /* draw the network */
    set_up_net();
    draw_links();

    XCopyPlane(XtDisplay(drawing_area), net_pix, XtWindow(drawing_area), copy_gc, 0,
        TopMargin+slider_value, da_width, da_height-TopMargin, 0, TopMargin, 1);    

    XCopyPlane(XtDisplay(drawing_area), net_pix, XtWindow(drawing_area), copy_gc, 0,
        0, da_width, TopMargin, 0, 0, 1);

    /* no check made for an incomplete network */

    /* update node_info list */    
    for(i=0;i<total_circles;i++)
        {
        node_info[i].in_connected=True;
        node_info[i].out_connected=True;
        }

/* biases section not currently needed for this function
    if(fscanf(file,"%s",str))
        {
        if (!strstr(str,"biases:"))
            return(False);
        fscanf(file,"%s",str);
        }
*/
    /* successful */
    return(True);
    }


/* Sets up net_array for actual neural net functions instead of from a file.  It
   essentially does a .net conversion as it creates net_array.  This is called
   every time the network is completed. */
void read_network_from_net_list()
    {
    int  i = 0, flag = 0, k, start_position;
    NetList *temp_list;

    /* initialize variables for the neural net functions in the other files */
    number_of_nodes=total_circles;
    number_of_inputs=num_in_nodes;
    number_of_outputs=num_out_nodes;
    number_of_weights=0;
    number_of_input_thresholds=number_of_inputs;
    number_of_thresholds=number_of_nodes - number_of_inputs;


    for(temp_list=net_list; temp_list; i+=4, temp_list=temp_list->next)
        {
        net_array[i]=temp_list->begin_in;
        net_array[i+1]=temp_list->end_in-temp_list->begin_in+1;
        net_array[i+2]=temp_list->begin_out;
        net_array[i+3]=temp_list->end_out-temp_list->begin_out+1;
 
        if (net_array[i + 2] >= number_of_inputs && !flag)
        if (net_array[i + 2] >= number_of_inputs && !flag)
            {
            third_layer_start = net_array[i];
            flag = 1;
            }
        number_of_weights = number_of_weights + net_array[i+1] * net_array[i+3];
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

        i++;
        start_position = 4 * i;
        }

    }
