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

#ifndef QB_SEARCH_AGENT_H_
#define QB_SEARCH_AGENT_H_

/**
 * @file QBSearchProgressListener.h Abstract search agent interface
 **/

#include <SvFoundation/SvCoreTypes.h>
#include <SvFoundation/SvArray.h>

/**
 * QBSearchAgent callbacks.
 *
 * QBSearchAgentCallbacks are callbacks is interface for communication with the upper layer,
 * managing results, progress, getting keywords, etc.
 **/
typedef const struct QBSearchAgentCallbacks_ {
    /**
     * Add results from the results array.
     *
     * @param[in] self_         owner handle
     * @param[in] agent         search agent that called this callback
     * @param[in] results       results array
     **/
    void (*addResults)(SvObject self_, SvObject agent, SvArray results);

    /**
     * Remove result.
     *
     * @param[in] self_         owner handle
     * @param[in] result        result to be removed
     **/
    void (*removeResult)(SvObject self_, SvObject result);

    /**
     * Update progress.
     *
     * @param[in] self_         owner handle
     * @param[in] agent         search agent that called this callback
     * @param[in] progress      current progress
     **/
    void (*updateProgress)(SvObject self, SvObject agent, int progress);

    /**
     * Get keywords array.
     *
     * @param[in] self_         owner handle
     * @return                  keywords array
     **/
    SvArray (*getKeywords)(SvObject self);

    /**
     * Get keywords string.
     *
     * @param[in] self_         owner handle
     * @return                  keywords string
     **/
    SvString (*getKeywordsString)(SvObject self_);

    /**
     * Notify upper layer about change in results.
     *
     * @param[in] self_         owner handle
     **/
    void (*changeNotify)(SvObject self_);
} QBSearchAgentCallbacks;

/**
 * QBSearchAgent interface.
 *
 * QBSearchAgent is interface for handling search agents by the upper layer
 **/
typedef struct QBSearchAgent_ {
    /**
     * Set callbacks.
     *
     * @param[in] self_         search agent handle
     * @param[in] callbacks     callback functions for communication with the upper layer
     * @param[in] owner         owner of search agent
     **/
    void (*setCallbacks)(SvObject self_, QBSearchAgentCallbacks *callbacks, SvObject owner);

    /**
     * Start agent.
     *
     * @param[in] self_         search agent handle
     **/
    void (*start)(SvObject self_);

    /**
     * Stop agent.
     *
     * @param[in] self_         search agent handle
     **/
    void (*stop)(SvObject self_);
} *QBSearchAgent;

SvInterface
QBSearchAgent_getInterface(void);

#endif /* QB_SEARCH_AGENT_H_ */
