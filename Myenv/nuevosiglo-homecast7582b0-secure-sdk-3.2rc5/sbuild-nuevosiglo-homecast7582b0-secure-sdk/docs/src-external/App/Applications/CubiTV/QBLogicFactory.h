/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2015 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef QB_LOGIC_FACTORY_H_
#define QB_LOGIC_FACTORY_H_

/**
 * @file QBLogicFactory.h
 * @brief Application logic factory singleton class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvCoreTypes.h>

/**
 * @defgroup QBLogicFactory Application logic factory singleton class
 * @ingroup CubiTV_logic
 * @{
 *
 * Logic factory singleton is a helper facility for providing custom
 * implementations of parts of application logic to be used instead of
 * default ones.
 **/

/**
 * Logic factory class.
 *
 * @class QBLogicFactory QBLogicFactory.h <Logic/QBLogicFactory.h>
 * @extends SvObject
 **/
typedef struct QBLogicFactory_ *QBLogicFactory;


/**
 * Get handle to a single instance of the logic factory.
 *
 * @relates QBLogicFactory
 *
 * @return  logic factory handle
 **/
extern QBLogicFactory
QBLogicFactoryGetInstance(void);


/**
 * Register implementation of an application logic in the factory.
 *
 * This method registers @a logicClass in the factory. Since then it will
 * be taken into account by QBLogicFactoryFindImplementationOf() when
 * searching for a suitable implementation of application logic class.
 *
 * If a base class of @a logicClass was registered before, it is replaced,
 * so the most specialized implementation is always used. If any subclass
 * of @a logicClass was registered before, this method returns an error.
 *
 * @memberof QBLogicFactory
 *
 * @param[in] self          application logic factory handle
 * @param[in] logicClass    class implementing some part of application logic
 * @param[out] errorOut     error info
 **/
extern void
QBLogicFactoryRegisterClass(QBLogicFactory self,
                            SvType logicClass,
                            SvErrorInfo *errorOut);

/**
 * Find implementation of an application logic.
 *
 * This method searches classes registered by QBLogicFactoryRegisterClass()
 * to find a class derived from @a logicClass. If there is no such class
 * registered, it simply returns @a logicClass as a perfectly good
 * implementation of itself.
 *
 * Factory doesn't have any possibility to decide which implementation
 * of given logic is the correct one if there is more than one, so it will
 * treat such situation as critical error.
 *
 * @memberof QBLogicFactory
 *
 * @param[in] self          application logic factory handle
 * @param[in] parentClass   default implementation of some part of application logic
 * @param[out] errorOut     error info
 * @return                  other registered implementation,
 *                          @a parentClass if no other implementation is registered,
 *                          @c NULL in case of error
 **/
extern SvType
QBLogicFactoryFindImplementationOf(QBLogicFactory self,
                                   SvType parentClass,
                                   SvErrorInfo *errorOut);

/**
 * @}
 **/


#endif
