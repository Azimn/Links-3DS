#ifndef LINKS_3DS_CFG_3DS_H
#define LINKS_3DS_CFG_3DS_H

/* Target facts that must override host configure assumptions. */

/*
 * The source archive is checksum-pinned to Links 2.30. These values normally
 * come from a native configure-generated config.h, which must not be reused
 * as a target capability map for libctru.
 */
#ifndef VERSION
#define VERSION "2.30"
#endif
#ifndef DEBUGLEVEL
#define DEBUGLEVEL 0
#endif

#ifndef HAVE_DIRENT_H
#define HAVE_DIRENT_H 1
#endif
#ifndef HAVE_SYS_TIME_H
#define HAVE_SYS_TIME_H 1
#endif
#ifndef HAVE_GETTIMEOFDAY
#define HAVE_GETTIMEOFDAY 1
#endif
#ifndef HAVE_STRUCT_TIMEZONE
#define HAVE_STRUCT_TIMEZONE 1
#endif

/* libctru exposes the standard socket length ABI. */
#ifndef HAVE_SOCKLEN_T
#define HAVE_SOCKLEN_T 1
#endif

/* Graphics mode uses the devkitPro 3ds-libpng portlib. */
#ifndef HAVE_PNG_H
#define HAVE_PNG_H 1
#endif

#ifndef HAVE_SETJMP_H
#define HAVE_SETJMP_H 1
#endif
#ifndef HAVE_SYS_WAIT_H
#define HAVE_SYS_WAIT_H 1
#endif

/* The 3DS homebrew runtime has no process creation model. */
#ifdef HAVE_FORK
#undef HAVE_FORK
#endif
#ifdef HAVE_VFORK
#undef HAVE_VFORK
#endif
#ifdef HAVE_EXECVE
#undef HAVE_EXECVE
#endif
#ifdef HAVE_WAITPID
#undef HAVE_WAITPID
#endif

/* Graphics are supplied exclusively by links_3ds_driver. */
#ifdef HAVE_X
#undef HAVE_X
#endif
#ifdef HAVE_DIRECTFB
#undef HAVE_DIRECTFB
#endif
#ifdef HAVE_SVGALIB
#undef HAVE_SVGALIB
#endif
#ifdef HAVE_FB
#undef HAVE_FB
#endif

#endif
