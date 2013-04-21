/*
 * DESC: main-sdl.c - langband-backend for SDL
 * Copyright (c) 2002-2003 - Stig Erik Sand�

 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * -----
 *
 * Has been adapted from Greg Velichansky's SDL visual module for regular
 * Angband:
 *
 * Copyright 2001 Gregory Velichansky (hmaon@bumba.net)
 * You may use it under the terms of the standard Angband license (below) or
 * the GNU General Public License (GPL) Version 2 or greater. (see below)
 * Ben Harrison's copyright appears in the file COPYING and at last
 * check allows you to use his code under the GPL as well.
 *
 * The Angband license states:
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 * 
 * The GNU GPL notice:
   main-sdl.c - SDL (http://libsdl.org) display module for Angband.
	Copyright (C) 2001  Gregory Velichansky (hmaon@bumba.net)
	Portions Copyright (C) 1997 Ben Harrison
	(see the file COPYING in the latest distribution of the Angband source code)

	This program is free software; you can redistribute it and/or
	modify it under the terms of the GNU General Public License
	as published by the Free Software Foundation; either version 2
	of the License, or (at your option) any later version.

	This program is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
	GNU General Public License for more details.

	You should have received a copy of the GNU General Public License
	along with this program; if not, please see 
	http://www.gnu.org/copyleft/gpl.html

*/

#include "autoconf.h"

#ifdef USE_SDL

#include "langband.h"
#include "lbwindows.h"
#include "lbsound.h"
#include "lbtools.h"


/* 
 *
 * Pre-processor configuration, data structure definitions, global variables,
 * and #includes.
 *
 * The #defines could be moved into config.h if this module was a standard
 * feature of Angband, if people find that helpful.
 *
 * If certain parts of this file are moved to a separate file, the data
 * structures would possibly need to be moved to a header file. That's probably
 * not a good idea since no other display module works that way.
 *
 */

#include "SDL_image.h"
//#include "SDL_mixer.h"
#include <string.h>

struct sdl_winconnection {

    SDL_Surface *face; // pointer to the paint-frame
    FontData *font_data; // pointer to the font-information for this frame
    graf_tiles *gt; // pointer to the graphical tiles this frame should use
    SDL_Surface *background; // pointer to the background surface

    Sint32 cx, cy; /* last known cursor coordinates */

};

typedef struct sdl_winconnection sdl_winconnection;

//extern int strtoii(const char *str, Uint32 *w, Uint32 *h);
extern char *formatsdlflags(Uint32 flags);

extern void Multikeypress(char *k);
extern int IsMovement(SDLKey k);
extern char *SDL_keysymtostr(SDL_keysym *ks); /* this is the important one. */

extern int SDL_init_screen_cursor(Uint32 w, Uint32 h);
extern int SDL_DrawCursor(SDL_Surface *dst, SDL_Rect *dr);


#ifndef SDL_DISABLE
#define SDL_DISABLE 0
#endif


#define CURS_MAG_X 0
#define CURS_MAG_Y 21


/*
 * Extra data to associate with each "window"
 *
 * Each "window" is represented by a "term_data" structure, which
 * contains a "term" structure, which contains a pointer (t->data)
 * back to the term_data structure.
 *
 * A font_data struct keeps the SDL_Surface and other info for tile graphics
 * which include fonts.
 */



static SDL_Surface *theWindow = NULL;

static TileInformation *tileInfo = NULL;

//FontData *screen_font = NULL;
static graf_tiles *screen_tiles = NULL;

/* color data copied straight from main-xxx.c */
SDL_Color sdl_colour_data[16] =
{
/* 0 */	{0, 0, 0, 0}, 
	{4, 4, 4, 0}, 
	{2, 2, 2, 0}, 
	{4, 2, 0, 0}, 
	{3, 0, 0, 0}, 
/* 5 */	{0, 2, 1, 0}, 
	{0, 0, 4, 0}, 
	{2, 1, 0, 0}, 
	{1, 1, 1, 0}, 
	{3, 3, 3, 0}, 
/* 10 */{4, 0, 4, 0}, 
	{4, 4, 0, 0}, 
	{4, 0, 0, 0}, 
	{0, 4, 0, 0}, 
	{0, 4, 4, 0}, 
	{3, 2, 1, 0}
};


