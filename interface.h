/*
 *  NeuralNet - interface.h
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

/* function prototypes */
void create_new_net_dialog();
void create_net_save_dialog();
void create_pattern_entry_dialog();
void create_save_wts_prompt_dialog();
void create_net_file_sel_dialog();
void create_pat_file_sel_dialog();
void create_wts_file_sel_dialog();
void create_pattern_dialog();
void create_wts_save_dialog();
void create_pattern_test_file_save_dialog();
void menuCB (Widget w, int client_data, XmAnyCallbackStruct *call_data);
void create_save_wts_changes_dialog();
void pattern_dialogCB(Widget w, int client_data, int call_data);
void save_pat_changesCB(Widget w, int client_data, XmAnyCallbackStruct *call_data);
void patdlog_menu_settings(Boolean state);


/* variables used from draw.c */
extern void create_question_dialog();
extern Widget drawing_area;
extern Pixmap net_pix;
extern Boolean draw, reset, net_complete;
extern int da_height, slider_value;
extern Node *node_info;
extern NetList *net_list;
extern char new_net_filename[200];

