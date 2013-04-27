#ifndef IH_SDL_HELPER_H
#define IH_SDL_HELPER_H

/* File: sdl/helper.h */

/*
 * Copyright (c) 2003 Paul A. Schifferer
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"

inline errr SDL_GetPixel(SDL_Surface *f,
                         Uint32 x, Uint32 y,
                         Uint8 *r, Uint8 *g, Uint8 *b);
inline errr SDL_PutPixel(SDL_Surface *f,
                         Uint32 x, Uint32 y,
                         Uint8 r, Uint8 g, Uint8 b);
errr SDL_ScaleBlit(SDL_Surface *src,
                   SDL_Rect *sr,
                   SDL_Surface *dst,
                   SDL_Rect *dr);

#endif /* IH_SDL_HELPER_H */