static void
sdl_init_tile_information(TileInformation *ti, int arrsize) {
    int i = 0;
    
    if (!ti) return;
    if (arrsize < 1) return;

    ti->array_size = arrsize;
    ti->num_tiles = 0;
    ti->tile_width = 0;
    ti->tile_height = 0;

    ti->tiles = malloc(arrsize * sizeof(SDL_Surface*));
    ti->tile_files = malloc(arrsize * sizeof(char*));
    ti->tile_columns = malloc(arrsize * sizeof(int));
    
    for (i=0; i < ti->array_size; i++) {
	ti->tiles[i] = NULL;
	ti->tile_files[i] = NULL;
	ti->tile_columns[i] = 0;
    }
}

int
sdl_find_image(const char *fname) {
    int i = 0;
    //term_data *td = &loc_terms[i];
    
    for (i=0; i < tileInfo->num_tiles; i++) {
	//DBGPUT("Checking %s vs %s\n", fname, tileInfo->tile_files[i]);
	if (tileInfo->tile_files[i] &&
	    !strcmp(tileInfo->tile_files[i], fname)) {
	    return i;
	}
    }
    return -1;
}


static int
sdl_load_image_data(const char *filename, int image_index, int tiled,
		int tile_width, int tile_height, unsigned int transcolour) {
    SDL_Surface *surf = NULL;
    surf = IMG_Load(filename);

    // to avoid warnings
    tiled = tile_width = tile_height = 0;
    
    if (surf) {
        int idx = image_index;

	// small hack if you wish to test transparency on white colour
	if (transcolour) {
	    Uint32 trans = SDL_MapRGB(surf->format, 255, 255, 255);
	    SDL_SetColorKey(surf, SDL_SRCCOLORKEY, trans);
	}
	
	tileInfo->tiles[idx] = surf;
	tileInfo->tile_files[idx] = malloc(strlen(filename) + 1);
	strcpy(tileInfo->tile_files[idx], filename); // fix!
	//DBGPUT("Returning idx %d for %s\n", idx, filename);
	return idx;
    }
    else {
	ERRORMSG("Was unable to load image %s\n", filename);
	return -1;
    }
}

int
sdl_load_plain_image(const char *filename, int image_index, unsigned int transcolour) {
    return sdl_load_image_data(filename,image_index, 0, -1, -1, transcolour);
}

int
sdl_get_image_width(int idx) {
    int max = tileInfo->array_size;
    if (idx >= 0 && idx < max) {
	return tileInfo->tiles[idx]->w;
    }
    else {
	return -1;
    }
}

int
sdl_get_image_height(int idx) {
    int max = tileInfo->array_size;
    if (idx >= 0 && idx < max) {
	return tileInfo->tiles[idx]->h;
    }
    else {
	return -1;
    }
}


int
sdl_load_gfx_image(const char *fname, int idx, unsigned int transcolour) {

    // char filename[1024];

    // sprintf(filename, "%s%s/%s", base_gfx_path, type , fname);

    if (idx < 0) {
	return -1;
    }

    if (idx >= 0) {
	return sdl_load_plain_image(fname, idx, transcolour);
    }
    else {
	ERRORMSG("Somehow loading of image %s screwed up.", fname);
	return -1;
    }
}


