/*****************************************************************************
** TiVo Poland Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2019 TiVo Poland Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by TiVo Poland Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from TiVo Poland Sp. z o.o.
**
** Any User wishing to make use of this Software must contact
** TiVo Poland Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QB_VARIANT_CALCULATOR_H_
#define QB_VARIANT_CALCULATOR_H_

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvInterface.h>

/**
 * @file QBVariantCalculator.h
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBVariantCalculator QBVariantCalculator class
 * @ingroup CubiTV
 * @{
 **/

/**
 * QBVariantCalculator class.
 *
 * @class QBVariantCalculator
 * @extends SvObject
 **/
struct QBVariantCalculator_ {
    struct SvObject_ super_; ///< super type
};
typedef struct QBVariantCalculator_ *QBVariantCalculator;

/**
 * QBVariantCalculator virtual method table
 **/
struct QBVariantCalculatorVTable_ {
    struct SvObjectVTable_ super_; ///< super type

    /**
     * Get first variant
     *
     * @param[in] self instance of QBVariantCalculator
     *
     * @return initial variant
     **/
    int (*calculateFirst)(QBVariantCalculator self);

    /**
     * Get next variant
     *
     * @param[in] self instance of QBVariantCalculator
     *
     * @return next variant
     **/
    int (*calculateNext)(QBVariantCalculator self);
};
typedef const struct QBVariantCalculatorVTable_ *QBVariantCalculatorVTable;


/**
 * Get runtime type identification object representing the QBVariantCalculator class.
 *
 * @relates QBVariantCalculator
 *
 * @return QBVariantCalculator type identification object
 **/
SvType
QBVariantCalculator_getType(void);

/// @copydoc QBVariantCalculatorVTable_::calculateFirst()
int
QBVariantCalculatorCalculateFirst(QBVariantCalculator self);

/// @copydoc QBVariantCalculatorVTable_::calculateNext()
int
QBVariantCalculatorCalculateNext(QBVariantCalculator self);
///@}

/**
 * @}
 **/

#endif // QB_VARIANT_CALCULATOR_H_
