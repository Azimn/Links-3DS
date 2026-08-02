#ifndef LINKS_3DS_COMPAT_UNISTD_H
#define LINKS_3DS_COMPAT_UNISTD_H

/*
 * Compatibility prelude for upstream Links sources compiled with devkitARM.
 *
 * Links 2.30 is configured natively so its feature tests can run. Some target
 * translation units then rely on declarations that glibc exposed transitively,
 * while newlib/libctru requires their defining headers explicitly. The browser
 * build force-includes this file for upstream Links objects only.
 */
#include_next <unistd.h>

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>

#endif
