/*
 *  XButil.h
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
#include <stdarg.h>

#include <X11/Intrinsic.h>
#include <X11/cursorfont.h>
#include <X11/StringDefs.h>

#include <Xm/Xm.h>
#include <Xm/BulletinB.h>
#include <Xm/SelectioB.h>
#include <Xm/MessageB.h>
#include <Xm/Form.h>

void XbCreateErrorDialog(Widget parent, char *title);

void XbError(char format[], ...);

void XbCreateInfoDialog(Widget parent, char *title);

void XbInfo(char format[], ...);

void XbCreateQuestionDialog(Widget parent, char *title);

void XbQuestion(char format[], ...);

Cursor XbCreateCursor(Widget w, char *cursor_bits, char *mask_bits, int width,
   int height, int x_hot, int y_hot);

void XbChangeCursor(Widget w, int shape);

void XbWatchCursor(Widget w);

void XbNormalCursor(Widget w);

void XbDrawCircle(Widget w, Window drawable, GC gc, int center_x, int center_y, int radius);

Widget XbCreateDialog(Widget parent, char *class_name, char *title, Widget *form, Arg *al, int ac);
