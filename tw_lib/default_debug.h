/**
 *  @file   debug.h
 *  @brief  head file for debug port define
 *  $Id: default_debug.h,v 1.4 2012/12/04 03:05:12 kevin Exp $
 *  $Author: kevin $
 *  $Revision: 1.4 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#ifndef _DEBUG_H
#define _DEBUG_H

#include "sys.h"

#ifndef DBG_LEVEL
#define DBG_LEVEL   0
#endif

#ifdef NDEBUG
#ifndef DBG_PRINT
	#include "doprnt.h"
	#define DBG_PRINT   no_printf
#endif

#define dbg(lvl, arg)  \
	{if(0) DBG_PRINT arg;}
#else  /* ! defined(NDEBUG) */

#ifndef DBG_PRINT
	#include "doprnt.h"
	#define DBG_PRINT   printf_tw
#endif

#define dbg(lvl, arg)  \
    {if (lvl <= DBG_LEVEL) {DBG_PRINT("%s:%d ", __FILE__, (int)__LINE__); DBG_PRINT arg;}}
#endif /* NDEBUG */

#define INFO(arg)   (DBG_PRINT("\x1b[32m%s:%d [INFO]\x1b[0m ", __FILE__, (int)__LINE__), DBG_PRINT arg)
#define WARN(arg)   (DBG_PRINT("\x1b[32m%s:%d [WARN]\x1b[0m ", __FILE__, (int)__LINE__), DBG_PRINT arg)
#define ERROR(arg)  (DBG_PRINT("\x1b[31m%s:%d [ERROR]\x1b[0m ", __FILE__, (int)__LINE__), DBG_PRINT arg)

#endif /* _DEBUG_H */
