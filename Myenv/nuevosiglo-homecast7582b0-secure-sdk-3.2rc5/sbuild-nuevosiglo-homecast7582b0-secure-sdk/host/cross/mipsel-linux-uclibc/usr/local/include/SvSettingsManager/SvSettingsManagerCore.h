/*****************************************************************************
** Sentivision K.K. Software License Version 1.1
**
** Copyright (C) 2002-2004 Sentivision K.K. All rights reserved.
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

#ifndef SV_SETTINGS_MANAGER_CORE_H_
#define SV_SETTINGS_MANAGER_CORE_H_

/**
 * @file SvSettingsManagerCore.h Settings Manager class
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <unistd.h>
#include <SvCore/SvErrorInfo.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvValue.h>
#include <SvSettingsManager/SvSettingsManagerTypes.h>
#include <SvSettingsManager/SvSettingsManagerErrorDomain.h>
#include <QBResourceManager/QBResourceManager.h>
#include <QBResourceManager/SvRBLocator.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvSettingsManagerCore Settings Manager class
 * @ingroup SvSettingsManager
 * @{
 *
 * Settings Manager class loads GUI settings from files, caches
 * loaded information and provides it to GUI components.
 **/

/**
 * Create Settings Manager instance.
 *
 * @memberof SvSettingsManager
 *
 * @param[out] errorOut    error info
 * @return                 newly created Settings Manager instance,
 *                         @c NULL in case of error
 **/
extern SvSettingsManager
SvSettingsManagerCreate(SvErrorInfo *errorOut);

/**
 * Get instance of the @ref QBResourceManager used by the settings manager.
 *
 * @memberof SvSettingsManager
 * @since 1.8.1
 *
 * @param[in] self         Settings Manager handle
 * @return                 resource manager handle or @c NULL in case of error
 **/
extern QBResourceManager
SvSettingsManagerGetResourceManager(SvSettingsManager self);

/**
 * Get instance of the @ref SvRBLocator used by the settings manager.
 *
 * @memberof SvSettingsManager
 * @since 1.8.1
 *
 * @param[in] self         Settings Manager handle
 * @return                 resource locator handle or @c NULL in case of error
 **/
extern SvRBLocator
SvSettingsManagerGetResourceLocator(SvSettingsManager self);

/**
 * Flush settings.
 *
 * This method removes all settings stored by the Settings Manager.
 *
 * @memberof SvSettingsManager
 *
 * @param[in] self         Settings Manager handle
 * @param[out] errorOut    error info
 **/
extern void
SvSettingsManagerFlushSettings(SvSettingsManager self,
                               SvErrorInfo *errorOut);

/**
 * Get current depth of settings stack.
 *
 * @memberof SvSettingsManager
 *
 * @param[in] self         Settings Manager handle
 * @param[out] errorOut    error info
 * @return                 depth of settings stack, @c -1 in case of error
 **/
extern ssize_t
SvSettingsManagerGetStackDepth(SvSettingsManager self,
                               SvErrorInfo *errorOut);

/**
 * Check if component configuration is already loaded.
 *
 * @memberof SvSettingsManager
 *
 * @param[in] self         Settings Manager handle
 * @param[in] fileName     symbolic component file name
 * @param[in] isUpToDate   @c true to also check if settings file
 *                         have changed since it's been loaded
 * @return                 @c true if component is loaded
 **/
extern bool
SvSettingsManagerIsComponentLoaded(SvSettingsManager self,
                                   bool isUpToDate,
                                   SvString fileName);

/**
 * Check if file containing component configuration is present in the file system.
 *
 * @memberof SvSettingsManager
 * @since 1.8.1
 *
 * @param[in] self         Settings Manager handle
 * @param[in] fileName     symbolic component file name
 * @return                 @c true if component file is available
 **/
extern bool
SvSettingsManagerIsComponentFilePresent(SvSettingsManager self,
                                        SvString fileName);

/**
 * Create font descriptor object.
 *
 * @memberof SvSettingsManager
 *
 * @param[in] self            Settings Manager handle
 * @param[in] fileName        symbolic component file name
 * @param[in] filePath        component settings file path
 * @param[out] errorOut       error info
 **/
extern void
SvSettingsManagerLoadComponent(SvSettingsManager self,
                               SvString fileName,
                               SvString filePath,
                               SvErrorInfo *errorOut);

/**
 * Change the search context to given component.
 *
 * This method changes the search context so that consecutive calls to
 * SvSettingsManagerGetValue() will search for widgets in the configuration
 * of another component.
 *
 * @memberof SvSettingsManager
 *
 * @param[in] self            Settings Manager handle
 * @param[in] fileName        symbolic component file name
 * @param[out] errorOut       error info
 **/