/*#define SCALETOCOLOR(x) (x=((x)*63+((x)-1)))*/
#define ScaleToColor(x) ((x)=((x)*60)+15)
/*#define ScaleToColor(x) ((x)=((x)*63))*/
void sdl_init_colour_data() {
    Uint8 i;
    static int already_done_init = 0;
    
    // we only wish to do this once!
    if (already_done_init) return;
    
    //DBGPUT("INIT COLOURS!!!\n");
    for (i = 0; i < 16; ++i) {
	sdl_colour_data[i].unused = 255; /* no reason. */
	if(!sdl_colour_data[i].r && !sdl_colour_data[i].g && !sdl_colour_data[i].b)
	    continue;
	ScaleToColor(sdl_colour_data[i].r);
	ScaleToColor(sdl_colour_data[i].g);
	ScaleToColor(sdl_colour_data[i].b);
    }
    already_done_init = 1;
}

int num_loaded_fonts = 0;
FontData **loaded_fonts = NULL;

FontData *
sdl_load_font(const char *fname, int ptsize, int style) {

    FontData *fd = NULL;
    const char *extension = NULL;
    int i;
        
    if (!fname) {
	ERRORMSG("No fontname provided.\n");
	return NULL;
    }

    if (!loaded_fonts) {

	loaded_fonts = malloc(20 * sizeof(FontData*));
	for (i = 0; i < 20; i++) {
	    loaded_fonts[i] = NULL;
	}
    }

    for (i = 0; i < num_loaded_fonts; i++) {
	if (loaded_fonts[i]) {
	    if (!strcmp(fname, loaded_fonts[i]->fontname)) {
		return loaded_fonts[i];
	    }
	}
    }
    
    
    extension = strrchr(fname, '.');

    if (!extension) {
	ERRORMSG("Don't know font-type for file %s.\n",
		 fname);
	return NULL;
    }

    extension++;

    if (0) { }
    else if (!strcmp(extension, "hex")) {
	fd = sdl_load_hex_font(fname, 1);
    }
#ifdef ALLOW_TTF
    else if (!strcmp(extension, "ttf")) {
	fd = sdl_load_ttf_font(fname, ptsize, style);
    }
#endif
    else {
	ERRORMSG("Don't know how to handle font-type '%s' for file %s\n",
		 extension, fname);
    }

    if (fd) {
	loaded_fonts[num_loaded_fonts++] = fd;
    }
    
    return fd;
}

LangbandFrame *
sdl_install_font_in_frame(LangbandFrame *lf) {

    sdl_winconnection *wc = NULL;
    
    if (lf) {
	wc = (sdl_winconnection*)lf->ui_connection;
    }

    if (!wc)
	return NULL;

    DBGPUT("Trying to install font %s in frame %s\n", lf->fontname, lf->name);
    wc->font_data = sdl_load_font(lf->fontname, lf->wanted_fontsize, lf->wanted_fontstyle);
	
    if (!wc->font_data) {
	
	return NULL;
    }
    
    if (lf->tile_width < wc->font_data->width)
	lf->tile_width = wc->font_data->width; 
    if (lf->tile_height < wc->font_data->height)
	lf->tile_height = wc->font_data->height; 

    DBGPUT("Frame %s has twid %d and thgt %d, fwid %d fhgt %d\n",
	   lf->name, lf->tile_width, lf->tile_height, wc->font_data->width, wc->font_data->height);

    return lf;
}

LangbandFrame *
sdl_connect_to_frame(LangbandFrame *lf) {

    sdl_winconnection *wc = NULL;

    if (!lf) {
	ERRORMSG("Illegal frame given to sdl_connect_to_frame().\n");
	return NULL;
    }

    wc = malloc(sizeof(sdl_winconnection));
    memset(wc, 0, sizeof(sdl_winconnection));

    //lf->azt = malloc(sizeof(angband_zterm));
    //WIPE(lf->azt, angband_zterm);

    lf->ui_connection = wc;

    //DBGPUT("Making window %d with tw %d and th %d\n", lf->key, lf->tile_width, lf->tile_height);
//    if (!sdl_install_font_in_frame(lf))
//	return NULL;
    
    wc->background = NULL;
    wc->face = theWindow;

    return lf;
}

