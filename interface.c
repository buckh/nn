/*
 *  NeuralNet - interface.c
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

/* must be changed on each system to reflect the location of neuralnet.hlp */
/*#define HELP_FILE   "/usr/local/lib/X11/neuralnet/neuralnet.hlp"*/

#define HELP_FILE "/opt/neuralnet/lib/X11/neuralnet/neuralnet.hlp"

#include <stdio.h>
#include <ctype.h>
#include <strings.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>

#include <X11/Intrinsic.h>
#include <X11/StringDefs.h>
#include <X11/cursorfont.h>
#include <X11/Xlib.h>

#include <Xm/Xm.h>
#include <Xm/Form.h>
#include <Xm/LabelG.h>
#include <Xm/Text.h>
#include <Xm/PushBG.h>
#include <Xm/SeparatoG.h>
#include <Xm/MessageB.h>
#include <Xm/List.h>
#include <Xm/RowColumn.h>
#include <Xm/CascadeB.h>
#include <Xm/BulletinB.h>
#include <Xm/ToggleBG.h>
#include <Xm/SelectioB.h>
#include <Xm/Scale.h>
#include <Xm/FileSB.h>
#include <Xm/Frame.h>
#include <Xm/DrawingA.h>
#include <Xm/ScrollBar.h>

#include "header.h"
#include "draw.h"
#include "interface.h"
#include "XButil.h"
#include "nxhelp/NXHelp.h"

/* bitmap for icon */
#include "network.icon"

/* constants used in callbacks--the values themselves are meaningless, must
   only be distinct */
#define QUIT         0
#define HELP         1
#define OK           2
#define CANCEL       3
#define OPEN_NET     4
#define SAVE_NET     5
#define OPEN_PAT     6
#define TEST_PAT     7
#define SAVE_PAT     8
#define OPEN_WTS     9
#define SAVE_WTS    10
#define GEN_WTS     11
#define CLOSE_NET   12
#define CLOSE_PAT   13
#define CLOSE_WTS   14
#define NEW_NET     15
#define ONE_HID     16
#define TWO_HID     17
#define THREE_HID   18
#define DISCONNECT  19
#define FULL_CONN   20
#define NET_SAVE_AS 21
#define WTS_SAVE_AS 22
#define MOMENTUM    23
#define GAIN        24
#define TEST        25
#define OPEN        26
#define SAVE        27
#define CLEAR       28
#define CLOSE       29
#define NEW         30
#define STEP        31
#define SET_STEP    32
#define CLEAR_STEP  33
#define ABOUT       34
#define ABOUT_STRING "\nNeuralNet 2.0\n\n\nEugene W. Hodges, IV\nCopyright 1992\n\n\n\
Send comments, suggestions, problems to ewhodges@eos.ncsu.edu\n"

XtAppContext context;
XmStringCharSet char_set=XmSTRING_DEFAULT_CHARSET;  /* convenience */

Widget toplevel, main_form, form, menu_bar, network_menu, pattern_menu, weights_menu,
              network_open_option, network_new_option, network_disconnect_option,
              pattern_open_option, pattern_test_option, 
              weights_open_option, weights_save_option, weights_gen_option, quit_option, 
              help_option, learn_scale, sep1, learn_button, total_iter_label, scale_label,
              total_iter_text, tss_label, tss_text, sep2, curr_files_label, sep3, 
              net_file_label, net_file_text, pat_file_label, pat_file_text, 
              wts_file_label, wts_file_text, files_rc, make_wts_prompt=NULL,
              net_file_error_dialog, pat_file_error_dialog, wts_file_error_dialog,
              net_file_open_dialog=NULL, pat_file_open_dialog=NULL, wts_file_open_dialog=NULL, 
              network_close_option, pattern_close_option, weights_close_option, 
              scroll, new_net_dialog=NULL,
              num_nodes_label, input_label, output_label, frame1, frame2,
              num_hid_layers_label, first_layer_label, second_layer_label, 
              third_layer_label, input_text, output_text, first_layer_text, 
              second_layer_text, third_layer_text, one_toggle, two_toggle, three_toggle, 
              new_net_sep1, new_net_sep2, new_net_ok_button, new_net_cancel_button,
              new_net_rc1, new_net_rc2, new_net_rc3, net_save_dialog=NULL,
              info_dialog, network_fullconn_option, full_conn_ques_dialog=NULL,
              network_saveas_option, weights_saveas_option, wts_save_dialog=NULL,
              pattern_dialog=NULL, momentum_dialog=NULL, gain_dialog=NULL,
              network_momentum_option, network_gain_option, network_save_option,
              pattern_text, pattern_file_label, pattern_test_text,
              pattern_test_file_open_dialog=NULL, pattern_test_file_save_dialog=NULL,
              overwrite_net_dialog=NULL, overwrite_pat_dialog=NULL, overwrite_wts_dialog=NULL,
              save_net_changes_dialog=NULL, save_wts_changes_dialog=NULL, 
              save_pat_changes_dialog=NULL, patdlog_file_menu, patdlog_open_option, 
              patdlog_new_option, patdlog_save_option, patdlog_close_option, 
              patdlog_cancel_option, pattern_step_button, pattern_clear_step_button,
              pattern_set_step_button;

/* current directory -- used in all file selection dialogs */
XmString curr_dir;

/* keep track of open files */
Boolean net_file_open = False,
        pat_file_open = False,
        wts_file_open = False;

char *motif_pattern_entered;      /* the pattern entered by the user in the test pattern
                                     dialog */

char wts_filename[200];           /* current weights filename */

int total_iterations;             /* total number of learning iterations performed */
static int scale_value = 1;       /* current value of learn scale */

int num_hid_layers=1;             /* number of hidden layers, changed by new_net_dialog */

static int tog_layers=1;          /* current number of layers chosen on New Net dialog
                                     (toggle buttons) */

int num_in_nodes,                 /* number of input nodes */
    num_out_nodes,                /* number of output nodes */
    num_hid_nodes[3];             /* number of nodes in each of the 3 middle layers */

Boolean disconnect=False,         /* differentiates between clicks for connecting and
                                     disconnecting nodes */
        quit=False,               /* set after user chooses 'Network -> Quit' to indicate
                                     that the program should quit after finishing with
                                     saving (or not) changes */
        net_changed=False,        /* flag to report if network changed */

        wts_changed=False,        /* flag to report if weights changed */
        pat_changed=False;        /* flag to report if test pattern text (top part of dialog)
                                     changed */

float momentum=0.1, gain=0.5;     /* momentum and gain for weights calculated in learn.c */


static char pattern_test_filename[200],  /* name of test pattern file */
            *filename;                   /* temporary filename used for overwrite callback */

static Boolean save_changes_finished=True,  /* used to properly handle save changes call 
                                               backs and reset filenames */
               close_net=False,             /* used by save weights changes to determine
                                               if it was called by 'Network -> Close' 
                                               originally */
               cancel_test=False;           /* used by test pattern dialog save changes to
                                               determine if it was called by the 'Cancel 
                                               button originally */

/* set fallback resources */
String fallback[] = {
                    "NeuralNet*fontList:              -adobe-helvetica-medium-r-normal--12-120-*-*-*-*-*-*",
                    NULL
                    };


/* handles menu sensitivity for 'Network' */
void net_menu_settings()
    {
    XtSetSensitive(network_open_option, !net_file_open);
    XtSetSensitive(network_new_option, !net_file_open);

    if (strlen(new_net_filename))
        XtSetSensitive(network_save_option, net_file_open);

    XtSetSensitive(network_saveas_option, net_file_open);
    XtSetSensitive(network_close_option, net_file_open);
    XtSetSensitive(network_fullconn_option, net_file_open);
    XtSetSensitive(network_disconnect_option, net_file_open);
    XtSetSensitive(network_gain_option, net_file_open);
    XtSetSensitive(network_momentum_option, net_file_open);

    XtSetSensitive(pattern_test_option, (net_file_open && net_complete && wts_file_open)); 
    }


/* handles menu sensitivity for 'Pattern' */
void pat_menu_settings()
    {
    XtSetSensitive(pattern_open_option, !pat_file_open);
    XtSetSensitive(pattern_close_option, pat_file_open);
    }


/* handles menu sensitivity for 'Weights' */
void wts_menu_settings()
    {
    XtSetSensitive(weights_open_option, !wts_file_open);
    XtSetSensitive(weights_saveas_option, wts_file_open);
    XtSetSensitive(weights_close_option, wts_file_open);
    XtSetSensitive(weights_gen_option, (net_complete && !wts_file_open));

    XtSetSensitive(pattern_test_option, (net_file_open && net_complete && wts_file_open)); 
    }


/* get new scale value */
void scaleCB(Widget w, int client_data, XmAnyCallbackStruct *call_data)
    {
    XmScaleGetValue(learn_scale, &scale_value);
    }


/* train network */
void learn_buttonCB(Widget w, int client_data, XmAnyCallbackStruct *call_data)
    {
    char s[200];

    /* make sure training is possible; if not, show error dialog */
    if (!net_complete)
        {
        XbError("The network is incomplete.  An incomplete network cannot be trained.");
        return;
        }
    if (net_file_open && pat_file_open && wts_file_open)
	{
        XbWatchCursor(main_form);

        /* train the network */
        iterations=scale_value;
        learn();

        /* adjust menu if necessary; set flag that weights have changed */
        if (strlen(wts_filename))
            XtSetSensitive(weights_save_option,True);
        wts_changed=True;

        /* display number of epochs (iterations) */
        total_iterations+=scale_value;
        sprintf(s,"%d",total_iterations);
        XmTextSetString(total_iter_text,s);
        XbNormalCursor(main_form);
	}
     else
	{
	strcpy(s,"The following have no open file:\n\n");
        if (!net_file_open)
            strcat(s,"Network (or select New from Menu and create one)\n");
        if (!pat_file_open)
            strcat(s,"Pattern\n");
        if (!wts_file_open)
            strcat(s,"Weights (or select Generate from Menu)");
        XbError(s);
        }
    
    }


/* create all of the widgets on the main interface (what the user sees when the program
   comes onto the screen */
void create_widgets()
    {
    int ac;
    Arg al[20];

    /* create scale label above scale */
    ac=0;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreate("Learn:  Number of Epochs",char_set)); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNrightPosition, 65); ac++;  
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_NONE); ac++;
    XtSetArg(al[ac], XmNtopOffset, 10); ac++;
    scale_label=XmCreateLabelGadget(form,"label",al,ac);
    XtManageChild(scale_label);
    
    /* create learn scale (number of epochs chosen by user) */
    ac=0; 
    XtSetArg(al[ac], XmNminimum, 1); ac++;
    XtSetArg(al[ac], XmNmaximum, 10000); ac++;  
    XtSetArg(al[ac], XmNscaleMultiple, 1); ac++;
    XtSetArg(al[ac], XmNorientation, XmHORIZONTAL); ac++;
    XtSetArg(al[ac], XmNprocessingDirection, XmMAX_ON_RIGHT); ac++; 
    XtSetArg(al[ac], XmNshowValue, True); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, scale_label); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNrightPosition, 65); ac++;  
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_NONE); ac++;  
    learn_scale=XmCreateScale(form,"scale",al,ac);
    XtManageChild(learn_scale);
    XtAddCallback(learn_scale, XmNvalueChangedCallback, scaleCB, NULL);

    /* create learn button */
    ac=0;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreate("Learn",char_set)); ac++;
    XtSetArg(al[ac], XmNshowAsDefault, True); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNtopPosition, 60); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNrightPosition, 15); ac++;  
    XtSetArg(al[ac], XmNbottomOffset, 12); ac++; 
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_FORM); ac++; 
    learn_button=XmCreatePushButtonGadget(form,"learn_button",al,ac);
    XtManageChild(learn_button);
    XtAddCallback(learn_button, XmNactivateCallback, learn_buttonCB, NULL);
    
    /* create text widget to display total epochs*/
    ac=0;
    XtSetArg(al[ac], XmNeditable,False); ac++;
    XtSetArg(al[ac], XmNcursorPositionVisible,False); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_NONE); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNleftPosition, 20); ac++;  
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNrightPosition, 40); ac++;  
    XtSetArg(al[ac], XmNbottomOffset, 15); ac++; 
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_FORM); ac++; 
    total_iter_text=XmCreateText(form,"text",al,ac);
    XtManageChild(total_iter_text);
    XmTextSetString(total_iter_text,"0");

    /* create label for total epochs */
    ac=0;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreate("Total Epochs",char_set)); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_NONE); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNleftPosition, 20); ac++;  
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNrightPosition, 40); ac++;  
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++; 
    XtSetArg(al[ac], XmNbottomWidget, total_iter_text); ac++;
    total_iter_label=XmCreateLabelGadget(form,"label",al,ac);
    XtManageChild(total_iter_label);
    
    /* create text widget to display total sum of squares */
    ac=0;
    XtSetArg(al[ac], XmNeditable,False); ac++;
    XtSetArg(al[ac], XmNcursorPositionVisible,False); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_NONE); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNleftPosition, 45); ac++;  
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNrightPosition, 65); ac++;  
    XtSetArg(al[ac], XmNbottomOffset, 15); ac++; 
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_FORM); ac++; 
    tss_text=XmCreateText(form,"text",al,ac);
    XtManageChild(tss_text);

    /* create label for total sum of squares */
    ac=0;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreate("Total Sum of Squares",char_set)); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_NONE); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNleftPosition, 45); ac++;  
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNrightPosition, 65); ac++;  
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++; 
    XtSetArg(al[ac], XmNbottomWidget, tss_text); ac++;
    tss_label=XmCreateLabelGadget(form,"label",al,ac);
    XtManageChild(tss_label);

    /* create separator between scale and file listing */
    ac=0;
    XtSetArg(al[ac], XmNseparatorType, XmDOUBLE_LINE); ac++;
    XtSetArg(al[ac], XmNmargin, 1); ac++;
    XtSetArg(al[ac], XmNorientation, XmVERTICAL); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_NONE); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNrightPosition, 67); ac++;  
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_FORM); ac++; 
    sep2=XmCreateSeparatorGadget(form,"separator",al,ac);
    XtManageChild(sep2);

    /* create label for file listing */
    ac=0;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreate("Current Files",char_set)); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNleftWidget, sep2); ac++;  
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_NONE); ac++; 
    curr_files_label=XmCreateLabelGadget(form,"label",al,ac);
    XtManageChild(curr_files_label);

    /* create separator between file listing label and file listing */
    ac=0;
    XtSetArg(al[ac], XmNmargin, 1); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, curr_files_label); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNleftWidget, sep2); ac++;  
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_NONE); ac++; 
    sep3=XmCreateSeparatorGadget(form,"separator",al,ac);
    XtManageChild(sep3);

    /* create rowcolumn widget to hold file listing labels and text widgets */
    ac=0;
    XtSetArg(al[ac], XmNpacking, XmPACK_NONE); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, sep3); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNleftWidget, sep2); ac++;  
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_NONE); ac++;
    files_rc=XmCreateRowColumn(form,"rowcol",al,ac);
    XtManageChild(files_rc);

    /* create label for network file */
    ac=0;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreate("Network:",char_set)); ac++;
    XtSetArg(al[ac], XmNx, 0); ac++;
    XtSetArg(al[ac], XmNy, 8); ac++;
    net_file_label=XmCreateLabelGadget(files_rc,"label",al,ac);
    XtManageChild(net_file_label);

    /* create text widget to display name of network file */
    ac=0;
    XtSetArg(al[ac], XmNcolumns, 14); ac++;
    XtSetArg(al[ac], XmNeditable,False); ac++;
    XtSetArg(al[ac], XmNx, 65); ac++;
    XtSetArg(al[ac], XmNy, 2); ac++;
    net_file_text=XmCreateText(files_rc,"text",al,ac);
    XtManageChild(net_file_text);
    XmTextSetString(net_file_text,"No file open.");

    /* create label for pattern file */
    ac=0;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreate("Pattern:",char_set)); ac++;
    XtSetArg(al[ac], XmNx, 0); ac++;
    XtSetArg(al[ac], XmNy, 48); ac++;
    pat_file_label=XmCreateLabelGadget(files_rc,"label",al,ac);
    XtManageChild(pat_file_label);

    /* create text widget to display name of pattern file */
    ac=0;
    XtSetArg(al[ac], XmNcolumns, 14); ac++;
    XtSetArg(al[ac], XmNeditable,False); ac++;
    XtSetArg(al[ac], XmNx, 65); ac++;
    XtSetArg(al[ac], XmNy, 42); ac++;
    pat_file_text=XmCreateText(files_rc,"text",al,ac);
    XtManageChild(pat_file_text);
    XmTextSetString(pat_file_text,"No file open.");

    /* create label for weights file */
    ac=0;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreate("Weights:",char_set)); ac++;
    XtSetArg(al[ac], XmNx, 0); ac++;
    XtSetArg(al[ac], XmNy, 88); ac++;
    wts_file_label=XmCreateLabelGadget(files_rc,"label",al,ac);
    XtManageChild(wts_file_label);

    /* create text widget to display name of weights file */
    ac=0;
    XtSetArg(al[ac], XmNcolumns, 14); ac++;
    XtSetArg(al[ac], XmNeditable,False); ac++;
    XtSetArg(al[ac], XmNx, 65); ac++;
    XtSetArg(al[ac], XmNy, 82); ac++;
    wts_file_text=XmCreateText(files_rc,"text",al,ac);
    XtManageChild(wts_file_text);
    XmTextSetString(wts_file_text,"No file open.");
    }


