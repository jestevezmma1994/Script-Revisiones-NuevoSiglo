/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2016 Cubiware Sp. z o.o. All rights reserved.
**
** Any rights which are not expressly granted in this License are entirely and
** exclusively reserved to and by Cubiware Sp. z o.o. You may not rent, lease,
** modify, translate, reverse engineer, decompile, disassemble, or create
** derivative works based on this Software. You may not make access to this
** Software available to others in connection with a service bureau,
** application service provider, or similar business, or make any other use of
** this Software without express written permission from Cubiware Sp z o.o.
**
** Any User wishing to make use of this Software must contact
** Cubiware Sp. z o.o. to arrange an appropriate license. Use of the Software
** includes, but is not limited to:
** (1) integrating or incorporating all or part of the code into a product for
**     sale or license by, or on behalf of, User to third parties;
** (2) distribution of the binary or source code to third parties for use with
**     a commercial product sold or licensed by, or on behalf of, User.
******************************************************************************/

#ifndef QB_CONTENT_PROVIDER_H_
#define QB_CONTENT_PROVIDER_H_

/**
 * @file QBContentProvider.h
 * @brief Content provider
 * @xmlonly<fullDocumentation>This file requires full documentation</fullDocumentation>
 * @endxmlonly
 **/

/**
 * @defgroup QBContentProvider Content provider abstract class
 * @ingroup QBContentManagerCore
 * @{
 **/

#include <stdlib.h>
#include <fibers/c/fibers.h>
#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvString.h>
#include <QBContentManager/QBContentCategory.h>
#include <QBContentManager/QBContentFilter.h>


/**
 * QBContentProvider class internals.
 **/
struct QBContentProvider_ {
    /**
     * super object
     */
    struct SvObject_ super_;
};

/**
 * Content provider abstract class.
 *
 * @class QBContentProvider QBContentProvider.h <QBContentManager/QBContentProvider.h>
 * @extends SvObject
 **/
typedef struct QBContentProvider_ *QBContentProvider;

/**
 * Content provider virtual methods table.
 **/
typedef struct QBContentProviderVTable_ {
    /**
     * virtual methods of the base class
     **/
    struct SvObjectVTable_ super_;

    /**
     * Start asynchronous tasks.
     *
     * @param[in] self_     content provider handle
     * @param[in] scheduler handle to a scheduler to be used by the agent
     **/
    void (*start)(QBContentProvider self_, SvScheduler scheduler);

    /**
     * Stop asynchronous tasks.
     *
     * @param[in] self_     content provider handle
     **/
    void (*stop)(QBContentProvider self_);

    /**
     * Refresh content category.
     *
     * @param[in] self_                 content provider handle
     * @param[in] category              content category to be refreshed
     * @param[in] index                 hint around which index data should be obtained
     * @param[in] force                 refresh even when the category is cached
     * @param[in] updateSingleObject    update only object at index
     **/
    void (*refresh)(QBContentProvider self_, QBContentCategory category, size_t index, bool force, bool updateSingleObject);

    /**
     * Drop all data
     *
     * @param[in] self_     content provider handle
     **/
    void (*clear)(QBContentProvider self_);

    /**
     * Hook for any initialization code that needs to be done after providers subtree is mounted.
     *
     * @param[in] self_     content provider handle
     **/
    void (*setup)(QBContentProvider self_);

    /**
     * Adds a listener to the provider (and all of its subproviders), the listener should implement
     * QBContentProviderListener interface.
     *
     * @param[in] self_     content provider handle
     * @param[in] listener  listener object handle
     **/
    void (*addListener)(QBContentProvider self_, SvObject listener);

    /**
     * Removes previously addded listener from the provider (and all of its subproviders).
     *
     * @param[in] self_     content provider handle
     * @param[in] listener  listener object handle
     **/
    void (*removeListener)(QBContentProvider self_, SvObject listener);

    /**
     * Get manager.
     *
     * @param[in] self_     content provider handle
     **/
    SvObject (*getManager)(QBContentProvider self_);

    /**
     * Set requested content language.
     *
     * @param[in] self_     content provider handle
     * @param[in] language  language name
     **/
    void (*setLanguage)(QBContentProvider self_, SvString lang);

    /**
     * Set requested content filter.
     *
     * @param[in] self_     content provider handle
     * @param[in] filter    content filter
     **/
    void (*setFilter)(QBContentProvider self, QBContentFilter filter);
} *QBContentProviderVTable;