static LangbandFrame *
sdl_update_frame_size(LangbandFrame *lf) {
    
    int max_col = 0, max_row = 0;

    if (lf->tile_width <= 0 || lf->tile_height <= 0) {
	ERRORMSG("Somehow tilewidth is %d and tileheight is %d, this is illegal.\n",
		 lf->tile_width, lf->tile_width);
	return NULL;
    }

    
    if (lf->allowed_width < 1) {
	lf->allowed_width = theWindow->w - lf->xoffset;
    }
    if (lf->allowed_height < 1) {
	lf->allowed_height = theWindow->h - lf->yoffset;
    }

    max_col = lf->allowed_width / lf->tile_width;
    max_row = lf->allowed_height / lf->tile_height;
    

    //DBGPUT("gah %p %p\n", Term, lf->azt);
    lf->columns = max_col;
    lf->rows = max_row;

    lf->frame_width  = lf->columns * lf->tile_width;
    lf->frame_height = lf->rows * lf->tile_height;

    //term_data_link(lf, max_col, max_row);

    //DBGPUT("returning %p\n", lf);

    return lf;
}

int
sdl_recalculate_frame_placements(int arg) {
    int i;
    
    for (i = 0; i < lbui_num_predefinedFrames; i++) {
	LangbandFrame *lf = lbui_get_frame(i, PREDEFINED);
	lf = sdl_update_frame_size(lf);
	if (!lf) {
	    ERRORMSG("Problems with frame %d\n", i);
	    arg++; // to avoide warning
	    return -1;
	}
    }
    return 0;
}


int
sdl_load_texture(int idx, const char *filename, int target_width, int target_height, int alpha) {
    // loads a texture in the given idx spot.
    int i, j;
    SDL_Surface *bg = NULL, *texture = NULL;

    if (!filename || strlen(filename) < 2) {
	// this means we're mostly called to clear the background, no warning needed
	DBGPUT("Fell out with %s.\n", filename);
	return -2;
    }
    
    texture = IMG_Load(filename);
    
    if (!texture) {
	ERRORMSG("Unable to find texture '%s'\n", filename);
	return -1;
    }

    // SDL_SetAlpha(texture, SDL_SRCALPHA,SDL_ALPHA_TRANSPARENT);
    
    bg = SDL_CreateRGBSurface(SDL_SWSURFACE|SDL_SRCALPHA,
			      target_width, target_height, 32,
			      texture->format->Rmask,
			      texture->format->Gmask,
			      texture->format->Bmask,
			      texture->format->Amask);

    //bg->format = texture->format;
    // do this in the y-direction too!
    for (j=0; j < target_height; j += texture->h) {
	for (i=0; i < target_width; i += texture->w) {
	    SDL_Rect dr;
	    dr.x = i;
	    dr.y = j;
	    dr.w = texture->w;
	    dr.h = texture->h;
	    //DBGPUT("Pasting to %d,%d\n", dr.x, dr.y);
	    JAI_BlitSurfaceAlpha(texture, 0, bg, &dr);
	}
    }

    bg = SDL_DisplayFormatAlpha(bg);

    if (bg && alpha >= 0) {
	SDL_SetAlpha(bg, 0, alpha);
    }
    
    SDL_FreeSurface(texture);

    tileInfo->tiles[idx] = bg;
    tileInfo->tile_files[idx] = malloc(strlen(filename) + 1);
    strcpy(tileInfo->tile_files[idx], filename); // fix!
    
    return 0;
}


/*
 * A "normal" system uses "main.c" for the "main()" function, and
 * simply adds a call to "init_xxx()" to that function, conditional
 * on some form of "USE_XXX" define.
 */


