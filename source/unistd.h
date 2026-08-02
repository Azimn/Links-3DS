#ifndef LINKS_3DS_COMPAT_UNISTD_H
#define LINKS_3DS_COMPAT_UNISTD_H

/*
 * Compatibility umbrella for upstream Links sources.
 *
 * The host-generated cfg.h can select code paths that assume standard POSIX
 * and C string declarations have already been exposed by links.h. Newlib does
 * not expose all of those declarations through the same transitive includes,
 * so the full 3DS build force-includes this header for upstream sources only.
 */
#include_next <unistd.h>
#include <string.h>

#endif
