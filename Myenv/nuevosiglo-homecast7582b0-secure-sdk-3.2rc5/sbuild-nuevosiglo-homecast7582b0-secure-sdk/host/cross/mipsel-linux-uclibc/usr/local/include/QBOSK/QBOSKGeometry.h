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

#ifndef QB_OSK_GEOMETRY_H_
#define QB_OSK_GEOMETRY_H_

/**
 * @file QBOSKGeometry.h
 * @brief On Screen Keyboard geometry class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBOSKGeometry On Screen Keyboard geometry class
 * @ingroup QBOSKCore
 * @{
 **/

#include <SvCore/SvErrorInfo.h>
#include <QBOSK/QBOSKTypes.h>
#include <CAGE/Core/Sv2DRect.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvArray.h>


/**
 * Get runtime type identification object representing
 * OSK geometry class.
 *
 * @return QBOSKGeometry type identification object
 **/
extern SvType
QBOSKGeometry_getType(void);

/**
 * Create new OSK geometry object.
 *
 * @param[in] keyWidth          OSK key width in pixels
 * @param[in] keyHeight         OSK key height in pixels
 * @param[in] keyHorizMargin    OSK key horizontal margin in pixels
 * @param[in] keyVertMargin     OSK key vertical margin in pixels
 * @param[in] padding           OSK padding
 * @param[out] errorOut error info
 * @return              created map, @c NULL in case of error
 **/
extern QBOSKGeometry
QBOSKGeometryCreate(unsigned int keyWidth, unsigned int keyHeight,
                    unsigned int keyHorizMargin, unsigned int keyVertMargin,
                    unsigned int padding,
                    SvErrorInfo *errorOut);

/**
 * Calculate OSK layout dimensions.
 *
 * @param[in] self      OSK geometry handle
 * @param[in] map       OSK map handle
 * @param[in] layoutID  identifier of one of the layouts of @a map
 * @param[out] dimensions layout dimensions
 * @param[out] errorOut error info
 **/
extern void
QBOSKGeometryCalculateLayoutDimensions(const QBOSKGeometry self,
                                       QBOSKMap map, SvString layoutID,
                                       Sv2DRect *dimensions,
                                       SvErrorInfo *errorOut);

/**
 * Calculate OSK layout dimensions for keys array
 *
 * @param[in] self          OSK geometry handle
 * @param[in] keys          keys array for given calculation
 * @param[in] variant       variant of keys used
 * @param[out] dimensions   OSK stripe dimensions
 * @param[out] errorOut     error info
 **/
extern void
QBOSKGeometryCalculateLayoutDimensionsForKeysVariant(const QBOSKGeometry self,
                                                     SvArray keys, int variant,
                                                     Sv2DRect *dimensions,
                                                     SvErrorInfo *errorOut);

/**
 * Calculate dimensions of the part of OSK layout that contains
 * all visible keys for given number of rows at given row position
 *
 * @param[in] self          OSK geometry handle
 * @param[in] rowCount      Number of rows to calculate
 * @param[in] minX          Lowest x coordinate of drawn stripe
 * @param[in] minY          Lowest y coordinate of drawn stripe
 * @param[out] dimensions   dimensions of the visible part of the layout
 * @param[out] errorOut     error info
 **/
extern void
QBOSKGeometryCalculateContentDimensionsForRowCountAndCoords(const QBOSKGeometry self,
                                                            unsigned int rowCount,
                                                            unsigned int minX,
                                                            unsigned int minY,
                                                            Sv2DRect *dimensions,
                                                            SvErrorInfo *errorOut);

/**
 * Calculate OSK key size and position.
 *
 * @param[in] self      OSK geometry handle
 * @param[in] key       OSK key handle
 * @param[in] variant   key variant (OSK layout number)
 * @param[in] yOffset   y offset of drawn keys - results from actual row number
 * @param[out] geometry OSK key size and position
 * @param[out] errorOut error info
 **/
extern void
QBOSKGeometryCalculateKeyGeometry(const QBOSKGeometry self,
                                  const QBOSKKey key, unsigned int variant, int yOffset,
                                  Sv2DRect *geometry,
                                  SvErrorInfo *errorOut);

/**
 * Calculate OSK internal parameters - row and cols number - for
 * given OSKMap and Layout.
 *
 * @param[in] self      OSK geometry handle
 * @param[in] map       OSK map handle
 * @param[in] layoutID  identifier of one of the layouts of @a map
 * @param[out] errorOut error info
 */
void
QBOSKGeometryCalculateRowsColsCount(const QBOSKGeometry self,
                                    QBOSKMap map, SvString layoutID,
                                    SvErrorInfo *errorOut);

/**
 * Get visible rows count
 *
 * @param[in] self      OSK geometry handle
 * @return visible rows count
 **/
extern unsigned int
QBOSKGeometryGetVisibleRowsCount(const QBOSKGeometry self);

/**
 * Get visible columns count
 *
 * @param[in] self      OSK geometry handle
 * @return visible columns count
 **/
extern unsigned int
QBOSKGeometryGetVisibleColumnsCount(const QBOSKGeometry self);


/**
 * @}
 **/

#endif
