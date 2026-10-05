#ifndef LINKS_3DS_COMPAT_3DS_H
#define LINKS_3DS_COMPAT_3DS_H

/* Central declaration and ABI compatibility layer for Links on libctru/newlib. */
#include <3ds.h>
#include <arpa/inet.h>
#include <dirent.h>
#include <errno.h>
#include <fcntl.h>
#include <math.h>
#include <netdb.h>
#include <netinet/in.h>
#include <signal.h>
#include <setjmp.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>

/*
 * Do not import host config.h into the ARM build. The host configure pass is
 * used only to generate Links' Makefile and object manifest. Target facts live
 * in cfg_3ds.h so Linux feature probes cannot silently select unsupported 3DS
 * code paths.
 */
#include "cfg_3ds.h"

#ifndef O_BINARY
#define O_BINARY 0
#endif

#ifndef MSG_NOSIGNAL
#define MSG_NOSIGNAL 0
#endif

#ifndef AI_ADDRCONFIG
#define AI_ADDRCONFIG 0
#endif

#endif
