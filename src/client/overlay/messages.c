
/* $Id: messages.c,v 1.3 2003/04/18 21:45:14 cipher Exp $ */

/*
 * Copyright (c) 2003 Paul A. Schifferer
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

/* Internal headers */
#include "ironhells.h"
#include "overlay.h"

void
IH_RenderOverlay_Messages(Overlay * overlay)
{
     int             i, height;

     /* Get font height.
      */
     height = IH_GetFontHeight(IH_FONT_NORMAL);

     /* Display the messages.
      */
     for(i = 0; i < ih.messages_shown; i++)
     {
          ihColor         color;
          ihFontPos       pos;
          cptr            msg;
          int             display_y;

          msg = message_str(i);
          if(!msg)
               continue;

          display_y = ih.display.height - ((i + 1) * height);

          pos.x.type = IH_POSITION_TYPE_PIXEL;
          pos.x.pixel = 0;
          pos.y.type = IH_POSITION_TYPE_PIXEL;
          pos.y.pixel = display_y;

          IH_AttrToColor(message_color(i), &color);

          IH_RenderText(IH_FONT_NORMAL, msg, &pos, &color, 0, NULL);
     }
}

void
IH_ReleaseOverlay_Messages(Overlay * overlay)
{
     if(!overlay)
          return;
}