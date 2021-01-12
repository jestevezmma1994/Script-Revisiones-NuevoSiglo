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

#ifndef SV_SETTINGS_MANAGER_CUIT_INTERFACE_H_
#define SV_SETTINGS_MANAGER_CUIT_INTERFACE_H_

/**
 * @file settings.h CUIT interface to the Settings Manager
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <stdbool.h>
#include <CAGE/Core/SvColor.h>
#include <CAGE/Core/SvBitmap.h>
#include <CAGE/Text/SvFont.h>
#include <CUIT/Core/types.h>
#include <QBResourceManager/SvRBObject.h>
#include <SvSettingsManager/SvSettingsManagerTypes.h>

#ifdef __cplusplus
extern "C" {
#endif


/**
 * @defgroup SvSettingsManagerService CUIT interface to the Settings Manager
 * @ingroup SvSettingsManager
 * @{
 *
 * This module contains definitions of some convenience functions,
 * designed to make using Settings Manager in CUIT applications easier.
 **/

/**
 * Initialize settings manager.
 *
 * This function initializes all data structures used by the Settings Manager.
 *
 * @param[in] ignored         dummy pointer, needed for API compatibility only
 * @return                    @c 0 in case of error, otherwise any other value
 **/
extern int
svSettingsInit(void *ignored);

/**
 * Free all resources used by the Settings Manager.
 *
 * @return                    @c 0 in case of error, otherwise any other value
 **/
extern int
svSettingsDeinit(void);

/**
 * Get handle to a default instance of the Settings Manager class.
 *
 * @return                    a handle to the Settings Manager instance,
 *                            @c NULL in case of error
 **/
extern SvSettingsManager
svSettingsGetManager(void);

/**
 * Check if settings file @a fileName is available.
 *
 * @param[in] fileName        settings file name
 * @return                    @c true if available
 **/
extern bool
svSettingsIsComponentAvailable(const char *fileName);

/**
 * Set search context to the component settings defined in @a fileName.
 *
 * @note This function is a convenience wrapper for
 * SvSettingsManagerPushComponent() method.
 *
 * @param[in] fileName        settings file name
 * @return                    @c 0 in case of error, otherwise any other value
 **/
extern int
svSettingsPushComponent(const char *fileName);

/**
 * Set search context to the temporary component containing empty
 * settings for widget with given @a name.
 *
 * @note This function is a convenience wrapper for
 * SvSettingsManagerPushTemporaryComponent() method.
 *
 * @param[in] name            widget name
 * @return                    @c 0 in case of error, otherwise any other value
 **/
extern int
svSettingsPushTemporaryComponent(const char *name);

/**
 * Set search context to the previous component settings.
 *
 * @note This function is a convenience wrapper for
 * SvSettingsManagerPopComponent() method.
 *
 * @return                    @c 0 in case of error, otherwise any other value
 **/
extern int
svSettingsPopComponent(void);

/**
 * Preload all bitmaps from current context.
 *
 * @note This function is a convenience wrapper for
 * SvSettingsManagerPreloadComponent() method.
 *
 * @return                   @c 0 in case of error, otherwise any other value
 */
extern int
svSettingsPreloadComponent(void);

/**
 * Associates current search context with simple ID value for easy
 * restoration in the future.
 *
 * @note This function is a convenience wrapper for
 * SvSettingsManagerSaveContext() method.
 *
 * @return                    context ID, @c 0 in case of error
 **/
extern unsigned int
svSettingsSaveContext(void);

/**
 * Restore search context to the previously saved state.
 *
 * @note This function is a convenience wrapper for
 * SvSettingsManagerRestoreContext() method.
 *
 * @param[in] ctxID           context ID received from svSettingsSaveContext()
 * @return                    @c 0 in case of error, otherwise any other value
 **/
extern int
svSettingsRestoreContext(unsigned int ctxID);

/**
 * Get value of one of widget's integer type parameters.
 *
 * @note This function is a convenience wrapper for
 * SvSettingsManagerGetValue() method.
 *
 * @param[in] name            widget name, local to the current component
 * @param[in] tag             parameter tag
 * @param[in] def             default value, returned when parameter
 *                            could not be found
 * @return                    requested value
 **/
extern int
svSettingsGetInteger(const char *name,
                     const char *tag,
                     int def);

/**
 * Set value of one of widget's integer type parameters.
 *
 * @note This function is a convenience wrapper for
 * SvSettingsManagerSetValue() method.
 *
 * @param[in] name            widget name, local to the current component
 * @param[in] tag             parameter tag
 * @param[in] value           parameter value
 * @return                    @c 0 in case of error, otherwise any other value
 **/
extern int
svSettingsSetInteger(const char *name,
                     const char *tag,
                     int value);

/**
 * Get value of one of widget's color type parameters.
 *
 * @note This function is a convenience wrapper for
 * SvSettingsManagerGetValue() method.
 *
 * @param[in] name            widget name, local to the current component
 * @param[in] tag             parameter tag
 * @param[in] def             default value, returned when parameter
 *                            could not be found
 * @return                    requested value
 **/