extern void
SvSettingsManagerPushComponent(SvSettingsManager self,
                               SvString fileName,
                               SvErrorInfo *errorOut);

/**
 * Change the search context to a temporary component containing
 * empty settings for widget with given @a name.
 *
 * @memberof SvSettingsManager
 *
 * @param[in] self            Settings Manager handle
 * @param[in] name            widget name
 * @param[out] errorOut       error info
 **/
extern void
SvSettingsManagerPushTemporaryComponent(SvSettingsManager self,
                                        const char *name,
                                        SvErrorInfo *errorOut);

/**
 * Change the search context to the previous component settings.
 *
 * This method changes the search context so that consecutive calls to
 * SvSettingsManagerGetValue() will search for widgets starting with those
 * assigned to the previous component.
 *
 * @memberof SvSettingsManager
 *
 * @param[in] self            Settings Manager handle
 * @param[out] errorOut       error info
 **/
extern void
SvSettingsManagerPopComponent(SvSettingsManager self,
                              SvErrorInfo *errorOut);

/**
 * Search for all bitmaps in current context and try to load them.
 *
 * @note This method is blocking.
 *
 * @memberof SvSettingsManager
 *
 * @param[in] self            Settings Manager handle
 * @param[out] errorOut       error info
 **/
extern void
SvSettingsManagerPreloadComponent(SvSettingsManager self,
                                  SvErrorInfo *errorOut);

/**
 * Associate current search context with simple ID value for easy
 * restoration in the future.
 *
 * @memberof SvSettingsManager
 *
 * @param[in] self            Settings Manager handle
 * @return                    identifier of the current search context,
 *                            @c 0 in case of error
 **/
extern SvSettingsManagerContextID
SvSettingsManagerSaveContext(SvSettingsManager self);

/**
 * Restore search context to the previously saved state.
 *
 * This method changes the search context to the previously saved
 * state. It is similar to SvSettingsManagerPushComponent(): it's
 * effects can be reverted back using SvSettingsManagerPopComponent().
 *
 * @memberof SvSettingsManager
 *
 * @param[in] self            Settings Manager handle
 * @param[in] ctxID           context ID received previously from
 *                            SvSettingsManagerSaveContext()
 * @param[out] errorOut       error info
 **/
extern void
SvSettingsManagerRestoreContext(SvSettingsManager self,
                                SvSettingsManagerContextID ctxID,
                                SvErrorInfo *errorOut);

/**
 * Check if a particular widget is defined in current search context.
 *
 * @memberof SvSettingsManager
 *
 * @param[in] self            Settings Manager handle
 * @param[in] name            widget name
 * @return                    @c true if widget labeled with @a name
 *                            is defined in search context, @c false if not
 **/
extern bool
SvSettingsManagerIsWidgetDefined(SvSettingsManager self,
                                 const char *name);

/**
 * Retrieve single parameter's value.
 *
 * This method returns the value of one of the parameters of the widget
 * labeled with @a name. Parameters will be searched for in following order:
 *
 *  - private settings in the specified widget,
 *  - common settings in the specified widget,
 *  - common settings in the parent widget,
 *  - ... and so on, up to top-level widget,
 *  - global parameters.
 *
 * @memberof SvSettingsManager
 *
 * @param[in] self            Settings Manager handle
 * @param[in] name            widget name, @c NULL for global parameter
 * @param[in] tag             parameter tag
 * @return                    requested value, @c NULL if not found
 **/
extern SvValue
SvSettingsManagerGetValue(SvSettingsManager self,
                          const char *name,
                          const char *tag);

/**
 * Set single parameter's value.
 *
 * This method changes the value of one of the parameters of the widget
 * labeled with @a name or a global parameter.
 *
 * @memberof SvSettingsManager
 *
 * @param[in] self            Settings Manager handle
 * @param[in] name            widget name, @c NULL for global parameter
 * @param[in] tag             parameter tag
 * @param[in] value           new value
 * @param[out] errorOut       error info
 **/
extern void
SvSettingsManagerSetValue(SvSettingsManager self,
                          const char *name,
                          const char *tag,
                          SvValue value,
                          SvErrorInfo *errorOut);

/**
 * Retrieve single parameter's array value.
 *
 * This method returns the array value of one of the parameters of the widget
 * labeled with @a name. Parameters will be searched in the same order as
 * described in SvSettingsManagerGetValue().
 *
 * @memberof SvSettingsManager
 *
 * @param[in] self            Settings Manager handle
 * @param[in] name            widget name, @c NULL for global parameter
 * @param[in] tag             parameter tag
 * @return                    requested parameters array, @c NULL if not found
 **/
extern SvSettingsParamsArray
SvSettingsManagerGetArray(SvSettingsManager self,
                          const char *name,
                          const char *tag);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
