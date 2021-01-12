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

#include "QBConaxMenuHandler.h"

#include <libintl.h>
#include <QBSecureLogManager.h>
#include <SvCore/SvCommonDefs.h>
#include <SvCore/SvEnv.h>
#include <SvCore/SvTime.h>
#include <SvCore/SvLog.h>
#include <SvFoundation/SvObject.h>
#include <SvFoundation/SvType.h>
#include <SvFoundation/SvString.h>
#include <SvFoundation/SvInterface.h>
#include <SvFoundation/SvValue.h>
#include <SvFoundation/SvURL.h>
#include <SvFoundation/SvStringBuffer.h>
#include <SvFoundation/SvIterator.h>
#include <SvFoundation/SvArray.h>
#include <QBDataModel3/QBTreeIterator.h>
#include <QBDataModel3/QBTreePath.h>
#include <QBDataModel3/QBTreeModel.h>
#include <settings.h>
#include <QBSmartcard2Interface.h>
#include <QBICSmartcardMail.h>
#include <QBICSmartcardInfo.h>
#include <QBICSmartcardEvent.h>
#include <QBICSmartcardPurse.h>
#include <QBICSmartcardSubscription.h>

#include <Windows/mainmenu.h>
#include <Utils/authenticators.h>
#include <Widgets/authDialog.h>
#include <Widgets/pinChangeDialog.h>
#include <Widgets/QBSimpleDialog.h>
#include <QBWidgets/QBDialog.h>
#include <Services/Conax/QBConaxMailIndicator.h>
#include <Logic/timeFormat.h>
#include <TranslationMerger.h>
#include <QBMenu/QBMenuEventHandler.h>
#include <XMB2/XMBMenuBar.h>
#include <QBMenu/QBMenu.h>
#include <main.h>


SV_DECL_INT_ENV_FUN_DEFAULT( env_log_level, 0, "ConaxMenuLogLevel", "" );
#define log_debug(fmt, ...) do { if (env_log_level() >= 1) { SvLogNotice("ConaxMenu : " fmt, ##__VA_ARGS__); } } while (0)
#define log_state(fmt, ...) do { if (1) { SvLogNotice("ConaxMenu : " fmt, ##__VA_ARGS__); } } while (0)


struct menu_data
{
  SvGenericObject  path;
  QBActiveTreeNode  root_node;
  QBActiveTreeNode  parent;
  bool  is_inserted;
};

static void menu_data_clear(struct menu_data* data)
{
  SVTESTRELEASE(data->path);
  SVTESTRELEASE(data->root_node);
}


struct QBConaxMenuHandler_s
{
  struct SvObject_ super_;

  AppGlobals  appGlobals;
  QBActiveTree  mainTree;
  SvWidget  menuBar;

  QBCASCmd cmd;
  QBActiveTreeNode  cmd_node;
  SvWidget  dialog; /// for pin-related commands
  SvWidget dialogHelper;    ///for accessing  various data about pin enter/change status

  struct menu_data  info;
  struct menu_data  mail;
  struct menu_data  subs;
  struct menu_data  events;
  struct menu_data  tokens;
  struct menu_data  mat;
  struct menu_data  pin;

  QBCASSmartcardState  card_state;
  unsigned int mail_cnt;
  unsigned int unread_mail_cnt;

  bool isProperTime;
};
typedef struct QBConaxMenuHandler_s* QBConaxMenuHandler;

SvLocal void QBConaxMenu_show_hide(QBConaxMenuHandler self);


static void remove_children(QBConaxMenuHandler self, QBActiveTreeNode parent_node)
{
  QBActiveTreeNodeRemoveSubTrees(parent_node, NULL);
}

static void remove_node(QBConaxMenuHandler self, QBActiveTreeNode node)
{
  QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(node), node, NULL);
}

static void add_node(QBConaxMenuHandler self, QBActiveTreeNode parent_node, QBActiveTreeNode node)
{
    //SvLogNotice("QBActiveTreeAddSubTree(tree=%p, parent=%p, node=%p", self->mainTree, parent_node, node);
    QBActiveTreeNodeAddSubTree(parent_node, node, NULL);

}

#define GET_OBJ( _node_, _name_)  QBActiveTreeNodeGetAttribute(_node_, SVSTRING(_name_))
#define GET_MAIL(_node_, _name_)  ((QBICSmartcardMail*) GET_OBJ(_node_, _name_))
#define GET_PURSE(_node_, _name_) ((QBICSmartcardPurse*) GET_OBJ(_node_, _name_))
#define GET_INT( _node_, _name_)  SvValueGetInteger((SvValue) GET_OBJ(_node_, _name_))
#define GET_BOOL(_node_, _name_)  SvValueGetBoolean((SvValue) GET_OBJ(_node_, _name_))

static QBActiveTreeNode get_first_child(QBActiveTreeNode parent_node)
{
  SvIterator iter = QBActiveTreeNodeChildNodesIterator(parent_node);
  return (QBActiveTreeNode) SvIteratorGetNext(&iter);
}

static void remove_please_wait_node(QBConaxMenuHandler self, QBActiveTreeNode parent_node)
{
  QBActiveTreeNode first = get_first_child(parent_node);
  if (first && GET_OBJ(first, "pleaseWait"))
    remove_node(self, first);
}

static void set_value(QBActiveTreeNode node, SvString name, SvString value)
{
  QBActiveTreeNodeSetAttribute(node, name, (SvGenericObject)value);
}

static void set_yes(QBActiveTreeNode node, SvString name)
{
  set_value(node, name, SVSTRING("yes"));
}

static void set_has_submenu(QBActiveTreeNode node)
{
  //set_yes(node, SVSTRING("conaxXMBItem:hasSubmenu"));
}

static QBActiveTreeNode add_subnode_(QBConaxMenuHandler self, QBActiveTreeNode parent_node,
                                     SvString caption, SvString subcaption,
                                     bool do_insert)
{
  QBActiveTreeNode node = QBActiveTreeNodeCreate(NULL, NULL, NULL);

  if (caption)
    set_value(node, SVSTRING("caption"), caption);
  if (subcaption)
    set_value(node, SVSTRING("subcaption"), subcaption);

  if (do_insert) {
    add_node(self, parent_node, node);
    SVRELEASE(node);
  };
  return node;
}

