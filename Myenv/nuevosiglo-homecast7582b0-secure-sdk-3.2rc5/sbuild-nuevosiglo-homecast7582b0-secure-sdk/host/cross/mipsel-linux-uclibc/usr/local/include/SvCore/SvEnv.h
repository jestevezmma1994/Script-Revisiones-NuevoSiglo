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

#ifndef SV_ENV_H_
#define SV_ENV_H_

#include <stdlib.h>
#include <SvCore/SvLog.h>


#ifdef __cplusplus
extern "C" {
#endif


#define SVENV_VAL_TYPE_NONE       0
#define SVENV_VAL_TYPE_MIN_MAX    1
#define SVENV_VAL_TYPE_DEFAULTS   2

typedef enum {
    SvEnv_Val_Type_NONE       = SVENV_VAL_TYPE_NONE,
    SvEnv_Val_Type_MIN_MAX    = SVENV_VAL_TYPE_MIN_MAX,
    SvEnv_Val_Type_DEFAULTS   = SVENV_VAL_TYPE_DEFAULTS
} SvEnv_Val_Type;

typedef struct {
    int idx;
    const char* name;
    int val;
    int defaultVal;
    const int* valTab;
    int valTabSize;
    SvEnv_Val_Type valType;

    const char* description;
    const char* module;
    bool initialized;
} SvEnvStruct;

extern int SvEnvAdd(const char* name, int defVal, const char* descr, const char* module, SvEnv_Val_Type valType, const int* valTab, int tabSize);
extern bool SvEnvGet(int idx, int* val);
extern bool SvEnvGetByName(const char* name, int* val);
extern int SvEnvGetCount(void);
extern int SvEnvUpdate(const char* name, const char* val);
extern void SvEnvSet(int idx, int val);
extern SvEnvStruct* SvEnvGetEnvData(int idx);

/**
 * Macro useful for defining a function for fast access to an integer
 * environment variable.
 **/
#define SV_DECL_INT_ENV_FUN_DEFAULT_(_fun_name_, _default_val_, _env_name_, _description_, _module_, _val_type_, _val_tab_ ) \
__attribute__((unused)) \
static int _fun_name_ (void) \
{ \
    static int idx = -1; \
    if (idx == -1)  \
        idx = SvEnvAdd(_env_name_, _default_val_, _description_, _module_, (SvEnv_Val_Type) _val_type_, _val_tab_, sizeof(_val_tab_) / sizeof(int)); \
    int res; \
    SvEnvGet( idx, &res); \
    return res; \
}\
static void sage_env_helper_ ## _fun_name_(void) __attribute__ ((constructor)); \
static void sage_env_helper_ ## _fun_name_(void) {\
    _fun_name_(); \
    if (getenv("SV_ENV_HELP") || getenv("HELP") ) \
        SvLogNotice("Env variable %s : %s (default %d)", _env_name_, _description_, (_default_val_)); \
}

#if SV_LOG_LEVEL > 0
#  define SV_DECL_INT_ENV_FUN_DEFAULT(_fun_name_, _default_val_, _env_name_, _description_ ) \
     SV_DECL_INT_ENV_FUN_DEFAULT_(_fun_name_, _default_val_, _env_name_, _description_, __FILE__, (SvEnv_Val_Type)SVENV_VAL_TYPE_NONE , 0)

#  define SV_DECL_INT_ENV_FUN_DEFAULT_DEFS(_fun_name_, _default_val_, _env_name_, _description_, _val_tab_ ) \
     SV_DECL_INT_ENV_FUN_DEFAULT_(_fun_name_, _default_val_, _env_name_, _description_, __FILE__, (SvEnv_Val_Type)SVENV_VAL_TYPE_DEFAULTS, _val_tab_)

#  define SV_DECL_INT_ENV_FUN_DEFAULT_MIN_MAX(_fun_name_, _default_val_, _env_name_, _description_, _val_tab_ ) \
     SV_DECL_INT_ENV_FUN_DEFAULT_(_fun_name_, _default_val_, _env_name_, _description_, __FILE__, (SvEnv_Val_Type)SVENV_VAL_TYPE_MIN_MAX, _val_tab_)

#else
#  define SV_DECL_INT_ENV_FUN_DEFAULT(_fun_name_, _default_val_, _env_name_, _description_ ) \
     SV_DECL_INT_ENV_FUN_DEFAULT_(_fun_name_, _default_val_, _env_name_, "", __FILE__, (SvEnv_Val_Type)SVENV_VAL_TYPE_NONE , 0)

#  define SV_DECL_INT_ENV_FUN_DEFAULT_DEFS(_fun_name_, _default_val_, _env_name_, _description_, _val_tab_ ) \
     SV_DECL_INT_ENV_FUN_DEFAULT_(_fun_name_, _default_val_, _env_name_, "", __FILE__, (SvEnv_Val_Type)SVENV_VAL_TYPE_DEFAULTS, _val_tab_)

#  define SV_DECL_INT_ENV_FUN_DEFAULT_MIN_MAX(_fun_name_, _default_val_, _env_name_, _description_, _val_tab_ ) \
     SV_DECL_INT_ENV_FUN_DEFAULT_(_fun_name_, _default_val_, _env_name_, "", __FILE__, (SvEnv_Val_Type)SVENV_VAL_TYPE_MIN_MAX, _val_tab_)
#endif

#define SV_DECL_INT_ENV_FUN(_fun_name_, _env_name_, _description_ ) \
  SV_DECL_INT_ENV_FUN_DEFAULT(_fun_name_, 0, _env_name_, _description_ )


#define SV_DECL_STRING_ENV_FUN_DEFAULT_(_fun_name_, _default_val_, _env_name_, _description_ ) \
__attribute__((unused)) \
static const char* _fun_name_ (void) \
{ \
  static const char* val = (const char*)(_default_val_); \
  static int initialized=0; \
  if (!initialized) { \
    initialized = 1; \
    const char* env = getenv(_env_name_); \
    if (!env) { \
      val = (const char*)(_default_val_); \
    } else { \
      val = env; \
    }; \
    /*SvLogNotice("Using value [%s] of env variable %s", val, _env_name_);*/ \
  }; \
  return val; \
}

#if SV_LOG_LEVEL > 0
#  define SV_DECL_STRING_ENV_FUN_DEFAULT(_fun_name_, _default_val_, _env_name_, _description_ ) \
     SV_DECL_STRING_ENV_FUN_DEFAULT_(_fun_name_, _default_val_, _env_name_, _description_)
#else
#  define SV_DECL_STRING_ENV_FUN_DEFAULT(_fun_name_, _default_val_, _env_name_, _description_ ) \
     SV_DECL_STRING_ENV_FUN_DEFAULT_(_fun_name_, _default_val_, _env_name_, "")
#endif

#define SV_DECL_STRING_ENV_FUN(_fun_name_, _env_name_, _description_ ) \
  SV_DECL_STRING_ENV_FUN_DEFAULT(_fun_name_, NULL, _env_name_, _description_ )


#ifdef __cplusplus
}
#endif

#endif