/* callback to open a network file */
void net_file_openCB(Widget w, int client_data, XmSelectionBoxCallbackStruct *call_data)
    {
    int ac;
    Arg al[5];
    char *name;

    XbWatchCursor(toplevel);
    switch (client_data)
        {
        case OK:
            XmStringGetLtoR(call_data->value, char_set, &name);
            /* read in file if possible */
            if ((nptr = fopen(name, "r"))!=NULL)
		{
                /* get the current working directory from the dialog */
                XmStringFree(curr_dir);
                ac = 0;
                XtSetArg(al[ac], XmNdirectory, &curr_dir); ac++;
                XtGetValues(net_file_open_dialog, al, ac);

                XtUnmanageChild(net_file_open_dialog);

                if (!read_net_list(nptr))
                    {
                    /* some error in format */
                    fclose(nptr);
                    XbNormalCursor(toplevel);
                    XbError("File is incorrect.  Please check. ");
                    return;
                    }

                /* display name of network file */
                XmTextSetString(net_file_text,name);
                XmTextShowPosition(net_file_text,XmTextGetLastPosition(net_file_text));

                /* set the global name variable */
                strcpy(new_net_filename,name);

                XtFree(name);

                /* set menu sensitivities, read in network (this time it's the function
                   in inputseed.c), generate weights */
                net_file_open=True;
                read_network_file(nptr); 
                make_weights();
                wts_filename[0]='\0';
                XtSetSensitive(weights_save_option,False);
                XmTextSetString(wts_file_text,"Generated weights.");
                net_complete=True;
                net_menu_settings();
                wts_file_open=True;
                wts_menu_settings();

                XbNormalCursor(toplevel);
		}
            else
                {
                XbNormalCursor(toplevel);
                XbError("The file specified could not\nbe opened for reading.\n");
		}
            break;
        case CANCEL:
            XtUnmanageChild(net_file_open_dialog);
            XbNormalCursor(toplevel);
            break;
        }

    }


/* validates a .pat file */
Boolean valid_pat_file()
    {
    char temp_str[200];
    float temp_flt;
    int i;
    Boolean empty=True;

    while(fscanf(pptr,"%s",temp_str) != EOF)
        {
        if (temp_str[0] == ';')
            {
            fgets(temp_str, 200, pptr);
            continue;
            }
        else if (!strstr(temp_str,"p"))
            {
            fclose(pptr);
            return(False);
            }

        for (i = num_in_nodes + num_out_nodes; i > 0; i--)
            {
            if (fscanf(pptr,"%f",&temp_flt) == EOF)
                {
                fclose(pptr);
                return(False);
                }
            empty=False;
            }
       }

    rewind (pptr);
    return(empty ? False : True);
    }


/* callback to open pattern file (pattern file for training, not testing) */
void pat_file_openCB(Widget w, int client_data, XmSelectionBoxCallbackStruct *call_data)
    {
    int ac;
    Arg al[5];
    char *name;
    char temp_str[200];

    XbWatchCursor(toplevel);
    switch (client_data)
        {
        case OK:
            XmStringGetLtoR(call_data->value, char_set, &name);

            /* get the current working directory from the dialog */
            XmStringFree(curr_dir);
            ac = 0;
            XtSetArg(al[ac], XmNdirectory, &curr_dir); ac++;
            XtGetValues(pat_file_open_dialog, al, ac);

            /* read in file if possible */
            if ((pptr = fopen(name, "r"))!=NULL)
		{
                if (!valid_pat_file())
                    {
                    sprintf(temp_str,"The following pattern file is not in the correct format:\n%s\n\n\
Check each line for the correct number of inputs and outputs.",name);
                    XbError(temp_str);
                    XbNormalCursor(toplevel);
                    return;
                    }

                if (read_pattern_file(pptr))
                    {
                    /* display file name and set menu sensitivities */
                    XmTextSetString(pat_file_text,name);
                    XmTextShowPosition(pat_file_text,XmTextGetLastPosition(pat_file_text));
                    pat_file_open=True;
                    pat_menu_settings();

                    /* initial total sum of squares */
                    iterations = (-1);
                    learn();
                    }

                XtUnmanageChild(pat_file_open_dialog);
                XtFree(name);
                XbNormalCursor(toplevel);
		}
            else
                {
                XbNormalCursor(toplevel);
                XbError("The file specified could not\nbe opened for reading.\n");
		}
            break;
        case CANCEL:
            XtUnmanageChild(pat_file_open_dialog);
            XbNormalCursor(toplevel);
            break;
        }

    }


/* callback to open weights file */
void wts_file_openCB(Widget w, int client_data, XmSelectionBoxCallbackStruct *call_data)
    {
    int ac;
    Arg al[5];
    char *name;

    XbWatchCursor(toplevel);
    switch (client_data)
        {
        case OK:
            XmStringGetLtoR(call_data->value, char_set, &name);

            /* get the current working directory from the dialog */
            XmStringFree(curr_dir);
            ac = 0;
            XtSetArg(al[ac], XmNdirectory, &curr_dir); ac++;
            XtGetValues(wts_file_open_dialog, al, ac);

            /* read in file if possible */
            if ((wptr = fopen(name, "r"))!=NULL)
		{
                XtUnmanageChild(wts_file_open_dialog);
                if (read_weight_file(wptr))
                    {
                    /* display file name and set menu sensitivities */
                    XmTextSetString(wts_file_text,name);
                    XmTextShowPosition(wts_file_text,XmTextGetLastPosition(wts_file_text));
                    strcpy(wts_filename,name);
                    XtFree(name);
                    XmTextSetString(total_iter_text, "0");
                    wts_file_open=True;
                    wts_menu_settings();

                    /* initial total sum of squares */
                    iterations = (-1);
                    learn();
                    }
               
                XbNormalCursor(toplevel);
		}
            else
                {
                XbNormalCursor(toplevel);
                XbError("The specified file could not\nbe opened for reading.\n");
		}
            break;
        case CANCEL:
            XtUnmanageChild(wts_file_open_dialog);
            XbNormalCursor(toplevel);
            break;
        }

    }


/* create network file open dialog ('Network -> Open') */
void create_net_file_sel_dialog()
    {
    Arg al[10];
    int ac;

    XbWatchCursor(toplevel);

    /* Create the file selection box used by open option. */
    ac=0;
    XtSetArg(al[ac],XmNmustMatch,True); ac++;
    XtSetArg(al[ac],XmNautoUnmanage,False); ac++;
    XtSetArg(al[ac],XmNdialogTitle,XmStringCreateLtoR(
        "NeuralNet: Open Network File",char_set)); ac++;
    XtSetArg(al[ac],XmNpattern,XmStringCreateLtoR("*.net",char_set)); ac++;
    net_file_open_dialog=XmCreateFileSelectionDialog(toplevel,"dialog",al,ac);
    XtAddCallback (net_file_open_dialog, XmNokCallback, net_file_openCB, OK);
    XtAddCallback (net_file_open_dialog, XmNcancelCallback, net_file_openCB, CANCEL);
    XtUnmanageChild(XmSelectionBoxGetChild(net_file_open_dialog,
        XmDIALOG_HELP_BUTTON));

    ac=0;
    XtSetArg(al[ac],XmNlabelString,XmStringCreateLtoR("Open",char_set)); ac++;
    XtSetValues(XmSelectionBoxGetChild(net_file_open_dialog,XmDIALOG_OK_BUTTON), al, ac);

    XbNormalCursor(toplevel);
    }


/* create pattern file open dialog ('Pattern -> Open') */
void create_pat_file_sel_dialog()
    {
    Arg al[10];
    int ac;

    XbWatchCursor(toplevel);

    /* Create the file selection box used by open option. */
    ac=0;
    XtSetArg(al[ac],XmNmustMatch,True); ac++;
    XtSetArg(al[ac],XmNautoUnmanage,False); ac++;
    XtSetArg(al[ac],XmNdialogTitle,XmStringCreateLtoR(
        "NeuralNet: Open Pattern File",char_set)); ac++;
    XtSetArg(al[ac],XmNpattern,XmStringCreateLtoR("*.pat",char_set)); ac++;
    pat_file_open_dialog=XmCreateFileSelectionDialog(toplevel,"dialog",al,ac);
    XtAddCallback (pat_file_open_dialog, XmNokCallback, pat_file_openCB, OK);
    XtAddCallback (pat_file_open_dialog, XmNcancelCallback, pat_file_openCB, CANCEL);
    XtUnmanageChild(XmSelectionBoxGetChild(pat_file_open_dialog,
        XmDIALOG_HELP_BUTTON));

    ac=0;
    XtSetArg(al[ac],XmNlabelString,XmStringCreateLtoR("Open",char_set)); ac++;
    XtSetValues(XmSelectionBoxGetChild(pat_file_open_dialog,XmDIALOG_OK_BUTTON), al, ac);

    XbNormalCursor(toplevel);
    }


/* create weights file open dialog ('Weights -> Open') */
void create_wts_file_sel_dialog()
    {
    Arg al[10];
    int ac;

    XbWatchCursor(toplevel);

    /* Create the file selection box used by open option. */
    ac=0;
    XtSetArg(al[ac],XmNmustMatch,True); ac++;
    XtSetArg(al[ac],XmNautoUnmanage,False); ac++;
    XtSetArg(al[ac],XmNdialogTitle,XmStringCreateLtoR(
        "NeuralNet: Open Weights File",char_set)); ac++;
    XtSetArg(al[ac],XmNpattern,XmStringCreateLtoR("*.wts",char_set)); ac++;
    wts_file_open_dialog=XmCreateFileSelectionDialog(toplevel,"dialog",al,ac);
    XtAddCallback (wts_file_open_dialog, XmNokCallback, wts_file_openCB, OK);
    XtAddCallback (wts_file_open_dialog, XmNcancelCallback, wts_file_openCB, CANCEL);
    XtUnmanageChild(XmSelectionBoxGetChild(wts_file_open_dialog,
        XmDIALOG_HELP_BUTTON));

    ac=0;
    XtSetArg(al[ac],XmNlabelString,XmStringCreateLtoR("Open",char_set)); ac++;
    XtSetValues(XmSelectionBoxGetChild(wts_file_open_dialog,XmDIALOG_OK_BUTTON), al, ac);
    
    XbNormalCursor(toplevel);
    }


/* callback for info message box. */
static void infoCB(Widget w, int client_data, XmAnyCallbackStruct *call_data)
    {
    switch(client_data)
        {
        case OK:
            XtUnmanageChild(info_dialog);
            XbChangeCursor(drawing_area, XC_sailboat);
            disconnect=True;
            break;
        case CANCEL:
            disconnect=False;
            XbNormalCursor(drawing_area);
            break;
        }
    }


/* create info dialog; used to tell user to select group of nodes to disconnect */
void create_info_dialog()
    {
    Arg al[4];
    int ac;

    ac=0;
    XtSetArg(al[ac], XmNdialogStyle,XmDIALOG_FULL_APPLICATION_MODAL); ac++;
    XtSetArg(al[ac], XmNdialogTitle,XmStringCreateLtoR(
        "NeuralNet:  Information Dialog", char_set)); ac++;
    XtSetArg(al[ac], XmNmessageString,XmStringCreateLtoR("Select node to disconnect.",
        char_set)); ac++;
    XtSetArg(al[ac], XmNmessageAlignment, XmALIGNMENT_CENTER); ac++;
    info_dialog = XmCreateInformationDialog(toplevel,"info_dialog", al, ac);

    XtAddCallback(info_dialog, XmNokCallback, infoCB, OK);
    XtAddCallback(info_dialog, XmNcancelCallback, infoCB, CANCEL);

    XtUnmanageChild(XmMessageBoxGetChild(info_dialog, XmDIALOG_HELP_BUTTON));
    }

/* callback for fully connecting network */
static void full_conn_quesCB(Widget w, int client_data, XmAnyCallbackStruct *call_data)
    {
    switch(client_data)
        {
        case OK:
            /* fully connect network */
            fully_connect();

            /* initialize other structures with new network */
            read_network_from_net_list();

            /* set appropriate flags, generate weights, set menu sensitivities, update
               displayed information */
            net_complete=True;
            make_weights();
            XmTextSetString(total_iter_text,"0");
            XmTextSetString(tss_text,"");
            total_iterations=0;
            wts_file_open=True;
            wts_menu_settings();  
            XmTextSetString(wts_file_text,"Generated weights.");
            XtSetSensitive(weights_save_option, False);
            XtSetSensitive(pattern_test_option, True);
            if (strlen(new_net_filename))
                XtSetSensitive(network_save_option, True);
            break;
        case CANCEL:            
            break;
        }

    XtUnmanageChild(full_conn_ques_dialog);
    }