/**
 * Get runtime type identification object representing
 * type of QBContentProvider class.
 *
 * @return QBContentProvider class type identification object
 **/
extern SvType
QBContentProvider_getType(void);

/**
 * Start asynchronous tasks.
 *
 * @param[in] self      content provider handle
 * @param[in] scheduler handle to a scheduler to be used by the agent
 **/
static inline void QBContentProviderStart(QBContentProvider self, SvScheduler scheduler)
{
    SvInvokeVirtual(QBContentProvider, self, start, scheduler);
}

/**
 * Stop asynchronous tasks.
 *
 * @param[in] self     content provider handle
 **/
static inline void QBContentProviderStop(QBContentProvider self)
{
    SvInvokeVirtual(QBContentProvider, self, stop);
}

/**
 * Refresh content category.
 *
 * @param[in] self                  content provider handle
 * @param[in] category              content category to be refreshed
 * @param[in] idx                   hint around which index data should be obtained
 * @param[in] force                 refresh even when the category is cached
 * @param[in] updateSingleObject    update only object at index
 **/
static inline void QBContentProviderRefresh(QBContentProvider self, QBContentCategory category, size_t idx, bool force, bool updateSingleObject)
{
    SvInvokeVirtual(QBContentProvider, self, refresh, category, idx, force, updateSingleObject);
}

/**
 * Drop all data
 *
 * @param[in] self      content provider handle
 **/
static inline void QBContentProviderClear(QBContentProvider self)
{
    SvInvokeVirtual(QBContentProvider, self, clear);
}

/**
 * Hook for any initialization code that needs to be done after providers subtree is mounted.
 *
 * @param[in] self      content provider handle
 **/
static inline void QBContentProviderSetup(QBContentProvider self)
{
    SvInvokeVirtual(QBContentProvider, self, setup);
}

/**
 * Adds a listener to the provider (and all of its subproviders), the listener should implement
 * QBContentProviderListener interface.
 *
 * @param[in] self      content provider handle
 * @param[in] listener  listener object handle
 **/
static inline void QBContentProviderAddListener(QBContentProvider self, SvObject listener)
{
    SvInvokeVirtual(QBContentProvider, self, addListener, listener);
}

/**
 * Removes previously addded listener from the provider (and all of its subproviders).
 *
 * @param[in] self      content provider handle
 * @param[in] listener  listener object handle
 **/
static inline void QBContentProviderRemoveListener(QBContentProvider self, SvObject listener)
{
    SvInvokeVirtual(QBContentProvider, self, removeListener, listener);
}

/**
 * Get manager.
 *
 * @param[in] self      content provider handle
 * @return              manager handle
 **/
static inline SvObject QBContentProviderGetManager(QBContentProvider self)
{
    return SvInvokeVirtual(QBContentProvider, self, getManager);
}

/**
 * Set requested content language.
 *
 * @param[in] self      content provider handle
 * @param[in] language  language name
 **/
static inline void QBContentProviderSetLanguage(QBContentProvider self, SvString language)
{
    SvInvokeVirtual(QBContentProvider, self, setLanguage, language);
}

/**
 * Set requested content filter.
 *
 * @param[in] self      content provider handle
 * @param[in] filter    content filter
 **/
static inline void QBContentProviderSetFilter(QBContentProvider self, QBContentFilter filter)
{
    SvInvokeVirtual(QBContentProvider, self, setFilter, filter);
}

/**
 * @}
 **/

#endif