static QBActiveTreeNode add_subnode(QBConaxMenuHandler self, QBActiveTreeNode parent_node,
                                    SvString caption, SvString subcaption)
{
  return add_subnode_(self, parent_node, caption, subcaption, true);
}

static void propagate_state_change(QBConaxMenuHandler self, struct menu_data* data)
{
  SvGenericObject parentPath = SvObjectCopy(data->path, NULL);
  SvInvokeInterface(QBTreePath, parentPath, truncate, -1);
  QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->mainTree, getIterator, parentPath, 0);
  size_t nodesCount = QBTreeIteratorGetNodesCount(&iter);
  QBActiveTreePropagateNodesChange(self->mainTree, parentPath, 0, nodesCount, NULL);
  SVRELEASE(parentPath);
}

SvLocal void QBConaxMenu_cb_smartcard_state(void* self_, QBCASSmartcardState state)
{
  QBConaxMenuHandler self = (QBConaxMenuHandler) self_;

  SvLogNotice("QBConaxMenu : smartcard_state : %d [%s]", state, QBCASSmartcardStateToString(state));
  self->card_state = state;

  QBConaxMenu_show_hide(self);
}

static const struct QBCASCallbacks_s s_cas_callbacks = {
  .smartcard_state  = &QBConaxMenu_cb_smartcard_state,
};


static bool handle_generic_response(QBConaxMenuHandler self, QBActiveTreeNode parent_node, int status)
{
  self->cmd = NULL;
  self->cmd_node = NULL;

  if (status != 0) {
    if (SvInvokeInterface(QBCAS, QBCASGetInstance(), getCASType) == QBCASType_conax) {
      SvString message = SvStringCreateWithFormat(gettext_noop("Failed to get information from the card (error = 0x%x)"), status);
      add_subnode(self, parent_node, message, NULL);
      SVRELEASE(message);
    }
    return true;
  };

  return false;
}

static bool handle_array_response(QBConaxMenuHandler self, QBActiveTreeNode parent_node, int status, SvArray array, const char* zero_msg)
{
  if (handle_generic_response(self, parent_node, status))
    return true;

  int cnt = SvArrayCount(array);

  if (cnt == 0)
  {
    SvString message = SvStringCreate(zero_msg, NULL);
    add_subnode(self, parent_node, message, NULL);
    SVRELEASE(message);
    return true;
  };

  return false;
}

SvLocal bool isBooleanAttributeTrue(QBActiveTreeNode node, SvString attr)
{
    SvValue obj = (SvValue) QBActiveTreeNodeGetAttribute(node, attr);
    if (obj) {
        if (SvObjectIsInstanceOf((SvObject) obj, SvValue_getType()) &&
            SvValueIsBoolean(obj)) {
            return SvValueGetBoolean(obj);
        } else {
            SvLogError("%s(): Invalid type (%s, expected bool) for attribute %s in ConaxMenu node.",
                       __func__, SvObjectGetTypeName((SvObject) obj), SvStringCString(attr));
        }
    }
    return false;
}

/**
 * Handles big items attributes for ConaxMenu nodes.
 *
 * Copies searchedAttribute's value from one node to another node as addedAttribute's value.
 * If searchedAttribute not found, use default value.
 *
 * @param[in] searchedNode          node that will be searched for searchedAttribute
 * @param[in] searchedAttribute     attribute that will be searched in searchedNode
 * @param[in, out] setNode          node that will get attribute addedAttribute
 * @param[in] addedAttribute        attribute that will be set to setNode
 * @param[in] defaultBigItems       default value of big items attribute for setNode
 **/
SvLocal void QBConaxMenuHandleBigItems(QBActiveTreeNode searchedNode, SvString searchedAttribute,
                                       QBActiveTreeNode setNode, SvString addedAttribute,
                                       bool defaultBigItems)
{
  bool found = false;
  SvObject value = QBActiveTreeNodeGetAttribute(searchedNode, searchedAttribute);
  if (value) {
    if (SvObjectIsInstanceOf(value, SvValue_getType()) &&
        SvValueIsBoolean((SvValue) value)) {
      found = true;
    } else {
      SvLogError("%s(): Invalid type (%s, expected bool) for attribute %s in ConaxMenu node.",
                 __func__, SvObjectGetTypeName(value), SvStringCString(searchedAttribute));
    }
  }

  if (!found) {
    SvValue defaultValue = SvValueCreateWithBoolean(defaultBigItems, NULL);
    QBActiveTreeNodeSetAttribute(setNode, addedAttribute, (SvObject) defaultValue);
    SVRELEASE(defaultValue);
  } else if (searchedNode != setNode) {
    QBActiveTreeNodeSetAttribute(setNode, addedAttribute, value);
  }
}

