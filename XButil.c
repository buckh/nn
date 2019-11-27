/*
 *  XButil.c
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

#include "XButil.h"

static Widget error_dialog, info_dialog, ques_dialog;


static void error_CB(Widget w, int client_data, XmAnyCallbackStruct *call_data)
/* callback for OK button in the error message box. */
    {
    XtUnmanageChild(error_dialog);
    }


void XbCreateErrorDialog(Widget parent, char *title)
    {
    Arg al[4];
    int ac;

    ac=0;
    XtSetArg(al[ac], XmNdialogStyle,XmDIALOG_FULL_APPLICATION_MODAL); ac++;
    XtSetArg(al[ac], XmNdialogTitle,XmStringCreateLtoR(title,
        XmSTRING_DEFAULT_CHARSET)); ac++;
    XtSetArg(al[ac], XmNmessageAlignment, XmALIGNMENT_CENTER); ac++;
    error_dialog = XmCreateErrorDialog(parent,"error_dialog", al, ac);

    XtAddCallback(error_dialog, XmNokCallback, error_CB, NULL);

    XtUnmanageChild(XmMessageBoxGetChild(error_dialog, XmDIALOG_CANCEL_BUTTON));
    XtUnmanageChild(XmMessageBoxGetChild(error_dialog, XmDIALOG_HELP_BUTTON));
    }


void XbError (char format[], ...)
    {
    Arg al[2];
    int ac;
    char s[1000];
    XmString err_msg;
    va_list arg_ptr;

    va_start(arg_ptr, format);
    vsprintf(s, format, arg_ptr);
    va_end(arg_ptr);

    err_msg=XmStringCreateLtoR(s, XmSTRING_DEFAULT_CHARSET);

    ac = 0;
    XtSetArg(al[ac], XmNmessageString, err_msg);  ac++;
    XtSetValues(error_dialog,al,ac);

    XmStringFree(err_msg);

    XtManageChild(error_dialog);
    }


static void infoCB(Widget w, int client_data, XmAnyCallbackStruct *call_data)
/* callback for info message box. */
    {
    XtUnmanageChild(info_dialog);
    }


void XbCreateInfoDialog(Widget parent, char *title)
    {
    Arg al[4];
    int ac;

    ac=0;
    XtSetArg(al[ac], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); ac++;
    XtSetArg(al[ac], XmNdialogTitle, XmStringCreateLtoR(title,
        XmSTRING_DEFAULT_CHARSET)); ac++;
    XtSetArg(al[ac], XmNmessageAlignment, XmALIGNMENT_CENTER); ac++;
    info_dialog = XmCreateInformationDialog(parent, "info_dialog", al, ac);

    XtAddCallback(info_dialog, XmNokCallback, infoCB, NULL);

    XtUnmanageChild(XmMessageBoxGetChild(info_dialog, XmDIALOG_HELP_BUTTON));
    XtUnmanageChild(XmMessageBoxGetChild(info_dialog, XmDIALOG_CANCEL_BUTTON));
    }


void XbInfo (char format[], ...)
    {
    Arg al[2];
    int ac;
    XmString info_msg;
    char s[1000];
    va_list arg_ptr;

    va_start(arg_ptr, format);
    vsprintf(s, format, arg_ptr);
    va_end(arg_ptr);

    info_msg=XmStringCreateLtoR(s, XmSTRING_DEFAULT_CHARSET);

    ac = 0;
    XtSetArg(al[ac], XmNmessageString, info_msg);  ac++;
    XtSetValues(info_dialog, al, ac);

    XmStringFree(info_msg);

    XtManageChild(info_dialog);
    }


static void quesCB(Widget w, int client_data, XmAnyCallbackStruct *call_data)
/* callback for question dialog. */
    {
    XtUnmanageChild(ques_dialog);
    }


