/*
 * NXHelp - NXHelp.h
 *	by Lance Lovette
 *	Code and display copyright 1992, by Lance Lovette.
 *	All Rights Reserved
 */

#ifndef _NXHELP_H_
#define _NXHELP_H_

#include <Xm/Xm.h>

extern void	XnHelpCB		(Widget widget,
				  	 char *clientData,
					 XmAnyCallbackStruct *callData); 

extern int	XnInitializeHelp	(XtAppContext context,
					 Widget widget,
					 char *helpFilename,
					 Boolean changeIcon);
#endif

/* 
 * XnHelpCB is the help callback.
 *	widget		-the widget that activated the callback.
 *	clientData	-the keyword.
 *	callData	-the widgets client data.
 *
 * XnInitizlizeHelp initializes the XnHelpCB.
 * This function must be called before an XnHelpCB is activated.
 * It returns 1 on success, 0 if there is an error or invalid parameter.
 *	context		-the calling application's application context.
 *	widget		-the calling application's parent widget.
 *	helpFilename	-the filename containing the help text.
 *	changeIcon	-described below.
 *
 * Description of the Help File.
 *
 * The first line of a help file must contain the following:
 *	[-NXHelpFile <program name> <helpfile version>-]
 * where the program name and helpfile version are substitued in
 * by you.  The program name is used as the help window title and
 * as the icon title.  Before you use the XnHelpCB you must call
 * XnInitializeHelp with the required parameters.  Which include
 * the help filename (and path) a widget (the toplevel shell of your
 * program), an application context from XtAppInitialize,
 * and a Boolean variable.  The variable is used to
 * let the help callback know if you want it to change the icon to
 * a watch (on your toplevel) while it is creating its window.
 * To use XnHelpCB just declare it as an XmNhelpCallback or whatever
 * and send it a string containing the keyword as client data.
 * eg:
 *	XtAddCallback(widget, XmNhelpCallback, XnHelpCB, "Index");
 *	XtAddCallback(widget, XmNhelpCallback, XnHelpCB, "Overview");
 *
 * A keyword is denoted by brackets '<' and '>'.
 * Sending XnHelpCB the keyword 'Index' creates an index of the help
 * file and displays it.  An index contains all the keywords in a selection
 * box.  The user selects a keyword and the text after the keyword
 * until the next keyword or END is loaded.  There are some special
 * keywords the callback recognizes:
 * 
 * If the keyword is a '!' a space is inserted into the index.
 * If the first character of a keyword is '.' then that
 *      keyword is not used as an index selection.
 * If the keyword is followed by an '=' the keyword following
 *	the '=' is used as the search keyword, and the text is
 *	searched again.
 * If the last character in the keyword is a '-' then this
 *	keyword is not selectable in the index list.
 * END as a keyword signals the end of the help text.
 *
 * Examples:
 *	<Overview> 			 Keyword 
 *	<File Menu->			 Not selectable
 *	<!>				 Blank selection
 *	<.  Startup Dialog>		 Invisible selection
 *	<Labels>=Making Labels		 Deferred selection
 *	<END>
 *
 * If a keyword is sent to XnHelpCB as client data then the text
 * for that keyword is loaded automatically.  The special index
 * character '.' is not part of the keyword, just a prefix.
 */