static inline SvColor
svSettingsGetColor(const char *name,
                   const char *tag,
                   SvColor def)
{
   return (SvColor) svSettingsGetInteger(name, tag, (int) def);
}

/**
 * Get value of one of widget's boolean type parameters.
 *
 * @note This function is a convenience wrapper for
 * SvSettingsManagerGetValue() method.
 *
 * @param[in] name            widget name, local to the current component
 * @param[in] tag             parameter tag
 * @param[in] def             default value, returned when parameter
 *                            could not be found
 * @return                    requested value
 **/
extern bool
svSettingsGetBoolean(const char *name,
                     const char *tag,
                     bool def);

/**
 * Set value of one of widget's boolean type parameters.
 *
 * @note This function is a convenience wrapper for
 * SvSettingsManagerSetValue() method.
 *
 * @param[in] name            widget name, local to the current component
 * @param[in] tag             parameter tag
 * @param[in] value           parameter value
 * @return                    @c 0 in case of error, otherwise any other value
 **/
extern int
svSettingsSetBoolean(const char *name,
                     const char *tag,
                     bool value);

/**
 * Get value of one of widget's floating point type parameters.
 *
 * @note This function is a convenience wrapper for
 * SvSettingsManagerGetValue() method.
 *
 * @param[in] name            widget name, local to the current component
 * @param[in] tag             parameter tag
 * @param[in] def             default value, returned when parameter
 *                            could not be found
 * @return                    requested value
 **/
extern double
svSettingsGetDouble(const char *name,
                    const char *tag,
                    double def);

/**
 * Set value of one of widget's floating point type parameters.
 *
 * @note This function is a convenience wrapper for
 * SvSettingsManagerSetValue() method.
 *
 * @param[in] name            widget name, local to the current component
 * @param[in] tag             parameter tag
 * @param[in] value           parameter value
 * @return                    @c 0 in case of error, otherwise any other value
 **/
extern int
svSettingsSetDouble(const char *name,
                     const char *tag,
                     double value);

/**
 * Get value of one of widget's string type parameters.
 *
 * @note This function is a convenience wrapper for
 * SvSettingsManagerGetValue() method.
 *
 * @param[in] name            widget name, local to the current component
 * @param[in] tag             parameter tag
 * @return                    requested value, @c NULL if not found
 **/
extern const char *
svSettingsGetString(const char *name,
                    const char *tag);

/**
 * Set value of one of widget's string type parameters.
 *
 * @note This function is a convenience wrapper for
 * SvSettingsManagerSetValue() method.
 *
 * @param[in] name            widget name, local to the current component
 * @param[in] tag             parameter tag
 * @param[in] value           parameter value
 * @return                    @c 0 in case of error, otherwise any other value
 **/
extern int
svSettingsSetString(const char *name,
                    const char *tag,
                    const char *value);

/**
 * Get resource ID of a bitmap type or font type parameter.
 *
 * @note This function is a convenience wrapper for
 * SvSettingsManagerGetValue() method.
 *
 * @param[in] name            widget name, local to the current component
 * @param[in] tag             parameter tag
 * @return                    requested value, SV_RID_INVALID if not found
 **/
extern SvRID
svSettingsGetResourceID(const char *name,
                        const char *tag);

/**
 * Set resource ID of a bitmap type or font type parameter.
 *
 * @note This function is a convenience wrapper for
 * SvSettingsManagerSetValue() method.
 *
 * @param[in] name            widget name, local to the current component
 * @param[in] tag             parameter tag
 * @param[in] value           parameter value
 * @return                    @c 0 in case of error, otherwise any other value
 **/
extern int
svSettingsSetResourceID(const char *name,
                        const char *tag,
                        SvRID value);

/**
 * Get value of one of widget's bitmap type parameters.
 *
 * @note This function is a convenience wrapper for
 * SvSettingsManagerGetValue() method.
 *
 * @note This function will work only for bitmaps, that can be loaded
 * synchronously, i.e. the ones that are available in local file system.
 *
 * @param[in] name            widget name, local to the current component
 * @param[in] tag             parameter tag
 * @return                    requested value, @c NULL if not found
 **/
extern SvBitmap
svSettingsGetBitmap(const char *name,
                    const char *tag);

/**
 * Create new font object from value of one of widget's font type parameters.
 *
 * @note This function is a convenience wrapper for
 * SvSettingsManagerGetValue() method.
 *
 * @note This function will work only for fonts, that can be loaded
 * synchronously, i.e. the ones that are available in local file system.
 *
 * @qb_allocator
 *
 * @param[in] name            widget name, local to the current component
 * @param[in] tag             parameter tag
 * @return                    created font, @c NULL if not found
 **/
extern SvFont
svSettingsCreateFont(const char *name,
                     const char *tag);