/* create question dialog to ask user whether or not to continue with fully connect option
   ('Network -> Fully Connect') */
void create_full_conn_ques_dialog()
    {
    Arg al[4];
    int ac;

    XbWatchCursor(toplevel);
    
    ac=0;
    XtSetArg(al[ac], XmNdialogStyle,XmDIALOG_FULL_APPLICATION_MODAL); ac++;
    XtSetArg(al[ac], XmNdialogTitle,XmStringCreateLtoR(
        "NeuralNet: Question Dialog", char_set)); ac++;
    XtSetArg(al[ac], XmNmessageString, 
        XmStringCreateLtoR("Okay to fully connect network?\nAll other connections will be lost.",
        char_set));  ac++;
    XtSetArg(al[ac], XmNmessageAlignment, XmALIGNMENT_CENTER); ac++;
    full_conn_ques_dialog = XmCreateQuestionDialog(toplevel,"full_conn_ques_dialog", al, ac);

    XtAddCallback(full_conn_ques_dialog, XmNokCallback, full_conn_quesCB, OK);
    XtAddCallback(full_conn_ques_dialog, XmNcancelCallback, full_conn_quesCB, CANCEL);

    XtUnmanageChild(XmMessageBoxGetChild(full_conn_ques_dialog, XmDIALOG_HELP_BUTTON));

    XbNormalCursor(toplevel);
    }


/* callback for all dialogs asking user whether or not to overwrite existing file */
void overwriteCB(Widget w, int client_data, XmAnyCallbackStruct *call_data)
    {
    int ac;
    Arg al[5];
    char label_filename[200], *to_be_saved, hold_name[200];
    FILE *file;

    switch(client_data)
        {
        case SAVE_NET:
            /* save network file if possible; update displayed information */
            strcpy(hold_name,new_net_filename);
            strcpy(new_net_filename,filename);
            if (save_net_list())
                {  
                net_file_open=True;
                XtSetSensitive(network_save_option, False); 
                XmTextSetString(net_file_text,filename);
                XmTextShowPosition(net_file_text,XmTextGetLastPosition(net_file_text));
                net_changed=False;
                }
            else
                {
                strcpy(new_net_filename,hold_name);
                XbError("The file specified could not\nbe opened for writing.  File not saved.");
                }
            break;
        case SAVE_PAT:
            /* save test pattern file if possible; update displayed file name */
            if ((file=fopen(filename,"w"))!=NULL)
                {
                strcpy(pattern_test_filename,filename);
                to_be_saved=XmTextGetString(pattern_text);
                fprintf(file,"%s",to_be_saved);
                free(to_be_saved);
                fclose(file);

                sprintf(label_filename,"File:  %s",filename);
                ac=0;
                XtSetArg(al[ac], XmNlabelString, XmStringCreate(label_filename,char_set)); ac++;
                XtSetValues(pattern_file_label, al, ac);
                pat_changed=False;
                }
            else
                XbError("The file specified could not\nbe opened for writing.  File not saved.");
            break;
        case SAVE_WTS:
            /* save weights file if possible; update displayed information */
            if ((w2ptr = fopen(wts_filename,"w"))!=NULL)
                {
                XmTextSetString(wts_file_text,filename);
                XmTextShowPosition(wts_file_text,XmTextGetLastPosition(wts_file_text));
                strcpy(wts_filename,filename);
                write_weights();
                fclose(w2ptr);
                wts_changed=False;
                }
             else
                XbError("Weights file could not be written due\nto this directory at this time.");
            break;
        case CANCEL:
            break;
        }

    XtFree(filename);
    XtUnmanageChild(w);
    }


/* create overwrite existing network file dialog */
void create_overwrite_net_dialog()
    {
    int ac;
    Arg al[5];
    
    XbWatchCursor(toplevel);
    
    ac=0;
    XtSetArg(al[ac], XmNdialogTitle, XmStringCreateLtoR("NeuralNet:  Overwrite?", 
        char_set)); ac++;
    XtSetArg(al[ac], XmNmessageString, 
        XmStringCreateLtoR("Specified filename exists.\nDo you wish to overwrite the file?",
        char_set));  ac++;
    XtSetArg(al[ac], XmNmessageAlignment, XmALIGNMENT_CENTER); ac++;
    overwrite_net_dialog = XmCreateQuestionDialog(toplevel,"overwrite_net_dialog", al, ac);

    XtAddCallback(overwrite_net_dialog, XmNokCallback, overwriteCB, SAVE_NET);
    XtAddCallback(overwrite_net_dialog, XmNcancelCallback, overwriteCB, CANCEL);

    XtUnmanageChild(XmMessageBoxGetChild(overwrite_net_dialog, XmDIALOG_HELP_BUTTON));
    
    XbNormalCursor(toplevel);
    }


/* create overwrite existing pattern file dialog */
void create_overwrite_pat_dialog()
    {
    int ac;
    Arg al[5];
    
    XbWatchCursor(toplevel);
    
    ac=0;
    XtSetArg(al[ac], XmNdialogTitle, XmStringCreateLtoR("NeuralNet:  Overwrite?", 
        char_set)); ac++;
    XtSetArg(al[ac], XmNmessageString, 
        XmStringCreateLtoR("Specified filename exists.\nDo you wish to overwrite the file?",
        char_set));  ac++;
    XtSetArg(al[ac], XmNmessageAlignment, XmALIGNMENT_CENTER); ac++;
    overwrite_pat_dialog = XmCreateQuestionDialog(toplevel,"overwrite_pat_dialog", al, ac);

    XtAddCallback(overwrite_pat_dialog, XmNokCallback, overwriteCB, SAVE_PAT);
    XtAddCallback(overwrite_pat_dialog, XmNcancelCallback, overwriteCB, CANCEL);

    XtUnmanageChild(XmMessageBoxGetChild(overwrite_pat_dialog, XmDIALOG_HELP_BUTTON));
    
    XbNormalCursor(toplevel);
    }


/* create overwrite existing weights file dialog */
void create_overwrite_wts_dialog()
    {
    int ac;
    Arg al[5];
    
    XbWatchCursor(toplevel);
    
    ac=0;
    XtSetArg(al[ac], XmNdialogTitle, XmStringCreateLtoR("NeuralNet:  Overwrite?", 
        char_set)); ac++;
    XtSetArg(al[ac], XmNmessageString, 
        XmStringCreateLtoR("Specified filename exists.\nDo you wish to overwrite the file?",
        char_set));  ac++;
    XtSetArg(al[ac], XmNmessageAlignment, XmALIGNMENT_CENTER); ac++;
    overwrite_wts_dialog = XmCreateQuestionDialog(toplevel,"overwrite_wts_dialog", al, ac);

    XtAddCallback(overwrite_wts_dialog, XmNokCallback, overwriteCB, SAVE_WTS);
    XtAddCallback(overwrite_wts_dialog, XmNcancelCallback, overwriteCB, CANCEL);

    XtUnmanageChild(XmMessageBoxGetChild(overwrite_wts_dialog, XmDIALOG_HELP_BUTTON));
    
    XbNormalCursor(toplevel);
    }


/* callback for save network changes dialog */
void save_net_changesCB(Widget w, int client_data, XmAnyCallbackStruct *call_data)
    {

    switch(client_data)
        {
        case SAVE_NET:
            /* check to see if net complete (incomplete net cannot be saved) */
            if (!net_complete)
                {
                XbError("The network is not yet complete.\nAn incomplete net cannot be saved.");
                quit=False;
                return;
                }

            /* display save network dialog */
            if (!net_save_dialog)
                create_net_save_dialog();
            XmFileSelectionDoSearch(net_save_dialog,XmStringCreateLtoR("*.net",char_set));
            if (strlen(new_net_filename))
                {
                XmTextSetString(XmSelectionBoxGetChild(net_save_dialog, XmDIALOG_TEXT), new_net_filename);
                XmTextShowPosition(XmSelectionBoxGetChild(net_save_dialog, XmDIALOG_TEXT),
                    XmTextGetLastPosition(XmSelectionBoxGetChild(net_save_dialog, XmDIALOG_TEXT)));
                }
            XtManageChild(net_save_dialog);
            save_changes_finished=False;
            break;
        case CANCEL:
            /* also called from net_save_dialogCB when its finished; use menuCB's close net;
               call menuCB's quit if quit was chosen earlier */
            net_changed=False;
            menuCB(NULL,CLOSE_NET,NULL);
            if (quit)
                menuCB(NULL,QUIT,NULL);
            break;
        }

    if (w)
        XtUnmanageChild(w);
    }


/* create save changes to network dialog */
void create_save_net_changes_dialog()
    {
    int ac;
    Arg al[5];
    
    XbWatchCursor(toplevel);
    
    ac=0;
    XtSetArg(al[ac], XmNdialogTitle, XmStringCreateLtoR("NeuralNet:  Save Changes?", 
        char_set)); ac++;
    XtSetArg(al[ac], XmNmessageString, XmStringCreateLtoR("Save changes to network?",
        char_set));  ac++;
    XtSetArg(al[ac], XmNmessageAlignment, XmALIGNMENT_CENTER); ac++;
    save_net_changes_dialog = XmCreateQuestionDialog(toplevel,"save_net_changes_dialog", al, ac);

    XtAddCallback(save_net_changes_dialog, XmNokCallback, save_net_changesCB, SAVE_NET);
    XtAddCallback(save_net_changes_dialog, XmNcancelCallback, save_net_changesCB, CANCEL);

    XtUnmanageChild(XmMessageBoxGetChild(save_net_changes_dialog, XmDIALOG_HELP_BUTTON));
    
    ac=0;
    XtSetArg(al[ac], XmNlabelString, XmStringCreateLtoR("Save...",char_set)); ac++;
    XtSetValues(XmMessageBoxGetChild(save_net_changes_dialog, XmDIALOG_OK_BUTTON),al,ac);

    ac=0;
    XtSetArg(al[ac], XmNlabelString, XmStringCreateLtoR("Discard",char_set)); ac++;
    XtSetValues(XmMessageBoxGetChild(save_net_changes_dialog, XmDIALOG_CANCEL_BUTTON),al,ac);

    XbNormalCursor(toplevel);
    }


/* callback to save changes to weights dialog */
void save_wts_changesCB(Widget w, int client_data, XmAnyCallbackStruct *call_data)
    {
    switch(client_data)
        {
        case SAVE_WTS:
            /* display the save weights dialog */
            if (!wts_save_dialog)
                create_wts_save_dialog();
            XmFileSelectionDoSearch(wts_save_dialog,XmStringCreateLtoR("*.wts",char_set));
            if (strlen(wts_filename))
                {
                XmTextSetString(XmSelectionBoxGetChild(wts_save_dialog, XmDIALOG_TEXT), wts_filename);
                XmTextShowPosition(XmSelectionBoxGetChild(wts_save_dialog, XmDIALOG_TEXT),
                    XmTextGetLastPosition(XmSelectionBoxGetChild(wts_save_dialog, XmDIALOG_TEXT)));
                }
            XtManageChild(wts_save_dialog);
            save_changes_finished=False;
            break;
        case CANCEL:
            /* also called from wts_save_dialogCB when its finished; use menuCB's close net;
               call menuCB's quit if quit was chosen earlier */
            wts_changed=False;
            menuCB(NULL,CLOSE_WTS,NULL);
            if (close_net)
                {
                menuCB(NULL,CLOSE_NET,NULL);
                }
            else if (quit)
                menuCB(NULL,QUIT,NULL);
            break;
        }

    if (w)
        XtUnmanageChild(w);
    }


/* create save changes to weights dialog */
void create_save_wts_changes_dialog()
    {
    int ac;
    Arg al[5];
    
    XbWatchCursor(toplevel);
    
    ac=0;
    XtSetArg(al[ac], XmNdialogTitle, XmStringCreateLtoR("NeuralNet:  Save Changes?", 
        char_set)); ac++;
    XtSetArg(al[ac], XmNmessageString, XmStringCreateLtoR("Save changes to weights?",
        char_set));  ac++;
    XtSetArg(al[ac], XmNmessageAlignment, XmALIGNMENT_CENTER); ac++;
    save_wts_changes_dialog = XmCreateQuestionDialog(toplevel,"save_wts_changes_dialog", al, ac);

    XtAddCallback(save_wts_changes_dialog, XmNokCallback, save_wts_changesCB, SAVE_WTS);
    XtAddCallback(save_wts_changes_dialog, XmNcancelCallback, save_wts_changesCB, CANCEL);

    XtUnmanageChild(XmMessageBoxGetChild(save_wts_changes_dialog, XmDIALOG_HELP_BUTTON));

    ac=0;
    XtSetArg(al[ac], XmNlabelString, XmStringCreateLtoR("Save...",char_set)); ac++;
    XtSetValues(XmMessageBoxGetChild(save_wts_changes_dialog, XmDIALOG_OK_BUTTON),al,ac);

    ac=0;
    XtSetArg(al[ac], XmNlabelString, XmStringCreateLtoR("Discard",char_set)); ac++;
    XtSetValues(XmMessageBoxGetChild(save_wts_changes_dialog, XmDIALOG_CANCEL_BUTTON),al,ac);

    XbNormalCursor(toplevel);
    }


/* callback to save network dialog */
void net_saveCB(Widget w, int client_data, XmSelectionBoxCallbackStruct *call_data)
    {
    FILE *file;
    char hold_name[200];

    XbWatchCursor(main_form);

    switch (client_data)
        {
        case OK:
            XmStringGetLtoR(call_data->value, char_set, &filename);
            if (strlen(filename) && (filename[strlen(filename)-1]!='/'))
                {
                /* check to see if file exists and ask user if want to overwrite if exists;
                   skip for a save changes call */
                if (((file=fopen(filename,"r"))!=NULL) && save_changes_finished)
                    {
                    fclose(file);
                    if (!overwrite_net_dialog)
                        create_overwrite_net_dialog();
                    XtUnmanageChild(w);
                    XtManageChild(overwrite_net_dialog);
                    XbNormalCursor(main_form);
                    return;
                    }
                else
                    {
                    /* save file if possible and update displayed information; restore file 
                       name if unable to save file */
                    strcpy(hold_name,new_net_filename);
                    strcpy(new_net_filename,filename);
                    if (save_net_list())
       	                {  
                        net_file_open=True;
                        XtSetSensitive(network_save_option, False); 
                        XmTextSetString(net_file_text,filename);
                        XmTextShowPosition(net_file_text,XmTextGetLastPosition(net_file_text));
                        }
                    else
                        strcpy(new_net_filename,hold_name);
 
                    /* if this was part of a save changes call, finish it up */
                    if (!save_changes_finished)
                        {
                        save_net_changesCB(NULL,CANCEL,NULL);
                        save_changes_finished=True;
                        }
                    }
                }
            else
                {
                XbNormalCursor(main_form);
                XbError("You must enter a filename.\n");
                return;
                }
            XtFree(filename);
            break;
         case CANCEL:
            /* if this was part of a save changes call, finish it up */
            if (!save_changes_finished)
                {
                save_net_changesCB(NULL,CANCEL,NULL);
                save_changes_finished=True;
                }
            break;
         }

    XtUnmanageChild(w);
    XbNormalCursor(main_form);
    }


