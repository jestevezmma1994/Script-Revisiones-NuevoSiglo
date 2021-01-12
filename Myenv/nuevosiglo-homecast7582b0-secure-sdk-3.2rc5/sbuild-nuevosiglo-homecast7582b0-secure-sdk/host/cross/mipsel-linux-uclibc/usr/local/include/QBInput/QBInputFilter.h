/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2011 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp. z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QB_INPUT_FILTER_H_
#define QB_INPUT_FILTER_H_

/**
 * @file QBInputFilter.h
 * @brief Input filter interface
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <QBInput/QBInputQueue.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup QBInputFilter Input filter interface
 * @ingroup QBInputCore
 * @{
 **/

/**
 * Get runtime type identification object representing
 * input filter interface.
 *
 * @return input filter interface
 **/
extern SvInterface QBInputFilter_getInterface(void);


/**
 * QBInputFilter interface.
 **/
typedef struct QBInputFilter_ {
    /**
     * Return input filter name.
     *
     * @param[in] self_  handle to an object implementing QBInputFilter
     * @return           driver name, @c NULL on error
     **/
    SvString (*getName)(SvObject self_);

    /**
     * Process input events.
     *
     * @param[in] self_  handle to an object implementing QBInputFilter
     * @param[in] outQueue destination queue
     * @param[in] inQueue source queue
     * @return           delay in microseconds after which this method
     *                   should be called again, @c 0 to call only when there
     *                   are new events to process
     **/
    unsigned int (*processEvents)(SvObject self_,
                                  QBInputQueue outQueue,
                                  QBInputQueue inQueue);
} *QBInputFilter;


/**
 * @}
 **/

#ifdef __cplusplus
}
#endif

#endif