int
lbui_init_sdl(int initarguments) {

    Uint32 initflags = SDL_INIT_VIDEO; /* What's the point, if not video? */
    //int fullscreen = 0;

    int i = 0;
    int sdl_window_flags = 0;
    int imgarrsize = 100;

    TileInformation *ti = NULL; // maybe update this later?
    
    initflags |= SDL_INIT_AUDIO;
    /* TODO perhaps use SDL_InitSubSystem() instead. */

    /* I don't think you'd want the following (except for core dump): */
//#ifdef SDL_NOPARACHUTE
    initflags |= SDL_INIT_NOPARACHUTE;
//#endif

    /* This isn't supposed to do anything on Windows but it may break things!
     * XXX XXX XXX */
    /*initflags |= SDL_INIT_EVENTTHREAD;*/


    if (SDL_Init(initflags) != 0) {
	return -1;
    }
    
    sdl_init_colour_data();

    if (initarguments & LANGBAND_FULLSCREEN) {
	sdl_window_flags |= SDL_FULLSCREEN;
    }
    
    // problems on Win
    //	if (fullscreen) {
    // sdl_window_flags |= SDL_FULLSCREEN | SDL_HWSURFACE | SDL_DOUBLEBUF;
    //	}

    //DBGPUT("going\n");
    // Let us create the base window first of all!

    {
	int width = 800, height = 600;

	if (initarguments & LANGBAND_1024) {
	    width = 1024;
	    height = 768;
	}
	else if (initarguments & LANGBAND_1280) {
	    width = 1280;
	    height = 1024;
	}
	//INFOMSG("Wid %d, hgt %d and flag %d\n", width, height, initarguments);
	theWindow = SDL_SetVideoMode(width, height, 0, sdl_window_flags);
    }

    if (theWindow == NULL) {
	ERRORMSG("SDL could not initialize video mode.");
	return -1;
    }

	
    SDL_WM_SetCaption("Langband", "Langband Main Screen");
    SDL_EnableKeyRepeat(SDL_DEFAULT_REPEAT_DELAY, SDL_DEFAULT_REPEAT_INTERVAL);
    // SDL_ShowCursor(SDL_DISABLE);
    SDL_EnableUNICODE((1 == 1));

    //DBGPUT("tile init\n");
    {
	// Initing tiles
	ti = malloc(sizeof(TileInformation));
	sdl_init_tile_information(ti, imgarrsize);
	tileInfo = ti;
	
	screen_tiles = malloc(sizeof(graf_tiles));
	
	screen_tiles->w = GFXTILE_WIDTH;
	screen_tiles->h = GFXTILE_HEIGHT;
	
	// FIX this to something else later!
	//   screen_tiles->dw = map->font_data->width;
	//screen_tiles->dh = map->font_data->height;
	screen_tiles->dw = GFXTILE_WIDTH; //map->font_data->width;
	screen_tiles->dh = GFXTILE_HEIGHT; //map->font_data->height;
	screen_tiles->face = NULL;

	// DBGPUT("read tile\n");

	//read_tiles(ti, david_gervais_tile_files);

    }

    //DBGPUT("Doing windows\n");
    for (i = 0; i < lbui_num_predefinedFrames; i++) {
	LangbandFrame *lf = lbui_get_frame(i, PREDEFINED);
	const char *frameName = NULL;
	//DBGPUT("Checking sub %d\n", i);
	if (!lf) {
	    DBGPUT("Did not find frame %d.\n", i);
	    continue;
	}
	frameName = lf->name;
	
	lf = sdl_connect_to_frame(lf);
	
	//DBGPUT("did sdlify %p\n", lf);
	
	if (!lf) {
	    ERRORMSG("Problems creating frame '%s'\n", frameName);
	    return -1;
	}
	// tweaking
	{
	    sdl_winconnection *wc = (sdl_winconnection*)lf->ui_connection;
	    wc->gt = screen_tiles; // improve later
	    lf->visible = 0;
	}
	//DBGPUT("end-loop\n");
    }


    lbui_activate_frame(FULL_TERM_IDX);
    
    //DBGPUT("return to sender\n");
    
    return 0;
}

int
sdl_cleanup(void) {

    //DBGPUT("cleaning up SDL %d %d\n", use_sound, current_soundsystem());
    
    if (theWindow) {
	SDL_FreeSurface(theWindow);
	theWindow = NULL;
    }

    if (tileInfo) {
	free(tileInfo); // may need to do subpointers too. 
	tileInfo = NULL;
    }

    if (screen_tiles) {
	free(screen_tiles);
	screen_tiles = NULL;
    }
    
    // this one seems to create nasty problems :-(
    //SDL_Quit();
    SDL_QuitSubSystem(SDL_INIT_EVERYTHING); // hack
    //DBGPUT("cleaned SDL\n");
    
    return 0;
}

