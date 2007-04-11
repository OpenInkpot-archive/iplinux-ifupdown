#line 2972 "ifupdown.nw"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>
#include <assert.h>

#include "header.h"
#line 3011 "ifupdown.nw"
#include <unistd.h>
#include <fcntl.h>
#line 3137 "ifupdown.nw"
#include <getopt.h>
#line 3409 "ifupdown.nw"
#include <fnmatch.h>
#line 3166 "ifupdown.nw"
int no_act = 0;
int verbose = 0;
#line 3184 "ifupdown.nw"
static void usage(char *execname);
static void help(char *execname);
static void version(char *execname);
#line 3563 "ifupdown.nw"
static int lookfor_iface(char **ifaces, int n_ifaces, char *iface);
#line 3581 "ifupdown.nw"
static void add_to_state(char ***ifaces, int *n_ifaces, int *max_ifaces, 
                         char *new_iface);
#line 3606 "ifupdown.nw"
static int lock_fd (int fd);
#line 3190 "ifupdown.nw"
static void usage(char *execname) {
	fprintf(stderr, "%s: Use --help for help\n", execname);
	exit(1);
}
#line 3197 "ifupdown.nw"
static void version(char *execname) {
	printf("%s version " IFUPDOWN_VERSION "\n", execname);
	printf("Copyright (c) 1999-2005 Anthony Towns\n\n");
	printf(

"This program is free software; you can redistribute it and/or modify\n"
"it under the terms of the GNU General Public License as published by\n"
"the Free Software Foundation; either version 2 of the License, or (at\n"
"your option) any later version.\n"

	);
	exit(0);
}
#line 3213 "ifupdown.nw"
static void help(char *execname) {
	printf("Usage: %s <options> <ifaces...>\n\n", execname);
	printf("Options:\n");
	printf("\t-h, --help\t\tthis help\n");
	printf("\t-V, --version\t\tcopyright and version information\n");
	printf("\t-a, --all\t\tde/configure all interfaces marked \"auto\"\n");
	printf("\t--allow CLASS\t\tignore non-\"allow-CLASS\" interfaces\n");
	printf("\t-i, --interfaces FILE\tuse FILE for interface definitions\n");
	printf("\t-n, --no-act\t\tprint out what would happen, but don't do it\n");
	printf("\t\t\t\t(note that this option doesn't disable mappings)\n");
	printf("\t-v, --verbose\t\tprint out what would happen before doing it\n");
	printf("\t--no-mappings\t\tdon't run any mappings\n");
	printf("\t--force\t\t\tforce de/configuration\n");
	exit(0);
}
#line 3567 "ifupdown.nw"
static int lookfor_iface(char **ifaces, int n_ifaces, char *iface) {
	int i;
	for (i = 0; i < n_ifaces; i++) {
		if (strncmp(iface, ifaces[i], strlen(iface)) == 0) {
			if (ifaces[i][strlen(iface)] == '=') {
				return i;
			}
		}
	}
	return -1;
}
#line 3586 "ifupdown.nw"
static void add_to_state(char ***ifaces, int *n_ifaces, int *max_ifaces, 
                         char *new_iface)
{
	assert(*max_ifaces >= *n_ifaces);
	if (*max_ifaces == *n_ifaces) {
		*max_ifaces = (*max_ifaces * 2) + 1;
		*ifaces = realloc(*ifaces, sizeof(**ifaces) * *max_ifaces);
		if (*ifaces == NULL) {
			perror("realloc");
			exit(1);
		}
	}

	(*ifaces)[(*n_ifaces)++] = new_iface;
}
#line 3610 "ifupdown.nw"
static int lock_fd (int fd) {
	struct flock lock;

	lock.l_type = F_WRLCK;
	lock.l_whence = SEEK_SET;
	lock.l_start = 0;
	lock.l_len = 0;

	if  (fcntl(fd, F_SETLKW, &lock) < 0) {
		return -1;
	}

	return 0;
}
#line 2989 "ifupdown.nw"
int main(int argc, char **argv) {
	
#line 3047 "ifupdown.nw"
int (*cmds)(interface_defn *) = NULL;
#line 3141 "ifupdown.nw"
struct option long_opts[] = {
	{"help",        no_argument,       NULL, 'h'},
	{"version",     no_argument,       NULL, 'V'},
	{"verbose",     no_argument,       NULL, 'v'},
	{"all",         no_argument,       NULL, 'a'},
	{"allow",	required_argument, NULL,  3 },
	{"interfaces",  required_argument, NULL, 'i'},
	{"exclude",     required_argument, NULL, 'e'},
	{"no-act",      no_argument,       NULL, 'n'},
	{"no-mappings", no_argument,       NULL,  1 },
	{"force",       no_argument,       NULL,  2 },
	{0,0,0,0}
};
#line 3171 "ifupdown.nw"
int do_all = 0;
int run_mappings = 1;
int force = 0;
char *allow_class = NULL;
char *interfaces = "/etc/network/interfaces";
char *statefile = "/etc/network/run/ifstate";
char *excludeint = NULL ;
#line 3338 "ifupdown.nw"
interfaces_file *defn;
#line 3497 "ifupdown.nw"
int n_target_ifaces;
char **target_iface;
#line 3554 "ifupdown.nw"
char **state = NULL; /* list of iface=liface */
int n_state = 0;
int max_state = 0;
#line 3628 "ifupdown.nw"
static FILE *state_fp = NULL;

#line 2992 "ifupdown.nw"
	
#line 3016 "ifupdown.nw"
{
	int i;
	for (i = 0; i <= 2; i++) {
		if (fcntl(i, F_GETFD) == -1) {
			if (errno == EBADF && open("/dev/null", 0) == -1) {
				fprintf(stderr,
					"%s: fd %d not available; aborting\n",
					argv[0], i);
				exit(2);
			} else if (errno == EBADF) {
				errno = 0; /* no more problems */
			} else {
				/* some other problem -- eeek */
				perror(argv[0]);
				exit(2);
			}
		}
	}
}

#line 2994 "ifupdown.nw"
	
#line 3053 "ifupdown.nw"
{
	char *command;

	
#line 3064 "ifupdown.nw"
if ((command = strrchr(argv[0],'/'))) {
	command++; /* first char after / */
} else {
	command = argv[0]; /* no /'s in argv[0] */
}
#line 3057 "ifupdown.nw"
	
#line 3072 "ifupdown.nw"
if (strcmp(command, "ifup")==0) {
	cmds = iface_up;
} else if (strcmp(command, "ifdown")==0) {
	cmds = iface_down;
} else {
	fprintf(stderr,"This command should be called as ifup or ifdown\n");
	exit(1);
}
#line 3058 "ifupdown.nw"
}
#line 2995 "ifupdown.nw"
	
#line 3234 "ifupdown.nw"
for(;;) {
	int c;
	c = getopt_long(argc, argv, "e:s:i:hVvna", long_opts, NULL);
	if (c == EOF) break;

	switch(c) {
		
#line 3252 "ifupdown.nw"
case 'i':
	interfaces = strdup(optarg);
	break;
#line 3257 "ifupdown.nw"
case 'v':
	verbose = 1;
	break;
#line 3262 "ifupdown.nw"
case 'a':
	do_all = 1;
	break;
#line 3267 "ifupdown.nw"
case 3:
	allow_class = strdup(optarg);
	break;
#line 3272 "ifupdown.nw"
case 'n':
	no_act = 1;
	break;
#line 3277 "ifupdown.nw"
case 1:
	run_mappings = 0;
	break;
#line 3282 "ifupdown.nw"
case 2:
	force = 1;
	break;
#line 3287 "ifupdown.nw"
case 'e':
	excludeint = strdup(optarg);
	break;
#line 3295 "ifupdown.nw"
case 'h':
	help(argv[0]);
	break;
#line 3300 "ifupdown.nw"
case 'V':
	version(argv[0]);
	break;
#line 3309 "ifupdown.nw"
default:
	usage(argv[0]);
	break;
#line 3241 "ifupdown.nw"
	}
}

#line 3321 "ifupdown.nw"
if (argc - optind > 0 && do_all) {
	usage(argv[0]);
}
#line 3327 "ifupdown.nw"
if (argc - optind == 0 && !do_all) {
	usage(argv[0]);
}

#line 2997 "ifupdown.nw"
	
#line 3342 "ifupdown.nw"
defn = read_interfaces(interfaces);
if ( !defn ) {
	fprintf(stderr, "%s: couldn't read interfaces file \"%s\"\n",
		argv[0], interfaces);
	exit(1);
}

#line 2999 "ifupdown.nw"
	
#line 3632 "ifupdown.nw"
{
	state_fp = fopen(statefile, no_act ? "r" : "a+");
	if (state_fp == NULL && !no_act) {
		fprintf(stderr, 
			"%s: failed to open statefile %s: %s\n",
			argv[0], statefile, strerror(errno));
		exit (1);
	}

	if (state_fp != NULL) {
		char buf[80];
		char *p;

		if (!no_act) {
			int flags;

			if ((flags = fcntl(fileno(state_fp), F_GETFD)) < 0
			    || fcntl(fileno(state_fp), F_SETFD, flags | FD_CLOEXEC) < 0) {
				fprintf(stderr, 
					"%s: failed to set FD_CLOEXEC on statefile %s: %s\n",
					argv[0], statefile, strerror(errno));
				exit(1);
			}

			if (lock_fd (fileno(state_fp)) < 0) {
				fprintf(stderr, 
					"%s: failed to lock statefile %s: %s\n",
					argv[0], statefile, strerror(errno));
				exit(1);
			}

		}

		rewind (state_fp);
		while((p = fgets(buf, sizeof buf, state_fp)) != NULL) {
			char *pch;

			pch = buf + strlen(buf) - 1;
			while(pch > buf && isspace(*pch)) pch--;
			*(pch+1) = '\0';

			pch = buf;
			while(isspace(*pch)) pch++;

			add_to_state(&state, &n_state, &max_state, strdup(pch));
		}
	}
}
#line 3508 "ifupdown.nw"
if (do_all) {
	if (
#line 3087 "ifupdown.nw"
(cmds == iface_up)
#line 3509 "ifupdown.nw"
                                     ) {
		allowup_defn *autos = find_allowup(defn, "auto");
		target_iface = autos ? autos->interfaces : NULL;
		n_target_ifaces = autos ? autos->n_interfaces : 0;
	} else if (
#line 3091 "ifupdown.nw"
(cmds == iface_down)
#line 3513 "ifupdown.nw"
                                            ) {
		target_iface = state;
		n_target_ifaces = n_state;
	} else {
		assert(0);
	}	
} else {
	target_iface = argv + optind;
	n_target_ifaces = argc - optind;
}
#line 3357 "ifupdown.nw"
{
	int i;
	for (
#line 3502 "ifupdown.nw"
i = 0; i < n_target_ifaces; i++
#line 3359 "ifupdown.nw"
                                      ) {
		char iface[80], liface[80];

		
#line 3526 "ifupdown.nw"
strncpy(iface, target_iface[i], sizeof(iface));
iface[sizeof(iface)-1] = '\0';

{
	char *pch;
	if ((pch = strchr(iface, '='))) {
		*pch = '\0';
		strncpy(liface, pch+1, sizeof(liface));
		liface[sizeof(liface)-1] = '\0';
	} else {
		strncpy(liface, iface, sizeof(liface));
		liface[sizeof(liface)-1] = '\0';
	}
}
#line 3363 "ifupdown.nw"
		if (!force) {
			
#line 3715 "ifupdown.nw"
{
	int already_up = lookfor_iface(state, n_state, iface);;

	if (
#line 3087 "ifupdown.nw"
(cmds == iface_up)
#line 3718 "ifupdown.nw"
                                     ) {
		if (already_up != -1) {
			fprintf(stderr, 
				"%s: interface %s already configured\n",
				argv[0], iface);
			continue;
		}
	} else if (
#line 3091 "ifupdown.nw"
(cmds == iface_down)
#line 3725 "ifupdown.nw"
                                            ) {
		if (already_up == -1) {
			fprintf(stderr, "%s: interface %s not configured\n",
				argv[0], iface);
			continue;
		}
		strncpy(liface, strchr(state[already_up], '=') + 1, 80);
		liface[79] = 0;
	} else {
		assert(0);
	}
}
#line 3365 "ifupdown.nw"
		}

		if (
#line 3100 "ifupdown.nw"
(allow_class != NULL)

#line 3367 "ifupdown.nw"
                                                    ) {
			
#line 3103 "ifupdown.nw"
{
	int i;
	allowup_defn *allowup = find_allowup(defn, allow_class);
	if (allowup == NULL)
		continue;

	for (i = 0; i < allowup->n_interfaces; i++) {
		if (strcmp(allowup->interfaces[i], iface) == 0)
			break;
	}
	if (i >= allowup->n_interfaces)
		continue;
}
#line 3369 "ifupdown.nw"
		}

		if (
#line 3125 "ifupdown.nw"
(excludeint != NULL && strstr(iface,excludeint) != NULL)
#line 3371 "ifupdown.nw"
                                               )  
			continue;

		if (
#line 3087 "ifupdown.nw"
(cmds == iface_up)
#line 3374 "ifupdown.nw"
                                       && run_mappings) {
			
#line 3392 "ifupdown.nw"
{
	mapping_defn *currmap;
	for (currmap = defn->mappings; currmap; currmap = currmap->next) {
		int i;
		for (i = 0; i < currmap->n_matches; i++) {
			
#line 3413 "ifupdown.nw"
if (fnmatch(currmap->match[i], liface, 0) != 0)
	continue;
#line 3398 "ifupdown.nw"
			
#line 3421 "ifupdown.nw"
if (verbose) {
	fprintf(stderr, "Running mapping script %s on %s\n",
		currmap->script, liface);
}
run_mapping(iface, liface, sizeof(liface), currmap);
#line 3399 "ifupdown.nw"
			break;
		}
	}
}
#line 3376 "ifupdown.nw"
		}

		
#line 3431 "ifupdown.nw"
{
	interface_defn *currif;
	int okay = 0;
	int failed = 0; 
	for (currif = defn->ifaces; currif; currif = currif->next) {
		if (strcmp(liface, currif->logical_iface) == 0) {
			okay = 1;

			
#line 3457 "ifupdown.nw"
{
	currif->real_iface = iface;

	if (verbose) {
		fprintf(stderr, "Configuring interface %s=%s (%s)\n", 
			iface, liface, currif->address_family->name);
	}

	switch(cmds(currif)) {
	    case -1:
		printf("Don't seem to be have all the variables for %s/%s.\n", 
			liface, currif->address_family->name);
		failed = 1;
		break;
	    case 0:
		failed = 1;
		break;
		/* not entirely successful */
	    case 1:
	    	failed = 0;
		break;
		/* successful */
	    default:
	    	printf("Internal error while configuring interface %s/%s (assuming it failed)\n", 
			liface, currif->address_family->name);
	    	failed = 1;
		/* what happened here? */
	}
	currif->real_iface = NULL;
}

#line 3441 "ifupdown.nw"
			if (failed) break;
			/* Otherwise keep going: this interface may have
			 * match with other address families */
		}
	}

	if (!okay && !force) {
		fprintf(stderr, "Ignoring unknown interface %s=%s.\n", 
			iface, liface);
	} else {
		
#line 3743 "ifupdown.nw"
{
	int already_up = lookfor_iface(state, n_state, iface);

	if (
#line 3087 "ifupdown.nw"
(cmds == iface_up)
#line 3746 "ifupdown.nw"
                                     ) {
		char *newiface = 
			malloc(strlen(iface) + 1 + strlen(liface) + 1);
		sprintf(newiface, "%s=%s", iface, liface);

		if (already_up == -1) {
			if (failed == 1) {
				printf("Failed to bring up %s.\n", liface);
			} else {
				add_to_state(&state, &n_state, &max_state, newiface);
			}
		} else {
			free(state[already_up]);
			state[already_up] = newiface;
		}
	} else if (
#line 3091 "ifupdown.nw"
(cmds == iface_down)
#line 3761 "ifupdown.nw"
                                            ) {
		if (already_up != -1) {
			state[already_up] = state[--n_state];
		}
	} else {
		assert(0);
	}
}
#line 3452 "ifupdown.nw"
	}
}
#line 3379 "ifupdown.nw"
		
#line 3691 "ifupdown.nw"
if (state_fp != NULL && !no_act) {
	int i;

	if (ftruncate(fileno(state_fp), 0) < 0)
	{
		fprintf(stderr, 
			"%s: failed to truncate statefile %s: %s\n",
			argv[0], statefile, strerror(errno));
		exit(1);
	}

	rewind(state_fp);
	for (i = 0; i < n_state; i++) {
		fprintf(state_fp, "%s\n", state[i]);
	}
	fflush(state_fp);
}
#line 3380 "ifupdown.nw"
	}
}
#line 3683 "ifupdown.nw"
if (state_fp != NULL) {
	fclose(state_fp);
	state_fp = NULL;
}

#line 3001 "ifupdown.nw"
	return 0;
}