/* create save network file dialog */
void create_net_save_dialog()
    {
    Arg al[10];
    int ac;

    XbWatchCursor(toplevel);
    
    /* Create the file selection box used by save as option. */
    ac=0;
    XtSetArg(al[ac],XmNautoUnmanage,False); ac++;
    XtSetArg(al[ac],XmNdialogTitle,XmStringCreateLtoR(
        "NeuralNet: Save Network File",char_set)); ac++;
    XtSetArg(al[ac],XmNpattern,XmStringCreateLtoR("*.net",char_set)); ac++;
    net_save_dialog=XmCreateFileSelectionDialog(toplevel,"dialog",al,ac);
    XtAddCallback (net_save_dialog, XmNokCallback, net_saveCB, OK);
    XtAddCallback (net_save_dialog, XmNcancelCallback, net_saveCB, CANCEL);
    XtUnmanageChild(XmSelectionBoxGetChild(net_save_dialog, XmDIALOG_HELP_BUTTON));

    XtSetSensitive(XmSelectionBoxGetChild(net_save_dialog, XmDIALOG_LIST), False);

    ac=0;
    XtSetArg(al[ac],XmNlabelString,XmStringCreateLtoR("Save",char_set)); ac++;
    XtSetValues(XmSelectionBoxGetChild(net_save_dialog,XmDIALOG_OK_BUTTON), al, ac);
   
    XbNormalCursor(toplevel);
    }


/* callback for save weights file dialog */
void wts_saveCB(Widget w, int client_data, XmSelectionBoxCallbackStruct *call_data)
    {
    FILE *file;

    XbWatchCursor(main_form);

    switch (client_data)
        {
        case OK:
            XmStringGetLtoR(call_data->value, char_set, &filename);
            if (strlen(filename) && (filename[strlen(filename)-1]!='/'))
                {
                /* check to see if file already exists, if so inquire about overwrite;
                   otherwise attempt to write file; skip overwrite check if this is
                   a save changes call */
                if (((file=fopen(filename,"r"))!=NULL) && save_changes_finished)
                    {
                    XbNormalCursor(main_form);
                    fclose(file);
                    if (!overwrite_wts_dialog)
                        create_overwrite_wts_dialog();
                    XtUnmanageChild(w);
                    XtManageChild(overwrite_wts_dialog);
                    return;
                    }
                else
                    {
                    /* attempt to write file */
                    if ((w2ptr=fopen(filename,"w"))==NULL)
                        {
                        /* unsuccessful */
                        XbNormalCursor(main_form);
                        XbError("Weights file could not be written\nto this directory at this time.");
                        return;
                        }
                    else
                        {
                        /* write weights to file, close file, update displayed information;
                           if save changes call, finish it */
                        write_weights();
                        fclose(w2ptr);
                        XmTextSetString(wts_file_text,filename);
                        XmTextShowPosition(wts_file_text,XmTextGetLastPosition(wts_file_text));
                        strcpy(wts_filename,filename);
                        if(!save_changes_finished)
                            {
                            save_wts_changesCB(NULL,CANCEL,NULL);
                            save_changes_finished=True;
                            }
                        }
                    
                    }
                }
            else
                {
                XbError("You must enter a filename.\n");
                return;
                }
            XtFree(filename);
            break;
         case CANCEL:
            /* if this was part of a save changes call, finish it up */
            if(!save_changes_finished)
                {
                save_wts_changesCB(NULL,CANCEL,NULL);
                save_changes_finished=True;
                }
            break;
         }

    XtUnmanageChild(w);
    XbNormalCursor(main_form);
    }


/* create save weights file dialog */
void create_wts_save_dialog()
    {
    Arg al[10];
    int ac;

    XbWatchCursor(toplevel);

    /* Create the file selection box used by open option. */
    ac=0;
    XtSetArg(al[ac],XmNautoUnmanage,False); ac++;
    XtSetArg(al[ac],XmNdialogTitle,XmStringCreateLtoR(
        "NeuralNet: Save Weights File",char_set)); ac++;
    XtSetArg(al[ac],XmNpattern,XmStringCreateLtoR("*.wts",char_set)); ac++;
    wts_save_dialog=XmCreateFileSelectionDialog(toplevel,"wts_save_dialog",al,ac);
    XtAddCallback (wts_save_dialog, XmNokCallback, wts_saveCB, OK);
    XtAddCallback (wts_save_dialog, XmNcancelCallback, wts_saveCB, CANCEL);
    XtUnmanageChild(XmSelectionBoxGetChild(wts_save_dialog, XmDIALOG_HELP_BUTTON));

    XtSetSensitive(XmSelectionBoxGetChild(wts_save_dialog, XmDIALOG_LIST), False);

    ac=0;
    XtSetArg(al[ac],XmNlabelString,XmStringCreateLtoR("Save",char_set)); ac++;
    XtSetValues(XmSelectionBoxGetChild(wts_save_dialog,XmDIALOG_OK_BUTTON), al, ac);
    
    XbNormalCursor(toplevel);
    }


/* callback for momentum prompt dialog */
void momentum_dialogCB(Widget w, int client_data, XmSelectionBoxCallbackStruct *call_data)
    {
    char *temp_str;

    switch (client_data)
        {
        case OK:
            /* get new momentum */
            XmStringGetLtoR(call_data->value, char_set, &temp_str);
            sscanf(temp_str,"%f",&momentum);
            XtFree(temp_str);
            break;
         case CANCEL:
            break;
         }

    XtUnmanageChild(momentum_dialog);
    }


/* create dialog for changing momentum ('Network -> Momentum') */
void create_momentum_dialog()
    {
    Arg al[10];
    int ac;

    XbWatchCursor(toplevel);
    
    ac = 0;
    XtSetArg(al[ac], XmNselectionLabelString, XmStringCreateLtoR
       ("Momentum:", char_set));  ac++;
    XtSetArg(al[ac],XmNdialogTitle,XmStringCreateLtoR(
       "NeuralNet:  Momentum",char_set)); ac++;
    momentum_dialog=XmCreatePromptDialog(toplevel,"momentum_dialog",al,ac);

    XtAddCallback (momentum_dialog, XmNokCallback, momentum_dialogCB, OK);
    XtAddCallback (momentum_dialog, XmNcancelCallback, momentum_dialogCB, CANCEL);
    XtUnmanageChild(XmSelectionBoxGetChild(momentum_dialog,XmDIALOG_HELP_BUTTON));

    XbNormalCursor(toplevel);
    }


/* callback for gain prompt dialog */
void gain_dialogCB(Widget w, int client_data, XmSelectionBoxCallbackStruct *call_data)
    {
    char *temp_str;

    switch (client_data)
        {
        case OK:
            /* get new gain */
            XmStringGetLtoR(call_data->value, char_set, &temp_str);
            sscanf(temp_str,"%f",&gain);
            XtFree(temp_str);
            break;
         case CANCEL:
            break;
         }

    XtUnmanageChild(gain_dialog);
    }


/* create prompt dialog for changing gain */
void create_gain_dialog()
    {
    Arg al[10];
    int ac;

    XbWatchCursor(toplevel);
    
    ac = 0;
    XtSetArg(al[ac], XmNselectionLabelString, XmStringCreateLtoR
       ("Gain:", char_set));  ac++;
    XtSetArg(al[ac],XmNdialogTitle,XmStringCreateLtoR(
       "NeuralNet:  Gain",char_set)); ac++;
    gain_dialog=XmCreatePromptDialog(toplevel,"gain_dialog",al,ac);

    XtAddCallback (gain_dialog, XmNokCallback, gain_dialogCB, OK);
    XtAddCallback (gain_dialog, XmNcancelCallback, gain_dialogCB, CANCEL);
    XtUnmanageChild(XmSelectionBoxGetChild(gain_dialog,XmDIALOG_HELP_BUTTON));
    
    XbNormalCursor(toplevel);
    }


/* callback for menu bar on main interface */
void menuCB (Widget w, int client_data, XmAnyCallbackStruct *call_data)
    {
    Arg al[10];
    int ac;
    char temp_str[20], *name;

    switch (client_data)
        {
        case NEW_NET:
            if (!new_net_dialog)
                create_new_net_dialog();
            XtManageChild(new_net_dialog);

            /* set num_hid_layers to the current number of hidden layers according
               to the state of the toggle buttons on the new network dialog */
            num_hid_layers=tog_layers;

            /* initialize status flags */
            net_complete=False;
            net_changed=False;
            break;
        case OPEN_NET:
            /* update file listing and manage dialog */
            if (!net_file_open_dialog)
                create_net_file_sel_dialog();

            /* set the current directory */
            ac = 0;
            XtSetArg(al[ac], XmNdirectory, curr_dir); ac++;
            XtSetValues(net_file_open_dialog, al, ac);

            XtManageChild(net_file_open_dialog);
            break;
        case OPEN_PAT:
            /* must have a network open or being created */
            if (!net_file_open)
                {
                XbError("You must open a network file or create a new network first.");
                return;
                }

            /* update file listing and manage dialog */
            if (!pat_file_open_dialog)
                create_pat_file_sel_dialog();

            /* set the current directory */
            ac = 0;
            XtSetArg(al[ac], XmNdirectory, curr_dir); ac++;
            XtSetValues(pat_file_open_dialog, al, ac);

            XtManageChild(pat_file_open_dialog);
            break;
        case OPEN_WTS:
            /* must have network opened from a file or completed a new one */
            if (!net_file_open)
                XbError("You must open a network file first.");
            else if (!net_complete)
                XbError("You must first finish creating the network.");
            else
		{
                /* update file listing and manage dialog */
                if (!wts_file_open_dialog)
                    create_wts_file_sel_dialog();

                /* set the current directory */
                ac = 0;
                XtSetArg(al[ac], XmNdirectory, curr_dir); ac++;
                XtSetValues(wts_file_open_dialog, al, ac);

                XtManageChild(wts_file_open_dialog);
		}
            break;
        case CLOSE_NET:
            /* set status for closing network */
            close_net=True;

            /* save changes to weights and/or network (weights first since closing network
               closes everything */
            if (wts_changed)
                {
                if (!save_wts_changes_dialog)
                  create_save_wts_changes_dialog();
                XtManageChild(save_wts_changes_dialog);
                }
            else if (net_changed)
                {
                if (!save_net_changes_dialog)
                    create_save_net_changes_dialog();
                XtManageChild(save_net_changes_dialog);
                }
            else
                {
                /* close the file streams in case not already closed */
                if (nptr)
                    {
                    fclose(nptr);
                    nptr=NULL;
                    }
                if (pptr)
                    {
                    fclose(pptr);
                    pptr=NULL;
                    if (pattern)
                        free(pattern);
                    pattern=NULL;
                    }

                /* free any allocated memory */
                if (node)
                    {
                    free(node);
                    node=NULL;
                    }

                /* reset and free everything */
                reset=True;
                node_monitor(NULL,NULL,NULL);
                net_file_open=False;
                net_menu_settings();
                pat_file_open=False;
                pat_menu_settings();
                wts_file_open=False;
                wts_menu_settings();
                if (pattern_dialog)
                    patdlog_menu_settings(False);
                XFreePixmap(XtDisplay(drawing_area),net_pix);
                XFlush(XtDisplay(drawing_area));
                free(node_info);
                draw=False;
                close_net=False;
                XClearWindow(XtDisplay(drawing_area),XtWindow(drawing_area));
                XmTextSetString(net_file_text,"No file open.");
                XmTextSetString(pat_file_text,"No file open.");
                XmTextSetString(wts_file_text,"No file open.");

                XmTextSetString(total_iter_text,"0");
                XmTextSetString(tss_text,"");
                total_iterations=0;

                /* free net list */
                delete_sll();
 
                /* reset scrollbar */
                ac=0;
                XtSetArg(al[ac], XmNvalue, 0); ac++;
                XtSetArg(al[ac], XmNmaximum, da_height); ac++;
                XtSetValues(scroll, al, ac);
                slider_value=0;

                /* reset test pattern dialog */
                if (pattern_dialog)
                    {
                    XmTextSetString(pattern_test_text, "");
                    XmTextSetString(pattern_text, "");
                    pat_changed=False;   /* since text changed callback will get called */

                    ac=0;
                    XtSetArg(al[ac], XmNlabelString,
                        XmStringCreate("File:  None.",char_set)); ac++;
                    XtSetValues(pattern_file_label,al,ac);
                    }

                /* default values */
                momentum=0.1;
                gain=0.5;

                /* clear filenames */
                new_net_filename[0]='\0';
                wts_filename[0]='\0';
                }
            break;
        case CLOSE_PAT:
            /* close file stream if open */
            if (pptr)
                {
                fclose(pptr);
                pptr=NULL;
                }
            /* free allocated memory */
            free(pattern);
            pattern=NULL;

            /* reset pattern stuff */
            pat_file_open=False;
            pat_menu_settings();
            XmTextSetString(pat_file_text,"No file open.");

            /* reset number of epochs and total sum of squares since pattern file used for
               training is now closed */
            XmTextSetString(total_iter_text,"0");
            XmTextSetString(tss_text,"");
            total_iterations=0;
            break;
        case CLOSE_WTS:
            /* ask to save changes */
            if (wts_changed)
                {
                if (!save_wts_changes_dialog)
                    create_save_wts_changes_dialog();
                XtManageChild(save_wts_changes_dialog);
                }
            else
                {     
                /* close file stream if open */
                if (wptr)    
                    {   
                    fclose(wptr);
                    wptr=NULL;
                    }

                /* reset weights stuff */
                wts_file_open=False;
                wts_menu_settings();
                XmTextSetString(wts_file_text,"No file open.");
                wts_filename[0]='\0';

                /* reset number of epochs and total sum of squares since weights file 
                   used for training is now closed */ 
                XmTextSetString(total_iter_text,"0");
                XmTextSetString(tss_text,"");
                total_iterations=0;
                }
            break;
        case GEN_WTS:
            /* generate random weights */
            make_weights(); 

            /* reset weights stuff */
            wts_filename[0]='\0';
            wts_file_open=True;
            wts_menu_settings();
            XtSetSensitive(weights_save_option, False);
            XmTextSetString(wts_file_text,"Generated weights.");

            /* reset number of epochs and total sum of squares since weights changed */ 
            XmTextSetString(total_iter_text,"0");
            XmTextSetString(tss_text,"");
            total_iterations=0;
            break;
        case SAVE_WTS:
            /* open file for writing */
            if ((w2ptr = fopen(wts_filename,"w")) == NULL)
                XbError("Weights file could not be written due\nto system error.");
            else
                {
                /* write weights to current file */
                write_weights();
                fclose(w2ptr);
                XtSetSensitive(weights_save_option, False);
                }
            break;
        case WTS_SAVE_AS:
            /* update file listing and show current filename as selection if there is a
               current filename */
            if (!wts_save_dialog)
                create_wts_save_dialog();

            /* set the current directory */
            ac = 0;
            XtSetArg(al[ac], XmNdirectory, curr_dir); ac++;
            XtSetValues(wts_save_dialog, al, ac);

            if (strlen(wts_filename))
                {
                XmTextSetString(XmSelectionBoxGetChild(wts_save_dialog, XmDIALOG_TEXT), wts_filename);
                XmTextShowPosition(XmSelectionBoxGetChild(wts_save_dialog, XmDIALOG_TEXT),
                    XmTextGetLastPosition(XmSelectionBoxGetChild(wts_save_dialog, XmDIALOG_TEXT)));
                }
            XtManageChild(wts_save_dialog);
            break;
        case TEST_PAT:
            /* manage test pattern dialog for applying test patterns */
            if (!pattern_dialog)
                create_pattern_dialog();
            XtManageChild(pattern_dialog);
            break;
        case SAVE_NET:
            /* check that network is complete (incomplete network cannot be saved) */
            if (!net_complete)
                {
                XbError("The network is not yet complete.\nAn incomplete net cannot be saved.");
                return;
                }

            /* update menu sensitivity, status flag, and save net list to current file */
            XtSetSensitive(network_save_option, False);
            net_changed=False;
            save_net_list();
            break;
        case DISCONNECT:
            /* ask user to select node to disconnect */
            XtManageChild(info_dialog);
            break;
        case FULL_CONN:
            /* verify choice of fully connecting network (change will erase old connection
               data) */
            if (!full_conn_ques_dialog)
                create_full_conn_ques_dialog();
            XtManageChild(full_conn_ques_dialog);
            break;
        case NET_SAVE_AS:
            /* check that the network is complete (incomplete network cannot be saved) */
            if (!net_complete)
                {
                XbError("The network is not yet complete.\nAn incomplete net cannot be saved.");
                return;
                }

            /* update file listing and set selection to current filename if there is one */
            if (!net_save_dialog)
                create_net_save_dialog();

            /* set the current directory */
            ac = 0;
            XtSetArg(al[ac], XmNdirectory, curr_dir); ac++;
            XtSetValues(net_save_dialog, al, ac);

            if (strlen(new_net_filename))
                {
                XmTextSetString(XmSelectionBoxGetChild(net_save_dialog, XmDIALOG_TEXT), new_net_filename);
                XmTextShowPosition(XmSelectionBoxGetChild(net_save_dialog, XmDIALOG_TEXT),
                    XmTextGetLastPosition(XmSelectionBoxGetChild(net_save_dialog, XmDIALOG_TEXT)));
                }

            XtManageChild(net_save_dialog);
            break;
        case MOMENTUM:
            /* initialize to current momentum and prompt for new one */
            if (!momentum_dialog)
                create_momentum_dialog();
            sprintf(temp_str,"%f",momentum);
            XmTextSetString(XmSelectionBoxGetChild(momentum_dialog,XmDIALOG_TEXT),temp_str);
            XtManageChild(momentum_dialog);
            break;
        case GAIN:
            /* initialize to current gain and prompt for new one */
            if (!gain_dialog)
                create_gain_dialog();
            sprintf(temp_str,"%f",gain);
            XmTextSetString(XmSelectionBoxGetChild(gain_dialog,XmDIALOG_TEXT),temp_str);
            XtManageChild(gain_dialog);
            break; 
        case ABOUT:
            XbInfo(ABOUT_STRING);
            break;
 	case QUIT:
            /* set flag showing that quit has been chosen (for use by save changes) */
            quit=True;

            /* ask to save changes, if any */
            if (wts_changed)
                {
                if (!save_wts_changes_dialog)
                  create_save_wts_changes_dialog();
                XtManageChild(save_wts_changes_dialog);
                }
            else if (net_changed)
                {
                if (!save_net_changes_dialog)
                    create_save_net_changes_dialog();
                XtManageChild(save_net_changes_dialog);
                }
            else
                {
                /* make sure all file streams are closed and allocated memory freed */
                if (net_file_open)
                    fclose(nptr);
                if (pat_file_open)
                    fclose(pptr);
                if (wts_file_open)
                    fclose(wptr);
                if (pattern)
                    free(pattern);
                if (node)
                    free(node);

                XtCloseDisplay(XtDisplay(toplevel));
	        exit(1);
	        break;
                }
	  }
    }