void XbCreateQuestionDialog(Widget parent, char *title)
    {
    Arg al[4];
    int ac;

    ac=0;
    XtSetArg(al[ac], XmNdialogStyle, XmDIALOG_FULL_APPLICATION_MODAL); ac++;
    XtSetArg(al[ac], XmNdialogTitle, XmStringCreateLtoR(title,
        XmSTRING_DEFAULT_CHARSET)); ac++;
    XtSetArg(al[ac], XmNmessageAlignment, XmALIGNMENT_CENTER); ac++;
    ques_dialog = XmCreateQuestionDialog(parent, "ques_dialog", al, ac);

    XtAddCallback(ques_dialog, XmNokCallback, quesCB, NULL);

    XtUnmanageChild(XmMessageBoxGetChild(ques_dialog, XmDIALOG_HELP_BUTTON));
    }


void XbQuestion (char format[], ...)
    {
    Arg al[2];
    int ac;
    XmString question;
    char s[1000];
    va_list arg_ptr;

    va_start(arg_ptr, format);
    vsprintf(s, format, arg_ptr);
    va_end(arg_ptr);

    question=XmStringCreateLtoR(s, XmSTRING_DEFAULT_CHARSET);

    ac = 0;
    XtSetArg(al[ac], XmNmessageString, question);  ac++;
    XtSetValues(ques_dialog, al, ac);

    XmStringFree(question);

    XtManageChild(ques_dialog);
    }


Cursor XbCreateCursor(Widget w, char *cursor_bits, char *mask_bits, 
    int width, int height, int x_hot, int y_hot)
    {
    XColor foreground, background, trash;
    Pixmap source, mask;
    Cursor cursor;

    XLookupColor(XtDisplay(w), DefaultColormapOfScreen(XtScreen(w)), "Black", 
        &trash, &foreground);
    XLookupColor(XtDisplay(w), DefaultColormapOfScreen(XtScreen(w)), "White", 
        &trash, &background);

    source=XCreateBitmapFromData(XtDisplay(w), XtWindow(w), cursor_bits, width,
        height);
    mask=XCreateBitmapFromData(XtDisplay(w), XtWindow(w), mask_bits, width, 
        height);
    cursor=XCreatePixmapCursor (XtDisplay(w), source, mask, &foreground, 
        &background, x_hot, y_hot);

    XFreePixmap(XtDisplay(w), source);
    XFreePixmap(XtDisplay(w), mask);

    return (cursor);
    }


void XbChangeCursor(Widget w, int shape)
    {
    Cursor c1;

    c1 = XCreateFontCursor(XtDisplay(w), shape);
    XDefineCursor(XtDisplay(w), XtWindow(w), c1);
    XFlush(XtDisplay(w));
    }


/* change the cursor to a wrist watch shape. */
void XbWatchCursor(Widget w)
    {
    Cursor c1;

    c1 = XCreateFontCursor(XtDisplay(w), XC_watch);
    XDefineCursor(XtDisplay(w), XtWindow(w),c1);
    XFlush(XtDisplay(w));
    }


/* return the cursor to its normal shape. */
void XbNormalCursor(Widget w)
    {
    XUndefineCursor(XtDisplay(w), XtWindow(w));
    XFlush(XtDisplay(w));
    }


void XbDrawCircle(Widget w, Window drawable, GC gc, int center_x, int center_y,
       int radius)
    {
    XDrawArc(XtDisplay(w), drawable, gc, center_x-radius, center_y-radius, radius*2,
        radius*2, 0, 360*64);
    }


Widget XbCreateDialog(Widget parent, char *class_name, char *title, 
       Widget *form, Arg *al, int ac)
    {
    Widget temp_form, temp_dialog;

    XbWatchCursor(parent);
    
    XtSetArg(al[ac], XmNdialogTitle, XmStringCreateLtoR(title, XmSTRING_DEFAULT_CHARSET)); ac++;
    XtSetArg(al[ac], XmNautoUnmanage, False); ac++;
    temp_dialog = XmCreatePromptDialog(parent, class_name, al, ac);

    XtUnmanageChild(XmSelectionBoxGetChild(temp_dialog, XmDIALOG_HELP_BUTTON));
    XtUnmanageChild(XmSelectionBoxGetChild(temp_dialog, XmDIALOG_TEXT));
    XtUnmanageChild(XmSelectionBoxGetChild(temp_dialog, XmDIALOG_PROMPT_LABEL));

    ac=0;
    *form=XmCreateForm(temp_dialog, class_name, al, ac);
    XtManageChild(*form);

    XbNormalCursor(parent);

    return(temp_dialog);
    }