#define ALSO_CLEAR_BG 0x01
#define DONT_PAINT 0x02

int
sdl_complex_blit(short win_num, short x, short y, unsigned int img, int flags) {

    // inefficient but should work
    LangbandFrame *lf = lbui_predefinedFrames[win_num];
    sdl_winconnection *wc = NULL;
    SDL_Rect sr, dr;
    SDL_Surface *orig = NULL;

    if (lf) {
	wc = (sdl_winconnection*)lf->ui_connection;
    }
    else {
	return 2;
    }

    // if (!(lf->visible)) return 1;


    dr.x = x * lf->tile_width  + lf->xoffset;
    dr.y = y * lf->tile_height + lf->yoffset;
    sr.w = dr.w = lf->tile_width;
    sr.h = dr.h = lf->tile_height;
    

    //DBGPUT("Writing %u to %d,%d,%d\n", img, win_num, x, y);
    
    if (img == 0) {
	if (lf->background >= 0 && tileInfo->tiles[lf->background]) {
	    sr.x = x * lf->tile_width;
	    sr.y = y * lf->tile_height;
	    SDL_BlitSurface(tileInfo->tiles[lf->background], &sr, wc->face, &dr);
	}
	else {
	    SDL_FillRect(wc->face, &dr, 0);
	}

	if (!(flags & DONT_PAINT)) {
	    SDL_UpdateRect(wc->face, dr.x, dr.y, dr.w, dr.h);
	}
	return 0;
    }
    
    else if (img < 65536) { // character
	unsigned int thechar = (img & 0x000000FF); // bits 1-8
	unsigned int attr = (img & 0x0000FF00) >> 8; // bits 9-16
	FontData *fd = wc->font_data;
	int xadj = (lf->tile_width > wc->font_data->width) ? (lf->tile_width - wc->font_data->width) : 0; 
	int yadj = (lf->tile_height > wc->font_data->height) ? (lf->tile_height - wc->font_data->height) : 0;
	xadj >>= 1;
	yadj >>= 1;

	//DBGPUT("1. Char = %c, attr = %d\n", thechar, attr);
	
	dr.x = (x * lf->tile_width)  + xadj + lf->xoffset;
	dr.y = (y * lf->tile_height) + yadj + lf->yoffset;
	dr.w = lf->tile_width;
	dr.h = lf->tile_height;

	//DBGPUT("2. Char = %c, attr = %d\n", thechar, attr);
	
	sr.w = fd->width;
	sr.h = fd->height;
	
	sr.x = 0;
	sr.y = thechar * fd->height;

	//DBGPUT("3. Char = %c, attr = %d\n", thechar, attr);
	
	if (thechar != 32) {// space will clean
	    if (fd->font_type == FONT_TYPE_HEX) {
		orig = fd->theFont;
		//DBGPUT("3.1. Char = %c, attr = %d\n", thechar, attr);
	    // SDL_BlitSurface(fd->theFont, &sr, f, dr);
	    
		SDL_SetColors(fd->theFont, &(sdl_colour_data[attr & 0xf]), 0xff, 1);
		// SDL_SetColors(fd->face, &(sdl_colour_data[a&0xf]), 0xff, 1); 
		SDL_SetColorKey(fd->theFont, SDL_SRCCOLORKEY, 0);
		
	    }
	    else if (fd->font_type == FONT_TYPE_TTF) {
		
		orig = fd->letters[thechar];
		//DBGPUT("Doing '%c' in col %d\n", thechar, attr);
		SDL_SetColors(orig, &(sdl_colour_data[attr & 0xf]), 1, 1);
		
		//SDL_SetColorKey(orig, SDL_SRCCOLORKEY, 0);

		//fd->letters[thechar]->format->palette[1]
		sr.y = 0;
		//sdl_display_char(
	    }

	}
	//DBGPUT("4. Char = %c, attr = %d\n", thechar, attr);
		
	/*    
	if (thechar != '#' && thechar != '.') {
	    DBGPUT("Writing '%c' (%d) char and %u attr to %d,%d (%d,%d,%d,%d -> %d,%d,%d,%d) %d\n",
		   thechar, thechar, attr, x, y, sr.x, sr.y, sr.w, sr.h, dr.x, dr.y, dr.w, dr.h, flags);
	}
	*/
		
    }
    
    else { // gfx
        int grp1_tilecols = 0;
	unsigned int tile = (img & 0x00FFFF00) >> 8; // bits 9-24
	unsigned int file = (img & 0xFF000000) >> 24; // bits 25-32
        orig = tileInfo->tiles[file];

        if (orig) {
           grp1_tilecols = orig->w / lf->tile_width;

	//DBGPUT("At %d,%d File %d and Tile %d\n", x, y, file, tile);
	
  	   sr.x = (tile % grp1_tilecols) * lf->tile_width; //td->gt->dw; //td->fnt->twid;
	   sr.y = (tile / grp1_tilecols) * lf->tile_height; //td->gt->dh; //td->fnt->hgt;
	}
	
	//DBGPUT("Writing %u file and %u tile to %d,%d (%d,%d -> %d,%d,%d,%d)\n",
	//       file, tile, x, y, sr.w, sr.h, dr.x, dr.y, dr.w, dr.h);
    }
    

    if (flags & ALSO_CLEAR_BG) {
	SDL_FillRect(wc->face, &dr, 0);
    }
    
    if (orig != NULL) {
	SDL_BlitSurface(orig, &sr, wc->face, &dr);
    }

    if (!(flags & DONT_PAINT)) {
	SDL_UpdateRect(wc->face, dr.x, dr.y, dr.w, dr.h);
    }
    
    
    return 0;
}

