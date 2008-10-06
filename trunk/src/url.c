/* Copyright (C) 2003-2008 Datapark corp. All rights reserved.
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

#include "dps_common.h"
#include "dps_url.h"
#include "dps_utils.h"
#include "dps_charsetutils.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <sys/types.h>

DPS_URL * __DPSCALL DpsURLInit(DPS_URL *url) {
  if (!url) {
    url = (DPS_URL*)DpsMalloc(sizeof(DPS_URL));
    if (url == NULL) return NULL;
    bzero((void*)url, sizeof(DPS_URL));
    url->freeme = 1;
  } else {
    int fr = url->freeme;
    bzero((void*)url, sizeof(DPS_URL));
    url->freeme = fr;
  }
  return url;
}

void __DPSCALL DpsURLFree(DPS_URL *url) {
	DPS_FREE(url->schema);
	DPS_FREE(url->specific);
	DPS_FREE(url->hostinfo);
	DPS_FREE(url->auth);
	DPS_FREE(url->hostname);
	DPS_FREE(url->path);
	DPS_FREE(url->directory);
	DPS_FREE(url->filename);
	DPS_FREE(url->anchor);
	DPS_FREE(url->query_string);
	if(url->freeme){
		DPS_FREE(url);
	} else {
	  url->port = url->default_port = 0;
	}
}

int DpsURLParse(DPS_URL *url,const char *str){
	char *schema,*anchor,*file,*query;
	char *s;
/*	size_t len = dps_strlen(str);*/
	
	DPS_FREE(url->schema);
	DPS_FREE(url->specific);
	DPS_FREE(url->hostinfo);
	DPS_FREE(url->hostname);
	DPS_FREE(url->anchor);
	DPS_FREE(url->auth);
	url->port=0;
	url->default_port=0;
	DPS_FREE(url->path);
	DPS_FREE(url->directory);
	DPS_FREE(url->filename);
	DPS_FREE(url->query_string);

/*	if(len >= DPS_URLSIZE)return(DPS_URL_LONG);  FIXME: Chage this cheking for configured parameter, not DPS_URLSIZE */
	s = (char*)DpsStrdup(str);
	if (s == NULL) return DPS_ERROR;

	url->len = dps_strlen(str);
	
	/* Find possible schema end than   */	
	/* Check that it is really schema  */
	/* It must consist of alphas only  */
	/* We will take in account digits  */
	/* also for oracle8:// for example */
	/* We must check it because        */
	/* It might be anchor also         */
	/* For example:                    */
	/* "mod/index.html#a:1"            */

	if((schema=strchr(s,':'))){
		const char * ch;
		for(ch=s;ch<schema;ch++){
			if(!isalnum(*ch)){
				/* Bad character       */
				/* so it is not schema */
				schema=0;break;
			}
		}
	}

	if(schema){
		/* Have scheme - absolute path */
		*schema=0;
		url->schema = (char*)DpsStrdup(s);
		url->specific = (char*)DpsStrdup(schema + 1);
		*schema=':';
		if(!strcasecmp(url->schema,"http"))url->default_port=80;
		else
		if(!strcasecmp(url->schema,"https"))url->default_port=443;
		else
		if(!strcasecmp(url->schema,"nntp"))url->default_port=119;
		else
		if(!strcasecmp(url->schema,"news"))url->default_port=119;
		else
		if(!strcasecmp(url->schema,"ftp"))url->default_port=21;

		if(!strncmp(url->specific,"//",2)){
			char	*ss,*hostname;
			
			/* Have hostinfo */
			if((ss=strchr(url->specific+2,'/'))){
				/* Have hostname with path */
				*ss=0;
				url->hostinfo = (char*)DpsStrdup(url->specific + 2);
				*ss='/';
				url->path = (char*)DpsStrdup(ss);
			}else{
				/* Hostname without path */
			        if ((ss = strchr(url->specific + 2, '?'))) {
				  /* Have hostname with parameters */
				  *ss = 0;
				  url->hostinfo = (char*)DpsStrdup(url->specific + 2);
				  *ss='?';
				  url->path = (char*)DpsStrdup("/");
				}else {
				  url->hostinfo = (char*)DpsStrdup(url->specific + 2);
				  url->path = (char*)DpsStrdup("/");
				}
			}
			if((hostname=strchr(url->hostinfo,'@'))){
				/* Username and password is given  */
				/* Store auth string user:password */
				*hostname=0;
				url->auth = (char*)DpsStrdup(url->hostinfo);
				*hostname='@';
				hostname++;
			}else{
				hostname = url->hostinfo;
			}
			/*
			FIXME:
			for(h=hostname;*h;h++){
				if( *h>='A' && *h<='Z')
				*h=(*h)-'A'+'a';
			}
			*/
	
			if((ss=strchr(hostname,':'))){
				*ss=0;
				url->hostname = (char*)DpsStrdup(hostname);
				*ss=':';
				url->port=atoi(ss+1);
			}else{
				url->hostname = (char*)DpsStrdup(hostname);
				url->port=0;
			}
		}else{
			/* Have not host but have schema                   */
			/* This is possible for:                           */
			/* file:  mailto:  htdb: news:                     */
			/* As far as we do not need mailto: just ignore it */
			
		        if(!strcasecmp(url->schema,"mailto") 
			   || !strcasecmp(url->schema,"javascript")
			   || !strcasecmp(url->schema,"feed")
			   ) {
			        DPS_FREE(s);
				return(DPS_URL_BAD);
			} else
			if(!strcasecmp(url->schema,"file"))
				url->path = (char*)DpsStrdup(url->specific);
			else
			if(!strcasecmp(url->schema,"exec"))
				url->path = (char*)DpsStrdup(url->specific);
			else
			if(!strcasecmp(url->schema,"cgi"))
				url->path = (char*)DpsStrdup(url->specific);
			else
			if(!strcasecmp(url->schema,"htdb"))
				url->path = (char*)DpsStrdup(url->specific);
			else
			if(!strcasecmp(url->schema,"news")){
				/* Now we will use localhost as NNTP    */
				/* server as it is not indicated in URL */
				url->hostname = (char*)DpsStrdup("localhost");
				url->path = (char*)DpsMalloc(dps_strlen(url->specific) + 2);
				if (url->path == NULL) {
				  DPS_FREE(s);
				  return DPS_ERROR;
				}
				sprintf(url->path,"/%s",url->specific);
				url->default_port=119;
			}else{
				/* Unknown strange schema */
			        DPS_FREE(s);
				return(DPS_URL_BAD);
			}
		}
	}else{
		url->path = (char*)DpsStrdup(s);
	}

	/* Cat an anchor if exist */
	if((anchor=strstr(url->path,"#")))*anchor=0;


	/* If path is not full just copy it to filename    */
	/* i.e. neither  /usr/local/ nor  c:/windows/temp/ */

	if((url->path != NULL) && (url->path[0]!='/') && (url->path[0]!='?') && (url->path[1]!=':')) { 
		/* Relative path */
		if(!strncmp(url->path,"./",2))
			url->filename = (char*)DpsStrdup(url->path + 2);
		else
			url->filename = (char*)DpsStrdup(url->path);
		url->path[0] = 0;
	}

	/* truncate path to query_string */
	/* and store query_string        */

	if((query=strrchr(url->path,'?'))){
		url->query_string = (char*)DpsStrdup(query);
		*(query) = 0;
	}
	
	DpsURLNormalizePath(url->path);
	
	/* Now find right '/' sign and copy the rest to filename */

	if((file=strrchr(url->path,'/'))&&(strcmp(file,"/"))){
		url->filename = (char*)DpsStrdup(file + 1);
		*(file+1)=0;
	}

	/* Now find right '/' sign and copy the rest to directory */

	if (file = strrchr(url->path,'/')) {
	  char *p_save = file;
	  for(file--; (file > url->path) && (*file != '/'); file--);
	  file++;
	  if (*file) {
	    *p_save = '\0';
	    url->directory = (char*)DpsStrdup(file);
	    *p_save = '/';
	  }
	}

	DPS_FREE(s);
	if (url->hostname != NULL) {
	  DpsRTrim(url->hostname, ".");
	  for (s = url->hostname; *s; s++) {
	    *s = dps_tolower(*s);
	    if (strchr(",'\";", (int)*s)) return DPS_URL_BAD;
	  }
	}
	if (url->hostinfo != NULL) {
	  DpsRTrim(url->hostinfo, ".");
	  s = strchr(url->hostinfo, '@');
	  for (s = (s == NULL) ? url->hostinfo : s + 1; *s; s++) *s = dps_tolower(*s);
	}
	if (url->schema != NULL) for (s = url->schema; *s; s++) *s = dps_tolower(*s);
	return DPS_OK;
}