/* adds a separator into the menu */
void make_menu_separator(Widget menu)
    {
    XtManageChild(XmCreateSeparatorGadget(menu,"sep",NULL,0));
    }


/* Creates a new menu on the menu bar. */
Widget make_help_menu(char *menu_name, KeySym mnemonic, Widget menu_bar)
    {
    int ac;
    Arg al[10];
    Widget menu, cascade;

    ac = 0;
    menu = XmCreatePulldownMenu (menu_bar, menu_name, al, ac);

    ac = 0;
    XtSetArg (al[ac], XmNsubMenuId, menu); ac++;
    XtSetArg (al[ac], XmNmnemonic, mnemonic); ac++;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreateLtoR(menu_name, char_set)); ac++;
    cascade = XmCreateCascadeButton (menu_bar, menu_name, al, ac);
    XtManageChild (cascade); 

    ac=0;
    XtSetArg(al[ac],XmNmenuHelpWidget,cascade); ac++;
    XtSetValues(menu_bar,al,ac);

    return(menu);
    }


/* adds an accelerator to a menu option. */
void add_accelerator(Widget w, char *acc_text, char *key)
    {
    int ac;
    Arg al[10];

    ac=0;
    XtSetArg(al[ac],XmNacceleratorText,
        XmStringCreate(acc_text,char_set)); ac++;
    XtSetArg(al[ac],XmNaccelerator,key); ac++;
    XtSetValues(w,al,ac);
    }


/* Adds an option to the menu bar. */
Widget make_menu_option(char *option_name, KeySym mnemonic, int client_data, Widget menu)
    {
    int ac;
    Arg al[10];
    Widget b;

    ac = 0;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreateLtoR(option_name,
        char_set)); ac++;
    XtSetArg (al[ac], XmNmnemonic, mnemonic); ac++;
    b=XtCreateManagedWidget(option_name,xmPushButtonGadgetClass,
        menu,al,ac);
    XtAddCallback (b, XmNactivateCallback, menuCB, client_data);
    return(b);
    }


/* Creates a new menu on the menu bar. */
Widget make_menu(char *menu_name, KeySym mnemonic, Widget menu_bar)
    {
    int ac;
    Arg al[10];
    Widget menu, cascade;

    ac = 0;
    menu = XmCreatePulldownMenu (menu_bar, menu_name, al, ac);

    ac = 0;
    XtSetArg (al[ac], XmNsubMenuId, menu); ac++;
    XtSetArg (al[ac], XmNmnemonic, mnemonic); ac++;
    XtSetArg(al[ac], XmNlabelString,
             XmStringCreateLtoR(menu_name, char_set)); ac++;
    cascade = XmCreateCascadeButton (menu_bar, menu_name, al, ac);
    XtManageChild (cascade); 

    return(menu);
    }


/* create menu bar and selections on the menu */
void create_menus(Widget menu_bar)
   {
    Widget menu, about_option;

    /* 'Network' */
    network_menu = make_menu("Network",'N',menu_bar);
    network_open_option = make_menu_option("Open",'O',OPEN_NET,network_menu);
    network_new_option = make_menu_option("New",'N',NEW_NET,network_menu);
    network_save_option = make_menu_option("Save",'S',SAVE_NET,network_menu);
    network_saveas_option = make_menu_option("Save As...",'A',NET_SAVE_AS,network_menu);
    network_close_option = make_menu_option("Close",'C',CLOSE_NET,network_menu);
    network_fullconn_option = make_menu_option("Fully Connect",'F',FULL_CONN,network_menu);
    add_accelerator(network_fullconn_option,"Meta+f","Meta<Key>f:");
    network_disconnect_option = make_menu_option("Disconnect",'D',DISCONNECT,network_menu);
    add_accelerator(network_disconnect_option,"Meta+d","Meta<Key>d:");
    network_momentum_option = make_menu_option("Momentum",'M',MOMENTUM,network_menu);
    add_accelerator(network_momentum_option,"Meta+m","Meta<Key>m:");
    network_gain_option = make_menu_option("Gain",'G',GAIN,network_menu);
    add_accelerator(network_gain_option,"Meta+g","Meta<Key>g:");
    make_menu_separator(network_menu);
    quit_option = make_menu_option("Exit",'Q',QUIT,network_menu);
    add_accelerator(quit_option,"meta+q","Meta<Key>q:");
    
    /* 'Pattern' */
    pattern_menu = make_menu("Pattern",'P',menu_bar);
    pattern_open_option = make_menu_option("Open   ",'O',OPEN_PAT,pattern_menu);
    pattern_test_option = make_menu_option("Test...",'T',TEST_PAT,pattern_menu);
    add_accelerator(pattern_test_option,"Meta+t","Meta<Key>t:");
    pattern_close_option = make_menu_option("Close   ",'C',CLOSE_PAT,pattern_menu);

    /* 'Weights' */
    weights_menu = make_menu("Weights",'W',menu_bar);
    weights_open_option = make_menu_option("Open",'O',OPEN_WTS,weights_menu);
    weights_save_option = make_menu_option("Save",'S',SAVE_WTS,weights_menu);
    weights_saveas_option = make_menu_option("Save As...",'A',WTS_SAVE_AS,weights_menu);
    weights_close_option = make_menu_option("Close",'C',CLOSE_WTS,weights_menu);
    weights_gen_option = make_menu_option("Generate",'G',GEN_WTS,weights_menu);

    /* 'Help' */
    menu=make_help_menu("Help",'H',menu_bar);
    about_option=make_menu_option("About",'A',ABOUT,menu);
    add_accelerator(about_option,"Meta+a","Meta<Key>a:");
    help_option=make_menu_option("Help",'H',HELP,menu);
    add_accelerator(help_option,"Meta+h","Meta<Key>h:");
    XtAddCallback(help_option,XmNactivateCallback,XnHelpCB,"Index");

    /* initialize sensitivities */
    net_menu_settings();
    pat_menu_settings();
    wts_menu_settings();
    }


/* checks to see if all characters in string s are numbers */
Boolean is_integer(char *s, int cnt)
    {
    int i=0;

    if (cnt>10)
        return(False);
    for (;i<cnt;i++)
        if (!isdigit(s[i]))
            return(False);

    return(True);
    }


/* callback for create new network dialog */
void new_netCB(Widget w, int client_data, int call_data) 
    {
    char *temp[5], err_msg[1000];
    int number_of_nodes;

    /* initialize err_msg */
    err_msg[0]='\0';

    switch(client_data)
	{
        /* first toggle button pressed */
        case ONE_HID:
	    XtSetSensitive(second_layer_label, False);
	    XtSetSensitive(second_layer_text, False);
            XtSetSensitive(third_layer_label, False);
            XtSetSensitive(third_layer_text, False);
            tog_layers=num_hid_layers=1;
	    XmTextSetString(second_layer_text, "");
            XmTextSetString(third_layer_text, "");
	    break;

        /* second toggle button pressed */
	case TWO_HID:
	    XtSetSensitive(second_layer_label, True);
	    XtSetSensitive(second_layer_text, True);
            XtSetSensitive(third_layer_label, False);
            XtSetSensitive(third_layer_text, False);
            tog_layers=num_hid_layers=2;
            XmTextSetString(third_layer_text, "");
	    break;

        /* third toggle button pressed */
	case THREE_HID:
	    XtSetSensitive(second_layer_label, True);
	    XtSetSensitive(second_layer_text, True);
            XtSetSensitive(third_layer_label, True);
            XtSetSensitive(third_layer_text, True);
            tog_layers=num_hid_layers=3;
	    break;

        /* ok button pressed */
        case OK:
            /* get the number of nodes in each layer */
            temp[0]=XmTextGetString(input_text);
            temp[1]=XmTextGetString(output_text);
            temp[2]=XmTextGetString(first_layer_text);
            temp[3]=XmTextGetString(second_layer_text);
            temp[4]=XmTextGetString(third_layer_text);

            /* number of nodes in input layer must be an integer and greater than zero */
            if (strlen(temp[0])==0)
                {
                strcat(err_msg,"There must be more than zero input nodes.\n");
                XmProcessTraversal(input_text,XmTRAVERSE_CURRENT);
                }
            else if (!is_integer(temp[0], strlen(temp[0])))
                {
                strcat(err_msg,"The number of input nodes must be a positive integer.\n");
                XmProcessTraversal(input_text,XmTRAVERSE_CURRENT);
                }
            else
                {
                sscanf(temp[0], "%d", &number_of_nodes);
                if (number_of_nodes > 545)
                    {
                    strcat(err_msg,"The number of nodes in any layer must be less than 545.  Please enter a lower number of input nodes.\n");
                    XmProcessTraversal(input_text,XmTRAVERSE_CURRENT);
                    }
                }

            /* number of nodes in output layer must be an integer and greater than zero */
            if (strlen(temp[1])==0)
                {
                if (!strlen(err_msg))
                    XmProcessTraversal(output_text,XmTRAVERSE_CURRENT);
                strcat(err_msg,"There must be more than zero output nodes.\n");
                }
            else if (!is_integer(temp[1], strlen(temp[1])))
                {
                if (!strlen(err_msg))
                    XmProcessTraversal(output_text,XmTRAVERSE_CURRENT);
                strcat(err_msg,"The number of output nodes must be a positive integer.\n");
                }
            else
                {
                sscanf(temp[1], "%d", &number_of_nodes);
                if (number_of_nodes > 545)
                    {
                    strcat(err_msg,"The number of nodes in any layer must be less than 545.  Please enter a lower number of output nodes.\n");
                    XmProcessTraversal(output_text,XmTRAVERSE_CURRENT);
                    }
                }

            /* number of nodes in first hidden layer must be an integer and greater than
               zero */
            if (strlen(temp[2])==0)
                {
                if (!strlen(err_msg))
                    XmProcessTraversal(first_layer_text,XmTRAVERSE_CURRENT);
                strcat(err_msg,"There must be more than zero nodes in the first hidden layer.\n");
                }
            else if (!is_integer(temp[2], strlen(temp[2])))
                {
                if (!strlen(err_msg))
                    XmProcessTraversal(first_layer_text,XmTRAVERSE_CURRENT);
                strcat(err_msg,"The number of first hidden layer nodes must be a positive integer.\n");
                }
            else
                {
                sscanf(temp[2], "%d", &number_of_nodes);
                if (number_of_nodes > 545)
                    {
                    strcat(err_msg,"The number of nodes in any layer must be less than 545.  Please enter a lower number of first layer nodes.\n");
                    XmProcessTraversal(first_layer_text,XmTRAVERSE_CURRENT);
                    }
                }

            /* if more than one hidden layer, number of nodes in second hidden layer must
               be an integer and greater than zero */
            if ((strlen(temp[3])==0) && (num_hid_layers>=2))
                {
                if (!strlen(err_msg))
                    XmProcessTraversal(second_layer_text,XmTRAVERSE_CURRENT);
                strcat(err_msg,"There must be more than zero nodes in the second layer for multiple hidden layers.\n");
                }
            else if ((!is_integer(temp[3], strlen(temp[3]))) && (num_hid_layers>=2))
                {
                if (!strlen(err_msg))
                    XmProcessTraversal(second_layer_text,XmTRAVERSE_CURRENT);
                strcat(err_msg,"The number of second hidden layer nodes must be a positive integer.\n");
                }
            else if (num_hid_layers>=2)
                {
                sscanf(temp[3], "%d", &number_of_nodes);
                if (number_of_nodes > 545)
                    {
                    strcat(err_msg,"The number of nodes in any layer must be less than 545.  Please enter a lower number of second layer nodes.\n");
                    XmProcessTraversal(second_layer_text,XmTRAVERSE_CURRENT);
                    }
                }

            /* if three hidden layers, number of nodes in the third hidden layer must be
               an integer and greater than zero */
            if ((strlen(temp[4])==0) && (num_hid_layers==3))
                {
                if (!strlen(err_msg))
                    XmProcessTraversal(third_layer_text,XmTRAVERSE_CURRENT);
                strcat(err_msg,"There must be more than zero nodes in the third layer for 3 hidden layers.\n");
                }
            else if((!is_integer(temp[4], strlen(temp[4]))) && (num_hid_layers==3))
                {
                if (!strlen(err_msg))
                    XmProcessTraversal(third_layer_text,XmTRAVERSE_CURRENT);
                strcat(err_msg,"The number of third hidden layer nodes must be a positive integer.\n");
                }
            else if (num_hid_layers==3)
                {
                sscanf(temp[4], "%d", &number_of_nodes);
                if (number_of_nodes > 545)
                    {
                    strcat(err_msg,"The number of nodes in any layer must be less than 545.  Please enter a lower number of third layer nodes.\n");
                    XmProcessTraversal(third_layer_text,XmTRAVERSE_CURRENT);
                    }
                }

            /* if any error messages needed, display them */
            if (strlen(err_msg))
                {
                XbError(err_msg);
                return;
                }

            sscanf(temp[0],"%d",&num_in_nodes);
            sscanf(temp[1],"%d",&num_out_nodes);
            sscanf(temp[2],"%d",&num_hid_nodes[0]);

            /* if no entry for number of second layer nodes, set to zero */
            if (!strlen(temp[3]))
                num_hid_nodes[1]=0;
            else
                sscanf(temp[3],"%d",&num_hid_nodes[1]);

            /* if no entry for number of third layer nodes, set to zero */
            if (!strlen(temp[4]))
                num_hid_nodes[2]=0;
            else
                sscanf(temp[4],"%d",&num_hid_nodes[2]);

            XtUnmanageChild(new_net_dialog);

            /* set up network as specified */
            set_up_net();

            /* set status flags, initialize filename for network */
            net_file_open=True;
            net_menu_settings();
            new_net_filename[0]='\0';

            break;
	case CANCEL:
	    XtUnmanageChild(new_net_dialog);
            break;
	}

    }


