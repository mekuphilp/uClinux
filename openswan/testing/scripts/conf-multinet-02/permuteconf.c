/*
 * This program reads a configuration file and then calls permute_conn
 *
 * This is a unit test case.
 *
 * Copyright (C) 2006 Michael Richardson <mcr@xelerance.com>
 * 
 * This program is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation; either version 2 of the License, or (at your
 * option) any later version.  See <http://www.fsf.org/copyleft/gpl.txt>.
 * 
 * This program is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 * or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * for more details.
 */

char readwriteconf_c_version[] = "@(#) Xelerance Openswan readwriteconf";

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
/* #include <linux/netdevice.h> */
#include <net/if.h>
/* #include <linux/types.h> */ /* new */
#include <sys/stat.h>
#include <limits.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>

/* #include <sys/socket.h> */

#include <netinet/in.h>
#include <arpa/inet.h>
/* #include <linux/ip.h> */
#include <netdb.h>

#include <unistd.h>
#include <getopt.h>
#include <ctype.h>
#include <stdio.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <openswan.h>

#include "sysdep.h"
#include "constants.h"
#include "oswalloc.h"
#include "oswconf.h"
#include "oswlog.h"
#include "ipsecconf/confread.h"
#include "ipsecconf/confwrite.h"
#include "ipsecconf/starterlog.h"
#include "ipsecconf/files.h"
#include "ipsecconf/starterwhack.h"

char *progname;
int verbose=0;
int warningsarefatal = 0;

static const char *usage_string = ""
    "Usage: permuteconn [--config file] \n";

static void usage(void)
{
    /* print usage */
    fputs(usage_string, stderr);
    exit(10);
}

extern char rootdir[PATH_MAX];       /* when evaluating paths, prefix this to them */
extern char rootdir2[PATH_MAX];       /* when evaluating paths, prefix this to them */

static struct option const longopts[] =
{
	{"config",              required_argument, NULL, 'C'},
	{"debug",               no_argument, NULL, 'D'},
	{"verbose",             no_argument, NULL, 'D'},
	{"rootdir",             required_argument, NULL, 'R'},
	{"rootdir2",            required_argument, NULL, 'S'},
	{"help",                no_argument, NULL, 'h'},
	{0, 0, 0, 0}
};


static int starter_print_connname(struct starter_config *cfg
				  , struct starter_conn *conn)
{
	char lt[SUBNETTOT_BUF], rt[SUBNETTOT_BUF];

	subnettot(&conn->left.subnet,  0, lt, SUBNETTOT_BUF);
	subnettot(&conn->right.subnet, 0, rt, SUBNETTOT_BUF);
	printf("conn name=%s leftsubnet=%s rightsubnet=%s\n",
	       conn->name, lt, rt);

	return 0; /* success */
}

int
main(int argc, char *argv[])
{
    int opt = 0;
    struct starter_config *cfg = NULL;
    err_t err = NULL;
    char *confdir = NULL;
    char *configfile = NULL;
    struct starter_conn *conn = NULL;

    progname = argv[0];
    rootdir[0]='\0';

    tool_init_log();

    while((opt = getopt_long(argc, argv, "", longopts, 0)) != EOF) {
	switch(opt) {
	case 'h':
	    //usage:
	    usage();
	    break;

	case 'D':
	    verbose++;
	    break;

	case 'W':
	    warningsarefatal++;
	    break;

	case 'C':
	    configfile = clone_str(optarg, "config file name");
	    break;

	case 'R':
	    printf("#setting rootdir=%s\n", optarg);
	    strncat(rootdir, optarg, sizeof(rootdir));
	    break;

	case 'S':
	    printf("#setting rootdir2=%s\n", optarg);
	    strncat(rootdir2, optarg, sizeof(rootdir2));
	    break;
	}
    }

    /* find config file */
    confdir = getenv(IPSEC_CONFDIR_VAR);
    if(confdir == NULL)
    {
	confdir = IPSEC_CONFDIR;
    }
	
    if(!configfile) {
	configfile = alloc_bytes(strlen(confdir)+sizeof("/ipsec.conf")+2,"conf file");

	/* calculate default value for configfile */
	configfile[0]='\0';
	strcpy(configfile, confdir);
	if(configfile[strlen(configfile)-1]!='/')
	{
	    strcat(configfile, "/");
	}
	strcat(configfile, "ipsec.conf");
    }

    if(verbose) {
	printf("opening file: %s\n", configfile);
    }

    starter_use_log (verbose, 1, verbose ? 0 : 1);

    cfg = confread_load(configfile, &err, NULL);
    
    if(!cfg) {
	printf("config file: %s can not be loaded: %s\n", configfile, err);
	exit(3);
    }
    
    /* load all conns marked as auto=add or better */
    for(conn = cfg->conns.tqh_first;
	conn != NULL;
	conn = conn->link.tqe_next)
    {
	    /* basic case, nothing special to synthize! */
	    if(!conn->left.strings_set[KSCF_SUBNETS] &&
	       !conn->right.strings_set[KSCF_SUBNETS]) {
		    continue;
	    }
	    
	    starter_permutate_conns(starter_print_connname
				    , cfg, conn);
    }

    exit(0);
}

void exit_tool(int x)
{
  exit(x);
}

/*
 * Local Variables:
 * c-basic-offset:4
 * c-style: pluto
 * End:
 *
 */