int
sdl_full_blit(short win_num, short x, short y, unsigned int img, short flag) {
    // also flushes
    return sdl_complex_blit(win_num, x, y, img, ALSO_CLEAR_BG | flag);
}

int
sdl_transparent_blit(short win_num, short x, short y, unsigned int img, short flag) {
    // also flushes
    return sdl_complex_blit(win_num, x, y, img, flag);
}

int
sdl_clear_coords(short win_num, short x, short y, short w, short h) {
    // also flushes
    LangbandFrame *lf = lbui_predefinedFrames[win_num];
    sdl_winconnection *wc = NULL;
    SDL_Rect dr;
    
    if (lf) {
	wc = (sdl_winconnection*)lf->ui_connection;
    }
    else {
	return 2;
    }

    // if (!(lf->visible)) return 1;
    
    dr.x = x * lf->tile_width  + lf->xoffset;
    dr.y = y * lf->tile_height + lf->yoffset;

    dr.w = w * lf->tile_width;
    dr.h = h * lf->tile_height;

    //DBGPUT("Clearing %d,%d,%d,%d\n", dr.x, dr.y, dr.w, dr.h);

    if (lf->background >= 0 && tileInfo->tiles[lf->background]) {
	SDL_Rect sr;
	sr.x = x * lf->tile_width;
	sr.y = y * lf->tile_height;
	sr.w = w * lf->tile_width;
	sr.h = h * lf->tile_height;
    
	SDL_BlitSurface(tileInfo->tiles[lf->background], &sr, wc->face, &dr);
    }
    // paint black
    else {
	SDL_FillRect(wc->face, &dr, 0);
    }
    
    SDL_UpdateRect(wc->face, dr.x, dr.y, dr.w, dr.h);
    
    return 0;
}

