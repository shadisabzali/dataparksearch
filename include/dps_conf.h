/* Copyright (C) 2003-2005 Datapark corp. All rights reserved.
   Copyright (C) 2000-2002 Lavtech.com corp. All rights reserved.

   This program is free software; you can redistribute it and/or modify
   it under the terms of the GNU General Public License as published by
   the Free Software Foundation; either version 2 of the License, or
   (at your option) any later version.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
   GNU General Public License for more details.

   You should have received a copy of the GNU General Public License
   along with this program; if not, write to the Free Software
   Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA 
*/

#ifndef _DPS_CONF_H
#define _DPS_CONF_H

#include <sys/types.h>

extern __C_LINK int __DPSCALL DpsEnvLoad(DPS_AGENT *Indexer, const char * name, dps_uint8 flags);
extern __C_LINK int __DPSCALL DpsEnvAddLine(DPS_CFG *C, char *str);

extern char *DpsParseEnvVar(DPS_ENV *Conf, const char *str);
extern int  DpsSearchMode(const char * mode);
extern int  DpsMatchMode(const char * mode);
extern int  DpsFollowType(const char * follow);
extern int  DpsMethod(const char * method);
extern enum dps_prmethod DpsPRMethod(const char *);
extern int  DpsWeightFactorsInit(const char *wf, int *res);

extern const char	*DpsMethodStr(int method);
extern __C_LINK const char * __DPSCALL DpsFollowStr(int follow);

extern size_t DpsGetArgs(char *str, char **av, size_t max);
extern size_t DpsRelEtcName(DPS_ENV *Env, char *res, size_t maxlen, const char *name);
extern size_t DpsRelVarName(DPS_ENV *Env, char *res, size_t maxlen, const char *name);

#endif