static void callback_subs(void* self_, const QBICSmartcardCmd* cmd, int status, SvArray subscriptions)
{
  QBConaxMenuHandler self = (QBConaxMenuHandler) self_;
  QBActiveTreeNode parent = self->subs.root_node;
  bool bigItems = isBooleanAttributeTrue(parent, SVSTRING("bigItems"));

  if (handle_array_response(self, parent, status, subscriptions, gettext_noop("No subscriptions")))
    goto out;

  int cnt = SvArrayCount(subscriptions);
  for (int i=0; i<cnt; ++i) {
    QBICSmartcardSubscription* subs = (QBICSmartcardSubscription*) SvArrayAt(subscriptions, i);

    SvStringBuffer buffer = SvStringBufferCreate(NULL);
    for (int subIdx = 0; subIdx < subs->cnt; ++subIdx) {
      if (subs->data[subIdx].start.tm_year == 0 || subs->data[subIdx].end.tm_year == 0) {
        SvString startDate = QBTimeFormatConvertConaxMenuDate(subs->data[subIdx].start_str);
        SvString endDate = QBTimeFormatConvertConaxMenuDate(subs->data[subIdx].end_str);

        SvStringBufferAppendFormatted(buffer, NULL, "%s      %s       %s",
                                      SvStringCString(startDate), SvStringCString(endDate),
                                      SvStringCString(subs->data[subIdx].entitlement));

        SVRELEASE(startDate);
        SVRELEASE(endDate);
      } else {
        /// Do not construct local time, since this is only date, and we should not be moving it a few hours back/forth
        struct tm  tt_start = subs->data[subIdx].start;
        struct tm  tt_end   = subs->data[subIdx].end;
        SvStringBufferAppendFormatted(buffer, NULL, "%04d.%02d.%02d              %04d.%02d.%02d               %s",
                                      tt_start.tm_year + 1900, tt_start.tm_mon + 1, tt_start.tm_mday,
                                      tt_end.tm_year + 1900,   tt_end.tm_mon + 1,   tt_end.tm_mday,
                                      SvStringCString(subs->data[subIdx].entitlement));

        SvLogNotice("2)%04d.%02d.%02d      %04d.%02d.%02d       %s",
                                      tt_start.tm_year + 1900, tt_start.tm_mon + 1, tt_start.tm_mday,
                                      tt_end.tm_year + 1900,   tt_end.tm_mon + 1,   tt_end.tm_mday,
                                      SvStringCString(subs->data[subIdx].entitlement));
      };
      if (subIdx < subs->cnt)
        SvStringBufferAppendCString(buffer, "\n", NULL);
    };
    SvString buffer_str = SvStringBufferCreateContentsString(buffer, NULL);

    SvString caption = NULL;
    SvString subcaption = NULL;

    const char* header = gettext("Start date                End date                   Entitlement");

    if (subs->label) {
      caption = SVRETAIN(subs->label);
      subcaption = SvStringCreateWithFormat("%s\n%s",
                                            header, SvStringCString(buffer_str));
    } else {
      caption = SvStringCreate(header, NULL);
      subcaption = SVRETAIN(buffer_str);
    };
    SVRELEASE(buffer_str);
    SVRELEASE(buffer);

    log_debug("[%s] : [%s]", SvStringCString(caption), SvStringCString(subcaption));

    if (bigItems) {
      add_subnode(self, parent, caption, subcaption);
    } else {
      QBActiveTreeNode node = add_subnode(self, parent, caption, NULL);
      QBActiveTreeNodeSetAttribute(node, SVSTRING("dialogTitle"), (SvObject) caption);
      QBActiveTreeNodeSetAttribute(node, SVSTRING("dialogText"), (SvObject) subcaption);
    }
    SVRELEASE(caption);
    SVRELEASE(subcaption);
  };

out:
  remove_please_wait_node(self, parent);
}

SvLocal SvString QBConaxMenuCreateEventAuthenticationPeriodString(QBICSmartcardEvent *event, bool useSeparator)
{
  const char *separator = useSeparator ? "-" : "";
  if (event->auth_start == 0 || event->auth_end == 0) {
    return SvStringCreateWithFormat("%s   %s   %s",
                                    SvStringCString(event->auth_start_str),
                                    separator,
                                    SvStringCString(event->auth_end_str));
  } else {
    /// Construct Local time
    struct tm  tt_start = { .tm_sec = 0 };
    SvTimeBreakDown(SvTimeConstruct(event->auth_start, 0), true, &tt_start);
    struct tm  tt_end = { .tm_sec = 0 };
    SvTimeBreakDown(SvTimeConstruct(event->auth_end,   0), true, &tt_end);
    return SvStringCreateWithFormat("%04d.%02d.%02d  %02d:%02d       %s       %04d.%02d.%02d  %02d:%02d",
                                    tt_start.tm_year + 1900, tt_start.tm_mon + 1, tt_start.tm_mday, tt_start.tm_hour, tt_start.tm_min,
                                    separator,
                                    tt_end.tm_year + 1900,   tt_end.tm_mon + 1,   tt_end.tm_mday,   tt_end.tm_hour,   tt_end.tm_min);
    };

}

static void callback_events(void* self_, const QBICSmartcardCmd* cmd, int status, SvArray event_tab)
{
  QBConaxMenuHandler self = (QBConaxMenuHandler) self_;
  QBActiveTreeNode parent = self->events.root_node;
  bool bigItems = isBooleanAttributeTrue(parent, SVSTRING("bigItems"));

  if (handle_array_response(self, parent, status, event_tab, gettext_noop("No events")))
    goto out;

  int cnt = SvArrayCount(event_tab);
  int i;
  for (i=0; i<cnt; ++i)
  {
    QBICSmartcardEvent* event = (QBICSmartcardEvent*) SvArrayAt(event_tab, i);
    const char* header = gettext("Start date                           End date");

    // Contains only start and end date without header
    SvString shortAuthInfo = QBConaxMenuCreateEventAuthenticationPeriodString(event, true);

    /* Contains start and end date with header, optional entitlement
     * and optional time/credits left. */
    SvStringBuffer longAuthInfoBuffer = SvStringBufferCreate(NULL);
    SvString authenticationPeriod = QBConaxMenuCreateEventAuthenticationPeriodString(event, false);
    SvStringBufferAppendString(longAuthInfoBuffer, authenticationPeriod, NULL);
    SVRELEASE(authenticationPeriod);

    if (event->entitlement) {
      SvStringBufferAppendFormatted(longAuthInfoBuffer, NULL, "    %s",
                                    SvStringCString(event->entitlement));
    }
    if (event->access_type != QBICSmartcardEventAccessType_unlimited) {
      const char* units = (event->access_type == QBICSmartcardEventAccessType_minutes) ?
                          "minutes" : "credits";
      SvStringBufferAppendFormatted(longAuthInfoBuffer, NULL, "\n%d %s left", event->time_left, units);
    }
    SvString longAuthInfo = SvStringBufferCreateContentsString(longAuthInfoBuffer, NULL);
    SVRELEASE(longAuthInfoBuffer);

    SvString caption = NULL;
    SvString subcaption = NULL;
    SvString dialogText = NULL;

    if (event->label) {
      caption = SVRETAIN(event->label);
      SvString fullInfo = SvStringCreateWithFormat("%s\n%s",
                                                   header,
                                                   SvStringCString(longAuthInfo));
      subcaption = bigItems ? fullInfo : SVRETAIN(shortAuthInfo);
      dialogText = bigItems ? NULL : fullInfo;
    } else {
      caption = SvStringCreate(header, NULL);
      subcaption = SVRETAIN(longAuthInfo);
    }
    SVRELEASE(shortAuthInfo);
    SVRELEASE(longAuthInfo);

    log_debug("[%s] : [%s]", SvStringCString(caption), SvStringCString(subcaption));

    QBActiveTreeNode node = add_subnode(self, parent, caption, subcaption);
    if (!bigItems) {
      SvString text = dialogText ? dialogText : subcaption;
      QBActiveTreeNodeSetAttribute(node, SVSTRING("dialogTitle"), (SvObject) caption);
      QBActiveTreeNodeSetAttribute(node, SVSTRING("dialogText"), (SvObject) text);
    }

    SVRELEASE(caption);
    SVRELEASE(subcaption);
    SVTESTRELEASE(dialogText);
  };

out:
  remove_please_wait_node(self, parent);
}

