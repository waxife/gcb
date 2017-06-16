/**
 *  @file   sys.h
 *  @brief  head file for system setting
 *  $Id: sys.h,v 1.5 2013/01/28 11:19:07 ken Exp $
 *  $Author: ken $
 *  $Revision: 1.5 $
 *
 *  Copyright (c) 2011 Terawins Inc. All rights reserved.
 * 
 *  @date   2011/08/12  ken 	New file.
 *
 */

#ifndef _SYS_H
#define _SYS_H

#define HELEN_VER	"v1.2_T123"

#include "config.h"

#ifndef DEBUG_MODE
	#define NDEBUG
#endif
#include "debug.h"

#ifndef EOF
 #define EOF -1
#endif

#ifndef NULL
 #define NULL ((void *) 0)
#endif

/* tw system init entry */
extern void tw_init (void);

#endif /* _SYS_H */