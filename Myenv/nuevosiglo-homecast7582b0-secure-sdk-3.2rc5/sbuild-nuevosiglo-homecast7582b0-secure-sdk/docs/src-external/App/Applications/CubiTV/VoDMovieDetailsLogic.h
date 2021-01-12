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

#ifndef VOD_MOVIE_DETAILS_LOGIC_H
#define VOD_MOVIE_DETAILS_LOGIC_H

/**
 * @file VoDMovieDetailsLogic.h VoD movie details logic
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <main_decl.h>
#include <CUIT/Core/types.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvHashTable.h>
#include <QBApplicationController.h>
#include <Windows/QBVoDMovieDetails.h>

/**
 * VoD Movie Details window logic
 */
typedef struct QBVoDMovieDetailsLogic_t *QBVoDMovieDetailsLogic;

/**
 * Initialize VoD Movie Details basic widgets.
 * This method tells VoD Movie Details window which
 * basic widgets we want to create.
 *
 * @param[in] basicElements     VoD Movie Details window basic elements handle
 **/
void
QBVoDMovieDetailsLogicBasicElementsInit(QBVoDMovieDetailsBasicElements basicElements);

/**
 * Create VoD Movie Details Logic instance.
 * VoD Movie Details Logic enables customizing VoD Movie Details window layout.
 * VoD Movie Details keeps a set of basic widgets such as cover, carousel, object description.
 * VoD Movie Details Logic can modify this set - add new widgets, modify/delete basic widgets,
 * customize handling different window actions.
 *
 * @param[in] appGlobals    application globals handle
 * @param[in] details       VoD Movie Details handle
 * @param[in] object        VoD Movie Details object
 * @param[in] actions       hashtable of content actions available for object
 * @param[in] action        content action
 * @return                  created object, @c NULL in case of error
 **/
QBVoDMovieDetailsLogic
QBVoDMovieDetailsLogicNew(AppGlobals appGlobals,
                          QBVoDMovieDetails details,
                          SvObject object,
                          SvHashTable actions,
                          QBContentAction action);

/**
 * Create widgets for VoD Movie Details window.
 * This method creates (or skips) basic widgets and (if needed) adds custom widgets to
 * VoD Movie Details window.
 *
 * @param[in] self              VoD Movie Details Logic handle
 * @param[in] app               application handle
 * @param[in] window            VoD Movie Details window handle
 * @param[in] object            VoD Movie Details object
 * @param[in] basicElements     VoD Movie Details window basic elements handle
 **/
void
QBVoDMovieDetailsLogicCreateWidgets(QBVoDMovieDetailsLogic self,
                                    SvApplication app,
                                    SvWidget window,
                                    SvObject object,
                                    QBVoDMovieDetailsBasicElements basicElements);

/**
 * Get adapted content object.
 * This method allows adaptation of selected object before processing it
 * in VoD Movie Details.
 *
 * @param[in] self      VoD Movie Details Logic handle
 * @param[in] object    VoD Movie Details object
 * @return              adapted object, @c NULL by default
 **/
SvDBRawObject
QBVoDMovieDetailsLogicGetAdaptedObject(QBVoDMovieDetailsLogic self,
                                       SvDBRawObject object);

/**
 * Fill a VoD Movie Details carousel slot for selected object.
 *
 * @param[in] self      VoD Movie Details Logic handle
 * @param[in] object    object for given slot
 * @param[in] slot      carousel slot handle
 * @param[in] icon      slot icon handle
 * @return              @c true if handled, @c false otherwise
 **/
bool
QBVoDMovieDetailsLogicSimilarCarouselSlotSetObject(QBVoDMovieDetailsLogic self,
                                                   SvObject object,
                                                   SvWidget slot,
                                                   SvWidget icon);

/**
 * Create content side menu.
 *
 * @param[in] self      VoD Movie Details Logic handle
 * @return              content menu handle, @c NULL in case of error/not handled
 **/
SvObject
QBVoDMovieDetailsLogicCreateContentMenu(QBVoDMovieDetailsLogic self);

/**
 * Show content side menu.
 *
 * @param[in] self      VoD Movie Details Logic handle
 * @return              @c true if handled, @c false otherwise
 **/
bool
QBVoDMovieDetailsLogicShowContentMenu(QBVoDMovieDetailsLogic self);

/**
 * Hide content side menu.
 *
 * @param[in] self      VoD Movie Details Logic handle
 * @return              @c true if handled, @c false otherwise
 **/
bool
QBVoDMovieDetailsLogicHideContentMenu(QBVoDMovieDetailsLogic self);

/**
 * Show object details.
 *
 * @param[in] self      VoD Movie Details Logic handle
 * @return              @c true if handled, @c false otherwise
 **/
bool
QBVoDMovieDetailsLogicShowObjectDetails(QBVoDMovieDetailsLogic self);

/**
 * Hide object details.
 *
 * @param[in] self      VoD Movie Details Logic handle
 * @return              @c true if handled, @c false otherwise
 **/
bool
QBVoDMovieDetailsLogicHideObjectDetails(QBVoDMovieDetailsLogic self);

/**
 * Handle creating VoD Movie Details carousel.
 *
 * @param[in] self              VoD Movie Details Logic handle
 * @param[in] basicElements     VoD Movie Details window basic elements handle
 **/
void
QBVoDMovieDetailsLogicSetUpCarousel(QBVoDMovieDetailsLogic self,
                                    QBVoDMovieDetailsBasicElements basicElements);

/**
 * Set currently active object.
 *
 * @param[in] self       VoD Movie Details Logic handle
 * @param[in] object     active object handle
 **/
void
QBVoDMovieDetailsLogicSetActiveElement(QBVoDMovieDetailsLogic self,
                                       SvDBRawObject object);

/**
 * Input Event Handler
 *
 * @param[in] self       VoD Movie Details Logic handle
 * @param[in] ev         event data
 * @param[in] data_      context of a calling window
 * @return               @c 0 if the event have not been consumed, otherwise any other value
 **/
bool
QBVoDMovieDetailsLogicHandleInputEvent(QBVoDMovieDetailsLogic self,
                                       SvInputEvent ev,
                                       SvObject data_);

/**
 * Cover Input Event Handler
 *
 * @param[in] self       VoD Movie Details Logic handle
 * @param[in] ev         event data
 * @param[in] data_      context of a calling window
 * @return               @c 0 if the event have not been consumed, otherwise any other value
 **/
bool
QBVoDMovieDetailsLogicHandleCoverInputEvent(QBVoDMovieDetailsLogic self,
                                            SvInputEvent ev,
                                            SvObject data_);

/**
 * Decides whether to play trailer preview instantly or not.
 *
 * @param[in] self      VoD Movie Details Logic handle
 * @return              @c true to play instantly, @c false otherwise
 **/
bool
QBVoDMovieDetailsLogicIsInstantTrailerPreview(QBVoDMovieDetailsLogic self);

#endif
