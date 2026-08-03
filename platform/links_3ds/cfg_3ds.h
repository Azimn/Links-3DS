#ifndef LINKS_3DS_CFG_3DS_H
#define LINKS_3DS_CFG_3DS_H

/* Target facts that must override host configure assumptions. */
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
