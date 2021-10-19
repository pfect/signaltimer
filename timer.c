/*
 * Timer to send signals
 *
 * Copyright (C) 2021  Pasi Patama <pasi@patama.net>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 * 
 */

#include <assert.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <inttypes.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <signal.h>
#include <dirent.h>
#include "log.h"
#include "ini.h"
#define INI_FILE "timer.ini"

int getprogrampid(char *targetbinary)
{
	const char* procdir = "/proc";
	size_t      processnamelen = 1024;
	char*       processname = calloc(1, processnamelen);
	int 		pid = -1;
	int 		foundpid=-1;

	DIR* dir = opendir(procdir);
	
	if (dir)
	{
	  struct dirent* de = 0;
	  while ((de = readdir(dir)) != 0)
	  {
		 if (strcmp(de->d_name, ".") == 0 || strcmp(de->d_name, "..") == 0)
			continue;

		 int res = sscanf(de->d_name, "%d", &pid);
		 if (res == 1)
		 {
			char cmdline_file[1024] = {0};
			sprintf(cmdline_file, "%s/%d/cmdline", procdir, pid);
			FILE* cmdline = fopen(cmdline_file, "r");
			if (getline(&processname, &processnamelen, cmdline) > 0)
			{
			   if (strstr(processname, targetbinary ) != 0)
			   {
				  foundpid = pid;
			   }
			}
			fclose(cmdline);
		 }
	  }
	  closedir(dir);
	}
   free(processname);
   return foundpid;
}

int main(int argc, char* argv[])
{
	
	char *targetbinary = NULL;
	char *signal = NULL;
	char *interval = NULL;	
	int pidfound=-1;
	int persistedpid=-1;
	int persistedinterval = 0;
	int timercount=0;
	
	log_set_level(LOG_INFO);
	
	ini_t *config = ini_load(INI_FILE);
	if ( config != NULL ) 
	{
		ini_sget(config, "timer", "targetbinary", NULL, &targetbinary);
		ini_sget(config, "timer", "signal", NULL, &signal);
		ini_sget(config, "timer", "interval", NULL, &interval);
		log_info("[%d] Timer v0.0 :: Target: %s Signal: %s Interval: %s ",getpid(),targetbinary,signal,interval);
	} 
	if ( config == NULL ) 
	{
		log_error("[%d] failed to load ini-file, exiting. ",getpid());
		return -1;
	}
	
	while ( 1 ) 
	{
		config = ini_load(INI_FILE);
		if ( config != NULL ) 
		{
			ini_sget(config, "timer", "interval", NULL, &interval);
			ini_free(config);
		}		
		
		if ( persistedinterval != atoi(interval) ) 
		{
			persistedinterval= atoi(interval);
			timercount=0;
			log_info("[%d] Updated interval: %s ",getpid(),interval);
		}
		
			log_debug("[%d] #1 ",getpid() ); // ***
		
		pidfound = getprogrampid(targetbinary);
		
			log_debug("[%d] #2 ",getpid() ); // ***
		
		if ( pidfound != -1 && persistedpid != pidfound ) {
			persistedpid = pidfound;
			log_info("[%d] Detected '%s' with PID: '%d' ",getpid(), targetbinary, pidfound );
			timercount = 0;
		} 
			
			log_debug("[%d] #3 ",getpid() ); // ***
			
		if ( pidfound == -1 && persistedpid != -1 ) 
		{
			log_info("[%d] Program PID (%d) is gone",getpid(),persistedpid );
			persistedpid = -1;
			timercount = 0;
		}
		
			log_debug("[%d] #4 ",getpid() ); // ***
		
		if ( persistedpid != -1 ) 
		{
			timercount++;
			if ( timercount == atoi(interval) )
			{
				int ret =  kill(persistedpid, atoi(signal) );
				log_info("[%d] Timer count: %d to %d => Sent SIGNAL: %d to PID: %d ",getpid(), timercount,atoi(interval),atoi(signal),persistedpid );
				timercount=0;
			}
		}
		
			log_debug("[%d] #5 ",getpid() ); // ***
		
		sleep (1);
	}
	
	return 0;
}