static void callback_tokens(void* self_, const QBICSmartcardCmd* cmd, int status, SvArray purse_tab)
{
  QBConaxMenuHandler self = (QBConaxMenuHandler) self_;
  QBActiveTreeNode parent = self->tokens.root_node;

  if (handle_array_response(self, parent, status, purse_tab, gettext_noop("No purses")))
    goto out;

  int cnt = SvArrayCount(purse_tab);
  int i;
  for (i=0; i<cnt; ++i)
  {
    QBICSmartcardPurse* purse = (QBICSmartcardPurse*) SvArrayAt(purse_tab, i);

    SvString purse_caption = SvStringCreateWithFormat(gettext("Purse: %s"), SvStringCString(purse->label));
    SvString subcaption = SvStringCreateWithFormat(ngettext("Balance: %d token", "Balance: %d tokens", purse->balance), purse->balance);
    log_debug("[%s] : [%s]", SvStringCString(purse_caption), SvStringCString(subcaption));

    QBActiveTreeNode node = add_subnode_(self, parent, purse_caption, subcaption, false);

    QBActiveTreeNodeSetAttribute(node, SVSTRING("conaxPurse:obj"), (SvGenericObject)purse);
    set_has_submenu(node);

    QBActiveTreeNode credits_node = add_subnode_(self, node, SVSTRING("Credits"), NULL, false);
    QBActiveTreeNodeAddChildNode(node, credits_node, NULL);
    set_yes(credits_node, SVSTRING("conaxPurse:credits"));

    SvString purse_window_caption = SvStringCreateWithFormat("%s  ->  %s", SvStringCString(purse_caption), "Credits");
    set_value(credits_node, SVSTRING("windowCaption"), purse_window_caption);
    SVRELEASE(purse_window_caption);

    set_has_submenu(credits_node);
    SVRELEASE(credits_node);

    QBActiveTreeNode debits_node = add_subnode_(self, node, SVSTRING("Debits"), NULL, false);
    QBActiveTreeNodeAddChildNode(node, debits_node, NULL);
    set_yes(debits_node, SVSTRING("conaxPurse:debits"));

    bool defaultBigItemsInDebits = true;
    QBConaxMenuHandleBigItems(parent, SVSTRING("bigItemsInDebits"),
                              debits_node, SVSTRING("bigItems"),
                              defaultBigItemsInDebits);

    SvString debit_window_caption = SvStringCreateWithFormat("%s  ->  %s", SvStringCString(purse_caption), "Debits");
    set_value(debits_node, SVSTRING("windowCaption"), debit_window_caption);
    SVRELEASE(debit_window_caption);

    set_has_submenu(debits_node);
    SVRELEASE(debits_node);

    add_node(self, parent, node);
    SVRELEASE(node);
    SVTESTRELEASE(purse_caption);
    SVTESTRELEASE(subcaption);
  };

out:
  remove_please_wait_node(self, parent);
}

static void callback_debits_status(void* self_, const QBICSmartcardCmd* cmd, int status, SvGenericObject purse_status_)
{
  QBConaxMenuHandler self = (QBConaxMenuHandler) self_;

  QBActiveTreeNode parent_node = self->cmd_node;

  SvArray debits_tab = NULL;
  if (purse_status_) {
    debits_tab = ((QBICSmartcardPurseStatus*)purse_status_)->debits;
  };

  if (handle_array_response(self, parent_node, status, debits_tab, gettext_noop("No debits")))
    goto out;

  int cnt = SvArrayCount(debits_tab);
  int i;
  for (i=0; i<cnt; ++i)
  {
    QBICSmartcardPurseDebit* debit = (QBICSmartcardPurseDebit*) SvArrayAt(debits_tab, i);

    SvString caption = debit->label;
    if (!caption)
      caption = SVSTRING("<no-name>");

    char time_str_buf[32];
    const char* time_str = NULL;
    if (debit->last_access_time == 0) {
      time_str = SvStringCString(debit->last_access_time_str);
    } else {
      struct tm  tt = { .tm_sec = 0 };
      SvTimeBreakDown(SvTimeConstruct(debit->last_access_time,  0), true, &tt);
      snprintf(time_str_buf, sizeof(time_str_buf), "%04d.%02d.%02d  %02d:%02d", tt.tm_year + 1900, tt.tm_mon + 1, tt.tm_mday, tt.tm_hour, tt.tm_min);
      time_str = time_str_buf;
    };

    SvString subcaption = SvStringCreateWithFormat(gettext("Tokens: %d\n"
                                                   "Date: %s"),
                                                   debit->consumed_tokens,
                                                   time_str);
    log_debug("[%s] : [%s]", SvStringCString(caption), SvStringCString(subcaption));

    add_subnode(self, parent_node, caption, subcaption);
    SVTESTRELEASE(subcaption);
  };

out:
  remove_please_wait_node(self, parent_node);
}