char * DpsURLNormalizePath(char * str){
	char * s=str;
	char * q;
	char * d;

	/* Hide query string */

	if((q=strchr(s,'?'))){
		*q++='\0';
		if(!*q)q=NULL;
	}

	/* Remove all "/../" entries */

	while((d=strstr(str,"/../"))){
		char * p;
		
		if(d>str){
			/* Skip non-slashes */
			for(p=d-1;(*p!='/')&&(p>str);p--);
			
			/* Skip slashes */
			while((p>(str+1))&&(*(p-1)=='/'))p--;
		}else{
			/* We are at the top level and have ../  */
			/* Remove it too to avoid loops          */
			p=str;
		}
		dps_memmove(p,d+3,dps_strlen(d)-2);
	}

	/* Remove remove trailig "/.." */

	d=str+dps_strlen(str);
	if((d-str>2)&&(!strcmp(d-3,"/.."))){
		d-=4;
		while((d>str)&&(*d!='/'))d--;
		if(*d=='/')d[1]='\0';
		else	dps_strcpy(str,"/");
	}

	/* Remove all "/./" entries */
	
	while((d=strstr(str,"/./"))){
		dps_memmove(d,d+2,dps_strlen(d)-1);
	}

	/* Remove the trailing "/."  */
	
	if((d=str+dps_strlen(str))>(str+2))
		if(!strcmp(d-2,"/."))
			*(d-1)='\0';

	/* Remove all "//" entries */
	while((d=strstr(str,"//"))){
		dps_memmove(d,d+1,dps_strlen(d));
	}

	
	/* Replace "%7E" with "~"         */
	/* Actually it is to be done      */
	/* for all valid characters       */
	/* which do not require escaping  */
	/* However I'm lazy, do it for 7E */
	/* as the most often "abused"     */

	while((d=strstr(str,"%7E"))){
		*d='~';
		dps_memmove(d+1,d+3,dps_strlen(d+3)+1);
	}

	/* Restore query string */

	if(q){
		char * e=str+dps_strlen(str);
		*e='?';
		dps_memmove(e+1,q,dps_strlen(q)+1);
	}

	return str;
}