/* create new network dialog */
void create_new_net_dialog()
    {
    int ac;
    Arg al[15];
    static char translations1[]="<Key>Return: next-tab-group()",
                translations2[]="<Key>Return: next-tab-group()next-tab-group()";
    XtTranslations trans_table1,trans_table2;
    Widget new_net_form;

    XbWatchCursor(toplevel);
    
    ac=0;
    XtSetArg(al[ac], XmNmarginWidth, 5); ac++;
    new_net_dialog=XbCreateDialog(main_form,"new_net_dialog","NeuralNet:  New Network",
           &new_net_form,al,ac);

    XtAddCallback(new_net_dialog, XmNokCallback, new_netCB, OK);    
    XtAddCallback(new_net_dialog, XmNcancelCallback, new_netCB, CANCEL);    

    /* create a label */
    ac=0;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreate("Number of Nodes",char_set)); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_NONE); ac++;
    XtSetArg(al[ac], XmNtopOffset, 5); ac++;
    num_nodes_label=XmCreateLabelGadget(new_net_form,"num_nodes_label",al,ac);
    XtManageChild(num_nodes_label);

    /* create a separator */
    ac=0;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, num_nodes_label); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_NONE); ac++; 
    XtSetArg(al[ac], XmNtopOffset, 10); ac++;
    new_net_sep1=XmCreateSeparatorGadget(new_net_form,"new_net_sep1",al,ac);
    XtManageChild(new_net_sep1);

    /* create a rowcol container */
    ac=0;
    XtSetArg(al[ac], XmNpacking, XmPACK_COLUMN); ac++;
    XtSetArg(al[ac], XmNorientation, XmHORIZONTAL); ac++;
    XtSetArg(al[ac], XmNnumColumns, 2); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, new_net_sep1); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_NONE); ac++;
    XtSetArg(al[ac], XmNtopOffset, 10); ac++;
    new_net_rc1=XmCreateRowColumn(new_net_form,"new_net_rc1",al,ac);
    XtManageChild(new_net_rc1);

    /* create a label */
    ac=0;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreate("Input         ",char_set)); ac++;
    input_label=XmCreateLabelGadget(new_net_rc1,"input_label",al,ac);
    XtManageChild(input_label);

    /* create a text widget */
    ac=0;
    XtSetArg(al[ac], XmNcolumns, 10); ac++;
    input_text=XmCreateText(new_net_rc1,"input_text",al,ac);
    XtManageChild(input_text);

    /* create a label */
    ac=0;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreate("Output       ",char_set)); ac++;
    output_label=XmCreateLabelGadget(new_net_rc1,"output_label",al,ac);
    XtManageChild(output_label);

    /* create a text widget */
    ac=0;
    XtSetArg(al[ac], XmNcolumns, 10); ac++;
    output_text=XmCreateText(new_net_rc1,"output_text",al,ac);
    XtManageChild(output_text);

    /* create a label */
    ac=0;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreate("Number of Hidden Layers",char_set)); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, new_net_rc1); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_NONE); ac++; 
    num_hid_layers_label=XmCreateLabelGadget(new_net_form,"num_hid_layers_label",al,ac);
    XtManageChild(num_hid_layers_label);

    /* create a rowcol container */
    ac=0;
    XtSetArg(al[ac], XmNradioBehavior, True); ac++;
    XtSetArg(al[ac], XmNisHomogeneous, True); ac++;
    XtSetArg(al[ac], XmNpacking, XmPACK_COLUMN); ac++;
    XtSetArg(al[ac], XmNorientation, XmHORIZONTAL); ac++;
    XtSetArg(al[ac], XmNentryAlignment, XmALIGNMENT_CENTER); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, num_hid_layers_label); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNleftPosition, 15); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_POSITION); ac++;
    XtSetArg(al[ac], XmNrightPosition, 90); ac++;
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_NONE); ac++; 
    new_net_rc2=XmCreateRowColumn(new_net_form, "new_net_rc2", al, ac);
    XtManageChild(new_net_rc2);

    /* create a toggle button */
    ac=0;
    XtSetArg(al[ac], XmNset, True); ac++;
    XtSetArg(al[ac], XmNlabelString,XmStringCreate("1  ",char_set)); ac++;
    one_toggle=XmCreateToggleButtonGadget(new_net_rc2, "one_toggle", al, ac);
    XtManageChild(one_toggle);
    XtAddCallback(one_toggle, XmNvalueChangedCallback, new_netCB, ONE_HID);

    /* create a toggle button */
    ac=0;
    XtSetArg(al[ac], XmNlabelString,XmStringCreate("2  ",char_set)); ac++;
    two_toggle=XmCreateToggleButtonGadget(new_net_rc2, "two_toggle", al, ac);
    XtManageChild(two_toggle);
    XtAddCallback(two_toggle, XmNvalueChangedCallback, new_netCB, TWO_HID);

    /* create a toggle button */
    ac=0;
    XtSetArg(al[ac], XmNlabelString,XmStringCreate("3  ",char_set)); ac++;
    three_toggle=XmCreateToggleButtonGadget(new_net_rc2, "three_toggle", al, ac);
    XtManageChild(three_toggle);
    XtAddCallback(three_toggle, XmNvalueChangedCallback, new_netCB, THREE_HID);

    /* create a rowcol container */
    ac=0;
    XtSetArg(al[ac], XmNpacking, XmPACK_COLUMN); ac++;
    XtSetArg(al[ac], XmNorientation, XmHORIZONTAL); ac++;
    XtSetArg(al[ac], XmNnumColumns, 3); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, new_net_rc2); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_NONE); ac++;
    new_net_rc3=XmCreateRowColumn(new_net_form,"new_net_rc3",al,ac);
    XtManageChild(new_net_rc3);

    /* create a label */
    ac=0;
    XtSetArg(al[ac], XmNlabelString,XmStringCreate("First Layer   ",char_set)); ac++;
    first_layer_label=XmCreateLabelGadget(new_net_rc3,"first_layer_label",al,ac);
    XtManageChild(first_layer_label);

    /* create a text widget */
    ac=0;
    XtSetArg(al[ac], XmNcolumns, 10); ac++;
    first_layer_text=XmCreateText(new_net_rc3,"first_layer_text",al,ac);
    XtManageChild(first_layer_text);

    /* create a label */
    ac=0;
    XtSetArg(al[ac], XmNlabelString,XmStringCreate("Second Layer",char_set)); ac++;
    second_layer_label=XmCreateLabelGadget(new_net_rc3,"second_layer_label",al,ac);
    XtManageChild(second_layer_label);

    /* create a text widget */
    ac=0;
    XtSetArg(al[ac], XmNcolumns, 10); ac++;
    second_layer_text=XmCreateText(new_net_rc3,"second_layer_text",al,ac);
    XtManageChild(second_layer_text);

    /* create a label */
    ac=0;
    XtSetArg(al[ac], XmNlabelString,XmStringCreate("Third Layer   ",char_set)); ac++;
    third_layer_label=XmCreateLabelGadget(new_net_rc3,"third_layer_label",al,ac);
    XtManageChild(third_layer_label);

    /* create a text widget */
    ac=0;
    XtSetArg(al[ac], XmNcolumns, 10); ac++;
    third_layer_text=XmCreateText(new_net_rc3,"third_layer_text",al,ac);
    XtManageChild(third_layer_text);

    /* initial dialog settings */
    XtSetSensitive(second_layer_label, False);
    XtSetSensitive(second_layer_text, False);
    XtSetSensitive(third_layer_label, False);
    XtSetSensitive(third_layer_text, False);

    /* change translations so that <return> moves to next text field */
    trans_table1=XtParseTranslationTable(translations1);
    trans_table2=XtParseTranslationTable(translations2);

    XtOverrideTranslations(input_text, trans_table1);
    XtOverrideTranslations(output_text, trans_table2);
    XtOverrideTranslations(first_layer_text, trans_table1); 
    XtOverrideTranslations(second_layer_text, trans_table1);
    XtOverrideTranslations(third_layer_text, trans_table1);
    
    XbNormalCursor(toplevel);
    }


/* read in pattern file specified in test pattern open file dialog */
Boolean read_pattern_test_file()
    {
    FILE *f;
    int file_length;
    struct stat stat_val;
    char *file_contents;

    if (stat(pattern_test_filename, &stat_val) == 0)
        {
        file_length = stat_val.st_size;

        /* try to open file in "r" mode. if okay then read it. */
        if ((f=fopen(pattern_test_filename,"r"))==NULL)
            {
            XbError("The specified file could not\nbe opened for reading.");
            return(False);
            }

        /* malloc a place for the string to be read to. */
        file_contents = (char *)XtMalloc(file_length+1);
        *file_contents = '\0';

        /* read the file string */
        fread(file_contents, sizeof(char), file_length, f);
        fclose(f);
        file_contents[file_length]='\0';

        /* give the string to the text widget. */
        XmTextSetString(pattern_text,file_contents);

        XtFree(file_contents);
        return(True);
        }
    else
        return(False);
    }


/* callback for open test pattern file dialog */
void pattern_test_file_openCB(Widget w, int client_data, XmSelectionBoxCallbackStruct *call_data)
    {
    int ac;
    Arg al[5];
    char *name, label_filename[200];

    XbWatchCursor(toplevel);
    switch (client_data)
        {
        case OK:
            XmStringGetLtoR(call_data->value, char_set, &name);
            strcpy(pattern_test_filename,name);

            /* get the current working directory from the dialog */
            XmStringFree(curr_dir);
            ac = 0;
            XtSetArg(al[ac], XmNdirectory, &curr_dir); ac++;
            XtGetValues(pattern_test_file_open_dialog, al, ac);

            /* read in test pattern file */
            if (read_pattern_test_file())
		{
                XtUnmanageChild(pattern_test_file_open_dialog);

                /* display current filename on dialog */
                sprintf(label_filename,"File:  %s",name);
                ac=0;
                XtSetArg(al[ac], XmNlabelString, XmStringCreate(label_filename,char_set)); ac++;
                XtSetValues(pattern_file_label, al, ac);
                XtFree(name);

                /* change menu sensitivities appropriately */
                patdlog_menu_settings(True);

                /* set status flag */
                pat_changed=False;

                XbNormalCursor(toplevel);
		}
            else
                {
                XbNormalCursor(toplevel);
                XbError("The specified file could not\nbe opened for reading.\n");
		}
            break;
        case CANCEL:
            XtUnmanageChild(pattern_test_file_open_dialog);
            XbNormalCursor(toplevel);
            break;
        }

    }


/* create test pattern file open dialog */
void create_pattern_test_file_open_dialog()
    {
    Arg al[10];
    int ac;

    XbWatchCursor(toplevel);
    XbWatchCursor(pattern_dialog);
    
    /* Create the file selection box used by open option. */
    ac=0;
    XtSetArg(al[ac],XmNmustMatch,True); ac++;
    XtSetArg(al[ac],XmNautoUnmanage,False); ac++;
    XtSetArg(al[ac],XmNdialogTitle,XmStringCreateLtoR(
        "NeuralNet: Open Test Pattern File",char_set)); ac++;
    XtSetArg(al[ac],XmNpattern,XmStringCreateLtoR("*.pat",char_set)); ac++;
    pattern_test_file_open_dialog=XmCreateFileSelectionDialog(toplevel,"pattern_test_file_open_dialog",al,ac);
    XtAddCallback (pattern_test_file_open_dialog, XmNokCallback, pattern_test_file_openCB, OK);
    XtAddCallback (pattern_test_file_open_dialog, XmNcancelCallback, pattern_test_file_openCB, CANCEL);
    XtUnmanageChild(XmSelectionBoxGetChild(pattern_test_file_open_dialog,
        XmDIALOG_HELP_BUTTON));

    ac=0;
    XtSetArg(al[ac],XmNlabelString,XmStringCreateLtoR("Open",char_set)); ac++;
    XtSetValues(XmSelectionBoxGetChild(pattern_test_file_open_dialog,XmDIALOG_OK_BUTTON), al, ac);
    
    XbNormalCursor(pattern_dialog);
    XbNormalCursor(toplevel);
    }


