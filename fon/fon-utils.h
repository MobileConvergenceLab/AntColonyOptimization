/** From "glib-dbus-sync" exmaple
 *
 */

/**
 * This program implements a GObject for a simple class, then
 * registers the object on the D-Bus and starts serving requests.
 *
 * This maemo code example is licensed under a MIT-style license,
 * that can be found in the file called "License" in the same
 * directory as this file.
 * Copyright (c) 2007 Nokia Corporation. All rights reserved.
 */
#ifndef FON_UTILS_H
#define FON_UTILS_H

#include <stdlib.h>
#include <glib.h>

#ifndef PROGNAME
#define PROGNAME "unnamed"
#endif /* PROGNAME */

#ifdef NO_DAEMON
#define dbg(fmtstr, args...) \
  (g_print(PROGNAME ":%s: " fmtstr "\n", __func__, ##args))
#else
#define dbg(dummy...)
#endif /* NO_DAEMON */

/**
 * Print out an error message and optionally quit (if fatal is TRUE)
 */
static inline void handleError(const char* msg, const char* reason,
                                         gboolean fatal) {
    g_printerr(PROGNAME ": ERROR: %s (%s)\n", msg, reason);
    if (fatal) {
        exit(EXIT_FAILURE);
    }
}


#endif /* FON_UTILS_H */
