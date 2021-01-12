/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2013 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef SV_SETTINGS_MANAGER_H_
#define SV_SETTINGS_MANAGER_H_

/**
 * @file SvSettingsManager.h Settings Manager library
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

#include <SvSettingsManager/SvSettingsManagerCore.h>


/**
 * @page ReferenceManual
 *
 * @section DescribingVisualGUILayout Describing visual GUI layout
 *
 * @subsection rationale Rationale
 * Due to the CUIT architecture and the way SWL widgets are implemented,
 * GUI applications developed at Cubiware used to contain a lot of hard-coded
 * values. In the initialization phase, when GUI is built, application code
 * creates tens or even hundreds of widgets, and it has to provide a lot of
 * values, such as size and position of every widget, bitmaps used, fonts,
 * font sizes, colors. Hard-coding all these values has a major drawback:
 * it is very hard to change these values, because they are scattered
 * all over the source code.
 *
 * To solve this problem, the @ref SvSettingsManager was introduced.
 * It is is a simple software module, that serves as a tool helping
 * to separate various settings of any component of the GUI application
 * from the application binary and accompanying libraries.
 *
 * @subsection func Functionality
 * Settings Manager allows to store constant values of various types
 * in external file, that accompanies the GUI application together with
 * other resources (such as bitmaps and fonts). It is possible to
 * select one of many available configurations (complete sets of settings)
 * at runtime.
 *
 * Typical GUI application consists of tens of widgets.
 * Settings Manager allows defining a separate set of settings
 * for every widget, as well as some default values for entire parts
 * of the application.
 *
 * Settings Manager API, together with some convenience wrappers,
 * provides fast and easy way to access these settings.
 *
 * @subsection usage Using Settings Manager from GUI code
 * The most common usage scenario is to create
 * a single instance of the Settings Manager, load the settings at
 * the very beginning of the application and keep it for the entire
 * application's life time. The convenience wrappers included in the
 * library provide such method of accessing the settings.
 *
 * @subsection format Format of the settings files
 * The exact format of settings file is relatively easy to understand.
 * Its syntax is defined in XML Schema, which gives us the possibility
 * to check whether the settings files installed on the set-top-box
 * have valid format. See @ref SvSettingsManagerExamples,
 * they should be pretty straightforward to understand.
 **/

/**
 * @defgroup SvSettingsManager SvSettingsManager: Settings Manager library
 * @ingroup UIT
 *
 * Settings Manager library manages information about visual GUI layout
 * (see @ref DescribingVisualGUILayout.)
 **/

/**
 * @defgroup SvSettingsManagerExamples Examples of settings files
 * @ingroup SvSettingsManager
 * @{
 *
 * Sample files in the format accepted by Settings Manager.
 *
 * @include fonts.settings
 *
 * @include example_5.settings
 *
 * @}
 **/


#endif