static void callback_credits_status(void* self_, const QBICSmartcardCmd* cmd, int status, SvGenericObject purse_status_)
{
  QBConaxMenuHandler self = (QBConaxMenuHandler) self_;

  QBActiveTreeNode parent_node = self->cmd_node;

  SvArray credits_tab = NULL;
  if (purse_status_) {
    credits_tab = ((QBICSmartcardPurseStatus*)purse_status_)->credits;
  };

  if (handle_array_response(self, parent_node, status, credits_tab, gettext_noop("No credits")))
    goto out;

  int cnt = SvArrayCount(credits_tab);
  int i;
  for (i=0; i<cnt; ++i)
  {
    QBICSmartcardPurseCredit* credit = (QBICSmartcardPurseCredit*) SvArrayAt(credits_tab, i);

    SvString caption = credit->label;
    if (!caption)
      caption = SVSTRING("<no-name>");
    SvString subcaption = SvStringCreateWithFormat(gettext("Tokens: %d"),
                                                   credit->tokens);
    log_debug("[%s] : [%s]", SvStringCString(caption), SvStringCString(subcaption));

    add_subnode(self, parent_node, caption, subcaption);
    SVTESTRELEASE(subcaption);
  };

out:
  remove_please_wait_node(self, parent_node);
}

static void callback_info(void* self_, const QBCASCmd cmd, int status, QBCASInfo info_)
{
  QBConaxMenuHandler self = (QBConaxMenuHandler) self_;
  QBActiveTreeNode parent = self->info.root_node;

  if (handle_generic_response(self, parent, status))
    goto out;

  const QBICSmartcardInfo info = (const QBICSmartcardInfo) info_;

  SvString subcaption = NULL;
  SvString str;

  str = SvStringCreate(gettext("Card Type"), NULL);
  subcaption = SvStringCreate(QBCASTypeToString(info->base.cas_type), NULL);
  add_subnode(self, parent, str, subcaption);
  SVRELEASE(str);
  SVRELEASE(subcaption);

  str = SvStringCreate(gettext("Software Version"), NULL);
  add_subnode(self, parent, str, info->base.sw_ver);
  SVRELEASE(str);

  if (info->base.cas_type == QBCASType_conax)
  {
    subcaption = SvStringCreateWithFormat("0x%x", info->host_version);
    str = SvStringCreate(gettext("Interface Version"), NULL);
    add_subnode(self, parent, str, subcaption);
    SVRELEASE(str);
    SVRELEASE(subcaption);
  };

  if (info->base.card_number_complete)
  {
    str = SvStringCreate(gettext("Card Number"), NULL);
    add_subnode(self, parent, str, info->base.card_number_complete);
    SVRELEASE(str);
  }
  else
  {
    if (info->base.card_number_part_unique) {
      str = SvStringCreate(gettext("Card Number"), NULL);
      add_subnode(self, parent, str, info->base.card_number_part_unique);
      SVRELEASE(str);
    }

    if (info->base.card_number_part_group) {
      str = SvStringCreate(gettext("Group Number"), NULL);
      add_subnode(self, parent, str, info->base.card_number_part_group);
      SVRELEASE(str);
    }
  };

  if (self->card_state == QBCASSmartcardState_correct) {
    if (info->base.cas_type == QBCASType_conax)
    {
      subcaption = SvStringCreateWithFormat("%d", info->session_num);
      str = SvStringCreate(gettext("Number of Sessions"), NULL);
      add_subnode(self, parent, str, subcaption);
      SVRELEASE(str);
      SVRELEASE(subcaption);

      subcaption = SvStringCreateWithFormat("%02d", info->country_indicator);
      str = SvStringCreate(gettext("Language"), NULL);
      add_subnode(self, parent, str, subcaption);
      SVRELEASE(str);
      SVRELEASE(subcaption);
    };

    subcaption = SvStringCreateWithFormat("0x%04X", info->base.ca_sys_id);
    str = SvStringCreate(gettext("CA SYS ID"), NULL);
    add_subnode(self, parent, str, subcaption);
    SVRELEASE(str);
    SVRELEASE(subcaption);
  }

  str = SvStringCreate(gettext("Chip ID"), NULL);
  add_subnode(self, parent, str, info->base.chip_id);
  SVRELEASE(str);

out:
  remove_please_wait_node(self, parent);
}


typedef int (QBConaxMenu_schedule_fun) (QBConaxMenuHandler self, QBActiveTreeNode node);

SvLocal int QBConaxMenu_schedule_info(QBConaxMenuHandler self, QBActiveTreeNode node)
{
  return SvInvokeInterface(QBCAS, QBCASGetInstance(), getInfo, &callback_info, self, &self->cmd);
}
SvLocal int QBConaxMenu_schedule_subs(QBConaxMenuHandler self, QBActiveTreeNode node)
{
  return SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), getSubscriptions, &callback_subs, self, &self->cmd);
}
SvLocal int QBConaxMenu_schedule_events(QBConaxMenuHandler self, QBActiveTreeNode node)
{
  return SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), getEvents, &callback_events, self, &self->cmd);
}
SvLocal int QBConaxMenu_schedule_tokens(QBConaxMenuHandler self, QBActiveTreeNode node)
{
  return SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), getPurses, &callback_tokens, self, &self->cmd);
}
SvLocal int QBConaxMenu_schedule_debits_status(QBConaxMenuHandler self, QBActiveTreeNode node)
{
  QBActiveTreeNode parent = QBActiveTreeNodeGetParentNode(node);
  QBICSmartcardPurse* purse = GET_PURSE(parent, "conaxPurse:obj");
  return SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), getPurseStatus, &callback_debits_status, self, &self->cmd,
                           purse->purse_ref, true, false);
}
SvLocal int QBConaxMenu_schedule_credits_status(QBConaxMenuHandler self, QBActiveTreeNode node)
{
  QBActiveTreeNode parent = QBActiveTreeNodeGetParentNode(node);
  QBICSmartcardPurse* purse = GET_PURSE(parent, "conaxPurse:obj");
  return SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), getPurseStatus, &callback_credits_status, self, &self->cmd,
                           purse->purse_ref, false, true);
}

SvLocal void QBConaxMenu_schedule_command(QBConaxMenuHandler self, QBActiveTreeNode node, QBConaxMenu_schedule_fun* fun)
{
  /// clear
  remove_children(self, node);

  /// schedule
  if (self->cmd) {
    SvInvokeInterface(QBCAS, QBCASGetInstance(), cancelCommand, self->cmd);
    self->cmd = NULL;
    self->cmd_node = NULL;
  };
  int res = fun(self, node);
  if (res < 0) {
    SvString str = SvStringCreate(gettext_noop("ERROR: Sending question to the smartcard failed"), NULL);
    add_subnode(self, node, str, NULL);
    SVRELEASE(str);
  } else {
    self->cmd_node = node;
    SvString str = SvStringCreate(gettext("Please wait..."), NULL);
    QBActiveTreeNode wait_node = add_subnode(self, node, str, NULL);
    SVRELEASE(str);
    set_yes(wait_node, SVSTRING("pleaseWait")); /// mark to remove after the command is completed
  };
}

