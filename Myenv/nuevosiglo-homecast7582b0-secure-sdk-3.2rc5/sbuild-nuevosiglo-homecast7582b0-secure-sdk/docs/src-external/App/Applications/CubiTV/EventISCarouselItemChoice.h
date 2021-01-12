/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2014 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef EVENTISCAROUSELITEMCHOICE_H_
#define EVENTISCAROUSELITEMCHOICE_H_

/**
 * @file EventISCarouselItemChoice.h
 * @brief EventIS Carousel menu item choice controller class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <main_decl.h>
#include <SvFoundation/SvString.h>
#include <Menus/EventIS/EventISCarouselItemController.h>

/**
 * Configuration of Item Choice controller.
 **/
typedef struct QBItemChoiceConfig_s {
    /// @c string that represents action which should be taken after pushing ENTER button
    SvString onEnterButton;
    /// @c string that represents action which should be taken after pushing OPTION button
    SvString onOptionButton;
} QBItemChoiceConfig;

/**
 * @defgroup QBEventISCarouselMenuChoice EventIS carousel menu item choice controller class.
 * @{
 **/

/**
 * EventIS carousel menu item choice controller class.
 * @class QBEventISCarouselMenuChoice
 * @extends SvObject
 * @implements XMBMenuBarEventHandler
 * @implements QBContentMgrTransactionListener
 * @implements QBMenuChoice
 * @implements QBMenuContextChoice
 **/
typedef struct QBEventISCarouselMenuChoice_t *QBEventISCarouselMenuChoice;

/**
 * Create QBEventISCarouselMenuChoice object.
 * @public
 * @memberof QBEventISCarouselMenuChoice
 *
 * @param[in] appGlobals            appGlobals handle
 * @param[in] vodProvider           vod provider handle
 * @param[in] serviceName           service name handle
 * @param[in] optionsNamesFilename  file name of side menu template
 * @param[in] config                object configuration
 * @return                          @c QBEventISCarouselMenuChoice handle if creation ends with success,
 *                                  @c NULL otherwise
 **/
QBEventISCarouselMenuChoice
QBEventISCarouselMenuChoiceNew(AppGlobals appGlobals, SvGenericObject vodProvider, SvString serviceName,
                               SvString optionsNamesFilename, const QBItemChoiceConfig *config);

/**
 * @}
 **/

#endif /* EVENTISCAROUSELITEMCHOICE_H_ */
