
/* $Id: ironhells.h,v 1.15 2003/03/24 06:04:50 cipher Exp $ */

#ifndef IH_IRONHELLS_H
#define IH_IRONHELLS_H

/*
 * Copyright (c) 2003 Paul A. Schifferer
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#ifdef HAVE_CONFIG_H
#include "autoconf.h"
#endif

#include "SDL.h"
#include "SDL_thread.h"
#ifdef USE_SDLTTF
#include "SDL_ttf.h"
#else
#include "sdl/bfont.h"
#endif

#include "list.h"
#include "sdl/render/pointer.h"
#include "sdl/render/tile.h"

struct IronHells
{
     /* General display variables.
      */
     SDL_Surface    *screen;
     SDL_Surface    *shader;
     int             is_fullscreen;
     int             desired_display_width, desired_display_height; // screen resolution
     int             display_width, display_height; // screen resolution
     int             display_depth;
     int             display_flags;

     int             mouse_x, mouse_y;

     /* Rendering variables.
      */
     int             scene;
     int             stage;
     int             changing_scene;
     int             scene_dirty;

     int             icon_size;

     SDL_Surface    *tile_array[IH_MAX_TILES];
     SDL_Surface    *object_array[IH_MAX_OBJECTS];
     SDL_Surface    *creature_array[IH_MAX_CREATURES];

#ifdef USE_SDLTTF
     TTF_Font       *normal_font;
     TTF_Font       *large_font;
#else
     BFont_Info     *normal_font;
     BFont_Info     *large_font;
#endif

     /* Rendering object containers.
      */
     ihList          icons;
     ihList          tiles;
     ihList          objects;
     ihList          creatures;
     ihList          misc;
     int             pointer;
     SDL_Surface    *splash;
     SDL_Surface    *background;
     char           *load_message;

     int             err_shown;
     char           *err_message;

     /* Thread communication variables.
      */
     SDL_Thread     *game_thread;
     struct
     {
          SDL_sem        *msg;  // for display messages
          SDL_sem        *map;  // for the game map
          SDL_sem        *player; // for the player data structures
          SDL_sem        *scene; // for scene transition
          SDL_sem        *talk; // for inter-thread communication
          SDL_sem        *overlay; // for the overlay list
     }
     sem;
     struct
     {
          int             valid;
          int             type;
          union
          {
               long            value;
               void           *ptr;
               SDL_Event       event;
          }
          data;
     }
     ipc;

     /* Miscellaneous variables.
      */
     int             done;
     int             playing;
     int             new_game;

     int             messages_shown;

     /* Variables for dialog overlays.
      */
     const byte     *dialog_prompt;
     byte           *input_buffer;
     int             input_len;
};

enum
{
     IH_ERROR_NONE,
     IH_ERROR_INVALID_DATA,
     IH_ERROR_SDL_VIDEO_ERROR,
     IH_ERROR_SDL_AUDIO_ERROR,
     IH_ERROR_CANT_LOAD_POINTER,
     IH_ERROR_CANT_LOAD_ICON,
     IH_ERROR_CANT_LOAD_TILE,
     IH_ERROR_CANT_LOAD_IMAGE,
     IH_ERROR_CANT_LOAD_FONT,
     IH_ERROR_NO_MANIFEST,

     IH_ERROR_MAX
};

#define IH_IMAGE_FORMAT_EXT "png"

#define IH_ICON_SIZE_SMALL_VALUE  32
#define IH_ICON_SIZE_MEDIUM_VALUE 40
#define IH_ICON_SIZE_LARGE_VALUE  64

#define IH_ALPHA_VALUE 64

#define IH_COLOR_BORDER_RED   80
#define IH_COLOR_BORDER_GREEN 80
#define IH_COLOR_BORDER_BLUE  80

#ifndef IH_MAIN
extern struct IronHells ih;
#endif

#endif /* IH_IRONHELLS_H */