SvLocal void QBConaxMenu_maturity_change_callback(void *self_, SvWidget dialog, SvString buttonTag, unsigned int keyCode)
{
  QBConaxMenuHandler self = self_;

  if (buttonTag && SvStringEqualToCString(buttonTag, "OK-button")) {
    SvObject path = QBActiveTreeCreateNodePath(self->appGlobals->menuTree, SVSTRING("ConaxMaturityRating"));
    if (path) {
      QBTreeIterator iter = SvInvokeInterface(QBTreeModel, self->appGlobals->menuTree, getIterator, path, 0);
      size_t nodesCount = QBTreeIteratorGetNodesCount(&iter);
      QBActiveTreePropagateNodesChange(self->appGlobals->menuTree, path, 0, nodesCount, NULL);
      SVRELEASE(path);
    }

    QBSecureLogEvent("Conax", "Notice.ConaxMenu.MaturityRatingChange", "JSON:{\"maturityRating\":\"%i\"}", SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), getMaturityRating));
  }

  self->dialog = NULL;
  SVRELEASE(self);
}

SvLocal void QBConaxMenu_common_dialog_callback(void *self_, SvWidget dialog, SvString buttonTag, unsigned int keyCode)
{
  QBConaxMenuHandler self = self_;

  if(self->dialog) {
      SVRELEASE(self);
  }

  self->dialog = NULL;
}

SvLocal void QBConaxMenuRunSimpleDialog(QBConaxMenuHandler self, SvString dialogTitle, SvString dialogText)
{
  svSettingsPushComponent("Dialog.settings");
  QBDialogParameters params = {
    .app        = self->appGlobals->res,
    .controller = self->appGlobals->controller,
    .widgetName = "Dialog",
    .ownerId    = 0,
  };
  if (self->dialog)
    QBDialogBreak(self->dialog);
  self->dialog = QBSimpleDialogCreate(&params, SvStringCString(dialogTitle), SvStringCString(dialogText));
  QBDialogRun(self->dialog, SVRETAIN(self), &QBConaxMenu_common_dialog_callback);
  svSettingsPopComponent();
}

SvLocal void QBConaxMenuSelected_impl(QBConaxMenuHandler self, QBActiveTreeNode node)
{
  /// ------ main nodes ------

  if (node == self->info.root_node) {
    QBConaxMenu_schedule_command(self, self->info.root_node, &QBConaxMenu_schedule_info);
    return;
  };

  if (node == self->subs.root_node) {
    QBConaxMenu_schedule_command(self, self->subs.root_node, &QBConaxMenu_schedule_subs);
    return;
  };

  if (node == self->events.root_node) {
    QBConaxMenu_schedule_command(self, self->events.root_node, &QBConaxMenu_schedule_events);
    return;
  };

  if (node == self->tokens.root_node) {
    QBConaxMenu_schedule_command(self, self->tokens.root_node, &QBConaxMenu_schedule_tokens);
    return;
  };

  if (node == self->pin.root_node) {
    if (self->dialog)
        QBDialogBreak(self->dialog);
    svSettingsPushComponent("ConaxMenu.settings");
    self->dialog = QBPinChangeDialogCreate(self->appGlobals, "PinChangeDialog", SVSTRING("DRM"), true, &self->dialogHelper);
    svSettingsPopComponent();
    QBDialogRun(self->dialog, SVRETAIN(self), &QBConaxMenu_common_dialog_callback);
    return;
  };

  /// ------ mail ------
  if (QBActiveTreeNodeGetParentNode(node) == self->mail.root_node)
  {
    /// mark the mail as read, if not yet read
    if (!QBConaxMailManagerIsNodeNoMessages(node)) {
      SvObject mail = QBConaxMailManagerGetMailFromNode(node);
      if (!mail) {
        SvLogError("%s(): invalid message node", __func__);
        return;
      }
      QBConaxMailManagerMarkMailOpened(self->appGlobals->conaxMailManager, mail);

      if (isBooleanAttributeTrue(self->mail.root_node, SVSTRING("bigItems")))
        return;

      SvString mailText = QBConaxMailManagerGetMailText(self->appGlobals->conaxMailManager, mail);
      if (!mailText)
        return;
      SvString title = SvStringCreateWithFormat("%s (%d)", gettext("Message"),
                                                QBConaxMailManagerGetMailSlot(self->appGlobals->conaxMailManager, mail));
      QBConaxMenuRunSimpleDialog(self, title, mailText);
      SVRELEASE(title);
    }

    return;
  }

  /// ------ subscription and event nodes ------
  if (QBActiveTreeNodeGetParentNode(node) == self->subs.root_node ||
      QBActiveTreeNodeGetParentNode(node) == self->events.root_node)
  {
    SvObject title = QBActiveTreeNodeGetAttribute(node, SVSTRING("dialogTitle"));
    SvObject text = QBActiveTreeNodeGetAttribute(node, SVSTRING("dialogText"));

    if (!title || !text || !SvObjectIsInstanceOf(title, SvString_getType()) ||
        !SvObjectIsInstanceOf(text, SvString_getType())) {
        return;
    }
    QBConaxMenuRunSimpleDialog(self, (SvString) title, (SvString) text);
    return;
  }

  /// -------- 1st sub-level for maturity rating --------

  if (QBActiveTreeNodeGetParentNode(node) == self->mat.root_node)
  {
    SvString id = QBActiveTreeNodeGetID(node);

    int rating = 0x0;
    if (SvStringEqualToCString(id, "ConaxRatingG")) {
      rating = 0x1;
    } else if (SvStringEqualToCString(id, "ConaxRatingPG")) {
      rating = 0x2;
    } else if (SvStringEqualToCString(id, "ConaxRatingA")) {
      rating = 0x4;
    } else if (SvStringEqualToCString(id, "ConaxRatingX")) {
      rating = 0x8;
    };

    int currentRating = SvInvokeInterface(QBSmartcard2Interface, QBCASGetInstance(), getMaturityRating);
    if (currentRating != rating) {
      if (self->dialog)
          QBDialogBreak(self->dialog);
      SvGenericObject authenticator = QBAuthenticateForConaxMaturityRating(self->appGlobals->accessMgr, rating);
      svSettingsPushComponent("ConaxMenu.settings");
      self->dialog = QBAuthDialogCreate(self->appGlobals, authenticator, gettext("Authentication required"), gettext("Please enter CA PIN"), true, NULL, NULL);
      svSettingsPopComponent();
      QBDialogRun(self->dialog, SVRETAIN(self), &QBConaxMenu_maturity_change_callback);
    };
    return;
  };

  /// -------- 2nd sub-level for debits/credits ---------

  if (QBActiveTreeNodeGetAttribute(node, SVSTRING("conaxPurse:debits")))
  {
    QBConaxMenu_schedule_command(self, node, &QBConaxMenu_schedule_debits_status);
    return;
  };

  if (QBActiveTreeNodeGetAttribute(node, SVSTRING("conaxPurse:credits")))
  {
    QBConaxMenu_schedule_command(self, node, &QBConaxMenu_schedule_credits_status);
    return;
  };

}