/**
 * Get value of one of widget's array type parameters.
 *
 * @note This function is a convenience wrapper for
 * SvSettingsManagerGetArray() method.
 *
 * @param[in] name            widget name, local to the current component
 * @param[in] tag             parameter tag
 * @return                    requested parameters array, @c NULL if not found
 **/
extern SvSettingsParamsArray
svSettingsGetArray(const char *name,
                   const char *tag);

/**
 * Check if widget is defined.
 *
 * @param[in] name            widget name, local to the current component
 * @return                    @c true if settings for widget with @a name
 *                            are available, @c false if not
 **/
extern bool
svSettingsIsWidgetDefined(const char *name);

/**
 * Create generic CUIT widget using its settings.
 *
 * This function creates a generic CUIT widget using svWidgetCreateBitmap()
 * or svWidgetCreateWithColor(). Settings for this widget have to contain
 * background bitmap (parameter 'bg' of bitmap type) or size defined by ratio
 * of parent width and height (parameters 'widthRatio' and 'heightRatio', both in double).
 * If you define widget size (both widthRatio and heightRatio),
 * you can pass 'bgColor' (color type) instead of 'bg'.
 * Of course, you can define all parameters at the same time.
 *
 * @param[in] app             CUIT application handle
 * @param[in] wname           widget name
 * @param[in] parentWidth     parent width
 * @param[in] parentHeight    parent height
 * @return                    new widget, @c NULL in case of error
 **/
extern SvWidget
svSettingsWidgetCreateWithRatio(SvApplication app,
                                const char *wname,
                                unsigned int parentWidth,
                                unsigned int parentHeight);

/**
 * Create generic CUIT widget using its settings.
 *
 * This function creates a generic CUIT widget using svWidgetCreateBitmap()
 * or svWidgetCreateWithColor(). Settings for this widget have to contain
 * background bitmap (parameter 'bg' of bitmap type) or size (parameters
 * 'width' and 'height', both integers). If you define widget size (both width
 * and height), you can pass 'bgColor' (color type) instead of 'bg'.
 * Of course, you can define all parameters at the same time.
 *
 * @param[in] app             CUIT application handle
 * @param[in] wname           widget name
 * @return                    new widget, @c NULL in case of error
 **/
extern SvWidget
svSettingsWidgetCreate(SvApplication app,
                       const char *wname);

/**
 * Create generic CUIT widget using its settings, given width and height.
 *
 * This function creates a generic CUIT widget using svWidgetCreateBitmap()
 * or svWidgetCreateWithColor(). Settings for this widget have to contain
 * background bitmap (parameter 'bg' of bitmap type).
 * You can pass 'bgColor' (color type) instead of 'bg'.
 * Of course, you can define all parameters at the same time.
 *
 * @param[in] app             CUIT application handle
 * @param[in] wname           widget name
 * @param[in] width           widget width
 * @param[in] height          widget height
 * @return                    new widget, @c NULL in case of error
 **/
SvWidget
svSettingsWidgetCreateWithParams(SvApplication app, const char *wname,
                                 int width, int height);

/**
 * Attach CUIT widget using ratio values from the Settings Manager.
 *
 * @param[in] parent          parent widget
 * @param[in] child           child widget
 * @param[in] childName       child widget name
 * @param[in] level           z-order of the child widget
 * @return                    @c 0 in case of error, otherwise any other value
 **/
extern int
svSettingsWidgetAttachWithRatio(SvWidget parent,
                                SvWidget child,
                                const char *childName,
                                int level);

/**
 * Attach CUIT widget to QBCUITWidget using ratio values from the Settings Manager.
 *
 * @param[in] parent          parent widget
 * @param[in] child           child widget
 * @param[in] childName       child widget name
 * @param[in] level           z-order of the child widget
 * @return                    @c 0 in case of error, otherwise any other value
 **/
extern int
svSettingsWidgetAttachToQBCUITWidgetWithRatio(QBCUITWidget parent,
                                              SvWidget child,
                                              const char *childName,
                                              int level);

/**
 * Attach CUIT widget using values from the Settings Manager.
 *
 * @param[in] parent          parent widget
 * @param[in] child           child widget
 * @param[in] childName       child widget name
 * @param[in] level           z-order of the child widget
 * @return                    @c 0 in case of error, otherwise any other value
 **/
extern int
svSettingsWidgetAttach(SvWidget parent,
                       SvWidget child,
                       const char *childName,
                       int level);

/**
 * Attach CUIT widget to QBCUITWidget using values from the Settings Manager.
 *
 * @param[in] parent          parent widget
 * @param[in] child           child widget
 * @param[in] childName       child widget name
 * @param[in] level           z-order of the child widget
 * @return                    @c 0 in case of error, otherwise any other value
 **/
extern int
svSettingsWidgetAttachToQBCUITWidget(QBCUITWidget parent,
                                     SvWidget child,
                                     const char *childName,
                                     int level);

/**
 * @}
 **/


#ifdef __cplusplus
}
#endif

#endif