int
sdl_flush_coords(short win_num, short x, short y, short w, short h) {
    
    LangbandFrame *lf = lbui_predefinedFrames[win_num];
    sdl_winconnection *wc = NULL;
    SDL_Rect dr;
    
    if (lf) {
	wc = (sdl_winconnection*)lf->ui_connection;
    }
    else {
	return 2;
    }
	
//    if (!(lf->visible)) return 1;
    
    dr.x = x * lf->tile_width  + lf->xoffset;
    dr.y = y * lf->tile_height + lf->yoffset;

    dr.w = w * lf->tile_width;
    dr.h = h * lf->tile_height;

    // this calculation is very important!!
    if (dr.w > (lf->allowed_width - (x * lf->tile_width))) {
	dr.w = (lf->allowed_width - (x * lf->tile_width));
    }
    
    // DBGPUT("Flush %d,%d,%d,%d\n", dr.x, dr.y, dr.w, dr.h);
    SDL_UpdateRect(wc->face, dr.x, dr.y, dr.w, dr.h);
    
    return 0;
}

#define ONLY_POLL 1

#define KBD_EVT 0x00
#define MOUSE_EVT 0x01

#define CTRL_KEY 0x02
#define ALT_KEY 0x04
#define SHIFT_KEY 0x08

#define L_M_BT 0x02
#define R_M_BT 0x04
#define M_M_BT 0x08

int
sdl_get_event(int option) {
    
    SDL_Event event; /* just a temporary place to hold an event */
    int retval = 0;
    int eventcode = 0;
    // first bit, is this a keyboard event 0, mouse event 1
    // if keyboard:
    // second bit is if ctrl was pressed
    // third bit is if alt was pressed
    // fourth bit is if shift was pressed
    // ninth bit and out is the actual code.
    // if mouse:
    // second bit is if it was left button
    // third bit is if it was right button
    // fourth bit is if it was middle button
    // bits 6-17 is x coord
    // bits 18-29 is y coord
    if (option & ONLY_POLL) {
	retval = SDL_PollEvent(&event);
	if (retval)
	    return 0; // fix
    }
    else {
	retval = SDL_WaitEvent(&event);
	if (retval == 0)
	    return 0;
    }

    if (event.type == SDL_KEYDOWN) {
	//DEBUGPUT("Key state %d\n", event.key.state);
	if (event.key.state == SDL_PRESSED) {
	    int charcode = event.key.keysym.unicode & 0xff;
	    int keycode = charcode ? charcode : (int)(event.key.keysym.sym);
	    
	    eventcode |= KBD_EVT;
	    if (event.key.keysym.mod & (KMOD_LCTRL|KMOD_RCTRL))
		eventcode |= CTRL_KEY;
	    if (event.key.keysym.mod & (KMOD_LALT|KMOD_RALT))
		eventcode |= ALT_KEY;
	    if (event.key.keysym.mod & (KMOD_LSHIFT|KMOD_RSHIFT))
		eventcode |= SHIFT_KEY;
	    eventcode |= keycode << 8; 
	    
	    //DEBUGPUT("Press %d %d %d %d\n", event.key.keysym.sym, event.key.keysym.unicode & 0xff,
	    //     event.key.keysym.mod, eventcode);

	    return eventcode;
	}
    }
    else if (event.type == SDL_MOUSEBUTTONDOWN) {
	eventcode |= MOUSE_EVT;
	if (event.button.button == SDL_BUTTON_LEFT)
	    eventcode |= L_M_BT;
	if (event.button.button == SDL_BUTTON_RIGHT)
	    eventcode |= R_M_BT;
	if (event.button.button == SDL_BUTTON_MIDDLE)
	    eventcode |= M_M_BT;

	eventcode |= event.button.x << 6;
	eventcode |= event.button.y << 18;
	
	return eventcode;
    }
    else if (event.type == SDL_MOUSEMOTION) {
	return -2;
    }
	

    return 0;
    
}

int
sdl_get_window_width() {
    if (theWindow)
	return theWindow->w;
    else
	return -2;
}

int
sdl_get_window_height() {
    if (theWindow)
	return theWindow->h;
    else
	return -2;
}

#endif /* USE_SDL */