SvLocal bool QBConaxMenuSelected(SvGenericObject self_, SvGenericObject node_, SvGenericObject nodePath_)
{
  if (!SvObjectIsInstanceOf(node_, QBActiveTreeNode_getType()))
    return false;
  QBConaxMenuHandler self = (QBConaxMenuHandler) self_;
  QBActiveTreeNode node = (QBActiveTreeNode) node_;

  QBConaxMenuSelected_impl(self, node);
  return false;
}

SvLocal void QBConaxMenu_update_nodes_attached_(QBConaxMenuHandler self, struct menu_data* data, bool can_insert)
{
  //SvLogNotice("QBConaxMenu_update_nodes_attached_(node=%p, inserted=%d, show=%d)", data->root_node, data->is_inserted, can_insert);
  if (!data->root_node)
    return;
  if (data->is_inserted == can_insert)
    return;
  data->is_inserted = can_insert;

  if (can_insert) {
    add_node(self, data->parent, data->root_node);
  } else {
    remove_node(self, data->root_node);
  };
}

SvLocal const char* QBConaxMenu_get_state_text(QBCASSmartcardState state)
{
  switch (state) {
  case QBCASSmartcardState_removed:    return gettext_noop("No Card Inserted");
  case QBCASSmartcardState_inserted:   return gettext_noop("Card Inserted");
  case QBCASSmartcardState_correct:    return NULL;
  case QBCASSmartcardState_incorrect_ex: return gettext_noop("Incorrect Conax Card");
  case QBCASSmartcardState_incorrect:  return gettext_noop("Incorrect Card");
  case QBCASSmartcardState_problem:    return gettext_noop("Card Problem - check card");
  };
  return "???";
}

SvLocal void QBConaxMenu_show_hide(QBConaxMenuHandler self)
{
  bool can_show = (self->card_state == QBCASSmartcardState_correct);

  //SvLogWarning("QBConaxMenu_update_nodes_attached : [%s] can_show=%d", QBCASSmartcardStateToString(self->card_state), can_show);

  // Conax 6.0 requires visible mailbox during all card states
  // QBConaxMenu_update_nodes_attached_(self, &self->mail,   can_show);
  QBConaxMenu_update_nodes_attached_(self, &self->subs,   can_show);
  QBConaxMenu_update_nodes_attached_(self, &self->events, can_show);
  QBConaxMenu_update_nodes_attached_(self, &self->tokens, can_show);
  QBConaxMenu_update_nodes_attached_(self, &self->mat,    can_show);
  QBConaxMenu_update_nodes_attached_(self, &self->pin,    can_show);

  if (!can_show && self->dialog) {
    SvLogNotice("QBConaxMenu_update_nodes_attached(%d) : closing dialog", can_show);
    QBDialogBreak(self->dialog);
  };

  if (self->cmd) {
    SvInvokeInterface(QBCAS, QBCASGetInstance(), cancelCommand, self->cmd);
    self->cmd = NULL;
    self->cmd_node = NULL;
  };

  if (self->info.root_node) {
    remove_children(self, self->info.root_node);

    const char* subcaption_str = QBConaxMenu_get_state_text(self->card_state);
    SvString subcaption = NULL;
    if (subcaption_str)
      subcaption = SvStringCreate(subcaption_str, NULL);
    set_value(self->info.root_node, SVSTRING("subcaption"), subcaption);
    SVTESTRELEASE(subcaption);
    propagate_state_change(self, &self->info);
  };

}


SvLocal void QBConaxMenuHandlerDestroy(void *self_)
{
  QBConaxMenuHandler self = self_;

  if (self->dialog)
    QBDialogBreak(self->dialog);

  menu_data_clear(&self->info);
  menu_data_clear(&self->mail);
  menu_data_clear(&self->subs);
  menu_data_clear(&self->events);
  menu_data_clear(&self->tokens);
  menu_data_clear(&self->mat);
  menu_data_clear(&self->pin);

}

SvLocal void QBConaxMenuClearSubmenusWithTempData(SvGenericObject self_);

SvLocal void QBConaxMenuMainMenuChanged(SvGenericObject self_, QBMainMenuState state, SvGenericObject nodePath_)
{
  log_debug("%s:%d state (%d)", __FUNCTION__, __LINE__, state);

  if (state == QBMainMenuState_Destroyed) {
    QBConaxMenuClearSubmenusWithTempData(self_);
  }
}

SvLocal SvType QBConaxMenuHandler_getType(void)
{
  static const struct SvObjectVTable_ objectVTable = {
    .destroy = QBConaxMenuHandlerDestroy
  };

  static const struct QBMenuEventHandler_ selectMethods = {
    .nodeSelected = QBConaxMenuSelected
  };

  static const struct QBMainMenuChanges_t mainMenuChangesMethods = {
    .changed = QBConaxMenuMainMenuChanged,
  };

  static SvType type = NULL;
  if (unlikely(!type)) {
    SvTypeCreateManaged("QBConaxMenuHandler",
                        sizeof(struct QBConaxMenuHandler_s),
                        SvObject_getType(),
                        &type,
                        SvObject_getType(), &objectVTable,
                        QBMenuEventHandler_getInterface(), &selectMethods,
                        QBMainMenuChanges_getInterface(), &mainMenuChangesMethods,
                        NULL);
  }

  return type;
}

