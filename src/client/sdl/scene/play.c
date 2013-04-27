
/* $Id: play.c,v 1.5 2003/03/17 22:45:40 cipher Exp $ */

/*
 * Copyright (c) 2003 Paul A. Schifferer
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "SDL.h"
#include "SDL_image.h"

#include "ironhells.h"
#include "sdl/scene/play.h"
#include "sdl/render/tile.h"
#include "sdl/render/icon.h"
#include "sdl/render/misc.h"
#include "sdl/render/overlay.h"
#include "sdl/render/text.h"

void
IH_InitScene_Play(void)
{
     ih.pointer = IH_POINTER_STANDARD;

     ih.screen = SDL_SetVideoMode(ih.desired_display_width,
                                  ih.desired_display_height,
                                  ih.display_depth, ih.display_flags);
     if(ih.screen == NULL)
     {
          fprintf(stderr,
                  "Couldn't set %dx%dx%d video mode: %s\n",
                  ih.display_width, ih.display_height, ih.display_depth,
                  SDL_GetError());
          SDL_Quit();
          exit(2);
     }

     ih.playing = TRUE;
     ih.display_width = ih.desired_display_width;
     ih.display_height = ih.desired_display_height;

     /* Reposition the icons.
      */
     IH_PositionIcons();
     IH_PositionMisc();
}

void
IH_ProcessScene_Play(SDL_Event * event)
{
}

void
IH_RenderScene_Play(void)
{
     fprintf(stderr, "IH_RenderScene_Play(): Rendering play screen.\n");

     IH_RenderTiles();
     IH_RenderIcons();
     IH_RenderMisc();
     IH_RenderOverlays();

     IH_RenderError();
}