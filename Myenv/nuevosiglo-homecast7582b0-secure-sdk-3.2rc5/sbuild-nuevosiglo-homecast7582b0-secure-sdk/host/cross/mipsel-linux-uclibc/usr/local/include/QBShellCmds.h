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

#ifndef QB_SHELL_COMMANDS_H_
#define QB_SHELL_COMMANDS_H_

/**
 * @file QBShellCmds.h
 * @brief Shell commands daemon API
 **/

typedef struct QBShellPipe_t *QBShellPipe;

struct QBShellPipe_t {
    int cmd_fd;
    int stderr_pipe;
    int stdout_pipe;
};

/**
 * Initialize QBShellCmds component.
 * @param[in] path path where shell_cmds daemon will store internal files.
 * @param[in] whitelist_name name of the executable component that
 *        will be using QBShellCmds. It is also name of whitelist
 *        file for shell_cmds daemon.
 * @return 0 on success, otherwise -1
 */
extern int
QBShellCmdsInit(const char *path, const char *whitelist_name);

extern void
QBShellCmdsDeinit(void);

extern int
QBShellExec(const char *cmd);


extern QBShellPipe
QBShellPipeOpen(const char *cmd, const char *mode);


extern int
QBShellPipeGetResult(QBShellPipe pipe_desc);

extern int
QBShellPipeRequestAsyncResult(QBShellPipe pipe_desc);
extern int
QBShellPipeHasAsyncResult(QBShellPipe pipe_desc);
extern int
QBShellPipeGetAsyncResult(QBShellPipe pipe_desc);


extern void
QBShellPipeClose(QBShellPipe pipe_desc);

extern int
QBShellPipeKill(QBShellPipe pipe_desc);

/**
 * @}
 **/

#endif /* QB_SHELL_COMMANDS_H_ */