/* callback for test pattern save file dialog */
void pattern_test_file_saveCB(Widget w, int client_data, XmSelectionBoxCallbackStruct *call_data)
    {
    int ac;
    Arg al[5];
    char label_filename[200], *to_be_saved;
    FILE *file;

    XbWatchCursor(main_form);

    switch (client_data)
        {
        case OK:
            XmStringGetLtoR(call_data->value, char_set, &filename);
            /* basic check on filename */
            if (strlen(filename) && (filename[strlen(filename)-1]!='/'))
                {
                /* if file exists, ask about overwrite; do not worry about overwrite if
                   it is a save changes call */
                if (((file=fopen(filename,"r"))!=NULL) && save_changes_finished)
                    {
                    XbNormalCursor(main_form);
                    fclose(file);
                    if (!overwrite_pat_dialog)
                        create_overwrite_pat_dialog();
                    XtUnmanageChild(w);
                    XtManageChild(overwrite_pat_dialog);
                    return;
                    }
                else
                    {
                    /* write file if possible */
                    if ((file=fopen(filename,"w"))!=NULL)
                        {
                        strcpy(pattern_test_filename,filename);

                        /* write string all at once to file */
                        to_be_saved=XmTextGetString(pattern_text);
                        fprintf(file,"%s",to_be_saved);
                        fclose(file);
                        XtFree(to_be_saved);

                        /* show name of current file on dialog */
                        sprintf(label_filename,"File:  %s",filename);
                        ac=0;
                        XtSetArg(al[ac], XmNlabelString, XmStringCreate(label_filename,char_set)); ac++;
                        XtSetValues(pattern_file_label, al, ac);

                        /* finish save changes if necessary */
                        if (!save_changes_finished)
                            {
                            save_pat_changesCB(NULL,CANCEL,NULL);
                            save_changes_finished=True;
                            }

                        /* clear status flag */
                        pat_changed=False;
                        }
                     else
                        XbError("The file specified could not\nbe opened for writing.  File not saved.");
                    }
                }
            else
                {
                XbError("You must enter a filename.\n");
                return;
                }
            XtFree(filename);
            break;
         case CANCEL:
            /* if this was a save changes call, finish it up */
            if (!save_changes_finished)
                {
                save_pat_changesCB(NULL,CANCEL,NULL);
                save_changes_finished=True;
                }
            break;
         }

    XtUnmanageChild(w);
    XbNormalCursor(main_form);
    }


/* create save test pattern file dialog */
void create_pattern_test_file_save_dialog()
    {
    Arg al[10];
    int ac;

    XbWatchCursor(toplevel);
    XbWatchCursor(pattern_dialog);
    
    /* Create the file selection box used by open option. */
    ac=0;
    XtSetArg(al[ac],XmNautoUnmanage,False); ac++;
    XtSetArg(al[ac],XmNdialogTitle,XmStringCreateLtoR(
        "NeuralNet: Save Test Pattern File",char_set)); ac++;
    XtSetArg(al[ac],XmNpattern,XmStringCreateLtoR("*.pat",char_set)); ac++;
    pattern_test_file_save_dialog=XmCreateFileSelectionDialog(toplevel,"pattern_test_file_save_dialog",al,ac);
    XtAddCallback (pattern_test_file_save_dialog, XmNokCallback, pattern_test_file_saveCB, OK);
    XtAddCallback (pattern_test_file_save_dialog, XmNcancelCallback, pattern_test_file_saveCB, CANCEL);
    XtUnmanageChild(XmSelectionBoxGetChild(pattern_test_file_save_dialog, XmDIALOG_HELP_BUTTON));

    XtSetSensitive(XmSelectionBoxGetChild(pattern_test_file_save_dialog, XmDIALOG_LIST), False);

    ac=0;
    XtSetArg(al[ac],XmNlabelString,XmStringCreateLtoR("Save",char_set)); ac++;
    XtSetValues(XmSelectionBoxGetChild(pattern_test_file_save_dialog,XmDIALOG_OK_BUTTON), al, ac);

    XbNormalCursor(pattern_dialog);
    XbNormalCursor(toplevel);
    }


/* callback for save changes question dialog */
void save_pat_changesCB(Widget w, int client_data, XmAnyCallbackStruct *call_data)
    {
    switch(client_data)
        {
        case SAVE_PAT:
            if (!pattern_test_file_save_dialog)
                create_pattern_test_file_save_dialog();

            /* update file listing, show current file name if one exists */
            XmFileSelectionDoSearch(pattern_test_file_save_dialog,XmStringCreateLtoR("*.*",char_set));
            if (strlen(pattern_test_filename))
                {
                XmTextSetString(XmSelectionBoxGetChild(pattern_test_file_save_dialog, XmDIALOG_TEXT),
                    pattern_test_filename);
                XmTextShowPosition(XmSelectionBoxGetChild(pattern_test_file_save_dialog, XmDIALOG_TEXT),
                    XmTextGetLastPosition(XmSelectionBoxGetChild(pattern_test_file_save_dialog, XmDIALOG_TEXT)));
                }
            XtManageChild(pattern_test_file_save_dialog);

            /* set status flag */
            save_changes_finished=False;
            break;
        case CANCEL:
            /* set status flags, finish close */
            pat_changed=False;
            save_changes_finished=True;
            if (cancel_test)
                pattern_dialogCB(NULL,CANCEL,NULL);
            else
                pattern_dialogCB(NULL,CLOSE,NULL);
            break;
        }

    if (w)
         XtUnmanageChild(w);
    }


/* create save test pattern changes question dialog */
void create_save_pat_changes_dialog()
    {
    int ac;
    Arg al[5];
    
    XbWatchCursor(toplevel);

    ac=0;
    XtSetArg(al[ac], XmNdialogTitle, XmStringCreateLtoR("NeuralNet:  Save Changes?", 
        char_set)); ac++;
    XtSetArg(al[ac], XmNmessageString, XmStringCreateLtoR("Save changes to pattern?",
        char_set));  ac++;
    XtSetArg(al[ac], XmNmessageAlignment, XmALIGNMENT_CENTER); ac++;
    save_pat_changes_dialog = XmCreateQuestionDialog(toplevel,"save_pat_changes_dialog", al, ac);

    XtAddCallback(save_pat_changes_dialog, XmNokCallback, save_pat_changesCB, SAVE_PAT);
    XtAddCallback(save_pat_changes_dialog, XmNcancelCallback, save_pat_changesCB, CANCEL);

    XtUnmanageChild(XmMessageBoxGetChild(save_pat_changes_dialog, XmDIALOG_HELP_BUTTON));
    
    ac=0;
    XtSetArg(al[ac], XmNlabelString, XmStringCreateLtoR("Save...",char_set)); ac++;
    XtSetValues(XmMessageBoxGetChild(save_pat_changes_dialog, XmDIALOG_OK_BUTTON),al,ac);

    ac=0;
    XtSetArg(al[ac], XmNlabelString, XmStringCreateLtoR("Discard",char_set)); ac++;
    XtSetValues(XmMessageBoxGetChild(save_pat_changes_dialog, XmDIALOG_CANCEL_BUTTON),al,ac);

    XbNormalCursor(toplevel);
    }


/* callback for test pattern dialog */
void pattern_dialogCB(Widget w, int client_data, int call_data) 
    {
    char test[100], *temp_str, new_pattern[10000], ch;
    Arg al[5];
    int i, ac;
    static int position=0;
    static Boolean first_step=True;
    static char *step_str=NULL;
    XmTextPosition text_position;

    switch(client_data)
        {
        case TEST:
        /* apply test pattern to network */

            /* initialize test, get pattern with which to test network */
            test[0]='\0';
            temp_str=XmTextGetString(pattern_test_text);
            motif_pattern_entered=temp_str;

            /* if comment, break */
            if(strstr(motif_pattern_entered, ";"))
                {
                XbError("Cannot use a comment line as test input.");
                break;
                }

            /* strip leading spaces */
            for(i=0;isspace(temp_str[i]) && (i<strlen(temp_str));i++)
                motif_pattern_entered=(temp_str+i+1);

            /* in case this is a .pat file with pXX at beginning of each line, strip off
               the pXX; do not mistake for decimal point */
            sscanf(motif_pattern_entered,"%s",test);
            if(!isdigit(test[0]) && (test[0]!='.'))
                motif_pattern_entered+=strlen(test);

            /* if nothing left, report error */
            if (!strlen(motif_pattern_entered))
                XbError("No test pattern is specified.\nPlease enter or copy a pattern into the Test Pattern text box.");
            else
                {
                /* test */
                XbWatchCursor(toplevel);
                compute();
                XbNormalCursor(toplevel);
                }

            XtFree(temp_str);
            break;
        case NEW:
            patdlog_menu_settings(True);
            break;
        case OPEN:
            /* update file listing and manage file selection dialog */
            if (!pattern_test_file_open_dialog)
                create_pattern_test_file_open_dialog();

            /* set the current working directory of the dialog */
            ac = 0;
            XtSetArg(al[ac], XmNdirectory, curr_dir); ac++;
            XtSetValues(pattern_test_file_open_dialog, al, ac);

            XtManageChild(pattern_test_file_open_dialog);
            break;
        case SAVE:
            if (!pattern_test_file_save_dialog)
                create_pattern_test_file_save_dialog();

            /* set the current working directory of the dialog */
            ac = 0;
            XtSetArg(al[ac], XmNdirectory, curr_dir); ac++;
            XtSetValues(pattern_test_file_save_dialog, al, ac);

            if (strlen(pattern_test_filename))
                {
                XmTextSetString(XmSelectionBoxGetChild(pattern_test_file_save_dialog, XmDIALOG_TEXT),
                    pattern_test_filename);
                XmTextShowPosition(XmSelectionBoxGetChild(pattern_test_file_save_dialog, XmDIALOG_TEXT),
                    XmTextGetLastPosition(XmSelectionBoxGetChild(pattern_test_file_save_dialog, XmDIALOG_TEXT)));
                }
            XtManageChild(pattern_test_file_save_dialog);
            break;
        case CLOSE:
            /* ask to save changes if necessary */
            if (pat_changed)
                {
                if (!save_pat_changes_dialog)
                    create_save_pat_changes_dialog();
                XtManageChild(save_pat_changes_dialog);
                return;
                }

            /* reset everything */
            XmTextSetString(pattern_text, "");
            patdlog_menu_settings(False);
            pattern_test_filename[0]='\0';
            pat_changed=False;

            ac=0;
            XtSetArg(al[ac], XmNlabelString,XmStringCreate("File:  None.",char_set)); ac++;
            XtSetValues(pattern_file_label,al,ac);
            break;
        case STEP:
        /* apply test pattern to network */

            new_pattern[0]='\0';
            if (first_step)
                {
                if (step_str)
                    XtFree(step_str);
                step_str=XmTextGetString(pattern_text);

                for(position; step_str[position] == ';'; position++)
                    while(step_str[++position] != '\n');

                sscanf(step_str+position,"%[^\n]",new_pattern);

                /* if new_pattern does not contain at least 1 digit per input node plus 
                   separating spaces, notify user */
                if (strlen(new_pattern) < 2*num_in_nodes-1)
                    {
                    XbError("There are no patterns left.\nResetting to top.");
                    position=0;
                    first_step=True;
                    return;
                    }
                temp_str=motif_pattern_entered=new_pattern;
                first_step=False;
                }
            else
                {
                for(position; step_str[position] == ';' || step_str[position] == '\n'; position++)
                    while(step_str[++position] != '\n');

                sscanf(step_str+position,"%[^\n]",new_pattern);
                /* if new_pattern does not contain at least 1 digit per input node plus 
                   separating spaces, notify user */
                if (strlen(new_pattern) < 2*num_in_nodes-1)
                    {
                    XbError("There are no patterns left.\nResetting to top.");
                    position=0;
                    first_step=True;
                    return;
                    }
                temp_str=motif_pattern_entered=new_pattern;
                }

            if (!temp_str)
                {
                XbError("There are no test patterns left.");
                return;
                }

            /* initialize test */
            test[0]='\0';

            /* strip leading spaces */
            for(i=0;isspace(temp_str[i]) && (i<strlen(temp_str));i++)
                motif_pattern_entered=(temp_str+i+1);

            /* in case this is a .pat file with pXX at beginning of each line, strip off
               the pXX; do not mistake for decimal point */
            sscanf(motif_pattern_entered,"%s",test);
            if(!isdigit(test[0]) && (test[0]!='.'))
                motif_pattern_entered+=strlen(test);

            /* if nothing left, report error */
            if (!strlen(motif_pattern_entered))
                XbError("No test pattern is specified.\nPlease enter or copy a pattern into the Test Pattern text box.");
            else
                {
                /* test */
                XbWatchCursor(toplevel);
                XmTextSetString(pattern_test_text,temp_str);
                position+=strlen(new_pattern)+1;
                compute();
                XbNormalCursor(toplevel);
                }          

            /* tell user that last pattern has just been tested, reset to top */
            if ((position+strlen(temp_str)+num_in_nodes) >= strlen(step_str))
                {
                XbError("This was the last test pattern.\nResetting to the top.");
                position=0;
                first_step=True;
                }
            break;
        case SET_STEP:
            text_position=XmTextGetCursorPosition(pattern_text);
            position=text_position;
            if (step_str)
                XtFree(step_str);
            first_step=False;
            step_str=XmTextGetString(pattern_text);
            while((*(step_str+position)!='\n') && (position > 0))
                position--;

            /* don't increment if position is 0 (obviously no '\n' found--we're at the beginning) */
            if (position)
                position++;
            break;
        case CLEAR_STEP:
            position=0;
            first_step=True;
            break;
        case CLEAR:
            /* clear only the test selection text widget (second test widget) */
            XmTextSetString(pattern_test_text, "");
            break;
        case CANCEL:
            /* ask to save changes if necessary */
            cancel_test=True;
            if (pat_changed)
                {
                if (!save_pat_changes_dialog)
                    create_save_pat_changes_dialog();
                XtManageChild(save_pat_changes_dialog);
                return;
                }
            XtUnmanageChild(pattern_dialog);

            /* reset status flag */
            pat_changed=False;
            cancel_test=False;
            break;
        }
    }


/* set the status flag if the test pattern text (top text widget) contents modified */
void pattern_text_changedCB(Widget w, int client_data, XmAnyCallbackStruct *call_data)
    {
    pat_changed=True;
    }


