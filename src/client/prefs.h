
/* $Id: prefs.h,v 1.3 2003/03/17 22:45:22 cipher Exp $ */

#ifndef IH_PREFS_H
#define IH_PREFS_H

/*
 * Copyright (c) 2003 Paul A. Schifferer
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"

/* Function prototypes.
 */
bool            IH_GetPrefs(void);
bool            IH_SavePrefs(void);

#endif /* IH_PREFS_H */