static bool init_menu_data(struct menu_data* data, SvString id, bool defaultBigItems, QBConaxMenuHandler self, AppGlobals appGlobals, QBTreePathMap pathMap)
{
  QBActiveTreeNode node = QBActiveTreeFindNode(appGlobals->menuTree, id);
  SvObject path = node ? QBActiveTreeCreateNodePath(appGlobals->menuTree, id) : NULL;
  if (!node || !path)
    return false;

  data->path = path;
  data->root_node = SVRETAIN(node);
  data->is_inserted = true;
  data->parent = QBActiveTreeNodeGetParentNode(data->root_node);

  QBConaxMenuHandleBigItems(node, SVSTRING("bigItems"),
                            node, SVSTRING("bigItems"), defaultBigItems);

  QBTreePathMapInsert(pathMap, path, (SvGenericObject)self, NULL);
  SvInvokeInterface(QBMenu, self->menuBar->prv, setEventHandlerForPath, path, (SvObject) self, NULL);
  return true;
}

void QBConaxMenuRegister(SvWidget menuBar, QBTreePathMap pathMap, AppGlobals appGlobals)
{
  QBActiveTreeNode conaxNode = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("Conax"));
  if (conaxNode && (SvInvokeInterface(QBCAS, QBCASGetInstance(), getCASType) != QBCASType_conax)) {
    QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(conaxNode), conaxNode, NULL);
    conaxNode = NULL;
    log_state("Conax CAS not present, menu disabled");
  }

  QBActiveTreeNode cryptoguardNode = QBActiveTreeFindNode(appGlobals->menuTree, SVSTRING("Cryptoguard"));
  if (cryptoguardNode && (SvInvokeInterface(QBCAS, QBCASGetInstance(), getCASType) != QBCASType_cryptoguard)) {
    QBActiveTreeNodeRemoveSubTree(QBActiveTreeNodeGetParentNode(cryptoguardNode), cryptoguardNode, NULL);
    cryptoguardNode = NULL;
    log_state("Cryptoguard CAS not present, menu disabled");
  }

  if (!conaxNode && !cryptoguardNode) {
    return;
  }

  QBConaxMenuHandler self = (QBConaxMenuHandler) SvTypeAllocateInstance(QBConaxMenuHandler_getType(), NULL);
  self->appGlobals = appGlobals;
  self->mainTree = appGlobals->menuTree;
  self->menuBar = menuBar;

  bool needed = false;

  if (SvInvokeInterface(QBCAS, QBCASGetInstance(), getCASType) == QBCASType_cryptoguard) {
    needed |= init_menu_data(&self->info, SVSTRING("CryptoguardInfo"), false, self, appGlobals, pathMap);
    needed |= init_menu_data(&self->subs, SVSTRING("CryptoguardSubscriptions"), true, self, appGlobals, pathMap);
    needed |= init_menu_data(&self->events, SVSTRING("CryptoguardEventStatus"), true, self, appGlobals, pathMap);
  } else {
    needed |= init_menu_data(&self->info, SVSTRING("ConaxInfo"), false, self, appGlobals, pathMap);
    needed |= init_menu_data(&self->mail, SVSTRING("ConaxMail"), true, self, appGlobals, pathMap);
    needed |= init_menu_data(&self->subs, SVSTRING("ConaxSubscriptions"), true, self, appGlobals, pathMap);
    needed |= init_menu_data(&self->events, SVSTRING("ConaxEventStatus"), true, self, appGlobals, pathMap);
    needed |= init_menu_data(&self->tokens, SVSTRING("ConaxTokenStatus"), false, self, appGlobals, pathMap);
    needed |= init_menu_data(&self->mat, SVSTRING("ConaxMaturityRating"), false, self, appGlobals, pathMap);
    needed |= init_menu_data(&self->pin, SVSTRING("ConaxChangePin"), false, self, appGlobals, pathMap);
  }

  if (!needed)
    goto out;

  self->card_state = SvInvokeInterface(QBCAS, QBCASGetInstance(), getState);
  QBConaxMenu_show_hide(self);

  SvInvokeInterface(QBCAS, QBCASGetInstance(), addCallbacks, (QBCASCallbacks)&s_cas_callbacks, self, "QBConaxMenu");

  appGlobals->conaxMenuHandler = (SvGenericObject) self;

  return;

out:
  SVRELEASE(self);
}

void QBConaxMenuOnSwitch(SvGenericObject self_, bool showing)
{
  QBConaxMenuHandler self = (QBConaxMenuHandler) self_;
  if (!self)
    return;

  if (!showing && self->dialog) {
    SvLogNotice("QBConaxMenuOnSwitch(%d) : closing dialog", showing);
    QBDialogBreak(self->dialog);
  };
}

SvLocal void QBConaxMenuClearSubmenusWithTempData(SvGenericObject self_)
{
  QBConaxMenuHandler self = (QBConaxMenuHandler) self_;
  if (!self)
    return;

  /// Below are all submenus that are showing data that is not refreshed automatically.
  /// Only when re-entering such menu node, the data is refreshed.
  /// But sometimes we are expected to refresh the data anyway at some weird moments.
  /// Below we are simply forcing the removal of all data nodes, so the user has to re-enter the sub-menu again.

  if (self->subs.root_node)
    remove_children(self, self->subs.root_node);
  if (self->events.root_node)
    remove_children(self, self->events.root_node);
  if (self->tokens.root_node)
    remove_children(self, self->tokens.root_node);
}

void QBConaxMenuHandlerStop(SvGenericObject self_)
{
  QBConaxMenuHandler self = (QBConaxMenuHandler) self_;
  if (!self)
    return;

  SvInvokeInterface(QBCAS, QBCASGetInstance(), removeCallbacks, (QBCASCallbacks)&s_cas_callbacks, self);
  self->appGlobals->conaxMenuHandler = NULL;
  SVRELEASE(self);
}