/* Adds an option to the menu bar. */
Widget make_pat_menu_option(char *option_name, KeySym mnemonic, int client_data, Widget menu)
    {
    int ac;
    Arg al[10];
    Widget b;

    ac = 0;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreateLtoR(option_name,
        char_set)); ac++;
    XtSetArg (al[ac], XmNmnemonic, mnemonic); ac++;
    b=XtCreateManagedWidget(option_name,xmPushButtonGadgetClass,
        menu,al,ac);
    XtAddCallback (b, XmNactivateCallback, pattern_dialogCB, client_data);
    return(b);
    }


/* create menu bar and selections on the menu */
void create_pattern_menus(Widget menu_bar)
   {
    /* 'File' */
    patdlog_file_menu=make_menu("File",'F',menu_bar);
    patdlog_open_option=make_pat_menu_option("Open",'O',OPEN,patdlog_file_menu);
    patdlog_new_option=make_pat_menu_option("New",'N',NEW,patdlog_file_menu);
    patdlog_save_option=make_pat_menu_option("Save",'S',SAVE,patdlog_file_menu);
    patdlog_close_option=make_pat_menu_option("Close",'C',CLOSE,patdlog_file_menu);
    
    make_menu_separator(patdlog_file_menu);
    patdlog_cancel_option = make_pat_menu_option("Exit Window",'E',CANCEL,patdlog_file_menu);
    add_accelerator(patdlog_cancel_option,"meta+e","Meta<Key>e:");
    
    /* initialize sensitivities */
    patdlog_menu_settings(False);
    }


/* set pattern dialog menu sensitivities */
void patdlog_menu_settings(Boolean state)
    {
    XtSetSensitive(patdlog_open_option,!state);
    XtSetSensitive(patdlog_new_option,!state);
    XtSetSensitive(patdlog_save_option,state);
    XtSetSensitive(patdlog_close_option,state);

    XtSetSensitive(pattern_text,state);
    XtSetSensitive(pattern_step_button,state);
    XtSetSensitive(pattern_set_step_button,state);
    XtSetSensitive(pattern_clear_step_button,state);
    }


/* create test pattern dialog */
void create_pattern_dialog()
    {
    int ac;
    Arg al[20];
    Widget pattern_menu, pattern_test_button,
           pattern_clear_button, pattern_top_label, 
           pattern_middle_label, pattern_sep, pattern_rc;

    XbWatchCursor(toplevel);   

    pattern_test_filename[0]='\0';

    /* create form dialog */
    ac=0;
    XtSetArg(al[ac], XmNhorizontalSpacing, 3); ac++;
    XtSetArg(al[ac], XmNverticalSpacing, 5); ac++;
    XtSetArg(al[ac], XmNautoUnmanage, False); ac++;
    XtSetArg(al[ac],XmNdialogTitle,XmStringCreateLtoR(
        "NeuralNet:  Pattern Editor",char_set)); ac++;
    pattern_dialog=XmCreateFormDialog(toplevel,"pattern_dialog",al,ac);

    /* create menus */
    ac=0;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    pattern_menu=XmCreateMenuBar(pattern_dialog,"pattern_menu",al,ac);
    XtManageChild(pattern_menu);

    /* create a label */
    ac=0;
    XtSetArg(al[ac], XmNlabelString, 
        XmStringCreate("Pattern(s)",char_set)); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, pattern_menu); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    pattern_top_label=XmCreateLabelGadget(pattern_dialog,"pattern_top_label",al,ac);
    
    /* create a text widget */
    ac=0;
    XtSetArg(al[ac], XmNrows, 20); ac++;
    XtSetArg(al[ac],XmNeditMode,XmMULTI_LINE_EDIT); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, pattern_top_label); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    pattern_text=XmCreateScrolledText(pattern_dialog,"pattern_text",al,ac);
    XtAddCallback(pattern_text, XmNvalueChangedCallback, pattern_text_changedCB, NULL);

    /* create a label */
    ac=0;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreate("Test Pattern",char_set)); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, pattern_text); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    pattern_middle_label=XmCreateLabelGadget(pattern_dialog,"pattern_middle_label",al,ac);
    
    /* create a text widget */
    ac=0;
    XtSetArg(al[ac], XmNscrollVertical, False); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, pattern_middle_label); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    pattern_test_text=XmCreateScrolledText(pattern_dialog,"pattern_test_text",al,ac);

    /* create a label */
    ac=0;
    XtSetArg(al[ac], XmNlabelString,
        XmStringCreate("File:  None.",char_set)); ac++;
    XtSetArg(al[ac], XmNalignment, XmALIGNMENT_BEGINNING); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, pattern_test_text); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    pattern_file_label=XmCreateLabelGadget(pattern_dialog,"pattern_file_label",al,ac);

    /* create a separator */
    ac=0;
    XtSetArg(al[ac], XmNmargin, 1); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, pattern_file_label); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    pattern_sep=XmCreateSeparatorGadget(pattern_dialog,"pattern_sep",al,ac);

    /* create a rowcolumn widget */
    ac=0;
    XtSetArg(al[ac], XmNentryAlignment, XmALIGNMENT_CENTER); ac++;
    XtSetArg(al[ac], XmNpacking, XmPACK_COLUMN); ac++;
    XtSetArg(al[ac], XmNorientation, XmVERTICAL); ac++;
    XtSetArg(al[ac], XmNnumColumns, 5); ac++;
    XtSetArg(al[ac], XmNadjustLast, False); ac++;
    XtSetArg(al[ac], XmNadjustMargin, True); ac++;
    XtSetArg(al[ac], XmNspacing, 10); ac++;
    XtSetArg(al[ac], XmNmarginHeight, 4); ac++;
    XtSetArg(al[ac], XmNmarginWidth, 10); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, pattern_sep); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    pattern_rc=XmCreateRowColumn(pattern_dialog,"pattern_rc",al,ac);

    /* create a button */
    ac=0;
    XtSetArg(al[ac], XmNheight, 80); ac++;
    XtSetArg(al[ac], XmNlabelString,XmStringCreate("Test",char_set)); ac++;
    XtSetArg(al[ac], XmNalignment, XmALIGNMENT_CENTER); ac++;
    pattern_test_button=XmCreatePushButtonGadget(pattern_rc, "pattern_test_button", al, ac);
    XtManageChild(pattern_test_button);
    XtAddCallback (pattern_test_button, XmNactivateCallback, pattern_dialogCB, TEST);    

    /* create a button */
    ac=0;
    XtSetArg(al[ac], XmNheight, 80); ac++;
    XtSetArg(al[ac], XmNlabelString,XmStringCreate("Step",char_set)); ac++;
    XtSetArg(al[ac], XmNalignment, XmALIGNMENT_CENTER); ac++;
    pattern_step_button=XmCreatePushButtonGadget(pattern_rc, "pattern_step_button", al, ac);
    XtManageChild(pattern_step_button);
    XtAddCallback (pattern_step_button, XmNactivateCallback, pattern_dialogCB, STEP);

    /* create a button */
    ac=0;
    XtSetArg(al[ac], XmNheight, 80); ac++;
    XtSetArg(al[ac], XmNlabelString,XmStringCreate("Set Step",char_set)); ac++;
    XtSetArg(al[ac], XmNalignment, XmALIGNMENT_CENTER); ac++;
    pattern_set_step_button=XmCreatePushButtonGadget(pattern_rc, 
        "pattern_set_step_button", al, ac);
    XtManageChild(pattern_set_step_button);
    XtAddCallback (pattern_set_step_button, XmNactivateCallback, pattern_dialogCB, SET_STEP);

    /* create a button */
    ac=0;
    XtSetArg(al[ac], XmNheight, 80); ac++;
    XtSetArg(al[ac], XmNlabelString,XmStringCreate("Clear Step",char_set)); ac++;
    XtSetArg(al[ac], XmNalignment, XmALIGNMENT_CENTER); ac++;
    pattern_clear_step_button=XmCreatePushButtonGadget(pattern_rc, 
        "pattern_clear_step_button", al, ac);
    XtManageChild(pattern_clear_step_button);
    XtAddCallback (pattern_clear_step_button, XmNactivateCallback, pattern_dialogCB, CLEAR_STEP);

    /* create a button */
    ac=0;
    XtSetArg(al[ac], XmNheight, 80); ac++;
    XtSetArg(al[ac], XmNalignment, XmALIGNMENT_CENTER); ac++;
    XtSetArg(al[ac], XmNlabelString,XmStringCreate("Clear Test",char_set)); ac++;
    pattern_clear_button=XmCreatePushButtonGadget(pattern_rc, "pattern_clear_button", al, ac);
    XtManageChild(pattern_clear_button);
    XtAddCallback (pattern_clear_button, XmNactivateCallback, pattern_dialogCB, CLEAR);
    
    /* manage all of the created children */
    XtManageChild(pattern_top_label);
    XtManageChild(pattern_text);
    XtManageChild(pattern_middle_label);
    XtManageChild(pattern_test_text);
    XtManageChild(pattern_file_label);
    XtManageChild(pattern_sep);
    XtManageChild(pattern_rc);

    /* create the 'File' menu */
    create_pattern_menus(pattern_menu);
 
    /* set the default button to the 'Test' button */
    ac=0;
    XtSetArg(al[ac], XmNdefaultButton, pattern_test_button); ac++;
    XtSetValues(pattern_dialog, al, ac);
    
    XbNormalCursor(toplevel);
    }


/* replacement warning handler to prevent XmScrollbar warning message */
void new_warning_handler(char *msg)
    {
    if (strstr(msg,"scrollbar")!=NULL)
	return;
    printf("%s",msg);
    }


void main(int argc,char *argv[])
    {
    Arg al[20];
    int ac;
    Pixmap pixmap;  /* icon pixmap */

    /* initialize variables */
    nptr=NULL;
    pptr=NULL;
    wptr=NULL;

    /* create the toplevel shell */
    toplevel = XtAppInitialize(&context,"NeuralNet",NULL,0,&argc,argv,fallback,NULL,0);

    /* initialize the help feature ('Help') */
    XnInitializeHelp(context,toplevel,HELP_FILE,True);

    /* set toplevel resources */
    ac=0;
    XtSetArg(al[ac], XmNtitle, "NeuralNet 2.0"); ac++;
    XtSetArg(al[ac], XmNiconName, "NeuralNet 2.0"); ac++;
    XtSetArg(al[ac], XmNminWidth, 100); ac++;
    XtSetArg(al[ac], XmNminHeight, 575); ac++;  
    XtSetArg(al[ac], XmNheight,575); ac++;
    XtSetArg(al[ac], XmNwidth,650); ac++;
    XtSetValues(toplevel,al,ac);

    /* Create the icon pixmap for the toplevel. */
    pixmap=XCreateBitmapFromData(XtDisplay(toplevel), XtScreen(toplevel)->root,
        network_bits, network_width, network_height);

    /* Set the icon pixmap for the toplevel. */
    if( pixmap != None )
        {
        ac=0;
        XtSetArg(al[ac], XmNiconPixmap, pixmap); ac++;
        XtSetValues(toplevel, al, ac);
        }

    /* create a form to hold the other widgets */
    ac=0;
    main_form=XmCreateForm(toplevel,"main_form",al,ac);
    XtManageChild(main_form);

    /* create a menu bar and attach it to the form. */
    ac=0;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    menu_bar=XmCreateMenuBar(main_form,"menu_bar",al,ac);
    XtManageChild(menu_bar);

    /* create a frame for form */
    ac=0;
    XtSetArg(al[ac], XmNshadowType, XmSHADOW_ETCHED_IN); ac++;
    XtSetArg(al[ac], XmNheight, 140); ac++; 
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNtopOffset, 4); ac++;
    XtSetArg(al[ac], XmNleftOffset, 2); ac++;
    XtSetArg(al[ac], XmNrightOffset, 2); ac++;
    XtSetArg(al[ac], XmNbottomOffset, 2); ac++;
    frame1=XmCreateFrame(main_form,"frame1",al,ac);
    XtManageChild(frame1);

    /* create a form to hold the other widgets */
    ac=0;
    XtSetArg(al[ac], XmNhorizontalSpacing,3); ac++;
    XtSetArg(al[ac], XmNverticalSpacing,3); ac++; 
    form=XmCreateForm(frame1,"form",al,ac);
    XtManageChild(form);

    /* create a scroll bar */
    ac=0;
    XtSetArg(al[ac], XmNminimum, 0); ac++;
    XtSetArg(al[ac], XmNmaximum, 575); ac++;
    XtSetArg(al[ac], XmNsliderSize, 575); ac++; 
    XtSetArg(al[ac], XmNincrement, 20); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, menu_bar); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNbottomWidget, frame1); ac++; 
    scroll=XmCreateScrollBar(main_form,"scroll",al,ac);
    XtManageChild(scroll);
    XtAddCallback(scroll,XmNvalueChangedCallback,scrollCB, NULL);
    XtAddCallback(scroll,XmNdragCallback, scrollCB, NULL);

    /* create a frame for drawing area */
    ac=0;
    XtSetArg(al[ac], XmNshadowType, XmSHADOW_IN); ac++;
    XtSetArg(al[ac], XmNtopAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNtopWidget, menu_bar); ac++; 
    XtSetArg(al[ac], XmNleftAttachment, XmATTACH_FORM); ac++;
    XtSetArg(al[ac], XmNrightAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNrightWidget, scroll); ac++;
    XtSetArg(al[ac], XmNbottomAttachment, XmATTACH_WIDGET); ac++;
    XtSetArg(al[ac], XmNbottomWidget, frame1); ac++; 
    XtSetArg(al[ac], XmNtopOffset, 2); ac++;
    XtSetArg(al[ac], XmNleftOffset, 2); ac++;
    XtSetArg(al[ac], XmNrightOffset, 4); ac++;
    frame2=XmCreateFrame(main_form,"frame2",al,ac);
    XtManageChild(frame2);

    /* create drawing area */
    ac=0;
    drawing_area=XmCreateDrawingArea(frame2,"drawing_area",al,ac);
    XtManageChild(drawing_area);
    XtAddCallback(drawing_area, XmNexposeCallback, da_exposeCB, NULL);
    XtAddCallback(drawing_area, XmNresizeCallback, da_resizeCB, NULL);
    XtAddEventHandler(drawing_area, ButtonReleaseMask, FALSE, node_monitor, NULL);

    /* create widgets shown on interface and three of the dialogs */
    create_menus(menu_bar);
    create_widgets(); 
    XbCreateErrorDialog(toplevel, "NeuralNet:  Error Dialog");
    XbCreateInfoDialog(toplevel, "NeuralNet:  Info Dialog");
    create_info_dialog();
    create_question_dialog();

    XtRealizeWidget(toplevel);

    /* initialize the sensitivities and the filenames */
    XtSetSensitive(weights_save_option, False);
    XtSetSensitive(network_save_option, False);
    new_net_filename[0]='\0';
    wts_filename[0]='\0';
    curr_dir = XmStringCreate("",char_set);

    /* replace motif warning handler (so scroll bar warning not shown) */
    XtSetWarningHandler(new_warning_handler);

/*    create_cursors();  */

    /* drawing stuff */
    get_display_info();
    setup_gcs();

/*    XSynchronize(XtDisplay(toplevel), True);  */

    XtAppMainLoop(context);
    }
