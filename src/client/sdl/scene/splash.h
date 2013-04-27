
/* $Id: splash.h,v 1.4 2003/03/17 22:45:40 cipher Exp $ */

#ifndef IH_SDL_SCENE_SPLASH_H
#define IH_SDL_SCENE_SPLASH_H

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
void            IH_InitScene_Splash(void);
void            IH_ProcessScene_Splash(SDL_Event * event);
void            IH_RenderScene_Splash(void);

/* Data definitions.
*/

#endif /* IH_SDL_SCENE_SPLASH_H */