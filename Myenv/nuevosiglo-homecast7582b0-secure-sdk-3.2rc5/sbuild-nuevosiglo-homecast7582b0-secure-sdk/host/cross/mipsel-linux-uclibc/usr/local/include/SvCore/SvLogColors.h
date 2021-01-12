/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2008 Sentivision K.K. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Sentivision K.K. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Sentivision K.K.
**
** Any User wishing to make use of this Software must contact Sentivision K.K.
** to arrange an appropriate license. Use of the Software includes, but is not
** limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef SV_LOG_COLORS_H_
#define SV_LOG_COLORS_H_


#define SV_LOG_COLORED

#ifdef SV_LOG_COLORED
#  define COLMARK_() /* nothing */
#  define COLMARK_red()     "\033[31;31m**** \033[m"
#  define COLMARK_green()   "\033[31;32m**** \033[m"
#  define COLMARK_yellow()  "\033[31;33m**** \033[m"
#  define COLMARK_blue()    "\033[31;34m**** \033[m"
#  define COLMARK_magenta() "\033[31;35m**** \033[m"
#  define COLMARK_cyan()    "\033[31;36m**** \033[m"

#  define COLMARK(mark_)  COLMARK_##mark_()

#  define COLBEG_() "\033[31;%dm"
#  define COLBEG(mark_)  COLMARK(mark_) COLBEG_()

#  define COLEND_AV(src_)  "\033[m", (((src_)->stype==VIDEO_STREAM)?32:34)

#  define COLOR_red()     31
#  define COLOR_green()   32
#  define COLOR_yellow()  33
#  define COLOR_blue()    34
#  define COLOR_magenta() 35
#  define COLOR_cyan()    36

#  define COLOR_ON(stream, color)     fprintf((stream), COLBEG_(), COLOR_##color())
#  define COLOR_OFF(stream)           fprintf((stream), "\033[m")
#  define COLOR_MARK(stream, color)   fprintf((stream), "\033[31;%dm######\t\033[m", COLOR_##color())

#  define COLEND() "\033[m"
#  define COLEND_COL(mark_)  COLEND(), COLOR_##mark_()
#else
#  define COLOR_ON(stream, color)
#  define COLOR_OFF(stream)
#  define COLOR_MARK(stream, color)
#  define COLMARK(mark_)
#  define COLBEG(mark_)
#  define COLEND_AV(src)
#  define COLEND_COL(n_)
#  define COLEND()
#endif


#endif
