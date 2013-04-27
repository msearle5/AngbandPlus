
/* $Id: play.h,v 1.2 2003/04/16 17:30:45 cipher Exp $ */

#ifndef IH_SCENE_PLAY_H
#define IH_SCENE_PLAY_H

/*
 * Copyright (c) 2003 Paul A. Schifferer
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

/* Function prototypes.
*/
void            IH_InitScene_Play(void);
void            IH_ProcessScene_Play(SDL_Event * event);
void            IH_RenderScene_Play(void);
void            IH_CleanupScene_Play(void);

/* Data definitions.
*/

#endif /* IH_SCENE_PLAY_H */