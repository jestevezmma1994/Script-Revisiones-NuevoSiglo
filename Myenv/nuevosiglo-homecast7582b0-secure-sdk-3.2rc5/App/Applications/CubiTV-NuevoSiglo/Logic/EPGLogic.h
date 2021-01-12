/*****************************************************************************
** Cubiware Sp. z o.o. Software License Version 1.0
**
** Copyright (C) 2008-2010 Cubiware Sp. z o.o. All rights reserved.
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

#ifndef HDMI_MODE_CHANGE_LOGIC_H_
#define HDMI_MODE_CHANGE_LOGIC_H_

/**
 * @file  EPGLogic.h
 * @brief EPG logic API
 **/

#include <main_decl.h>
#include <stdbool.h>

#include <CUIT/Core/types.h>
#include <QBWindowContext.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvFoundation/SvString.h>
#include <SvEPGDataLayer/Data/SvTVChannel.h>
#include <SvPlayerKit/SvEPGEvent.h>

/**
 * @defgroup QBEPGLogic EPG logic class
 * @ingroup CubiTV_logic
 * @{
 **/
typedef struct QBEPGLogic_t* QBEPGLogic;

QBEPGLogic QBEPGLogicNew(AppGlobals appGlobals) __attribute__((weak));
bool QBEPGLogicHandleInputEvent(QBEPGLogic self, SvInputEvent ev) __attribute__((weak));


void QBEPGLogicDefaultViewSet(AppGlobals appGlobals, QBWindowContext ctx) __attribute__((weak));

/**
 * Update time limits from EPG views (e.g. block back in time from some reason).
 *
 * @param[in]       self                EPG Logic handle
 * @param[in,out]   backTimeLimit       back time limit to update
 * @param[in, out]  forwardTimeLimit    forward time limit to update
 * @return                              true if time limits are updated, otherwise false
 **/
bool QBEPGLogicUpdateDefaultTimeLimitsForViews(QBEPGLogic self, int *backTimeLimit, int *forwardTimeLimit) __attribute__((weak));

/**
 * Handle manual selected channel in EPG grid
 *
 * @param[in] self      EPG logic handle
 * @param[in] channel   TV channel handle
 **/
void QBEPGLogicHandleManualChannelSelection(QBEPGLogic self, SvTVChannel channel) __attribute__((weak));

/**
 * Starts EPG logic.
 *
 * @param[in] self      EPG logic handle
 **/
void QBEPGLogicStart(QBEPGLogic self) __attribute__((weak));

/**
 * Stops EPG logic.
 *
 * @param[in] self      EPG logic handle
 **/
void QBEPGLogicStop(QBEPGLogic self) __attribute__((weak));

/**
 * This function is called when EPG context window has been just created and is full initialized.
 *
 * @param [in] self     EPG logic handle
 **/
void QBEPGLogicPostWidgetsCreate(QBEPGLogic self) __attribute__((weak));

/**
 * This function is called before EPG context windows is destroy. All objects from context should
 * be still valid.
 *
 * @param [in] self      EPG logic handle
 **/
void QBEPGLogicPreWidgetsDestroy(QBEPGLogic self) __attribute__((weak));

SvString QBEPGLogicGetChannelName(QBEPGLogic self, SvTVChannel channel) __attribute__((weak));

/**
 * This function returns catchup availability time received from middleware in response callback.
 *
 * @param [in] self      EPG logic handle
 * @return               catchup availability time received from middleware in response callback
 **/
time_t QBEPGLogicGetCatchupAvailabilityLimitSecond(QBEPGLogic self);

/**
 * This function is called to check if EPGEvent should be highlighted in EPG.
 *
 * @param[in] self       EPG logic handle
 * @param[in] event      EPG event handle
 * @return               true if event should be highlighted in EPG
 **/
bool QBEPGLogicShouldEventBeHighlighted(QBEPGLogic self, SvEPGEvent event) __attribute__((weak));

/**
 * @}
 **/

#endif
