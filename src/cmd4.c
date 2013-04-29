/* File: cmd4.c */

/* Screen refresh, change character name, message recall, interacting 
 * with options (inc. text of cheat options), macros, visuals, also level 
 * feelings, screen dumps/loading, and known Uniques/Artifacts/Objects/
 * recalling the contents of the home.
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"


/* max length of note output */
#define LINEWRAP	75

/* used for knowledge display */
int browser_rows = 16;
int knowledge_row = 2;
int header_row = 4;

typedef struct monster_list_entry monster_list_entry;
/*
 * Structure for building monster "lists"
 */
struct monster_list_entry
{
  s16b r_idx;			/* Monster race index */
  
  byte amount;
};

/*
 * Hack -- redraw the screen
 *
 * This command performs various low level updates, clears all the "extra"
 * windows, does a total redraw of the main window, and requests all of the
 * interesting updates and redraws that I can think of.
 *
 * This command is also used to "instantiate" the results of the user
 * selecting various things, such as graphics mode, so it must call
 * the "TERM_XTRA_REACT" hook before redrawing the windows.
 */
void do_cmd_redraw(void)
{
  int j;
  
  term *old = Term;
  
  /* Low level flush */
  Term_flush();
  
  /* Reset "inkey()" */
  flush();

  /* Hack -- react to changes */
  Term_xtra(TERM_XTRA_REACT, 0);
  
  /* Combine and Reorder the pack (later) */
  p_ptr->notice |= (PN_COMBINE | PN_REORDER);
  
  /* Update torch */
  p_ptr->update |= (PU_TORCH);
  
  /* Update stuff */
  p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);
  
  /* Forget view */
  p_ptr->update |= (PU_FORGET_VIEW);
  
  /* Update view */
  p_ptr->update |= (PU_UPDATE_VIEW);
  
  /* Update monsters */
  p_ptr->update |= (PU_MONSTERS);
  
  /* Redraw everything */
  p_ptr->redraw |= (PR_WIPE | PR_BASIC | PR_EXTRA | PR_MAP | PR_EQUIPPY);
  
  /* Window stuff */
  p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_SPELL | PW_PLAYER_0 | 
		    PW_PLAYER_1);
  
  /* Window stuff */
  p_ptr->window |= (PW_MESSAGE | PW_OVERHEAD | PW_DUNGEON | PW_MONSTER | 
		    PW_OBJECT);
  
  /* Clear screen */
  Term_clear();

  /* Hack -- update */
  handle_stuff();
  
  
  /* Redraw every window */
  for (j = 0; j < 8; j++)
    {
      /* Dead window */
      if (!angband_term[j]) continue;
      
      /* Activate */
      Term_activate(angband_term[j]);
      
      /* Redraw */
      Term_redraw();
      
      /* Refresh */
      Term_fresh();
      
      /* Restore */
      Term_activate(old);
    }
}


/*
 * Map resizing whenever the main term changes size
 */
void resize_map(void)
{
  /* Only if the dungeon exists */
  if (!character_dungeon) return;
  
  /* Mega-Hack -- no panel yet */
  panel_row_min = 0;
  panel_row_max = 0;
  panel_col_min = 0;
  panel_col_max = 0;
  
  if (character_dungeon)
    {
      verify_panel();
    }
  
  /* Combine and Reorder the pack (later) */
  p_ptr->notice |= (PN_COMBINE | PN_REORDER);
  
  /* Update torch */
  p_ptr->update |= (PU_TORCH);
  
  /* Update stuff */
  p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS);
  
  /* Forget view */
  p_ptr->update |= (PU_FORGET_VIEW);
  
  /* Update view */
  p_ptr->update |= (PU_UPDATE_VIEW);
  
  /* Update monsters */
  p_ptr->update |= (PU_MONSTERS);
  
  /* Redraw everything */
  p_ptr->redraw |= (PR_WIPE | PR_BASIC | PR_EXTRA | PR_MAP | PR_EQUIPPY);
  
  /* Hack -- update */
  handle_stuff();
  
  /* Redraw */
  Term_redraw();
  
  /* Refresh */
  Term_fresh();
}

/*
 * Redraw a term when it is resized
 */
void redraw_window(void)
{
  /* Only if the dungeon exists */
  if (!character_dungeon) return;
  
  /* Hack - Activate term zero for the redraw */
  Term_activate(&term_screen[0]);
  
  /* Hack -- react to changes */
  Term_xtra(TERM_XTRA_REACT, 0);
  
  /* Window stuff */
  p_ptr->window |= (PW_INVEN | PW_EQUIP | PW_SPELL | PW_PLAYER_0 | 
		    PW_PLAYER_1);
  
  /* Window stuff */
  p_ptr->window |= (PW_MESSAGE | PW_OVERHEAD | PW_DUNGEON | PW_MONSTER | 
		    PW_OBJECT);
  
  /* Hack -- update */
  handle_stuff();
  
  /* Redraw */
  Term_redraw();
  
  /* Refresh */
  Term_fresh();
}

/*
 * Hack -- change name
 */
void do_cmd_change_name(void)
{
  key_event ke;
  
  int mode = 0;
  
  cptr p;
  
  /* Prompt */
  if (small_screen)
    p = "['c' change name,'f' file,'h' next mode,ESC]";
  else
    p = "['c' to change name, 'f' to file, 'h' to change mode, or ESC]";
  
  /* Save screen */
  screen_save();
  
  /* Forever */
  while (1)
    {
      /* Display the player */
      display_player(mode);
      
      /* Prompt */
      Term_putstr(2, 23, -1, TERM_WHITE, p);
      
      /* Query */
      ke = inkey_ex();
      
      /* Exit */
      if (ke.key == ESCAPE) break;
      
      /* Mouse input */
      if (ke.key == '\xff')
	{ 
	  if (ke.mousey == 23)
	    {
	      if ((ke.mousex > 2) && 
		  (ke.mousex < (small_screen ? 18 : 21))) ke.key = 'c';
	      if ((ke.mousex > (small_screen ? 18 : 22)) && 
		  (ke.mousex < (small_screen ? 27 : 34))) ke.key = 'f';
	      if ((ke.mousex > (small_screen ? 27 : 35)) && 
		  (ke.mousex < (small_screen ? 41 : 54))) ke.key = 'h';
	      if ((ke.mousex > (small_screen ? 41 : 58)) && 
		  (ke.mousex < (small_screen ? 45 : 62))) break;
	    }
	  else
	    /* Default to 'h' */
	    ke.key = 'h';
	}

	    
      /* Change name */
      if (ke.key == 'c')
	{
	  get_name();
	}
      
      /* File dump */
      else if (ke.key == 'f')
	{
	  char tmp[81];
	  
	  sprintf(tmp, "%s.txt", op_ptr->base_name);
	  
	  if (get_string("File name: ", tmp, 81))
	    {
	      if (tmp[0] && (tmp[0] != ' '))
		{
		  file_character(tmp, FALSE);
		}
	    }
	}
      
      /* Toggle mode.  Changed back to 2.8.2 format. */
      else if (ke.key == 'h')
	{
	  mode++;
	}

      
      
      /* Oops */
      else
	{
	  bell("Illegal command for change name!");
	}
      
      /* Flush messages */
      msg_print(NULL);
    }
  
  /* Load screen */
  screen_load();
}


/*
 * Recall the most recent message
 */
void do_cmd_message_one(void)
{
  /* Recall one message XXX XXX XXX */
  c_prt(message_color(0), format( "> %s", message_str(0)), 0, 0);
}


/*
 * Show previous messages to the user -BEN-
 *
 * The screen format uses line 0 and 23 for headers and prompts,
 * skips line 1 and 22, and uses line 2 thru 21 for old messages.
 *
 * This command shows you which commands you are viewing, and allows
 * you to "search" for strings in the recall.
 *
 * Note that messages may be longer than 80 characters, but they are
 * displayed using "infinite" length, with a special sub-command to
 * "slide" the virtual display to the left or right.
 *
 * Attempt to only hilite the matching portions of the string.
 */
void do_cmd_messages(void)
{
  s16b i, j, n;
  int q;
  key_event ke;
  
  char shower[80];
  char finder[80];
  
  int wid, hgt;
  
  /* Wipe finder */
  strcpy(finder, "");
  
  /* Wipe shower */
  strcpy(shower, "");
  
  /* Total messages */
  n = message_num();
  
  /* Start on first message */
  i = 0;
  
  /* Start at leftmost edge */
  q = 0;
  
  /* Save the screen */
  screen_save();
  
  /* Get size */
  Term_get_size(&wid, &hgt);
  
  /* Process requests until done */
  while (1)
    {
      /* Clear screen */
      Term_clear();
      
      /* Dump messages */
      for (j = 0; (j < hgt-4) && (i + j < n); j++)
	{
	  cptr msg = message_str(i+j);
	  byte attr = message_color(i+j);
	  
	  /* Hack -- fake monochrome */
	  if (fake_monochrome) attr = TERM_WHITE;
	  
	  /* Apply horizontal scroll */
	  msg = ((int)strlen(msg) >= q) ? (msg + q) : "";
	  
	  /* Dump the messages, bottom to top */
	  Term_putstr(0, hgt-3 - j, -1, attr, msg);
	  
	  /* Hilite "shower" */
	  if (shower[0])
	    {
	      cptr str = msg;
	      
	      /* Display matches */
	      while ((str = strstr(str, shower)) != NULL)
		{
		  int len = strlen(shower);
		  
		  /* Display the match */
		  Term_putstr(str - msg, hgt-3 - j, len, TERM_YELLOW, shower);
		  
		  /* Advance */
		  str += len;
		}
	    }
	}
      
      /* Display header XXX XXX XXX */
      prt(format("Message Recall (%d-%d of %d), Offset %d",
		 i, i + j - 1, n, q), 0, 0);
      
      /* Display prompt (not very informative) */

      if (small_screen)

	prt("[Press '-' for older, '+' for newer, ..., or ESCAPE]", hgt-1, 0);

      else

	prt("[Press 'p' for older, 'n' for newer, ..., or ESCAPE]", hgt-1, 0);


      
      /* Get a command */
      ke = inkey_ex();
      
      /* Exit on Escape */
      if (((ke.mousex > 45) && (ke.mousex < 52) && 
	   (ke.mousey == Term->hgt - 1)) || (ke.key == ESCAPE)) break;
      
      /* Hack -- Save the old index */
      j = i;
      
      /* Horizontal scroll */
      if (ke.key == '4')
	{
	  /* Scroll left */
	  q = (q >= wid / 2) ? (q - wid / 2) : 0;
	  
	  /* Success */
	  continue;
	}
      
      /* Horizontal scroll */
      if (ke.key == '6')
	{
	  /* Scroll right */
	  q = q + wid / 2;
	  
	  /* Success */
	  continue;
	}
      
      /* Hack -- handle show */
      if (ke.key == '=')
	{
	  /* Prompt */
	  prt("Show: ", hgt-1, 0);
	  
	  /* Get a "shower" string, or continue */
	  if (!askfor_aux(shower, 80)) continue;
	  
	  /* Okay */
	  continue;
	}
      
      /* Hack -- handle find */
      if (ke.key == '/')
	{
	  s16b z;
	  
	  /* Prompt */
	  prt("Find: ", hgt-1, 0);
	  
	  /* Get a "finder" string, or continue */
	  if (!askfor_aux(finder, 80)) continue;
	  
	  /* Show it */
	  strcpy(shower, finder);
	  
	  /* Scan messages */
	  for (z = i + 1; z < n; z++)
	    {
	      cptr msg = message_str(z);
	      
	      /* Search for it */
	      if (strstr(msg, finder))
		{
		  /* New location */
		  i = z;
		  
		  /* Done */
		  break;
		}
	    }
	}
      
      /* Recall 1 older message */
      if ((ke.key == '8') || (ke.key == '\n') || (ke.key == '\r'))
	{
	  /* Go newer if legal */
	  if (i + 1 < n) i += 1;
	}
      
      /* Recall 10 older messages */

      if (small_screen)
	{
	  if (((ke.mousex > 6) && (ke.mousex < 20) && 
	       (ke.mousey == Term->hgt - 1)) || (ke.key == '+'))
	    {
	      /* Go older if legal */
	      if (i + 10 < n) i += 10;
	    }
	  
	  /* Recall 10 newer messages */
	  if (((ke.mousex > 21) && (ke.mousex < 35) && 
	       (ke.mousey == Term->hgt - 1)) || (ke.key == '-'))
	    {
	      /* Go newer (if able) */
	      i = (i >= 10) ? (i - 10) : 0;
	    }
	}
      else
	{
	  /* Recall 20 older messages */
	  if ((ke.key == 'p') || (ke.key == KTRL('P')) || (ke.key == ' ') || 
	      ((ke.mousex > 6) && (ke.mousex < 20) &&
	       (ke.mousey == Term->hgt - 1)))
	    {
	      /* Go older if legal */
	      if (i + 20 < n) i += 20;
	    }
	  
	  /* Recall 20 newer messages */
	  if ((ke.key == 'n') || (ke.key == KTRL('N')) || 
	      ((ke.mousex > 21) && (ke.mousex < 35) &&
	       (ke.mousey == Term->hgt - 1)))
	    {
	      /* Go newer (if able) */
	      i = (i >= 20) ? (i - 20) : 0;
	    }
	}

      
      /* Recall 1 newer messages */
      if (ke.key == '2')
	{
	  /* Go newer (if able) */
	  i = (i >= 1) ? (i - 1) : 0;
	}
      
      /* Hack -- Error of some kind */
      if (i == j) bell(NULL);
    }
  
  /* Restore the screen */
  screen_load();
}



/*
 * Autosave options -- textual names
 */
static cptr autosave_text[1] =
{
  "autosave"
};

/*
 * Autosave options -- descriptions
 */
static cptr autosave_desc[1] =
{
  "Timed autosave"
};

s16b toggle_frequency(s16b current)
{
  if (current == 0) return 50;
  if (current == 50) return 100;
  if (current == 100) return 250;
  if (current == 250) return 500;
  if (current == 500) return 1000;
  if (current == 1000) return 2500;
  if (current == 2500) return 5000;
  if (current == 5000) return 10000;
  if (current == 10000) return 25000;
  
  else return 0;
}


/*
 * Interact with autosave options.  From Zangband.
 */
static void do_cmd_options_autosave(cptr info)
{
  char ch;
  
  int i, k = 0, n = 1;
  
  char buf[80];
  
  
  /* Clear screen */
  Term_clear();
  
  /* Interact with the player */
  while (TRUE)
    {
      /* Prompt XXX XXX XXX */
      sprintf(buf, "%s (RET to advance, y/n to set, 'F' for frequency, ESC to accept) ", info);
      prt(buf, 0, 0);
      
      /* Display the options */
      /* Display the options */
      for (i = 0; i < n; i++)
	{
	  byte a = TERM_WHITE;
	  
	  /* Color current option */
	  if (i == k) a = TERM_L_BLUE;
	  
	  /* Display the option text */
	  sprintf(buf, "%-48s: %s  (%s)",
		  autosave_desc[i],
		  autosave ? "yes" : "no ",
		  autosave_text[i]);
	  c_prt(a, buf, i + 2, 0);
	  
	  prt(format("Timed autosave frequency: every %d turns", 
		     autosave_freq), 5, 0);
	}
      
      
      /* Hilite current option */
      move_cursor(k + 2, 50);
      
      /* Get a key */
      ch = inkey();
      
      /* Analyze */
      switch (ch)
	{
	case ESCAPE:
	  {
	    return;
	  }
	  
	case '-':
	case '8':
	  {
	    k = (n + k - 1) % n;
	    break;
	  }
	  
	case ' ':
	case '\n':
	case '\r':
	case '2':
	  {
	    k = (k + 1) % n;
	    break;
	  }
	  
	case 'y':
	case 'Y':
	case '6':
	  {
	    
	    autosave = TRUE;
	    k = (k + 1) % n;
	    break;
	  }
	  
	case 'n':
	case 'N':
	case '4':
	  {
	    autosave = FALSE;
	    k = (k + 1) % n;
	    break;
	  }
	  
	case 'f':
	case 'F':
	  {
	    autosave_freq = toggle_frequency(autosave_freq);
	    prt(format("Timed autosave frequency: every %d turns",
		       autosave_freq), 5, 0);
	    break;
	  }
	  
	default:
	  {
	    bell("Illegal command for Autosave options!");
	    break;
	  }
	}
    }
}

/*
 * Ask for a "user pref line" and process it
 *
 * XXX XXX XXX Allow absolute file names?
 */
void do_cmd_pref(void)
{
  char tmp[81];

  /* Default */
  strcpy(tmp, "");
  
  /* Ask for a "user pref command" */
  if (!get_string("Pref: ", tmp, 81)) return;
  
  /* Process that pref command */
  (void)process_pref_file_aux(tmp);
}




/*
 * Ask for a "user pref file" and process it.
 *
 * This function should only be used by standard interaction commands,
 * in which a standard "Command:" prompt is present on the given row.
 *
 * Allow absolute file names?  XXX XXX XXX
 */
static bool do_cmd_pref_file_hack(int row)
{
  char ftmp[80];
  
  /* Prompt */
  prt("Command: Load a user pref file", row, 0);
  
  /* Prompt */
  prt("File: ", row + 2, 0);
  
  /* Default filename */
  sprintf(ftmp, "%s.prf", op_ptr->base_name);
  
  /* Ask for a file (or cancel) */
  if (!askfor_aux(ftmp, 80)) return (FALSE);
  
  /* Process the given filename */
  if (process_pref_file(ftmp))
    {
      /* Mention failure */
      msg_format("Failed to load '%s'!", ftmp);
    }
  else
    {
      /* Mention success */
      msg_format("Loaded '%s'.", ftmp);
    }
  
  return (TRUE);
}


/*
 * Interact with some options
 */
static void do_cmd_options_aux(int page, cptr info)
{
  key_event ke;
  
  int i, k = 0, n = 0;
  
  int opt[OPT_PAGE_PER];
  
  char buf[80];
  
  
  /* Scan the options */
  for (i = 0; i < OPT_PAGE_PER; i++)
    {
      /* Collect options on this "page" */
      if (option_page[page][i] != 255)
	{
	  opt[n++] = option_page[page][i];
	}
    }
  
  
  /* Clear screen */
  Term_clear();
  
  /* Interact with the player */
  while (TRUE)
    {
      /* Prompt XXX XXX XXX */

      if (small_screen)
	{
	  sprintf(buf, "%s ", info);
	  prt(buf, 0, 0);
	  prt("(RET to advance, y/n to set, ESC to accept) ", 1, 0);
	}
      else
	{	
	  sprintf(buf, "%s (RET to advance, y/n to set, ESC to accept) ", 
		  info);
	  prt(buf, 0, 0);
	  prt("                                            ", 1, 0);
	}


      /* Display the options */
      for (i = 0; i < n; i++)
	{
	  byte a = TERM_WHITE;
	  
	  /* Color current option */
	  if (i == k) a = TERM_L_BLUE;
	  
	  /* Display the option text */

	  if (small_screen)
	    {
	      sprintf(buf, "%-22s: %s", option_text[opt[i]],
		      op_ptr->opt[opt[i]] ? "yes" : "no "); 
	      c_prt(a, buf, i + 2, 0);
	    }
	  else
	    {  
	      sprintf(buf, "%-48s: %s  (%s)", 
		      option_desc[opt[i]],
		      op_ptr->opt[opt[i]] ? "yes" : "no ", 
		      option_text[opt[i]]);
	      c_prt(a, buf, i + 2, 0);
	    }


	}

      /* Hilite current option */
      move_cursor(k + 2, (small_screen ? 24 : 50));
 
      /* Get a key */
      ke = inkey_ex();

      /* Mouse input */
      if ((ke.key == '\xff') && (ke.mousey == (small_screen ? 1 : 0)))
	{
	  int base; 

	  base = (small_screen ? -1 : strlen(info));
	  
	  if ((ke.mousex > base + 1) && (ke.mousex < base + 16))
	    ke.key = '\r';
	  else if (ke.mousex == base + 18)
	    ke.key = 'y';
	  else if (ke.mousex == base + 20)
	    ke.key = 'n';
	  else if ((ke.mousex > base + 29) && (ke.mousex < base + 43))
	    ke.key = ESCAPE;
	}
	    
      
      /* Analyze */
      switch (ke.key)
	{
	case ESCAPE:
	  {
	    /* Hack -- Notice use of any "cheat" options */
	    for (i = OPT_cheat_start; i < OPT_cheat_end+1; i++)
	      {
		if (op_ptr->opt[i])
		  {
		    /* Set score option */
		    op_ptr->opt[OPT_score_start + (i - OPT_cheat_start)] 
		      = TRUE;
		  }
	      }
	    
	    return;
	  }
	  
	case '-':
	case '8':
	  {
	    k = (n + k - 1) % n;
	    break;
	  }
	  
	case ' ':
	case '\n':
	case '\r':
	case '2':
	  {
	    k = (k + 1) % n;
	    break;
	  }
	  
	case 't':
	case '5':
	  {
	    op_ptr->opt[opt[k]] = !op_ptr->opt[opt[k]];
	    break;
	  }
	  
	case 'y':
	case '6':
	  {
	    op_ptr->opt[opt[k]] = TRUE;
	    k = (k + 1) % n;
	    break;
	  }
	  
	case 'n':
	case '4':
	  {
	    op_ptr->opt[opt[k]] = FALSE;
	    k = (k + 1) % n;
	    break;
	  }
	
	case '\xff':
	  {
	    if ((ke.mousey > 1) && (ke.mousey < n + 2))
	      {
		k = ke.mousey - 2;

		if (ke.mousex > (small_screen ? 22 : 48)) 
		    op_ptr->opt[opt[k]] = !op_ptr->opt[opt[k]];

		break;
	      }
	    else
	      {
		/* Fall through */
	      }

	  }
  
	default:
	  {
	    bell("Illegal command for normal options!");
	    break;
	  }
	}
    }
}


/*
 * Modify the "window" options
 */
static void do_cmd_options_win(void)
{
  int i, j, d;
  
  int y = 0;
  int x = 0;
  
  char ch;
  key_event ke;
  
  u32b old_flag[8];

  
  
  /* Memorize old flags */
  for (j = 0; j < 8; j++)
    {
      /* Acquire current flags */
      old_flag[j] = op_ptr->window_flag[j];
    }
  
  
  /* Clear screen */
  Term_clear();
  
  /* Interact */
  while (1)
    {
      /* Prompt */
      prt("Window flags (<dir> to move, 't' to toggle, or ESC)", 0, 0);
      
      /* Display the windows */
      for (j = 0; j < 8; j++)
	{
	  byte a = TERM_WHITE;
	  
	  cptr s = angband_term_name[j];
	  
	  /* Use color */
	  if (j == x) a = TERM_L_BLUE;
	  
	  /* Window name, staggered, centered */
	  Term_putstr(35 + j * 5 - strlen(s) / 2, 2 + j % 2, -1, a, s);
	}
      
      /* Display the options */
      for (i = 0; i < 16; i++)
	{
	  byte a = TERM_WHITE;
	  
	  cptr str = window_flag_desc[i];
	  
	  /* Use color */
	  if (i == y) a = TERM_L_BLUE;
	  
	  /* Unused option */
	  if (!str) str = "(Unused option)";
	  
	  /* Flag name */
	  Term_putstr(0, i + 5, -1, a, str);
	  
	  /* Display the windows */
	  for (j = 0; j < 8; j++)
	    {
	      byte a = TERM_WHITE;
	      
	      char c = '.';
	      
	      /* Use color */
	      if ((i == y) && (j == x)) a = TERM_L_BLUE;
	      
	      /* Active flag */
	      if (op_ptr->window_flag[j] & (1L << i)) c = 'X';

	      /* Flag value */
	      Term_putch(35 + j * 5, i + 5, a, c);
	    }
	}
      
      /* Place Cursor */
      Term_gotoxy(35 + x * 5, y + 5);
      
      /* Get key */
      ke = inkey_ex();
      ch = ke.key;

      /* Mouse input */
      if (ch == '\xff')
	{
	  /* Prompt line first */
	  if (!ke.mousey)
	    {
	      if ((ke.mousex > 28) && (ke.mousex < 42))
		ch = 't';
	      else if ((ke.mousex > 46) && (ke.mousex < 50))
		ch = ESCAPE;
	    }


	  /* Get location */
	  for (i = 0; i < 16; i++)
	    for (j = 0; j < 8; j++)
	      {
		if ((ke.mousex == 35 + j * 5) && (ke.mousey == i + 5))
		  {
		    if ((ke.mousex == 35 + x * 5) && (ke.mousey == y + 5))
		      ch = 't';
		    else
		      {
			Term_gotoxy(ke.mousex, ke.mousey);
			x = j;
			y = i;
			continue;
		      }
		  }
	      }
	}

      /* Hack - done with the mouse */
      if (ch == '\xff') continue;
      
      /* Allow escape */
      if ((ch == ESCAPE) || (ch == 'q')) break;
      
      /* Toggle */
      if ((ch == '5') || (ch == 't'))
	{
	  /* Hack -- ignore the main window */
	  if (x == 0)
	    {
	      bell("Cannot set main window flags!");
	    }
	  
	  /* Toggle flag (off) */
	  else if (op_ptr->window_flag[x] & (1L << y))
	    {
	      op_ptr->window_flag[x] = 0;
	    }
	  
	  /* Toggle flag (on) */
	  else
	    {
	      op_ptr->window_flag[x] = (1L << y);
	    }
	  
	  /* Continue */
	  continue;
	}
      
      /* Extract direction */
      d = target_dir(ch);
      
      /* Move */
      if (d != 0)
	{
	  x = (x + ddx[d] + 8) % 8;
	  y = (y + ddy[d] + 16) % 16;
	}
      
      /* Oops */
      else
	{
	  bell("Illegal command for window options!");
	}
    }
  
  /* Notice changes */
  for (j = 0; j < 8; j++)
    {
      term *old = Term;
      
      /* Dead window */
      if (!angband_term[j]) continue;
      
      /* Ignore non-changes */
      if (op_ptr->window_flag[j] == old_flag[j]) continue;
      
      /* Activate */
      Term_activate(angband_term[j]);
      
      /* Erase */
      Term_clear();
      
      /* Refresh */
      Term_fresh();
      
      /* Restore */
      Term_activate(old);
    }
}



/*
 * Write all current options to the given preference file in the
 * lib/user directory. Modified from KAmband 1.8.
 */
static errr option_dump(cptr fname)
{
  int i, j;
  
  FILE *fff;
  
  char buf[1024];
  
  /* Build the filename */
  path_build(buf, 1024, ANGBAND_DIR_USER, fname);
  
  /* File type is "TEXT" */
  FILE_TYPE(FILE_TYPE_TEXT);
  
  /* Append to the file */
  fff = my_fopen(buf, "a");
  
  /* Failure */
  if (!fff) return (-1);
  
  
  /* Skip some lines */
  fprintf(fff, "\n\n");
  
  /* Start dumping */
  fprintf(fff, "# Automatic option dump\n\n");
  
  /* Dump options (skip cheat, adult, score) */
  for (i = 0; i < OPT_cheat_start; i++)
    {
      /* Require a real option */
      if (!option_text[i]) continue;
      
      /* Comment */
      fprintf(fff, "# Option '%s'\n", option_desc[i]);
      
      /* Dump the option */
      if (op_ptr->opt[i])
	{
	  fprintf(fff, "Y:%s\n", option_text[i]);
	}
      else
	{
	  fprintf(fff, "X:%s\n", option_text[i]);
	}
      
      /* Skip a line */
      fprintf(fff, "\n");
    }
  
  /* Dump window flags */
  for (i = 1; i < 8; i++)
    {
      /* Require a real window */
      if (!angband_term[i]) continue;
      
      /* Check each flag */
      for (j = 0; j < 32; j++)
	{
	  /* Require a real flag */
	  if (!window_flag_desc[j]) continue;
	  
	  /* Comment */
	  fprintf(fff, "# Window '%s', Flag '%s'\n",
		  angband_term_name[i], window_flag_desc[j]);
	  
	  /* Dump the flag */
	  if (op_ptr->window_flag[i] & (1L << j))
	    {
	      fprintf(fff, "W:%d:%d:1\n", i, j);
	    }
	  else
	    {
	      fprintf(fff, "W:%d:%d:0\n", i, j);
	    }
	  
	  /* Skip a line */
	  fprintf(fff, "\n");
	}
    }
  
  /* Close */
  my_fclose(fff);
  
  /* Success */
  return (0);
}

/*
 * Set or unset various options.
 *
 * After using this command, a complete redraw is performed,
 * whether or not it is technically required.
 */
void do_cmd_options(void)
{
  key_event ke;
  
  
  /* Save screen */
  screen_save();
  
  
  /* Interact */
  while (1)
    {
      /* Clear screen */
      Term_clear();
      
      if (small_screen)
	{
	  /* Why are we here */
	  prt("FAangband options (ESC to quit)", 0, 0);
	  
	  /* Give some choices */
	  prt("(1) User Interface", 1, 5);
	  prt("(2) Disturbance Options", 2, 5);
	  prt("(3) Game-Play Options", 3, 5);
	  prt("(4) Efficiency Options", 4, 5);
	  prt("(5) Birth Options", 5, 5);
	  
	  /* Cheating */
	  prt("(6) Cheat Options", 6, 5);
	  
	  /* Squelch Menus */
	  prt("(I) Item squelching", 7, 5);
	  
	  /* Special choices */
	  prt("(D) Base Delay Factor", 8, 5);
	  prt("(H) Hitpoint Warning", 9, 5);
	  prt("(P) Panel Change Factor", 10, 5);
	  prt("(A) Autosave Options", 11, 5);
	  
	  /* Load and Save */
	  prt("(R) Load a pref file", 12, 5);
	  prt("(S) Options to a file", 13, 5);
	}
      else
	{
	  /* Why are we here */
	  prt("FAangband options (ESC to quit)", 2, 0);
	  
	  /* Give some choices */
	  prt("(1) User Interface Options", 4, 5);
	  prt("(2) Disturbance Options", 5, 5);
	  prt("(3) Game-Play Options", 6, 5);
	  prt("(4) Efficiency Options", 7, 5);
	  prt("(5) Birth Options - For Character Creation (Only)", 8, 5);
	  
	  /* Cheating */
	  prt("(6) Cheat Options", 9, 5);
	  
	  /* Window flags */
	  prt("(W) Window flags", 11, 5);
	  
	  /* Squelch Menus */
	  prt("(I) Item squelching menus", 12, 5);
	  
	  /* Special choices */
	  prt("(D) Base Delay Factor", 13, 5);
	  prt("(H) Hitpoint Warning", 14, 5);
	  prt("(P) Panel Change Factor", 15, 5);
	  prt("(A) Autosave Options", 16, 5);
	  
	  /* Load and Save */
	  prt("(R) Load a user pref file", 4, 40);
	  prt("(S) Append options to a file", 5, 40);
	  
	  /* Prompt */
	  prt("Command: ", 18, 0);
	}


      /* Get command */
      ke = inkey_ex();
      
      /* Exit */
      if (ke.key == ESCAPE) break;

      /* Mouse input */
      if (ke.key == '\xff')
	{      
	  if (small_screen)
	    {
	      switch (ke.mousey)
		{
		  
		case 0:
		  {
		    if ((ke.mousex > 17) && (ke.mousex < 31)) 
		      {
			/* Flush messages */
			msg_print(NULL);
			
			/* Load screen */
			screen_load();
			
			return;
		      }
		  }
		case 1:
		  {
		    ke.key = '1';
		    break;
		  }
		case 2:
		  {
		    ke.key = '2';
		    break;
		  }
		case 3:
		  {
		    ke.key = '3';
		    break;
		  }
		case 4:
		  {
		    ke.key = '4';
		    break;
		  }
		case 5:
		  {
		    ke.key = '5';
		    break;
		  }
		case 6:
		  {
		    ke.key = '6';
		    break;
		  }
		case 7:
		  {
		    ke.key = 'I';
		    break;
		  }
		case 8:
		  {
		    ke.key = 'D';
		    break;
		  }
		case 9:
		  {
		    ke.key = 'H';
		    break;
		  }
		case 10:
		  {
		    ke.key = 'P';
		    break;
		  }
		case 11:
		  {
		    ke.key = 'A';
		    break;
		  }
		case 12:
		  {
		    ke.key = 'R';
		    break;
		  }
		case 13:
		  {
		    ke.key = 'S';
		    break;
		  }
		}
	    }
	  else
	    {
	      switch (ke.mousey)
		{
		case 2:
		  {
		    if ((ke.mousex > 17) && (ke.mousex < 31)) 
		      {
			/* Flush messages */
			msg_print(NULL);
			
			/* Load screen */
			screen_load();
			
			return;
		      }
		    break;
		  }
		case 4:
		  {
		    if (ke.mousex < 40)
		      ke.key = '1';
		    else
		      ke.key = 'R';
		    break;
		  }
		case 5:
		  {
		    if (ke.mousex < 40)
		      ke.key = '2';
		    else
		      ke.key = 'S';
		    break;
		  }
		case 6:
		  {
		    ke.key = '3';
		    break;
		  }
		case 7:
		  {
		    ke.key = '4';
		    break;
		  }
		case 8:
		  {
		    ke.key = '5';
		    break;
		  }
		case 9:
		  {
		    ke.key = '6';
		    break;
		  }
		case 11:
		  {
		    ke.key = 'W';
		    break;
		  }
		case 12:
		  {
		    ke.key = 'I';
		    break;
		  }
		case 13:
		  {
		    ke.key = 'D';
		    break;
		  }
		case 14:
		  {
		    ke.key = 'H';
		    break;
		  }
		case 15:
		  {
		    ke.key = 'P';
		    break;
		  }
		case 16:
		  {
		    ke.key = 'A';
		    break;
		  }
		}
	    }
	  
	  
	}
      
      /* Analyze */
      switch (ke.key)
	{
	  /* General Options */
	case '1':
	  {
	    /* Process the general options */
	    do_cmd_options_aux(0, "User Interface Options");
	    break;
	  }
	  
	  /* Disturbance Options */
	case '2':
	  {
	    /* Spawn */
	    do_cmd_options_aux(1, "Disturbance Options");
	    break;
	  }
	  
	  /* Inventory Options */
	case '3':
	  {
	    /* Spawn */
	    do_cmd_options_aux(2, "Game-Play Options");
	    break;
	  }
	  
	  /* Efficiency Options */
	case '4':
	  {
	    /* Spawn */
	    do_cmd_options_aux(3, "Efficiency Options");
	    break;
	  }
	  
	  /* Birth Options */
	case '5':
	  {
	    /* Spawn */
	    do_cmd_options_aux(4, "Birth Options");
	    break;
	  }
	  
	  /* Cheating Options */
	case 'C':
	case '6':
	  {
	    /* Spawn */
	    do_cmd_options_aux(5, "Cheaters never win (seriously!)");
	    break;
	  }
	  
	  /* Window flags */
	case 'W':
	case 'w':
	  {
	    /* Spawn */
	    do_cmd_options_win();
	    break;
	  }
	  
	  /* Squelching Menus */
	case 'I':
	case 'i':
	  {
	    /* Spawn */
	    do_cmd_squelch();
	    break;
	  }
	  
	  /* Hack -- Delay Speed */
	case 'D':
	case 'd':
	  {
	    /* Prompt */

	    if (!small_screen)
	      prt("Command: Base Delay Factor", 18, 0);
	    

	    /* Get a new value */
	    while (1)
	      {
		int line = 22, dd = 2;
		int msec = op_ptr->delay_factor * op_ptr->delay_factor;
		prt(format("Current factor: %d (%d msec)",
			   op_ptr->delay_factor, msec), line, 0);

		if (small_screen)
		  {
		    line = 15;
		    dd = 1;
		    prt("Delay Factor (-- or ++)", line - dd, 0);
		  }
		else
		  {
		    prt("Delay Factor (-- or ++)/(0-9 or ESC to accept): ", 
			line -dd, 0);
		  }


		ke = inkey_ex();
		if (ke.key == ESCAPE) break;
		if (isdigit(ke.key)) op_ptr->delay_factor = D2I(ke.key);
		else if ((ke.key == '\xff') && (ke.mousey == line - dd) && 
			 ((ke.mousex == 14) || (ke.mousex == 15)) &&
			 (op_ptr->delay_factor))
		  op_ptr->delay_factor--;
		else if ((ke.key == '\xff') && (ke.mousey == line - dd) && 
			 ((ke.mousex == 20) || (ke.mousex == 21)) &&
			 (op_ptr->delay_factor < 9))
		  op_ptr->delay_factor++;
		else if ((ke.key == '\xff') && (ke.mousey == line - dd) && 
			 (ke.mousex > 31) && (ke.mousex < 35))
		  break;
		else bell("Illegal delay factor!");
	      }
	    
	    break;
	  }
	  
	  /* Hack -- Panel Change */
	case 'P':
	case 'p':
	  {
	    /* Prompt */

	    if (!small_screen)
	      prt("Command: Panel Change Factor", 18, 0);

	    
	    /* Get a new value */
	    while (1)
	      {
		int pdist = (op_ptr->panel_change + 1) * 2;
		int line = 22, dd = 2;

		if (small_screen) 
		  {
		    line = 15;
		    dd = 1;
		    prt(format("Current change: %d (%d/%d)", 
			       op_ptr->panel_change, pdist, pdist * 2), 15, 0);
		    prt("Panel Change (-- or ++)", 14, 0);
		  }
		else
		  {  
		    prt(format("Current panel change factor: %d (%d from top/bottom, %d from sides)",
			       op_ptr->panel_change, pdist, pdist * 2), 22, 0);
		    prt("Panel Change (-- or ++)/(0-4 or ESC to accept): ", 
			20, 0);
		  }


		ke = inkey_ex();
		if (ke.key == ESCAPE) break;
		if (isdigit(ke.key) && (D2I(ke.key) < 5)) 
		  op_ptr->panel_change = D2I(ke.key);
		else if ((ke.key == '\xff') && (ke.mousey == line - dd) && 
			 ((ke.mousex == 14) || (ke.mousex == 15)) &&
			 (op_ptr->panel_change))
		  op_ptr->panel_change--;
		else if ((ke.key == '\xff') && (ke.mousey == line - dd) && 
			 ((ke.mousex == 20) || (ke.mousex == 21)) &&
			 (op_ptr->panel_change < 4))
		  op_ptr->panel_change++;
		else if ((ke.key == '\xff') && (ke.mousey == line - dd) && 
			 (ke.mousex > 31) && (ke.mousex < 35))
		  break;
		else bell("Illegal panel change!");
	      }
	    
	    break;
	  }
	  /* Hack -- hitpoint warning factor */
	case 'H':
	case 'h':
	  {
	    /* Prompt */
	    if (!small_screen)
	      prt("Command: Hitpoint Warning", 18, 0);


	    
	    /* Get a new value */
	    while (1)
	      {
		int line = 22, dd = 2;
		
		prt(format("Current hitpoint warning:%2d%%",
			   op_ptr->hitpoint_warn * 10), line, 0);

		if (small_screen) 
		  {
		    line = 15;
		    dd = 1;
		    prt("Hitpoint Warning (-- or ++)", 14, 0);
		  }
		else
		  {
		    prt("Hitpoint Warning (-- or ++)/(0-9 or ESC to accept): ", 
			20, 0);
		  }

		
		ke = inkey_ex();
		if (ke.key == ESCAPE) break;
		if (isdigit(ke.key)) op_ptr->hitpoint_warn = D2I(ke.key);
		else if ((ke.key == '\xff') && (ke.mousey == line - dd) && 
			 ((ke.mousex == 18) || (ke.mousex == 19)) &&
			 (op_ptr->hitpoint_warn))
		  op_ptr->hitpoint_warn--;
		else if ((ke.key == '\xff') && (ke.mousey == line - dd) && 
			 ((ke.mousex == 24) || (ke.mousex == 25)) &&
			 (op_ptr->hitpoint_warn < 9))
		  op_ptr->hitpoint_warn++;
		else if ((ke.key == '\xff') && (ke.mousey == line - dd) && 
			 ((ke.mousex > 35) || (ke.mousex < 39)))
		  break;
		else bell("Illegal hitpoint warning!");
	      }
	    
	    break;
	  }  
	case 'a':
	case 'A':
	  {
	    (void) do_cmd_options_autosave("Autosave");
	    break;
	  }
	  
	  
	  	  
	case 'r':
	case 'R':
	  {
	    /* Ask for and load a user pref file */
	    (void) do_cmd_pref_file_hack(18);
	    break;
	  }
	  
	case 's':
	case 'S':
	  {
	    char ftmp[80];
	    
	    /* Prompt */
	    prt("Command: Write options to a file", 18, 0);
	    
	    /* Prompt */
	    prt("File: ", 20, 0);
	    
	    /* Default filename */
	    sprintf(ftmp, "%s.prf", op_ptr->base_name);
	    
	    /* Ask for a file */
	    if (!askfor_aux(ftmp, 80)) break;
	    
	    /* Drop priv's */
	    safe_setuid_drop();
	    
	    /* Dump the options */
	    if (option_dump(ftmp))
	      {
		/* Failure */
		msg_print("Failed!");
	      }
	    else
	      {
		/* Success */
		msg_print("Done.");
	      }
	    
	    /* Grab priv's */
	    safe_setuid_grab();
	    
	    break;
	  }
	  
	  /* Unknown option */
	default:
	  {
	    /* Oops */
	    bell("Illegal command for options!");
	    break;
	  }
	}
      
      /* Flush messages */
      msg_print(NULL);
    }
  
  
  /* Load screen */
  screen_load();

  /* Hack - update panel in case of screen, frame option changes */
  verify_panel();
}


#ifdef ALLOW_MACROS

/*
 * Hack -- append all current macros to the given file
 */
static errr macro_dump(cptr fname)
{
  int i;
  
  FILE *fff;
  
  char buf[1024];
  
  
  /* Build the filename */
  path_build(buf, 1024, ANGBAND_DIR_USER, fname);
  
  /* File type is "TEXT" */
  FILE_TYPE(FILE_TYPE_TEXT);
  
  /* Append to the file */
  fff = my_fopen(buf, "a");
  
  /* Failure */
  if (!fff) return (-1);
  
  
  /* Skip space */
  fprintf(fff, "\n\n");
  
  /* Start dumping */
  fprintf(fff, "# Automatic macro dump\n\n");
  
  /* Dump them */
  for (i = 0; i < macro__num; i++)
    {
      /* Start the macro */
      fprintf(fff, "# Macro '%d'\n\n", i);
      
      /* Extract the action */
      ascii_to_text(buf, macro__act[i]);
      
      /* Dump the macro */
      fprintf(fff, "A:%s\n", buf);
      
      /* Extract the action */
      ascii_to_text(buf, macro__pat[i]);
      
      /* Dump normal macros */
      fprintf(fff, "P:%s\n", buf);
      
      /* End the macro */
      fprintf(fff, "\n\n");
    }
  
  /* Start dumping */
  fprintf(fff, "\n\n\n\n");
  
  
  /* Close */
  my_fclose(fff);
  
  /* Success */
  return (0);
}


/*
 * Hack -- ask for a "trigger" (see below)
 *
 * Note the complex use of the "inkey()" function from "util.c".
 *
 * Note that both "flush()" calls are extremely important.  This may
 * no longer be true, since "util.c" is much simpler now.  XXX XXX XXX
 */
static void do_cmd_macro_aux(char *buf)
{
  int i, n = 0;
  
  char tmp[1024];
  
  
  /* Flush */
  flush();
  
  /* Do not process macros */
  inkey_base = TRUE;
  
  /* First key */
  i = inkey();
  
  /* Read the pattern */
  while (i)
    {
      /* Save the key */
      buf[n++] = i;
      
      /* Do not process macros */
      inkey_base = TRUE;
      
      /* Do not wait for keys */
      inkey_scan = TRUE;
      
      /* Attempt to read a key */
      i = inkey();
    }
  
  /* Terminate */
  buf[n] = '\0';
  
  /* Flush */
  flush();
  
  
  /* Convert the trigger */
  ascii_to_text(tmp, buf);
  
  /* Hack -- display the trigger */
  Term_addstr(-1, TERM_WHITE, tmp);
}


/*
 * Hack -- ask for a keymap "trigger" (see below)
 *
 * Note that both "flush()" calls are extremely important.  This may
 * no longer be true, since "util.c" is much simpler now.  XXX XXX XXX
 */
static void do_cmd_macro_aux_keymap(char *buf)
{
  char tmp[1024];
  
  
  /* Flush */
  flush();
  
  
  /* Get a key */
  buf[0] = inkey();
  buf[1] = '\0';
  
  
  /* Convert to ascii */
  ascii_to_text(tmp, buf);
  
  /* Hack -- display the trigger */
  Term_addstr(-1, TERM_WHITE, tmp);
  
  
  /* Flush */
  flush();
}


/*
 * Hack -- append all keymaps to the given file
 */
static errr keymap_dump(cptr fname)
{
  int i;
  
  FILE *fff;
  
  char key[1024];
  char buf[1024];
  
  int mode;
  
  
  /* Roguelike */
  if (rogue_like_commands)
    {
      mode = KEYMAP_MODE_ROGUE;
    }
  
  /* Original */
  else
    {
      mode = KEYMAP_MODE_ORIG;
    }
  
  
  /* Build the filename */
  path_build(buf, 1024, ANGBAND_DIR_USER, fname);
  
  /* File type is "TEXT" */
  FILE_TYPE(FILE_TYPE_TEXT);
  
  /* Append to the file */
  fff = my_fopen(buf, "a");
  
  /* Failure */
  if (!fff) return (-1);
  
  
  /* Skip space */
  fprintf(fff, "\n\n");
  
  /* Start dumping */
  fprintf(fff, "# Automatic keymap dump\n\n");
  
  /* Dump them */
  for (i = 0; i < 256; i++)
    {
      cptr act;
      
      /* Loop up the keymap */
      act = keymap_act[mode][i];
      
      /* Skip empty keymaps */
      if (!act) continue;
      
      /* Start the keymap */
      fprintf(fff, "# Keymap '%d'\n\n", i);
      
      /* Encode the action */
      ascii_to_text(buf, act);
      
      /* Dump the action */
      fprintf(fff, "A:%s\n", buf);
      
      /* Encode the key */
      buf[0] = i;
      buf[1] = '\0';
      ascii_to_text(key, buf);
      
      /* Dump the key */
      fprintf(fff, "C:%d:%s\n", mode, key);
      
      /* End the keymap */
      fprintf(fff, "\n\n");
    }
  
  /* Start dumping */
  fprintf(fff, "\n\n\n");
  
  
  /* Close */
  my_fclose(fff);
  
  /* Success */
  return (0);
}


#endif

/*
 * Interact with "macros"
 *
 * Note that the macro "action" must be defined before the trigger.
 *
 * Could use some helpful instructions on this page.  XXX XXX XXX
 */
void do_cmd_macros(void)
{
  int i;

  key_event ke;
  
  char tmp[1024];
  
  char buf[1024];
  
  int mode;
  
  
  /* Roguelike */
  if (rogue_like_commands)
    {
      mode = KEYMAP_MODE_ROGUE;
    }
  
  /* Original */
  else
    {
      mode = KEYMAP_MODE_ORIG;
    }
  
  
  /* File type is "TEXT" */
  FILE_TYPE(FILE_TYPE_TEXT);
  
  
  /* Save screen */
  screen_save();
  
  
  /* Process requests until done */
  while (1)
    {
      /* Clear screen */
      Term_clear();
      
      /* Describe */
      prt("Interact with Macros", 1, 0);
      prt("ESC to quit", 2, 0);
      
      
      /* Describe that action */
      prt("Current action (if any) shown below:", 16, 0);
      
      /* Analyze the current action */
      ascii_to_text(buf, macro_buffer);
      
      /* Display the current action */
      prt(buf, 17, 0);
      
      
      /* Selections */
      prt("(1) Load a user pref file", 3, 5);
#ifdef ALLOW_MACROS
      prt("(2) Append macros to a file", 4, 5);
      prt("(3) Query a macro", 5, 5);
      prt("(4) Create a macro", 6, 5);
      prt("(5) Remove a macro", 7, 5);
      prt("(6) Append keymaps to a file", 8, 5);
      prt("(7) Query a keymap", 9, 5);
      prt("(8) Create a keymap", 10, 5);
      prt("(9) Remove a keymap", 11, 5);
      prt("(0) Enter a new action", 12, 5);
#endif /* ALLOW_MACROS */
      
      /* Prompt */
      prt("Command: ", 13, 0);
      
      /* Get a command */
      ke = inkey_ex();
      i = ke.key;

      /* Mouse input */
      if (i == '\xff')
	{
	  if (ke.mousey == 2) 
	    i = ESCAPE;
	  else if ((ke.mousey > 1) && (ke.mousey < 13))
	    {
	      i = I2D((ke.mousey - 2) % 10);
	      prt("(keyboard input only!)", 3, 0);
	    }
	}
      
      /* Leave */
      if (i == ESCAPE) break;
      
      /* Load a 'macro' file */
      else if (i == '1')
	{
	  /* Ask for and load a user pref file */
	  (void) do_cmd_pref_file_hack(13);
	}
      
#ifdef ALLOW_MACROS
      
      /* Save macros */
      else if (i == '2')
	{
	  /* Prompt */
	  prt("Command: Append macros to a file", 13, 0);
	  
	  /* Prompt */
	  prt("File: ", 15, 0);
	  
	  /* Default filename */
	  sprintf(tmp, "%s.prf", op_ptr->base_name);
	  
	  /* Ask for a file */
	  if (!askfor_aux(tmp, 80)) continue;
	  
	  /* Drop priv's */
	  safe_setuid_drop();
	  
	  /* Dump the macros */
	  (void)macro_dump(tmp);
	  
	  /* Grab priv's */
	  safe_setuid_grab();
	  
	  /* Prompt */
	  msg_print("Appended macros.");
	}
      
      /* Query a macro */
      else if (i == '3')
	{
	  int k;
	  
	  /* Prompt */
	  prt("Command: Query a macro", 13, 0);
	  
	  /* Prompt */
	  prt("Trigger: ", 15, 0);
	  
	  /* Get a macro trigger */
	  do_cmd_macro_aux(buf);
	  
	  /* Acquire action */
	  k = macro_find_exact(buf);
	  
	  /* Nothing found */
	  if (k < 0)
	    {
	      /* Prompt */
	      msg_print("Found no macro.");
	    }
	  
	  /* Found one */
	  else
	    {
	      /* Obtain the action */
	      strcpy(macro_buffer, macro__act[k]);
	      
	      /* Analyze the current action */
	      ascii_to_text(buf, macro_buffer);
	      
	      /* Display the current action */
	      prt(buf, 17, 0);
	      
	      /* Prompt */
	      msg_print("Found a macro.");
	    }
	}
      
      /* Create a macro */
      else if (i == '4')
	{
	  /* Prompt */
	  prt("Command: Create a macro", 13, 0);
	  
	  /* Prompt */
	  prt("Trigger: ", 15, 0);
	  
	  /* Get a macro trigger */
	  do_cmd_macro_aux(buf);
	  
	  /* Clear */
	  clear_from(17);
	  
	  /* Prompt */
	  prt("Action: ", 17, 0);
	  
	  /* Convert to text */
	  ascii_to_text(tmp, macro_buffer);
	  
	  /* Get an encoded action */
	  if (askfor_aux(tmp, 80))
	    {
	      /* Convert to ascii */
	      text_to_ascii(macro_buffer, tmp);
	      
	      /* Link the macro */
	      macro_add(buf, macro_buffer);
	      
	      /* Prompt */
	      msg_print("Added a macro.");
	    }
	}
      
      /* Remove a macro */
      else if (i == '5')
	{
	  /* Prompt */
	  prt("Command: Remove a macro", 13, 0);
	  
	  /* Prompt */
	  prt("Trigger: ", 15, 0);
	  
	  /* Get a macro trigger */
	  do_cmd_macro_aux(buf);
	  
	  /* Link the macro */
	  macro_add(buf, buf);
	  
	  /* Prompt */
	  msg_print("Removed a macro.");
	}
      
      /* Save keymaps */
      else if (i == '6')
	{
	  /* Prompt */
	  prt("Command: Append keymaps to a file", 13, 0);
	  
	  /* Prompt */
	  prt("File: ", 15, 0);
	  
	  /* Default filename */
	  sprintf(tmp, "%s.prf", op_ptr->base_name);
	  
	  /* Ask for a file */
	  if (!askfor_aux(tmp, 80)) continue;
	  
	  /* Drop priv's */
	  safe_setuid_drop();
	  
	  /* Dump the macros */
	  (void)keymap_dump(tmp);
	  
	  /* Grab priv's */
	  safe_setuid_grab();
	  
	  /* Prompt */
	  msg_print("Appended keymaps.");
	}
      
      /* Query a keymap */
      else if (i == '7')
	{
	  cptr act;
	  
	  /* Prompt */
	  prt("Command: Query a keymap", 13, 0);
	  
	  /* Prompt */
	  prt("Keypress: ", 15, 0);
	  
	  /* Get a keymap trigger */
	  do_cmd_macro_aux_keymap(buf);
	  
	  /* Look up the keymap */
	  act = keymap_act[mode][(byte)(buf[0])];
	  
	  /* Nothing found */
	  if (!act)
	    {
	      /* Prompt */
	      msg_print("Found no keymap.");
	    }
	  
	  /* Found one */
	  else
	    {
	      /* Obtain the action */
	      strcpy(macro_buffer, act);
	      
	      /* Analyze the current action */
	      ascii_to_text(buf, macro_buffer);
	      
	      /* Display the current action */
	      prt(buf, 17, 0);
	      
	      /* Prompt */
	      msg_print("Found a keymap.");
	    }
	}
      
      /* Create a keymap */
      else if (i == '8')
	{
	  /* Prompt */
	  prt("Command: Create a keymap", 13, 0);
	  
	  /* Prompt */
	  prt("Keypress: ", 15, 0);
	  
	  /* Get a keymap trigger */
	  do_cmd_macro_aux_keymap(buf);
	  
	  /* Clear */
	  clear_from(16);
	  
	  /* Prompt */
	  prt("Action: ", 16, 0);
	  
	  /* Convert to text */
	  ascii_to_text(tmp, macro_buffer);
	  
	  /* Get an encoded action */
	  if (askfor_aux(tmp, 80))
	    {
	      /* Convert to ascii */
	      text_to_ascii(macro_buffer, tmp);
	      
	      /* Free old keymap */
	      string_free(keymap_act[mode][(byte)(buf[0])]);
	      
	      /* Make new keymap */
	      keymap_act[mode][(byte)(buf[0])] = string_make(macro_buffer);
	      
	      /* Prompt */
	      msg_print("Added a keymap.");
	      
	      angband_keymap_flag = TRUE;
	    }
	}
      
      /* Remove a keymap */
      else if (i == '9')
	{
	  /* Prompt */
	  prt("Command: Remove a keymap", 13, 0);
	  
	  /* Prompt */
	  prt("Keypress: ", 15, 0);
	  
	  /* Get a keymap trigger */
	  do_cmd_macro_aux_keymap(buf);
	  
	  /* Free old keymap */
	  string_free(keymap_act[mode][(byte)(buf[0])]);
	  
	  /* Make new keymap */
	  keymap_act[mode][(byte)(buf[0])] = NULL;
	  
	  /* Prompt */
	  msg_print("Removed a keymap.");
	  
	  angband_keymap_flag = TRUE;
	}
      
      /* Enter a new action */
      else if (i == '0')
	{
	  /* Prompt */
	  prt("Command: Enter a new action", 13, 0);
	  
	  /* Go to the correct location */
	  Term_gotoxy(0, 17);
	  
	  /* Hack -- limit the value */
	  tmp[80] = '\0';
	  
	  /* Get an encoded action */
	  if (!askfor_aux(buf, 80)) continue;
	  
	  /* Extract an action */
	  text_to_ascii(macro_buffer, buf);
	}
      
#endif /* ALLOW_MACROS */
      
      /* Oops */
      else
	{
	  /* Oops */
	  bell("Illegal command for macros!");
	}
      
      /* Flush messages */
      msg_print(NULL);
    }
  
  
  /* Load screen */
  screen_load();
}


/* String used to show a color sample */
#define COLOR_SAMPLE (small_screen ? "##" : "###")

/*
 * Asks the player for an extended color. It is done in two steps:
 * 1. Asks for the base color.
 * 2. Asks for a specific shade.
 * It erases the given line.
 * If the user press ESCAPE no changes are made to attr.
 */
static void askfor_shade(byte *attr, int y)
{
  byte base, shade, temp;
  bool changed = FALSE;
  char *msg, *pos;
  key_event ke;
  
  /* Start with the given base color */
  base = GET_BASE_COLOR(*attr);
  
  /* 1. Query for base color */
  while (1)
    {
      /* Clear the line */
      Term_erase(0, y, 255);
      Term_erase(0, y + 1, 255);
      
      /* Prompt */
      c_put_str(TERM_WHITE, "1. Choose base color (use arrows or ++/--/done):"
		, y, 0);
      
      /* Format the query */
      msg = format("%s %s (attr = %d) ", COLOR_SAMPLE, 
		   color_names[base], base);
      
      /* Display it */
      c_put_str(TERM_WHITE, msg, y + 1, 3);
      
      /* Find the sample */
      pos = strstr(msg, COLOR_SAMPLE);
      
      /* Show it using the proper color */
      c_put_str(base, COLOR_SAMPLE, y + 1, pos - msg + 3);
      
      /* Place the cursor at the end of the message */
      Term_gotoxy(strlen(msg) + 3, y + 1);
      
      /* Get a command */
      ke = inkey_ex();

      /* Mouse Input */
      if (ke.key == '\xff')
	{
	  if (ke.mousey == 3) ke.key = ESCAPE;
	  else if (ke.mousey == y)
	    {
	      if ((ke.mousex > 35) && (ke.mousex < 38)) ke.key = '6';
	      if ((ke.mousex > 38) && (ke.mousex < 41)) ke.key = '4';
	      if ((ke.mousex > 41) && (ke.mousex < 46)) ke.key = '\r';
	    }
	}
      
      /* Cancel */
      if (ke.key == ESCAPE)
    	{
	  /* Clear the line */
	  Term_erase(0, y, 255);
	  Term_erase(0, y + 1, 255);
	  return;
    	}
      
      /* Accept the current base color */
      if ((ke.key == '\r') || (ke.key == '\n')) break;
      
      /* Move to the previous color if possible */
      if (((ke.key == '4') || (ke.key == '-')) && (base > 0))
    	{
	  --base;
	  /* Reset the shade, see below */
	  changed = TRUE;
	  continue;
    	}
      
      /* Move to the next color if possible */
      if (((ke.key == '6') || (ke.key == '+')) && (base < MAX_BASE_COLORS - 1))
    	{
	  ++base;
	  /* Reset the shade, see below */
	  changed = TRUE;
	  continue;
    	}
    }
  
  /* The player selected a different base color, start from shade 0 */
  if (changed)	shade = 0;
  /* We assume that the player is editing the current shade, go there */
  else		shade = GET_SHADE(*attr);
  
  /* 2. Query for specific shade */
  while (1)
    {
      /* Clear the line */
      Term_erase(0, y, 255);
      Term_erase(0, y + 1, 255);
      
      /* Create the real color */
      temp = MAKE_EXTENDED_COLOR(base, shade);
      
      /* Prompt */
      c_put_str(TERM_WHITE, "2. Choose shade (use arrows or ++/--/done):"
		, y, 0);
      
      /* Format the message */
      msg = format("%s %s (attr = %d) ", COLOR_SAMPLE, 
		   get_ext_color_name(temp), temp);
      
      /* Display it */
      c_put_str(TERM_WHITE, msg, y + 1, 3);
      
      /* Find the sample */
      pos = strstr(msg, COLOR_SAMPLE);
      
      /* Show it using the proper color */
      c_put_str(temp, COLOR_SAMPLE, y + 1, pos - msg + 3);
      
      /* Place the cursor at the end of the message */
      Term_gotoxy(strlen(msg) + 3, y + 1);
      
      /* Get a command */
      ke = inkey_ex();
      
      /* Mouse Input */
      if (ke.key == '\xff')
	{
	  if (ke.mousey == 3) ke.key = ESCAPE;
	  else if (ke.mousey == y)
	    {
	      if ((ke.mousex > 30) && (ke.mousex < 33)) ke.key = '6';
	      if ((ke.mousex > 33) && (ke.mousex < 36)) ke.key = '4';
	      if ((ke.mousex > 36) && (ke.mousex < 41)) ke.key = '\r';
	    }
	}
      
      /* Cancel */
      if (ke.key == ESCAPE)
    	{
	  /* Clear the line */
	  Term_erase(0, y, 255);
	  Term_erase(0, y + 1, 255);
	  return;
    	}
      
      /* Accept the current shade */
      if ((ke.key == '\r') || (ke.key == '\n')) break;
      
      /* Move to the previous shade if possible */
      if (((ke.key == '4') || (ke.key == '-')) && (shade > 0))
    	{
	  --shade;
	  continue;
    	}
      
      /* Move to the next shade if possible */
      if (((ke.key == '6') || (ke.key == '+')) && (shade < MAX_SHADES - 1))
    	{
	  ++shade;
	  continue;
    	}
    }
  
  /* Assign the selected shade */
  *attr = temp;
  
  /* Clear the line. It is needed to fit in the current UI */
  Term_erase(0, y, 255);
  Term_erase(0, y + 1, 255);
}

/*
 * Interact with "visuals"
 */
void do_cmd_visuals(void)
{
  int i;

  key_event ke;
  
  FILE *fff;
  
  char tmp[160];
  
  char buf[1024];
  
  const char *empty_symbol = "<< ? >>";
  const char *empty_symbol2 = "\0";
  const char *empty_symbol3 = "\0";
  
  if (use_trptile && use_bigtile)
    {
      empty_symbol = "// ?????? \\\\";
      empty_symbol2 = "   ??????   ";
      empty_symbol3 = "\\\\ ?????? //";
    }
  else if (use_dbltile && use_bigtile)
    {
      empty_symbol = "// ???? \\\\";
      empty_symbol2 = "\\\\ ???? //";
    }
  else if (use_trptile)
    {
      empty_symbol = "// ??? \\\\";
      empty_symbol2 = "   ???   ";
      empty_symbol3 = "\\\\ ??? //";
    }
  else if (use_dbltile)
    {
      empty_symbol = "// ?? \\\\";
      empty_symbol2 = "\\\\ ?? //";
    }
  else if (use_bigtile) empty_symbol = "<< ?? >>";
  
  /* File type is "TEXT" */
  FILE_TYPE(FILE_TYPE_TEXT);
  
  
  /* Save screen */
  screen_save();
  
  
  /* Interact until done */
  while (1)
    {
      /* Clear screen */
      Term_clear();
      
      /* Ask for a choice */
      prt("Interact with Visuals", 2, 0);
      prt("ESC to quit", 3, 0);
      
      /* Give some choices */
      prt("(1) Load a user pref file", 4, 5);
#ifdef ALLOW_VISUALS
      prt("(2) Dump monster attr/chars", 5, 5);
      prt("(3) Dump object attr/chars", 6, 5);
      prt("(4) Dump feature attr/chars", 7, 5);
		prt("(5) Dump flavor attr/chars", 8, 5);
      prt("(6) Change monster attr/chars", 9, 5);
      prt("(7) Change object attr/chars", 10, 5);
      prt("(8) Change feature attr/chars", 11, 5);
		prt("(9) Change flavor attr/chars", 12, 5);
#endif
      prt("(0) Reset visuals", 13, 5);
      
      /* Prompt */
      prt("Command: ", 15, 0);
      
      /* Prompt */
      ke = inkey_ex();
      i = ke.key;

      /* Mouse input */
      if (i == '\xff')
	{
	  if (ke.mousey == 3) 
	    i = ESCAPE;
	  else if ((ke.mousey > 3) && (ke.mousey < 14))
	    i = I2D((ke.mousey - 3) % 10);
	}
      
      /* Done */
      if (i == ESCAPE) break;

      /* Load a 'pref' file */
      else if (i == '1')
	{
	  /* Ask for and load a user pref file */
	  (void) do_cmd_pref_file_hack(15);
	}
      
#ifdef ALLOW_VISUALS
      
      /* Dump monster attr/chars */
      else if (i == '2')
	{
	  /* Prompt */
	  prt("Command: Dump monster attr/chars", 15, 0);
	  
	  /* Prompt */
	  prt("File: ", 17, 0);
	  
	  /* Default filename */
	  sprintf(tmp, "%s.prf", op_ptr->base_name);
	  
	  /* Get a filename */
	  if (!askfor_aux(tmp, 80)) continue;
	  
	  /* Build the filename */
	  path_build(buf, 1024, ANGBAND_DIR_USER, tmp);
	  
	  /* Drop priv's */
	  safe_setuid_drop();
	  
	  /* Append to the file */
	  fff = my_fopen(buf, "a");
	  
	  /* Grab priv's */
	  safe_setuid_grab();
	  
	  /* Failure */
	  if (!fff) continue;
	  
	  /* Start dumping */
	  fprintf(fff, "\n\n");
	  fprintf(fff, "# Monster attr/char definitions\n\n");
	  
	  /* Dump monsters */
	  for (i = 0; i < MAX_R_IDX; i++)
	    {
	      monster_race *r_ptr = &r_info[i];
	      
	      /* Skip non-entries */
	      if (!r_ptr->name) continue;
	      
	      /* Dump a comment */
	      fprintf(fff, "# %s\n", (r_name + r_ptr->name));
	      
	      /* Dump the monster attr/char info */
	      fprintf(fff, "R:%d:0x%02X:0x%02X\n\n", i,
		      (byte)(r_ptr->x_attr), (byte)(r_ptr->x_char));
	    }
	  
	  /* All done */
	  fprintf(fff, "\n\n\n\n");
	  
	  /* Close */
	  my_fclose(fff);
	  
	  /* Message */
	  msg_print("Dumped monster attr/chars.");
	}
      
      /* Dump object attr/chars */
      else if (i == '3')
	{
	  /* Prompt */
	  prt("Command: Dump object attr/chars", 15, 0);
	  
	  /* Prompt */
	  prt("File: ", 17, 0);
	  
	  /* Default filename */
	  sprintf(tmp, "%s.prf", op_ptr->base_name);
	  
	  /* Get a filename */
	  if (!askfor_aux(tmp, 80)) continue;
	  
	  /* Build the filename */
	  path_build(buf, 1024, ANGBAND_DIR_USER, tmp);
	  
	  /* Drop priv's */
	  safe_setuid_drop();
	  
	  /* Append to the file */
	  fff = my_fopen(buf, "a");
	  
	  /* Grab priv's */
	  safe_setuid_grab();
	  
	  /* Failure */
	  if (!fff) continue;
	  
	  /* Start dumping */
	  fprintf(fff, "\n\n");
	  fprintf(fff, "# Object attr/char definitions\n\n");
	  
	  /* Dump objects */
	  for (i = 0; i < MAX_K_IDX; i++)
	    {
	      object_kind *k_ptr = &k_info[i];
	      
	      /* Skip non-entries */
	      if (!k_ptr->name) continue;
	      
	      /* Dump a comment */
	      fprintf(fff, "# %s\n", (k_name + k_ptr->name));
	      
	      /* Dump the object attr/char info */
	      fprintf(fff, "K:%d:0x%02X:0x%02X\n\n", i,
		      (byte)(k_ptr->x_attr), (byte)(k_ptr->x_char));
	    }
	  
	  /* All done */
	  fprintf(fff, "\n\n\n\n");
	  
	  /* Close */
	  my_fclose(fff);
	  
	  /* Message */
	  msg_print("Dumped object attr/chars.");
	}
      
      /* Dump feature attr/chars */
      else if (i == '4')
	{
	  /* Prompt */
	  prt("Command: Dump feature attr/chars", 15, 0);
	  
	  /* Prompt */
	  prt("File: ", 17, 0);
	  
	  /* Default filename */
	  sprintf(tmp, "%s.prf", op_ptr->base_name);
	  
	  /* Get a filename */
	  if (!askfor_aux(tmp, 80)) continue;
	  
	  /* Build the filename */
	  path_build(buf, 1024, ANGBAND_DIR_USER, tmp);
	  
	  /* Drop priv's */
	  safe_setuid_drop();
	  
	  /* Append to the file */
	  fff = my_fopen(buf, "a");
	  
	  /* Grab priv's */
	  safe_setuid_grab();
	  
	  /* Failure */
	  if (!fff) continue;
	  
	  /* Start dumping */
	  fprintf(fff, "\n\n");
	  fprintf(fff, "# Feature attr/char definitions\n\n");
	  
	  /* Dump features */
	  for (i = 0; i < MAX_F_IDX; i++)
	    {
	      feature_type *f_ptr = &f_info[i];
	      
	      /* Skip non-entries */
	      if (!f_ptr->name) continue;
	      
	      /* Dump a comment */
	      fprintf(fff, "# %s\n", (f_name + f_ptr->name));
	      
	      /* Dump the feature attr/char info */
	      fprintf(fff, "F:%d:0x%02X:0x%02X\n\n", i,
		      (byte)(f_ptr->x_attr), (byte)(f_ptr->x_char));
	    }
	  
	  /* All done */
	  fprintf(fff, "\n\n\n\n");
	  
	  /* Close */
	  my_fclose(fff);
	  
	  /* Message */
	  msg_print("Dumped feature attr/chars.");
	}
      
      /* Dump flavor attr/chars */
      else if (i == '5')
	{
	  char ftmp[80];
	  
	  /* Prompt */
	  prt("Command: Dump flavor attr/chars", 15, 0);
	  
	  /* Prompt */
	  prt("File: ", 17, 0);
	  
	  /* Default filename */
	  strnfmt(ftmp, sizeof(ftmp), "%s.prf", op_ptr->base_name);
	  
	  /* Get a filename */
	  if (!askfor_aux(ftmp, sizeof(ftmp))) continue;
	  
	  /* Build the filename */
	  path_build(buf, sizeof(buf), ANGBAND_DIR_USER, ftmp);
	  
	  /* Drop priv's */
	  safe_setuid_drop();
	  
	  /* Append to the file */
	  fff = my_fopen(buf, "a");
	  
	  /* Grab priv's */
	  safe_setuid_grab();
	  
	  /* Failure */
	  if (!fff) continue;
	  
	  /* Start dumping */
	  fprintf(fff, "\n\n");
	  fprintf(fff, "# Flavor attr/char definitions\n\n");
	  
	  /* Skip some lines */
	  fprintf(fff, "\n\n");
	  
	  /* Start dumping */
	  fprintf(fff, "# Flavor attr/char definitions\n\n");
	  
	  /* Dump flavors */
	  for (i = 0; i < MAX_FL_IDX; i++)
	    {
	      flavor_type *flavor_ptr = &flavor_info[i];
	      
	      /* Dump a comment */
	      fprintf(fff, "# %s\n", (flavor_text + flavor_ptr->text));
	      
	      /* Dump the flavor attr/char info */
	      fprintf(fff, "L:%d:0x%02X:0x%02X\n\n", i,
		      (byte)(flavor_ptr->x_attr), (byte)(flavor_ptr->x_char));
	    }
	  
	  /* All done */
	  fprintf(fff, "\n\n\n\n");
	  
	  /* Close */
	  my_fclose(fff);
	  
	  /* Message */
	  msg_print("Dumped flavor attr/chars.");
	}
      
      /* Modify monster attr/chars */
      else if (i == '6')
	{
	  static int r = 0;
	  
	  /* Prompt */
	  prt("Command: Change monster attr/chars", 15, 0);
	  
	  /* Hack -- query until done */
	  while (1)
	    {
	      monster_race *r_ptr = &r_info[r];
	      
	      byte da = (byte)(r_ptr->d_attr);
	      byte dc = (byte)(r_ptr->d_char);
	      byte ca = (byte)(r_ptr->x_attr);
	      byte cc = (byte)(r_ptr->x_char);
	      
	      int linec = (use_trptile ? 20: (use_dbltile ? 19 : 18));

	      /* Label the object */
	      Term_putstr(3, 16, -1, TERM_WHITE,
			  format("Monster = %d, Name = %-40.40s",
				 r, (r_name + r_ptr->name)));
	      
	      /* Label the Default values */
	      Term_putstr(5, 17, -1, TERM_WHITE,
			  format("Default attr/char = %3u / %3u", da, dc));
	      Term_putstr(35, 17, -1, TERM_WHITE, empty_symbol);
	      if (use_dbltile || use_trptile) 
		Term_putstr (35, 18, -1, TERM_WHITE, empty_symbol2);
	      if (use_trptile) 
		Term_putstr (35, 19, -1, TERM_WHITE, empty_symbol3);
	      
	      Term_putch(38, 17, da, dc);
	      
#ifdef USE_TRANSPARENCY
	      if (use_bigtile || use_dbltile || use_trptile)
		{
		  big_putch(38, 17, da, dc);
		}
#endif
	      
	      /* Label the Current values */
	      Term_putstr(5, linec, -1, TERM_WHITE,
			  format("Current attr/char = %3u / %3u", ca, cc));
	      Term_putstr(35, linec, -1, TERM_WHITE, empty_symbol);
	      if (use_dbltile || use_trptile) 
		Term_putstr (35, linec + 1, -1, TERM_WHITE, empty_symbol2); 
	      if (use_trptile) 
		Term_putstr (35, linec + 2, -1, TERM_WHITE, empty_symbol3); 
	      Term_putch(38, linec, ca, cc);

#ifdef USE_TRANSPARENCY
	      if (use_bigtile || use_dbltile || use_trptile)
		{
		  big_putch(38, linec++, ca, cc);
		}
#endif
	      
	      /* Prompt */
	      Term_putstr(0, linec + 2, -1, TERM_WHITE,
			  "Command (n/N/a/A/c/C): ");

	      /* Get a command */
	      ke = inkey_ex();
	      i = ke.key;

	      /* Mouse input */
	      if (i == '\xff')
		{
		  if (ke.mousey == 3) i = ESCAPE;
		  if (ke.mousey == linec + 2)
		    {
		      if (ke.mousex == 9) i = 'n';
		      if (ke.mousex == 11) i = 'N';
		      if (ke.mousex == 13) i = 'a';
		      if (ke.mousex == 15) i = 'A';
		      if (ke.mousex == 17) i = 'c';
		      if (ke.mousex == 19) i = 'C';
		    }
		}
	      
	      /* All done */
	      if (i == ESCAPE) break;
	      
	      /* Analyze */
	      if (i == 'n') r = (r + MAX_R_IDX + 1) % MAX_R_IDX;
	      if (i == 'N') r = (r + MAX_R_IDX - 1) % MAX_R_IDX;
	      if (i == 'a') r_ptr->x_attr = (byte)(ca + 1);
	      if (i == 'A') r_ptr->x_attr = (byte)(ca - 1);
	      if (i == 'c') r_ptr->x_char = (byte)(cc + 1);
	      if (i == 'C') r_ptr->x_char = (byte)(cc - 1);
	      if (i == 's') askfor_shade(&r_ptr->x_attr, 22);
	      
	    }
	}
      
      /* Modify object attr/chars */
      else if (i == '7')
	{
	  static int k = 0;
	  
	  /* Prompt */
	  prt("Command: Change object attr/chars", 15, 0);
	  
	  /* Hack -- query until done */
	  while (1)
	    {
	      object_kind *k_ptr = &k_info[k];
	      
	      byte da = (byte)k_ptr->d_attr;
	      byte dc = (byte)k_ptr->d_char;
	      byte ca = (byte)k_ptr->x_attr;
	      byte cc = (byte)k_ptr->x_char;
	      
	      int linec = (use_trptile ? 20: (use_dbltile ? 19 : 18));

	      /* Label the object */
	      Term_putstr(5, 16, -1, TERM_WHITE,
	      	  format("Object = %d, Name = %-40.40s",
	      		 k, (k_name + k_ptr->name)));
	      
	      /* Label the Default values */
	      Term_putstr(5, 17, -1, TERM_WHITE,
			  format("Default attr/char = %3u / %3u", da, dc));
	      Term_putstr(35, 17, -1, TERM_WHITE, empty_symbol);
	      if (use_dbltile || use_trptile) 
		Term_putstr (35, 18, -1, TERM_WHITE, empty_symbol2);
	      if (use_trptile) 
		Term_putstr (35, 19, -1, TERM_WHITE, empty_symbol3);
	      
	      Term_putch(38, 17, da, dc);
	      
#ifdef USE_TRANSPARENCY
	      if (use_bigtile || use_dbltile || use_trptile)
		{
		  big_putch(38, 17, da, dc);
		}
#endif
	      
	      /* Label the Current values */
	      Term_putstr(5, linec, -1, TERM_WHITE,
			  format("Current attr/char = %3u / %3u", ca, cc));
	      Term_putstr(35, linec, -1, TERM_WHITE, empty_symbol);
	      if (use_dbltile || use_trptile) 
		Term_putstr (35, linec + 1, -1, TERM_WHITE, empty_symbol2); 
	      if (use_trptile) 
		Term_putstr (35, linec + 2, -1, TERM_WHITE, empty_symbol3); 
	      Term_putch(38, linec, ca, cc);

#ifdef USE_TRANSPARENCY
	      if (use_bigtile || use_dbltile || use_trptile)
		{
		  big_putch(38, linec++, ca, cc);
		}
#endif
	      
	      /* Prompt */
	      Term_putstr(0, linec + 2, -1, TERM_WHITE,
			  "Command (n/N/a/A/c/C): ");

	      /* Get a command */
	      ke = inkey_ex();
	      i = ke.key;

	      /* Mouse input */
	      if (i == '\xff')
		{
		  if (ke.mousey == 3) i = ESCAPE;
		  if (ke.mousey == linec + 2)
		    {
		      if (ke.mousex == 9) i = 'n';
		      if (ke.mousex == 11) i = 'N';
		      if (ke.mousex == 13) i = 'a';
		      if (ke.mousex == 15) i = 'A';
		      if (ke.mousex == 17) i = 'c';
		      if (ke.mousex == 19) i = 'C';
		    }
		}
	      
	      /* All done */
	      if (i == ESCAPE) break;
	      
	      /* Analyze */
	      if (i == 'n') k = (k + MAX_K_IDX + 1) % MAX_K_IDX;
	      if (i == 'N') k = (k + MAX_K_IDX - 1) % MAX_K_IDX;
	      if (i == 'a') k_info[k].x_attr = (byte)(ca + 1);
	      if (i == 'A') k_info[k].x_attr = (byte)(ca - 1);
	      if (i == 'c') k_info[k].x_char = (byte)(cc + 1);
	      if (i == 'C') k_info[k].x_char = (byte)(cc - 1);
	      if (i == 's') askfor_shade(&k_info[k].x_attr, 22);
	    }
	}
      
      /* Modify feature attr/chars */
      else if (i == '8')
	{
	  static int f = 0;
	  
	  /* Prompt */
	  prt("Command: Change feature attr/chars", 15, 0);
	  
	  /* Hack -- query until done */
	  while (1)
	    {
	      feature_type *f_ptr = &f_info[f];
	      
	      byte da = (byte)f_ptr->d_attr;
	      byte dc = (byte)f_ptr->d_char;
	      byte ca = (byte)f_ptr->x_attr;
	      byte cc = (byte)f_ptr->x_char;
	      
	      int linec = (use_trptile ? 20: (use_dbltile ? 19 : 18));

	      /* Label the object */
	      Term_putstr(5, 16, -1, TERM_WHITE,
			  format("Terrain = %d, Name = %-40.40s",
				 f, (f_name + f_ptr->name)));
	      
	      /* Label the Default values */
	      Term_putstr(5, 17, -1, TERM_WHITE,
			  format("Default attr/char = %3u / %3u", da, dc));
	      Term_putstr(35, 17, -1, TERM_WHITE, empty_symbol);
	      if (use_dbltile || use_trptile) 
		Term_putstr (35, 18, -1, TERM_WHITE, empty_symbol2);
	      if (use_trptile) 
		Term_putstr (35, 19, -1, TERM_WHITE, empty_symbol3);
	      
	      Term_putch(38, 17, da, dc);
	      
#ifdef USE_TRANSPARENCY
	      if (use_bigtile || use_dbltile || use_trptile)
		{
		  big_putch(38, 17, da, dc);
		}
#endif
	      
	      /* Label the Current values */
	      Term_putstr(5, linec, -1, TERM_WHITE,
			  format("Current attr/char = %3u / %3u", ca, cc));
	      Term_putstr(35, linec, -1, TERM_WHITE, empty_symbol);
	      if (use_dbltile || use_trptile) 
		Term_putstr (35, linec + 1, -1, TERM_WHITE, empty_symbol2); 
	      if (use_trptile) 
		Term_putstr (35, linec + 2, -1, TERM_WHITE, empty_symbol3); 
	      Term_putch(38, linec, ca, cc);

#ifdef USE_TRANSPARENCY
	      if (use_bigtile || use_dbltile || use_trptile)
		{
		  big_putch(38, linec++, ca, cc);
		}
#endif
	      
	      /* Prompt */
	      Term_putstr(0, linec + 2, -1, TERM_WHITE,
			  "Command (n/N/a/A/c/C/'s'hade): ");

	      /* Get a command */
	      ke = inkey_ex();
	      i = ke.key;

	      /* Mouse input */
	      if (i == '\xff')
		{
		  if (ke.mousey == 3) i = ESCAPE;
		  if (ke.mousey == linec + 2)
		    {
		      if (ke.mousex == 9) i = 'n';
		      if (ke.mousex == 11) i = 'N';
		      if (ke.mousex == 13) i = 'a';
		      if (ke.mousex == 15) i = 'A';
		      if (ke.mousex == 17) i = 'c';
		      if (ke.mousex == 19) i = 'C';
		      if ((ke.mousex > 20) && (ke.mousex < 28)) i = 's';
		    }
		}
	      
	      /* All done */
	      if (i == ESCAPE) break;
	      
	      /* Analyze */
	      if (i == 'n') f = (f + MAX_F_IDX + 1) % MAX_F_IDX;
	      if (i == 'N') f = (f + MAX_F_IDX - 1) % MAX_F_IDX;
	      if (i == 'a') f_info[f].x_attr = (byte)(ca + 1);
	      if (i == 'A') f_info[f].x_attr = (byte)(ca - 1);
	      if (i == 'c') f_info[f].x_char = (byte)(cc + 1);
	      if (i == 'C') f_info[f].x_char = (byte)(cc - 1);
	      if (i == 's') askfor_shade(&f_info[f].x_attr, 22);
	    }
	}
      
      /* Modify flavor attr/chars */
      else if (i == '9') 
	{
	  static int f = 0;
	  
	  /* Prompt */
	  prt("Command: Change flavor attr/chars", 15, 0);
	  
	  /* Hack -- query until done */
	  while (1)
	    {
	      flavor_type *flavor_ptr = &flavor_info[f];
	      
	      byte da = (byte)(flavor_ptr->d_attr);
	      byte dc = (byte)(flavor_ptr->d_char);
	      byte ca = (byte)(flavor_ptr->x_attr);
	      byte cc = (byte)(flavor_ptr->x_char);
	      
	      int linec = (use_trptile ? 20: (use_dbltile ? 19 : 18));

	      /* Label the object */
	      Term_putstr(5, 16, -1, TERM_WHITE,
			  format("Flavor = %d, Text = %-40.40s",
				 f, (flavor_text + flavor_ptr->text)));
	      
	      /* Label the Default values */
	      Term_putstr(10, 17, -1, TERM_WHITE,
			  format("Default attr/char = %3u / %3u", da, dc));
	      Term_putstr(35, 17, -1, TERM_WHITE, empty_symbol);
	      if (use_dbltile || use_trptile) 
		Term_putstr (35, 18, -1, TERM_WHITE, empty_symbol2);
	      if (use_trptile) 
		Term_putstr (35, 19, -1, TERM_WHITE, empty_symbol3);
	      
	      Term_putch(38, 17, da, dc);
	      
#ifdef USE_TRANSPARENCY
	      if (use_bigtile || use_dbltile || use_trptile)
		{
		  big_putch(38, 17, da, dc);
		}
#endif
	      
	      /* Label the Current values */
	      Term_putstr(5, linec, -1, TERM_WHITE,
			  format("Current attr/char = %3u / %3u", ca, cc));
	      Term_putstr(35, linec, -1, TERM_WHITE, empty_symbol);
	      if (use_dbltile || use_trptile) 
		Term_putstr (35, linec + 1, -1, TERM_WHITE, empty_symbol2); 
	      if (use_trptile) 
		Term_putstr (35, linec + 2, -1, TERM_WHITE, empty_symbol3); 
	      Term_putch(38, linec, ca, cc);
	      
#ifdef USE_TRANSPARENCY
	      if (use_bigtile || use_dbltile || use_trptile)
		{
		  big_putch(38, linec++, ca, cc);
		}
#endif
	      if (use_trptile) linec++;
	      
	      /* Prompt */
	      Term_putstr(0, linec + 2, -1, TERM_WHITE,
			  "Command (n/N/a/A/c/C/'s'hade): ");
	      
	      /* Get a command */
	      ke = inkey_ex();
	      i = ke.key;
	      
	      /* Mouse input */
	      if (i == '\xff')
		{
		  if (ke.mousey == 3) i = ESCAPE;
		  if (ke.mousey == linec + 2)
		    {
		      if (ke.mousex == 9) i = 'n';
		      if (ke.mousex == 11) i = 'N';
		      if (ke.mousex == 13) i = 'a';
		      if (ke.mousex == 15) i = 'A';
		      if (ke.mousex == 17) i = 'c';
		      if (ke.mousex == 19) i = 'C';
		      if ((ke.mousex > 20) && (ke.mousex < 28)) i = 's';
		    }
		}
	      
	      /* All done */
	      if (i == ESCAPE) break;
	      
	      /* Analyze */
	      if (i == 'n') f = (f + MAX_FL_IDX + 1) % MAX_FL_IDX;
	      if (i == 'N') f = (f + MAX_FL_IDX - 1) % MAX_FL_IDX;
	      if (i == 'a') flavor_info[f].x_attr = (byte)(ca + 1);
	      if (i == 'A') flavor_info[f].x_attr = (byte)(ca - 1);
	      if (i == 'c') flavor_info[f].x_char = (byte)(cc + 1);
	      if (i == 'C') flavor_info[f].x_char = (byte)(cc - 1);
	      if (i == 's') askfor_shade(&flavor_info[f].x_attr, 22);
	    }
	}
      
#endif
      
      /* Reset visuals */
      else if (i == '0')
	{
	  /* Reset */
	  reset_visuals(TRUE);
	  
	  /* Message */
	  msg_print("Visual attr/char tables reset.");
	}
      
      /* Unknown option */
      else
	{
	  bell("Illegal command for visuals!");
	}
      
      /* Flush messages */
      msg_print(NULL);
    }
  
  
  /* Load screen */
  screen_load();
}


/*
 * Asks to the user for specific color values.
 * Returns TRUE if the color was modified.
 */
static bool askfor_color_values(int idx)
{
  	char str[10];

  	int k, r, g, b;

  	/* Get the default value */
  	sprintf(str, "%d", angband_color_table[idx][1]);

  	/* Query, check for ESCAPE */
  	if (!get_string("Red (0-255) ", str, sizeof(str))) return FALSE;

  	/* Convert to number */
  	r = atoi(str);

  	/* Check bounds */
  	if (r < 0) r = 0;
  	if (r > 255) r = 255;

  	/* Get the default value */
  	sprintf(str, "%d", angband_color_table[idx][2]);

  	/* Query, check for ESCAPE */
  	if (!get_string("Green (0-255) ", str, sizeof(str))) return FALSE;

  	/* Convert to number */
  	g = atoi(str);

  	/* Check bounds */
  	if (g < 0) g = 0;
  	if (g > 255) g = 255;

  	/* Get the default value */
  	sprintf(str, "%d", angband_color_table[idx][3]);

 	/* Query, check for ESCAPE */
  	if (!get_string("Blue (0-255) ", str, sizeof(str))) return FALSE;

 	/* Convert to number */
  	b = atoi(str);

  	/* Check bounds */
  	if (b < 0) b = 0;
  	if (b > 255) b = 255;

  	/* Get the default value */
  	sprintf(str, "%d", angband_color_table[idx][0]);

  	/* Query, check for ESCAPE */
  	if (!get_string("Extra (0-255) ", str, sizeof(str))) return FALSE;

  	/* Convert to number */
  	k = atoi(str);

  	/* Check bounds */
  	if (k < 0) k = 0;
  	if (k > 255) k = 255;

  	/* Do nothing if the color is not modified */
  	if ((k == angband_color_table[idx][0]) &&
        (r == angband_color_table[idx][1]) &&
        (g == angband_color_table[idx][2]) &&
        (b == angband_color_table[idx][3])) return FALSE;

  	/* Modify the color table */
 	angband_color_table[idx][0] = k;
 	angband_color_table[idx][1] = r;
 	angband_color_table[idx][2] = g;
  	angband_color_table[idx][3] = b;

  	/* Notify the changes */
  	return TRUE;
}


/* These two are used to place elements in the grid */
#define COLOR_X(idx) (((idx) / MAX_BASE_COLORS) * (small_screen ? 3 : 5) + 1)
#define COLOR_Y(idx) ((idx) % MAX_BASE_COLORS + 6)

/* We only can edit a portion of the color table */
#define MAX_COLORS 128

/* Hack - Note the cast to "int" to prevent overflow */
#define IS_BLACK(idx) \
((int)angband_color_table[idx][1] + (int)angband_color_table[idx][2] + \
 (int)angband_color_table[idx][3] == 0)

/* We show black as dots to see the shape of the grid */
#define BLACK_SAMPLE (small_screen ? ".." : "...")

/*
 * The screen used to modify the color table. Only 128 colors can be modified.
 * The remaining entries of the color table are reserved for graphic mode.
 */
static void modify_colors(void)
{
  int x, y, idx, old_idx;
  key_event ke;
  char msg[100];
  
  /* Flags */
  bool do_move, do_update;
  
  /* Clear the screen */
  Term_clear();
  
  /* Draw the color table */
  for (idx = 0; idx < MAX_COLORS; idx++)
    {
      /* Get coordinates, the x value is adjusted to show a fake cursor */
      x = COLOR_X(idx) + 1;
      y = COLOR_Y(idx);
      
      /* Show a sample of the color */
      if (IS_BLACK(idx)) c_put_str(TERM_WHITE, BLACK_SAMPLE, y, x);
      else c_put_str(idx, COLOR_SAMPLE, y, x);
    }
  
  /* Show screen commands and help */
  if (small_screen)
    {
      y = 2;
      x = 26;
      c_put_str(TERM_WHITE, "Commands:", y, x);
      c_put_str(TERM_WHITE, "ESC: Return", y + 1, x);
      c_put_str(TERM_WHITE, "Arrows: Move to color", y + 3, x);
      c_put_str(TERM_WHITE, "k,K: Incr,Decr extra", y + 4, x);
      c_put_str(TERM_WHITE, "r,R: Incr,Decr red", y + 5, x);
      c_put_str(TERM_WHITE, "g,G: Incr,Decr green", y + 6, x);
      c_put_str(TERM_WHITE, "b,B: Incr,Decr blue", y + 7, x);
      c_put_str(TERM_WHITE, "c: Copy from color", y + 8, x);
      c_put_str(TERM_WHITE, "v: Set specific vals", y + 9, x);
      c_put_str(TERM_WHITE, "Column 1: base colors", y + 11, x);
      c_put_str(TERM_WHITE, "Column 2: first shade", y + 12, x);  
      c_put_str(TERM_WHITE, "etc.", y + 13, x);  
      c_put_str(TERM_WHITE, "Shades look like base colors in 16 color ports.",
		23, 0);
    }
  else
    {
      y = 2;
      x = 42;
      c_put_str(TERM_WHITE, "Commands:", y, x);
      c_put_str(TERM_WHITE, "ESC: Return", y + 2, x);
      c_put_str(TERM_WHITE, "Arrows: Move to color", y + 3, x);
      c_put_str(TERM_WHITE, "k,K: Incr,Decr extra value", y + 4, x);
      c_put_str(TERM_WHITE, "r,R: Incr,Decr red value", y + 5, x);
      c_put_str(TERM_WHITE, "g,G: Incr,Decr green value", y + 6, x);
      c_put_str(TERM_WHITE, "b,B: Incr,Decr blue value", y + 7, x);
      c_put_str(TERM_WHITE, "c: Copy from color", y + 8, x);
      c_put_str(TERM_WHITE, "v: Set specific values", y + 9, x);
      c_put_str(TERM_WHITE, "First column: base colors", y + 11, x);
      c_put_str(TERM_WHITE, "Second column: first shade, etc.", y + 12, x);  
      c_put_str(TERM_WHITE, "Shades look like base colors in 16 color ports.",
		23, 0);
    }
  
  /* Hack - We want to show the fake cursor */
  do_move = TRUE;
  do_update = TRUE;
  
  /* Start with the first color */
  idx = 0;
  
  /* Used to erase the old position of the fake cursor */
  old_idx = -1;
  
  while (1)
    {
      /* Movement request */
      if (do_move)
    	{
	  
	  /* Erase the old fake cursor */
	  if (old_idx >= 0)
	    {
	      /* Get coordinates */
	      x = COLOR_X(old_idx);
	      y = COLOR_Y(old_idx);
	      
	      /* Draw spaces */
	      c_put_str(TERM_WHITE, " ", y, x);
	      c_put_str(TERM_WHITE, " ", y, x + (small_screen ? 3 : 4));
	    }
	  
	  /* Show the current fake cursor */
	  /* Get coordinates */
	  x = COLOR_X(idx);
	  y = COLOR_Y(idx);
	  
	  /* Draw the cursor */
	  c_put_str(TERM_WHITE, ">", y, x);
	  c_put_str(TERM_WHITE, "<", y, x + (small_screen ? 3 : 4));
	  
	  /* Format the name of the color */
	  my_strcpy(msg, format("Color = %d (0x%02X), Name = %s", idx, idx,
				get_ext_color_name(idx)), sizeof(msg));
	  
	  /* Show the name and some whitespace */
	  c_put_str(TERM_WHITE, format("%-40s", msg), 2, 0);
    	}
      
      /* Color update request */
      if (do_update)
    	{
	  /* Get coordinates, adjust x */
	  x = COLOR_X(idx) + 1;
	  y = COLOR_Y(idx);
	  
	  /* Hack - Redraw the sample if needed */
	  if (IS_BLACK(idx)) c_put_str(TERM_WHITE, BLACK_SAMPLE, y, x);
	  else c_put_str(idx, COLOR_SAMPLE, y, x);
	  
	  /* Notify the changes in the color table to the terminal */
	  Term_xtra(TERM_XTRA_REACT, 0);
	  
	  /* The user is playing with white, redraw all */
	  if (idx == TERM_WHITE) Term_redraw();
	  
	  /* Or reduce flickering by redrawing the changes only */
	  else Term_redraw_section(x, y, x + 2, y);
    	}

      /* Common code, show the values in the color table */
      if (do_move || do_update)
    	{
	  /* Format the view of the color values */
	  my_strcpy(msg, format("K = %d / R,G,B = %d, %d, %d",
	    			angband_color_table[idx][0],
	    			angband_color_table[idx][1],
	    			angband_color_table[idx][2],
				angband_color_table[idx][3]), sizeof(msg));
	  
	  /* Show color values and some whitespace */
	  c_put_str(TERM_WHITE, format("%-40s", msg), 4, 0);
	  
    	}
      
      /* Reset flags */
      do_move = FALSE;
      do_update = FALSE;
      old_idx = -1;
      
      /* Get a command */
      if (!get_com_ex("Command: Modify colors ", &ke)) break;

      /* Mouse input */
      if (ke.key == '\xff')
	{
	  /* Escape */
	  if (ke.mousey == (small_screen ? 3 : 4)) break;

	  /* Set divider */
	  x = (small_screen ? 26 : 42);
	  if (ke.mousex < x)
	    {
	      /* Remember cursor */
	      old_idx = idx;

	      /* Look for new location */
	      for (idx = 0; idx < MAX_COLORS; idx++)
		{
		  if ((ke.mousey == COLOR_Y(idx)) && (ke.mousex > COLOR_X(idx))
		      && (ke.mousex < COLOR_X(idx) + 4)) 
		    {
		      /* Found it */
		      do_move = TRUE;
		      break;
		    }
		}
	      /* Didn't find it */
	      if (idx == MAX_COLORS)
		{
		  idx = old_idx;
		  old_idx = -1;
		}
	    }

	  else
	    {
	      switch(ke.mousey)
		{
		  /* k, K */
		case 6:
		  {
		    if (((ke.mousex > x + 4) && (ke.mousex < x + 9)) ||
			(ke.mousex == x)) ke.key = 'k';
		    if (((ke.mousex > x + 9) && (ke.mousex < x + 14)) ||
			(ke.mousex == x + 2)) ke.key = 'K';
		    break;
		  }
		case 7:
		  {
		    if (((ke.mousex > x + 4) && (ke.mousex < x + 9)) ||
			(ke.mousex == x)) ke.key = 'r';
		    if (((ke.mousex > x + 9) && (ke.mousex < x + 14)) ||
			(ke.mousex == x + 2)) ke.key = 'R';
		    break;
		  }
		case 8:
		  {
		    if (((ke.mousex > x + 4) && (ke.mousex < x + 9)) ||
			(ke.mousex == x)) ke.key = 'g';
		    if (((ke.mousex > x + 9) && (ke.mousex < x + 14)) ||
			(ke.mousex == x + 2)) ke.key = 'G';
		    break;
		  }
		case 9:
		  {
		    if (((ke.mousex > x + 4) && (ke.mousex < x + 9)) ||
			(ke.mousex == x)) ke.key = 'b';
		    if (((ke.mousex > x + 9) && (ke.mousex < x + 14)) ||
			(ke.mousex == x + 2)) ke.key = 'B';
		    break;
		  }
		case 10:
		  {
		    ke.key = 'c';
		    break;
		  }
		case 11:
		  {
		    ke.key = 'v';
		    break;
		  }
		}
	    }
	}
      
      switch(ke.key)
	{
	  /* Down */
	case '2':
	  {
	    /* Check bounds */
	    if (idx + 1 >= MAX_COLORS) break;
	    
	    /* Erase the old cursor */
	    old_idx = idx;
	    
	    /* Get the new position */
	    ++idx;
	    
	    /* Request movement */
	    do_move = TRUE;
	    break;
	  }
	  
	  /* Up */
	case '8':
	  {
	    
	    /* Check bounds */
	    if (idx - 1 < 0) break;
	    
	    /* Erase the old cursor */
	    old_idx = idx;
	    
	    /* Get the new position */
	    --idx;
	    
	    /* Request movement */
	    do_move = TRUE;
	    break;
	  }

	  /* Left */
	case '4':
	  {
	    /* Check bounds */
	    if (idx - 16 < 0) break;
	    
	    /* Erase the old cursor */
	    old_idx = idx;
	    
	    /* Get the new position */
	    idx -= 16;
	    
	    /* Request movement */
	    do_move = TRUE;
	    break;
	  }
	  
	  /* Right */
	case '6':
	  {
	    /* Check bounds */
	    if (idx + 16 >= MAX_COLORS) break;
	    
	    /* Erase the old cursor */
	    old_idx = idx;
	    
	    /* Get the new position */
	    idx += 16;
	    
	    /* Request movement */
	    do_move = TRUE;
	    break;
	  }
	  
	  /* Copy from color */
	case 'c':
	  {
	    char str[10];
	    int src;
	    
	    /* Get the default value, the base color */
	    sprintf(str, "%d", GET_BASE_COLOR(idx));
	    
	    /* Query, check for ESCAPE */
	    if (!get_string(format("Copy from color (0-%d, def. base) ",
				   MAX_COLORS - 1), str, sizeof(str))) break;
	    
	    /* Convert to number */
	    src = atoi(str);
	    
	    /* Check bounds */
	    if (src < 0) src = 0;
	    if (src >= MAX_COLORS) src = MAX_COLORS - 1;
	    
	    /* Do nothing if the colors are the same */
	    if (src == idx) break;
	    
	    /* Modify the color table */
	    angband_color_table[idx][0] = angband_color_table[src][0];
	    angband_color_table[idx][1] = angband_color_table[src][1];
	    angband_color_table[idx][2] = angband_color_table[src][2];
	    angband_color_table[idx][3] = angband_color_table[src][3];
	    
	    /* Request update */
	    do_update = TRUE;
	    break;
	  }
	  
	  /* Increase the extra value */
	case 'k':
	  {
	    /* Get a pointer to the proper value */
	    byte *k_ptr = &angband_color_table[idx][0];
	    
	    /* Modify the value */
	    *k_ptr = (byte)(*k_ptr + 1);
	    
	    /* Request update */
	    do_update = TRUE;
	    break;
	  }
	  
	  /* Decrease the extra value */
	case 'K':
	  {
	    
	    /* Get a pointer to the proper value */
	    byte *k_ptr = &angband_color_table[idx][0];
	    
	    /* Modify the value */
	    *k_ptr = (byte)(*k_ptr - 1);
	    
	    /* Request update */
	    do_update = TRUE;
	    break;
	  }
	  
	  /* Increase the red value */
	case 'r':
	  {
	    /* Get a pointer to the proper value */
	    byte *r_ptr = &angband_color_table[idx][1];
	    
	    /* Modify the value */
	    *r_ptr = (byte)(*r_ptr + 1);
	    
	    /* Request update */
	    do_update = TRUE;
	    break;
	  }
	  
	  /* Decrease the red value */
	case 'R':
	  {
	    
	    /* Get a pointer to the proper value */
	    byte *r_ptr = &angband_color_table[idx][1];
	    
	    /* Modify the value */
	    *r_ptr = (byte)(*r_ptr - 1);
	    
	    /* Request update */
	    do_update = TRUE;
	    break;
	  }
	  
	  /* Increase the green value */
	case 'g':
	  {
	    /* Get a pointer to the proper value */
	    byte *g_ptr = &angband_color_table[idx][2];
	    
	    /* Modify the value */
	    *g_ptr = (byte)(*g_ptr + 1);
	    
	    /* Request update */
	    do_update = TRUE;
	    break;
	  }
	  
	  /* Decrease the green value */
	case 'G':
	  {
	    /* Get a pointer to the proper value */
	    byte *g_ptr = &angband_color_table[idx][2];
	    
	    /* Modify the value */
	    *g_ptr = (byte)(*g_ptr - 1);
	    
	    /* Request update */
	    do_update = TRUE;
	    break;
	  }
	  
	  /* Increase the blue value */
	case 'b':
	  {
	    /* Get a pointer to the proper value */
	    byte *b_ptr = &angband_color_table[idx][3];
	    
	    /* Modify the value */
	    *b_ptr = (byte)(*b_ptr + 1);
	    
	    /* Request update */
	    do_update = TRUE;
	    break;
	  }
	  
	  /* Decrease the blue value */
	case 'B':
	  {
	    /* Get a pointer to the proper value */
	    byte *b_ptr = &angband_color_table[idx][3];
	    
	    /* Modify the value */
	    *b_ptr = (byte)(*b_ptr - 1);
	    
	    /* Request update */
	    do_update = TRUE;
	    break;
	  }
	  
	  /* Ask for specific values */
	case 'v':
	  {
	    do_update = askfor_color_values(idx);
	    break;
	  }
	}
    }
}


/*
 * Interact with "colors"
 */
void do_cmd_colors(void)
{
  int i;

  key_event ke;
  
  FILE *fff;
  
  char tmp[160];
  
  char buf[1024];
  
  
  /* File type is "TEXT" */
  FILE_TYPE(FILE_TYPE_TEXT);
  
  
  /* Save screen */
  screen_save();
  
  
  /* Interact until done */
  while (1)
    {
      /* Clear screen */
      Term_clear();
      
      /* Ask for a choice */
      prt("Interact with Colors", 2, 0);
      prt("ESC to exit", 3, 0);
      
      /* Give some choices */
      prt("(1) Load a user pref file", 4, 5);
#ifdef ALLOW_COLORS
      prt("(2) Dump colors", 5, 5);
      prt("(3) Modify colors", 6, 5);
#endif
      
      /* Prompt */
      prt("Command: ", 8, 0);
      
      /* Prompt */
      ke = inkey_ex();
      i = ke.key;

      /* Mouse input */
      if (i == '\xff')
	{
	  if (ke.mousey == 3) 
	    i = ESCAPE;
	  else if ((ke.mousey > 3) && (ke.mousey < 7))
	    i = I2D(ke.mousey - 3);
	}
      
      
      /* Done */
      if (i == ESCAPE) break;
      
      /* Load a 'pref' file */
      if (i == '1')
	{
	  /* Ask for and load a user pref file */
	  if (!do_cmd_pref_file_hack(8)) break;
	  
	  /* Mega-Hack -- react to changes */
	  Term_xtra(TERM_XTRA_REACT, 0);
	  
	  /* Mega-Hack -- redraw */
	  Term_redraw();
	}
      
#ifdef ALLOW_COLORS
      
      /* Dump colors */
      else if (i == '2')
	{
	  /* Prompt */
	  prt("Command: Dump colors", 8, 0);
	  
	  /* Prompt */
	  prt("File: ", 10, 0);
	  
	  /* Default filename */
	  sprintf(tmp, "%s.prf", op_ptr->base_name);
	  
	  /* Get a filename */
	  if (!askfor_aux(tmp, 80)) continue;
	  
	  /* Build the filename */
	  path_build(buf, 1024, ANGBAND_DIR_USER, tmp);
	  
	  /* Drop priv's */
	  safe_setuid_drop();
	  
	  /* Append to the file */
	  fff = my_fopen(buf, "a");
	  
	  /* Grab priv's */
	  safe_setuid_grab();
	  
	  /* Failure */
	  if (!fff) continue;
	  
	  /* Start dumping */
	  fprintf(fff, "\n\n");
	  fprintf(fff, "# Color redefinitions\n\n");
	  
	  /* Dump colors */
	  for (i = 0; i < 256; i++)
	    {
	      int kv = angband_color_table[i][0];
	      int rv = angband_color_table[i][1];
	      int gv = angband_color_table[i][2];
	      int bv = angband_color_table[i][3];
	      
	      cptr name = "unknown";
	      
	      /* Skip non-entries */
	      if (!kv && !rv && !gv && !bv) continue;
	      
	      /* Extract the color name */
	      if (i < 16) name = color_names[i];
	      
	      /* Dump a comment */
	      fprintf(fff, "# Color '%s'\n", name);
	      
	      /* Dump the monster attr/char info */
	      fprintf(fff, "V:%d:0x%02X:0x%02X:0x%02X:0x%02X\n\n",
		      i, kv, rv, gv, bv);
	    }
	  
	  /* All done */
	  fprintf(fff, "\n\n\n\n");
	  
	  /* Close */
	  my_fclose(fff);
	  
	  /* Message */
	  msg_print("Dumped color redefinitions.");
	}
      
      /* Edit colors */
      else if (i == '3')
	{
	  modify_colors();
	}
      
#endif
      
      /* Unknown option */
      else
	{
	  bell("Illegal command for colors!");
	}
      
      /* Flush messages */
		msg_print(NULL);
    }
  
  
  /* Load screen */
  screen_load();
}



/*
 * Note something in the message recall or character notes file.  Lifted
 * from NPPangband, patch originally by Chris Kern.
 */
void do_cmd_note(char *note, int what_stage)
{
  char buf[120];
  
  /* Default */
  strcpy(buf, "");
  
  /* If a note is passed, use that, otherwise accept user input. */
  if (streq(note, ""))
    {
      
      if (!get_string("Note: ", buf, 70)) return;
      
    }
  
  else my_strcpy(buf, note, sizeof(buf));
  
  /* Ignore empty notes */
  if (!buf[0] || (buf[0] == ' ')) return;
  
  if (adult_take_notes)
    {
      int length, length_info;
      char info_note[43];
      char place[32];
      int region = stage_map[what_stage][LOCALITY];
      int level = stage_map[what_stage][DEPTH];
      
      
      /*Artifacts use stage artifact created.  All others
       *use player stage.
       */
      
      /*get location for recording */
      
      if (level)
	strnfmt(place, sizeof(place), "%15s%4d ", locality_name[region], 
		level);
      else
	strnfmt(place, sizeof(place), "%15s Town", locality_name[region]);
      
      
      /* Make preliminary part of note */
      strnfmt(info_note, sizeof(info_note), "|%9lu| %s | %2d  | ", turn, 
	      place, p_ptr->lev);
      
      /*write the info note*/
      fprintf(notes_file, info_note);
      
      /*get the length of the notes*/
      length_info = strlen(info_note);
      length = strlen(buf);
      
      /*break up long notes*/
      if((length + length_info) > LINEWRAP)
	{
	  bool keep_going = TRUE;
	  int startpoint = 0;
	  int endpoint, n;
	  
	  while (keep_going)
	    {
	      
	      /*don't print more than the set linewrap amount*/
	      endpoint = startpoint + LINEWRAP - strlen(info_note) + 1;
	      
	      /*find a breaking point*/
	      while (TRUE)
		{
		  /*are we at the end of the line?*/
		  if (endpoint >= length)
		    {
		      /*print to the end*/
		      endpoint = length;
		      keep_going = FALSE;
		      break;
		    }
		  
		  /* Mark the most recent space or dash in the string */
		  else if ((buf[endpoint] == ' ') ||
			   (buf[endpoint] == '-')) break;
		  
		  /*no spaces in the line, so break in the middle of text*/
		  else if (endpoint == startpoint)
		    {
		      endpoint = startpoint + LINEWRAP - strlen(info_note) + 1;
		      break;
		    }
		  
		  /* check previous char */
		  endpoint--;
		}
	      
	      /*make a continued note if applicable*/
	      if (startpoint) fprintf(notes_file, "|  continued...                  |     |  ");
		    
	      /* Write that line to file */
	      for (n = startpoint; n <= endpoint; n++)
		{
		  char ch;
		  
		  /* Ensure the character is printable */
		  ch = (isprint(buf[n]) ? buf[n] : ' ');
		  
		  /* Write out the character */
		  fprintf(notes_file, "%c", ch);
		  
		}
	      
	      /*break the line*/
	      fprintf(notes_file, "\n");
	      
	      /*prepare for the next line*/
	      startpoint = endpoint + 1;
	    }
	  
	}
      
      /* Add note to buffer */
      else
	{
	  fprintf(notes_file, "%s", buf);
	  
	  /*break the line*/
	  fprintf(notes_file, "\n");
	}
      
      
    }
  
  else 
    {
      /* Add the note to the message recall */
      msg_format("Note: %s", buf);
    }
}


/*
 * Mention the current version
 */
void do_cmd_version(void)
{
  /* Silly message */
  msg_format("You are playing FAangband %d.%d.%d.  Type '?' for more info.",
	     VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
}



/*
 * Array of feeling strings
 */
static cptr do_cmd_feeling_text[11] =
  {
    "Looks like any other level.",
    "You feel there is something special about this level.",
    "You have a superb feeling about this level.",
    "You have an excellent feeling...",
    "You have a very good feeling...",
    "You have a good feeling...",
    "You feel strangely lucky...",
    "You feel your luck is turning...",
    "You like the look of this place...",
    "This level can't be all bad...",
    "This seems a very quiet place..."
  };


/*
 * Note that "feeling" is set to zero unless some time has passed.
 * Note that this is done when the level is GENERATED, not entered.
 */
void do_cmd_feeling(void)
{
  /* Verify the feeling */
  if (feeling < 0) feeling = 0;
  if (feeling > 10) feeling = 10;
  
  /* No useful feeling in town */
  if (!p_ptr->depth)
    {
      msg_print("Looks like a typical town.");
      return;
    }
  
  /* No useful feelings until enough time has passed */
  if (!do_feeling)
    {
      msg_print("You are still uncertain about this level...");
      return;
    }
  
  /* Display the feeling */
  if (p_ptr->themed_level) msg_format("%s", themed_feeling);
  else msg_print(do_cmd_feeling_text[feeling]);
}




/*
 * Array of feeling strings
 */
static cptr do_cmd_challenge_text[14] =
{
  "challenges you from beyond the grave!",
  "thunders 'Prove worthy of your traditions - or die ashamed!'.",
  "desires to test your mettle!",
  "has risen from the dead to test you!",
  "roars 'Fight, or know yourself for a coward!'.",
  "summons you to a duel of life and death!",
  "desires you to know that you face a mighty champion of yore!",
  "demands that you prove your worthiness in combat!",
  "calls you unworthy of your ancestors!",
  "challenges you to a deathmatch!",
  "walks Middle-Earth once more!",
  "challenges you to demonstrate your prowess!",
  "demands you prove yourself here and now!",
  "asks 'Can ye face the best of those who came before?'."
};




/* 
 * Personalize, randomize, and announce the challenge of a player ghost. -LM-
 */
void ghost_challenge(void)
{
  monster_race *r_ptr = &r_info[r_ghost];
  
  msg_format("%^s, the %^s %s", ghost_name, r_name + r_ptr->name, 
	     do_cmd_challenge_text[rand_int(14)]);
}




/*
 * Encode the screen colors
 */
static char hack[17] = "dwsorgbuDWvyRGBU";


/*
 * Hack -- load a screen dump from a file
 */
void do_cmd_load_screen(void)
{
  int i, y, x;
  
  byte a = 0;
  char c = ' ';
  
  bool okay = TRUE;
  
  FILE *fff;
  
  char tmp_val[80], buf[1024];
  
  int len;
  
  
  /* Ask for a file (or cancel) */
  (void) strcpy(tmp_val, "dump.txt");
  
  /* Prompt */
  prt("File: ", 0, 0);
  
  /* Ask for a file */
  if (!askfor_aux(tmp_val, 80)) return;
  
  /* Hack -- Erase prompt */
  prt("", 0, 0);
  
  /* Build the filename */
  path_build(buf, 1024, ANGBAND_DIR_USER, tmp_val);
  
  /* Append to the file */
  fff = my_fopen(buf, "r");
  
  /* Oops */
  if (!fff) return;
  
  
  /* Save screen */
  screen_save();


  /* Clear the screen */
  Term_clear();
  
  
  /* Load the screen */
  for (y = 0; okay; y++)
    {
      /* Get a line of data */
      if (my_fgets(fff, buf, 1024)) okay = FALSE;
      
      /* Stop on blank line */
      if (!buf[0]) break;
      
      /* Get the width */
      len = strlen(buf);
      
      /* XXX Restrict to current screen size */
      if (len >= Term->wid) len = Term->wid;
      
      /* Show each row */
      for (x = 0; x < len; x++)
	{
	  /* Put the attr/char */
	  Term_draw(x, y, TERM_WHITE, buf[x]);
	}
    }
  
  /* Get the blank line */
  /* if (my_fgets(fff, buf, 1024)) okay = FALSE; */
  
  
  /* Load the screen */
  for (y = 0; okay; y++)
    {
      /* Get a line of data */
      if (my_fgets(fff, buf, 1024)) okay = FALSE;
      
      /* Stop on blank line */
      if (!buf[0]) break;
      
      /* Get the width */
      len = strlen(buf);
      
      /* XXX Restrict to current screen size */
      if (len >= Term->wid) len = Term->wid;
      
      /* Show each row */
      for (x = 0; x < len; x++)
	{
	  /* Get the attr/char */
	  (void)(Term_what(x, y, &a, &c));
	  
	  /* Look up the attr */
	  for (i = 0; i < 16; i++)
	    {
	      /* Use attr matches */
	      if (hack[i] == buf[x]) a = i;
	    }
	  
	  /* Put the attr/char */
	  Term_draw(x, y, a, c);
	}
      
      /* End the row */
      /*		fprintf(fff, "\n"); */
    }
  
  
  /* Get the blank line */
  /* if (my_fgets(fff, buf, 1024)) okay = FALSE; */
  
  
  /* Close it */
  my_fclose(fff);
  
  
  /* Message */
  msg_print("Screen dump loaded.");
  msg_print(NULL);
  
  
  /* Load screen */
  screen_load();
}

/*
 * Hack -- save a screen dump to a file
 */
void do_cmd_save_screen(void)
{
  char tmp_val[256];
  
  /* Ask for a file */
  strcpy(tmp_val, "dump.html");
  if (!get_string("File: ", tmp_val, sizeof(tmp_val))) return;
  
  html_screenshot(tmp_val);
  msg_print("Dump saved.");
}


/*
 * Hack -- save a screen dump to a file (no longer used -NRM-)
 */
void do_cmd_save_screen_old(void)
{
  int y, x;
  
  byte a = 0;
  char c = ' ';
  
  FILE *fff;
  
  char tmp_val[80], buf[1024];
  
  
  /* Ask for a file (or cancel) */
  (void) strcpy(tmp_val, "dump.txt");
  
  /* Prompt */
  prt("File: ", 0, 0);
  
  /* Ask for a file */
  if (!askfor_aux(tmp_val, 80)) return;
  
  /* Hack -- Erase prompt */
  prt("", 0, 0);
  
  /* Build the filename */
  path_build(buf, 1024, ANGBAND_DIR_USER, tmp_val);
  
  /* File type is "TEXT" */
  FILE_TYPE(FILE_TYPE_TEXT);
  
  /* Hack -- drop permissions */
  safe_setuid_drop();
  
  /* Append to the file */
  fff = my_fopen(buf, "w");
  
  /* Hack -- grab permissions */
  safe_setuid_grab();
  
  /* Oops */
  if (!fff) return;
  
  
  /* Save screen */
  screen_save();
  

  /* Dump the screen */
  for (y = 0; y < Term->hgt; y++)
    {
      /* Dump each row */
      for (x = 0; x < Term->wid; x++)
	{
	  /* Get the attr/char */
	  (void)(Term_what(x, y, &a, &c));
	  
	  /* Dump it */
	  buf[x] = c;
	}
      
      /* Terminate */
      buf[x] = '\0';
      
      /* End the row */
      fprintf(fff, "%s\n", buf);
    }
  
  /* Skip a line */
  fprintf(fff, "\n");
  
  
  /* Dump the screen */
  for (y = 0; y < Term->hgt; y++)
    {
      /* Dump each row */
      for (x = 0; x < Term->wid; x++)
	{
	  /* Get the attr/char */
	  (void)(Term_what(x, y, &a, &c));
	  
	  /* Dump it */
	  buf[x] = hack[a & 0x0F];
	}
      
      /* Terminate */
      buf[x] = '\0';
      
      /* End the row */
      fprintf(fff, "%s\n", buf);
    }
  
  /* Skip a line */
  fprintf(fff, "\n");
  
  
  /* Close it */
  my_fclose(fff);
  
  
  /* Message */
  msg_print("Screen dump saved.");
  msg_print(NULL);
  
  
  /* Load screen */
  screen_load();
}


/*
 * Move the cursor in a browser window
 */
static void browser_cursor(key_event ke, int *column, int max, int *grp_cur, 
			   int grp_cnt, int grp_top, int *list_cur, 
			   int list_cnt, int list_top)
{
  int d;
  int col = *column;
  int grp = *grp_cur;
  int list = *list_cur;
  int mult = 1;

  /* Set the multplier */
  if (use_dbltile) mult = 2;
  else if (use_trptile) mult = 3;
  
  /* Mouse input */
  if (ke.key == '\xff')
    {
      if (ke.mousey == header_row + 1)
	{
	  /* Do scrolling */
	  if (ke.mousex == 0) ke.key = '4';
	  else if (ke.mousex == 1) ke.key = '8';
	  else if (ke.mousex == 2) ke.key = '6';
	  else if (ke.mousex == 3) ke.key = '2';
	  else return;
	}
      else if (ke.mousey > header_row + 1)
	{
	  /* Don't go past the end of the list */
	  if (ke.mousey > header_row + browser_rows + 1)
	    ke.mousey = header_row + browser_rows + 1;
	  
	  /* Group column */
	  if (((!small_screen) && (ke.mousex < max + 2)) || 
	      ((small_screen) && (col == 0)))
	    {
	      int old_grp = grp;
	      col = 0;
	      grp = ke.mousey - header_row - 2 + grp_top;
	      if (grp >= grp_cnt)	grp = grp_cnt - 1;
	      if (grp != old_grp)	list = 0;
	    }
	  /* List column */
	  else 
	    {
	      col = 1;
	      list = (ke.mousey - header_row - 2 + list_top) / mult;
	      if (list < 0) list = 0;
	      if (list >= list_cnt) list = list_cnt - 1;
	    }
	  (*grp_cur) = grp;
	  (*list_cur) = list;
	  (*column) = col;
	  return;
	}
      else return;
    }

  /* Extract direction */
  d = target_dir(ke.key);
  
  if (!d) return;
  
  /* Diagonals - hack */
  if ((ddx[d] > 0) && ddy[d])
    {
      /* Browse group list */
      if (!col)
	{
	  int old_grp = grp;
	  
	  /* Move up or down */
	  grp += ddy[d] * browser_rows;
	  
	  /* Verify */
	  if (grp >= grp_cnt)	grp = grp_cnt - 1;
	  if (grp < 0) grp = 0;
	  if (grp != old_grp)	list = 0;
	}
      
      /* Browse sub-list list */
      else
	{
	  /* Move up or down */
	  list += ddy[d] * browser_rows / mult;
	  
	  /* Verify */
	  if (list >= list_cnt) list = list_cnt - 1;
	  if (list < 0) list = 0;
	}
      
      (*grp_cur) = grp;
      (*list_cur) = list;
      
      return;
    }
  
  if (ddx[d])
    {
      col += ddx[d];
      if (col < 0) col = 0;
      if (col > 1) col = 1;
      
      (*column) = col;
      
      return;
    }
  
  /* Browse group list */
  if (!col)
    {
      int old_grp = grp;
      
      /* Move up or down */
      grp += ddy[d];
      
      /* Verify */
      if (grp >= grp_cnt)	grp = grp_cnt - 1;
      if (grp < 0) grp = 0;
      if (grp != old_grp)	list = 0;
    }
  
  /* Browse sub-list list */
  else
    {
      /* Move up or down */
      list += ddy[d];
      
      /* Verify */
      if (list >= list_cnt) list = list_cnt - 1;
      if (list < 0) list = 0;
    }
  
  (*grp_cur) = grp;
  (*list_cur) = list;
}

/*
 * Display the object groups.
 */
static void display_object_group_list(int col, int row, int wid, int per_page,
				      int grp_idx[], grouper group_item_list[],
				      int grp_cur, int grp_top)
{
  int i;
  
  /* Display lines until done */
  for (i = 0; i < per_page && (grp_idx[i] >= 0); i++)
    {
      /* Get the group index */
      int grp = grp_idx[grp_top + i];
      
      /* Choose a color */
      byte attr = (grp_top + i == grp_cur) ? TERM_L_BLUE : TERM_WHITE;
      
      /* Erase the entire line */
      Term_erase(col, row + i, wid);
      
      /* Display the group label */
      c_put_str(attr, group_item_list[new_group_index[grp]].name, 
		row + i, col);
    }
}


/*
 * Build a list of artifact indexes in the given group. Return the number
 * of eligible artifacts in that group.
 */
static int collect_artifacts(int grp_cur, int object_idx[])
{
  int k, which = 0;  
  int i, object_cnt = 0;
  bool *okay;
  
  store_type *st_ptr;
  
  /* Get the store number of the home */
  for (k = 0; k < NUM_TOWNS; k++)
    {
      /* Found the town */
      if (p_ptr->home == towns[k])
	{
	  which += (k < NUM_TOWNS_SMALL ? 3 : STORE_HOME);
	  break;
	}
      /* Next town */
      else
	which += (k < NUM_TOWNS_SMALL ? MAX_STORES_SMALL : MAX_STORES_BIG);
    }

  /* Activate the store */
  st_ptr = &store[which];
  
  /* make a list of artifacts not found */
  /* Allocate the "object_idx" array */
  C_MAKE(okay, MAX_A_IDX, bool);
  
  /* Default first,  */
  for (i = 0; i < MAX_A_IDX; i++)
    {
      artifact_type *a_ptr = &a_info[i];
      
      /*start with false*/
      okay[i] = FALSE;
      
      /* Skip "empty" artifacts */
      if (a_ptr->tval + a_ptr->sval == 0) continue;
      
      /* Skip "uncreated" artifacts */
      if (!a_ptr->creat_stat) continue;
      
      /*assume all created artifacts are good at this point*/
      okay[i] = TRUE;
    }
  
  /* Process objects in the dungeon */
  for (i = 1; i < o_max; i++)
    {
      /*get the object*/
      object_type *o_ptr = &o_list[i];
      
      /* Skip dead objects */
      if (!o_ptr->k_idx) continue;
      
      /* Ignore non-artifacts */
      if (!o_ptr->name1) continue;
      
      /* Ignore known items */
      if (object_known_p(o_ptr)) continue;
      
      /* We found a created, unidentified artifact */
      okay[o_ptr->name1] = FALSE;
      
    }
  
  /*
   * Scan the inventory for unidentified artifacts
   * Notice we are doing the inventory and equipment in the same loop.
   */
  for (i = 0; i < INVEN_TOTAL; i++)
    {
      /* First, the item actually in the slot */
      object_type *o_ptr = &inventory[i];
      
      /* Nothing there */
      if (!(o_ptr->k_idx)) continue;
      
      /* Ignore non-artifacts */
      if (!o_ptr->name1) continue;
      
      /* Ignore known items */
      if (object_known_p(o_ptr)) continue;
      
      /* We found a created, unidentified artifact */
      okay[o_ptr->name1] = FALSE;
    }
  
  /* Look for items in the home, if there is anything there */
  if (st_ptr->stock_num)
    {
      /*go through each item in the house*/
      for (i = 0; i < st_ptr->stock_num; i++)
	{
	  /* Point to the item */
	  object_type *o_ptr = &st_ptr->stock[i];;
	  
	  /*nothing there*/
	  if (!(o_ptr->k_idx)) continue;
	  
	  /* Ignore non-artifacts */
	  if (!o_ptr->name1) continue;
	  
	  /* Ignore known items */
	  if (object_known_p(o_ptr)) continue;
	  
	  /* We found a created, unidentified artifact */
	  okay[o_ptr->name1] = FALSE;
	}
      
    }
  
  if (cheat_know)
    {
      for (i = 0; i < MAX_A_IDX; i++)
	{
	  artifact_type *a_ptr = &a_info[i];
	  
	  /* Skip "empty" artifacts */
	  if (a_ptr->tval + a_ptr->sval == 0) continue;
	  
	  /*assume all created artifacts are good at this point*/
	  okay[i] = TRUE;
	}
    }

  
  /* Finally, go through the list of artifacts and categorize the good ones */
  for (i = 0; i < MAX_A_IDX; i++)
    {
      byte group_tval;
      int j;
      
      /* Access the artifact */
      artifact_type *a_ptr = &a_info[i];
      
      /* Skip empty artifacts */
      if (a_ptr->tval + a_ptr->sval == 0) continue;
      
      /* Require artifacts ever seen*/
      if (okay[i] == FALSE) continue;
      
      /* Get a list of x_char in this group */
      /* Aggregate tvals in this group */
      for (j = new_group_index[grp_cur];
	   j < new_group_index[grp_cur + 1];
	   j++)
	{
	  group_tval = group_item[j].tval;
	  
	  /* Check for artifact in the group */
	  if (a_ptr->tval == group_tval)
	    {
	      /* Add the artifact */
	      object_idx[object_cnt++] = i;
	    }
	}
    }
  
  /* Terminate the list */
  object_idx[object_cnt] = 0;
  
  /*clear the array*/
  C_KILL(okay, MAX_A_IDX, bool);
  
  /* Return the number of races */
  return object_cnt;
  
}


/*
 * Display the objects in a group.
 */
static void display_artifact_list(int col, int row, int per_page, 
				  int object_idx[], int object_cur, 
				  int object_top)
{
  int i, z;
  char o_name[80];
  object_type *i_ptr;
  object_type object_type_body;
  
  /* Display lines until done */
  for (i = 0; i < per_page && object_idx[i]; i++)
    {
      /* Get the object index */
      int a_idx = object_idx[object_top + i];
      artifact_type *a_ptr = &a_info[a_idx];
      
      
      /* Choose a color */
      byte attr = TERM_WHITE;
      byte cursor = TERM_L_BLUE;
      attr = ((i + object_top == object_cur) ? cursor : attr);
      
      /* Get local object */
      i_ptr = &object_type_body;
      
      /* Obtain the base object type */
      z = lookup_kind(a_ptr->tval, a_ptr->sval);
      
      /* Wipe the object */
      object_prep(i_ptr, z);
      
      /* Paranoia */
      strcpy(o_name, "Unknown Artifact");
      
      /* Make it an artifact */
      i_ptr->name1 = a_idx;
      
      /* Describe the artifact */
      object_desc_spoil(o_name, i_ptr, TRUE, 0);
      
      /* Display the name */
      c_prt(attr, o_name, row + i, col);
      
      if (cheat_know)
	{
	  c_prt(attr, format ("%3d", a_idx), row + i, 
		(small_screen ? 36 : 68));
	  c_prt(attr, format ("%3d", a_ptr->level), row + i, 
		(small_screen ? 40 : 72));
	  c_prt(attr, format ("%3d", a_ptr->rarity), row + i, 
		(small_screen ? 44 : 76));
	}
      
    }
  
  /* Clear remaining lines */
  for (; i < per_page; i++)
    {
      Term_erase(col, row + i, 255);
    }
}


/*
 * Display known artifacts
 */
static void do_cmd_knowledge_artifacts(void)
{
  int i, len, max;
  int grp_cur, grp_top;
  int artifact_old, artifact_cur, artifact_top;
  int grp_cnt, grp_idx[100];
  int artifact_cnt;
  int *artifact_idx;
  
  int column = 0;
  bool flag;
  bool redraw;
  
  /* Allocate the "artifact_idx" array */
  C_MAKE(artifact_idx, MAX_A_IDX, int);
  
  max = 0;
  grp_cnt = 0;
  
  /* Check every group */
  for (i = 0; new_group_index[i] >= 0; i++)
    {
      /* Measure the label */
      len = strlen(group_item[new_group_index[i]].name);
      
      /* Save the maximum length */
      if (len > max) max = len;
      
      /* See if artifact are known */
      if (collect_artifacts(i, artifact_idx))
	{
	  /* Build a list of groups with known artifacts */
	  grp_idx[grp_cnt++] = i;
	}
    }
      
  /* Terminate the list */
  grp_idx[grp_cnt] = -1;
  
  grp_cur = grp_top = 0;
  artifact_cur = artifact_top = 0;
  artifact_old = -1;
  
  flag = FALSE;
  redraw = TRUE;
  
  while (!flag)
    {
      key_event ke;
      
      if (redraw)
	{
	  clear_from(0);
	  
	  prt("Knowledge - artifacts", knowledge_row, 0);
	  prt("ESC to quit", knowledge_row + 1, 0);

	  if (small_screen)
	    {
	      if (!column)
		prt("Group", header_row, 0);
	      else
		prt("Name", header_row, 3);
	    }
	  else
	    {
	      prt("Group", header_row, 0);
	      prt("Name", header_row, max + 3);
	    }
	  
	  if (cheat_know && ((!small_screen) || (column)))
	    {
	      prt("Idx", header_row, (small_screen ? 36 : 68));
	      prt("Dep", header_row, (small_screen ? 40 : 72));
	      prt("Rar", header_row, (small_screen ? 44 : 76));
	    }
	  
	  Term_putstr(0, 5, 4, TERM_WHITE, "<^>v");
	  for (i = 4; i < (small_screen ? 46 : 78); i++)
	    {
	      Term_putch(i, 5, TERM_WHITE, '=');
	    }

	  if (small_screen)
	    {
	      for (i = 0; i < browser_rows; i++)
		{
		  clear_from(6 + i);
		}
	    }
	  else
	    {
	      for (i = 0; i < browser_rows; i++)
		{
		  Term_putch(max + 1, 6 + i, TERM_WHITE, '|');
		}
	      
	      redraw = FALSE;
	    }
	}
      
      /* Scroll group list */
      if (grp_cur < grp_top) grp_top = grp_cur;
      if (grp_cur >= grp_top + browser_rows) 
	grp_top = grp_cur - browser_rows + 1;
      
      /* Scroll artifact list */
      if (artifact_cur < artifact_top) artifact_top = artifact_cur;
      if (artifact_cur >= artifact_top + browser_rows) 
	artifact_top = artifact_cur - browser_rows + 1;

      /* Display a list of object groups */
      if ((!small_screen) || (!column))
	display_object_group_list(0, 6, (small_screen ? 48 : max), 
				  browser_rows, grp_idx, 
				  group_item, grp_cur, grp_top);
      
      /* Get a list of objects in the current group */
      artifact_cnt = collect_artifacts(grp_idx[grp_cur], artifact_idx);
      
      /* Display a list of objects in the current group */
      if ((!small_screen) || (column))
	display_artifact_list((small_screen ? 0 : max) + 3, 6, browser_rows, 
			      artifact_idx, artifact_cur, artifact_top);
      
      /* The "current" object changed */
      if (artifact_old != artifact_idx[artifact_cur])
	{
	  /* Hack -- handle stuff */
	  handle_stuff();
	  
	  /* Remember the "current" object */
	  artifact_old = artifact_idx[artifact_cur];
	}
      
      if ((!column) || small_screen)
	{
	  Term_gotoxy(0, 6 + (grp_cur - grp_top));
	}
      else
	{
	  Term_gotoxy(max + 3, 6 + (artifact_cur - artifact_top));
	}
      
      ke = inkey_ex();

      /* Mouse escape */
      if ((ke.key == '\xff') && (ke.mousey == knowledge_row + 1))
	ke.key = ESCAPE;
      
      switch (ke.key)
	{
	case ESCAPE:
	  {
	    flag = TRUE;
	    break;
	  }
	  
	default:
	  {
	    /* Move the cursor */
	    browser_cursor(ke, &column, max, &grp_cur, grp_cnt, grp_top, 
			   &artifact_cur, artifact_cnt, artifact_top);
	    break;
	  }
	}
    }
  
  /* XXX XXX Free the "object_idx" array */
  C_KILL(artifact_idx, MAX_A_IDX, int);
}


/*
 * Build a list of objects indexes in the given group. Return the number
 * of objects in the group.
 */
static int collect_objects(int grp_cur, int object_idx[])
{
  int i, j, k, object_cnt = 0;
  
  /* Get a list of x_char in this group */
  byte group_tval;
  
  /* Check every object */
  for (i = 0; i < MAX_K_IDX; i++)
    {
      /* Access the object type */
      object_kind *k_ptr = &k_info[i];
      
      /*used to check for allocation*/
      k = 0;
      
      /* Skip empty objects */
      if (!k_ptr->name) continue;
      
      /* Skip items with no distribution (including special artifacts) */
      /* Scan allocation pairs */
      for (j = 0; j < 4; j++)
	{
	  /*add the rarity, if there is one*/
	  k += k_ptr->chance[j];
	}
      
      /*not in allocation table*/
      if (!(k))  continue;
      
      /* Require objects ever seen*/
      if (!(k_ptr->aware || cheat_know)) continue;
      
      /* Get a list of x_char in this group */
      /* Aggregate tvals in this group */
      for (j = new_group_index[grp_cur]; j < new_group_index[grp_cur + 1]; j++)
	{
	  group_tval = group_item[j].tval;
	  
	  /* Check for artifact in the group */
	  if (k_ptr->tval == group_tval)
	    {
	      /* Add the objecct */
	      object_idx[object_cnt++] = i;
	    }
	}
    }
  
  /* Terminate the list */
  object_idx[object_cnt] = 0;
  
  /* Return the number of objects */
  return object_cnt;
}


/*
 * Display the objects in a group.
 */
static void display_object_list(int col, int row, int per_page, 
				int object_idx[], int object_cur, 
				int object_top)
{
  int i;
  byte a, c;
  int disp_row = 0;
  int mult;

  /* Set the multiplier */
  mult = (use_trptile ? 3 : (use_dbltile ? 2 : 1));
      
  /* Display lines until done */
  for (i = 0; i < per_page && (object_idx[i] != 0); i++)
    {
      char buf[80];
      
      /* Get the object index */
      int k_idx = object_idx[object_top + i];
      
      /* Access the object */
      object_kind *k_ptr = &k_info[k_idx];
      
      /* Choose a color */
      byte attr = ((k_ptr->aware) ? TERM_WHITE : TERM_SLATE);
      byte cursor = ((k_ptr->aware) ? TERM_L_BLUE : TERM_BLUE);
      attr = ((i + object_top == object_cur) ? cursor : attr);
      
      /* Acquire the basic "name" of the object*/
      strip_name(buf, k_idx);

      /* Get the display row */
      disp_row =  mult * i;
      
      /* Display the name */
      c_prt(attr, buf, row + disp_row, col);
      
      if (cheat_know) c_prt(attr, format ("%d", k_idx), row + disp_row, 
			    (small_screen ? 36 : 68));
      
      /* Clear the symbol area */
      Term_erase((small_screen ? 40 : 72), row + disp_row, 20);
      if (use_dbltile || use_trptile) 
	Term_erase((small_screen ? 40 : 72), row + disp_row + 1, 20);
      if (use_trptile)
	Term_erase((small_screen ? 40 : 72), row + disp_row + 2, 20);
      
      /* Obtain attr/char */
      if (k_ptr->aware)
	{
	  a = (k_ptr->flavor) && (k_ptr->tval != TV_SCROLL) ? 
	    (flavor_info[k_ptr->flavor].x_attr) : k_ptr->x_attr;
	  c = k_ptr->flavor  && (k_ptr->tval != TV_SCROLL) ? 
	    (flavor_info[k_ptr->flavor].x_char): k_ptr->x_char;
	
	  /* Display symbol */
	  Term_putch((small_screen ? 40 : 72), row + disp_row, a, c);
#ifdef USE_TRANSPARENCY
	  if (use_bigtile || use_dbltile || use_trptile)
	    {
	      big_putch((small_screen ? 40 : 72), row + disp_row, a, c);
	    }
#endif
	}
    }
  
  /* Clear remaining lines */
  for (i = disp_row + mult; i < per_page * mult; i++)
    {
      Term_erase(col, row + i, 255);
    }
}



/*
 * Display known objects
 */
static void do_cmd_knowledge_objects(void)
{
  int i, len, max;
  int grp_cur, grp_top;
  int object_old, object_cur, object_top;
  int grp_cnt, grp_idx[100];
  int object_cnt;
  int *object_idx;
  
  int column = 0;
  bool flag;
  bool redraw;
  
  /* Allocate the "object_idx" array */
  C_MAKE(object_idx, MAX_K_IDX, int);
  
  max = 0;
  grp_cnt = 0;
  
  /* Check every group */
  for (i = 0; group_item[new_group_index[i]].name != NULL; i++)
    {
      /* Measure the label */
      len = strlen(group_item[new_group_index[i]].name);
      
      /* Save the maximum length */
      if (len > max) max = len;
      
      /* See if any monsters are known */
      if (collect_objects(i, object_idx))
	{
	  /* Build a list of groups with known monsters */
	  grp_idx[grp_cnt++] = i;
	}
    }
  
  /* Terminate the list */
  grp_idx[grp_cnt] = -1;
  
  grp_cur = grp_top = 0;
  object_cur = object_top = 0;
  object_old = -1;
  
  flag = FALSE;
  redraw = TRUE;
  
  while (!flag)
    {
      key_event ke;
      int mult = 1;
      
      /* Set the multiplier */
      if (use_dbltile) mult = 2;
      else if (use_trptile) mult = 3;
      
      if (redraw)
	{
	  clear_from(0);
	  
	  prt("Knowledge - objects", knowledge_row, 0);
	  prt("ESC to quit", knowledge_row + 1, 0);
	  if (small_screen)
	    {
	      if (!column)
		prt("Group", header_row, 0);
	      else
		prt("Name", header_row, 3);
	    }
	  else
	    {
	      prt("Group", header_row, 0);
	      prt("Name", header_row, max + 3);
	    }
	  if ((!small_screen) || (column))
	    {
	      if (cheat_know) 
		prt("Idx", header_row, (small_screen ? 36 : 68));
	      prt("Sym", header_row, (small_screen ? 41 : 73));
	    }
	  
	  Term_putstr(0, 5, 4, TERM_WHITE, "<^>v");
	  for (i = 4; i < (small_screen ? 46 : 78); i++)
	    {
	      Term_putch(i, 5, TERM_WHITE, '=');
	    }

	  if (small_screen)
	    {
	      for (i = 0; i < browser_rows; i++)
		{
		  clear_from(6 + i);
		}
	    }
	  else
	    {
	      for (i = 0; i < browser_rows; i++)
		{
		  Term_putch(max + 1, 6 + i, TERM_WHITE, '|');
		}
	      
	      redraw = FALSE;
	    }
	}
      
      /* Scroll group list */
      if (grp_cur < grp_top) grp_top = grp_cur;
      if (grp_cur >= grp_top + browser_rows) 
	grp_top = grp_cur - browser_rows + 1;
      
      /* Scroll monster list */
      if (object_cur < object_top) object_top = object_cur;
      if (object_cur >= object_top + browser_rows / mult)
	{ 
	  object_top = object_cur - (browser_rows / mult) + 1;
	  if (object_top < 0) object_top = 0;
	}
      
      /* Display a list of object groups */
      if ((!small_screen) || (!column))
	display_object_group_list(0, 6, max, browser_rows, grp_idx, 
				  group_item, grp_cur, grp_top);
      
      /* Get a list of objects in the current group */
      object_cnt = collect_objects(grp_idx[grp_cur], object_idx);
      
      /* Display a list of objects in the current group */
      if ((!small_screen) || (column))
	display_object_list((small_screen ? 0 : max) + 3, 6, 
			    browser_rows / mult, object_idx, object_cur, 
			    object_top);
      
      /* Mega Hack -- track this monster race */
      if (object_cnt) object_kind_track(object_idx[object_cur]);
      
      /* The "current" object changed */
      if (object_old != object_idx[object_cur])
	{
	  /* Hack -- handle stuff */
	  handle_stuff();
	  
	  /* Remember the "current" object */
	  object_old = object_idx[object_cur];
	}
      
      if ((!column) || (small_screen))
	{
	  Term_gotoxy(0, 6 + (grp_cur - grp_top));
	}
      else
	{
	  Term_gotoxy(max + 3, 6 + mult *(object_cur - object_top));
	}
      
      ke = inkey_ex();
      
      /* Mouse escape */
      if ((ke.key == '\xff') && (ke.mousey == knowledge_row + 1))
	ke.key = ESCAPE;
      
      switch (ke.key)
	{
	case ESCAPE:
	  {
	    flag = TRUE;
	    break;
	  }
	  
	default:
	  {
	    /* Move the cursor */
	    browser_cursor(ke, &column, max, &grp_cur, grp_cnt, grp_top, 
			   &object_cur, object_cnt, object_top);
	    break;
	  }
	}
    }
  
  /* XXX XXX Free the "object_idx" array */
  C_KILL(object_idx, MAX_K_IDX, int);
}


/*
 * Display contents of the Home. Code taken from the player death interface 
 * and the show known objects function. -LM-
 */
static void do_cmd_knowledge_home(void)
{
  int k, which = 0;
  
  FILE *fff;
  
  object_type *o_ptr;
  char o_name[120];
  
  char file_name[1024];
  
  store_type *st_ptr;
  
  /* Temporary file */
  fff = my_fopen_temp(file_name, sizeof(file_name));
  
  /* Failure */
  if (!fff)
    {
      msg_print("Could not open a temporary file to show the contents of your	home.");
      return;
    }

  /* Get the store number of the home */
  for (k = 0; k < NUM_TOWNS; k++)
    {
      /* Found the town */
      if (p_ptr->home == towns[k])
	{
	  which += (k < NUM_TOWNS_SMALL ? 3 : STORE_HOME);
	  break;
	}
      /* Next town */
      else
	which += (k < NUM_TOWNS_SMALL ? MAX_STORES_SMALL : MAX_STORES_BIG);
    }

  /* Activate the store */
  st_ptr = &store[which];

  /* Home -- if anything there */
  if (st_ptr->stock_num)
    {
      /* Display contents of the home */
      for (k = 0; k < st_ptr->stock_num; k++)
	{
	  /* Acquire item */
	  o_ptr = &st_ptr->stock[k];
	  
	  /* Acquire object description */
	  object_desc(o_name, o_ptr, TRUE, 4);
	  
	  /* Print a message */
	  fprintf(fff, "     %s\n", o_name);
	}
    }
  
  /* Close the file */
  my_fclose(fff);
  
  /* Display the file contents */
  show_file(file_name, "Contents of Your Home", 0, 0);
  
  /* Remove the file */
  fd_kill(file_name);
}

/*
 * Description of each monster group.
 */
static cptr monster_group_text[] =
  {
    "Uniques",			       	/*All uniques, all letters*/
    "Ants",  				/*'a'*/
    /* Unused */			/*'A'*/
    "Bats",				/*'b'*/
    "Birds",				/*'B'*/
    "Centipedes",			/*'c'*/
    "Canines",				/*'C'*/
    "Dragons",				/*'d'*/
    "Ancient Dragons/Wyrms",		/*'D'*/
    "Floating Eyes",			/*'e'*/
    "Elementals",			/*'E'*/
    "Felines",				/*'f'*/
    "Dragon Flies",			/*'F'*/
    "Golems",				/*'g'*/
    "Ghosts",				/*'G'*/
    "Humanoids",        		/*'h'*/
    "Hybrids",				/*'H'*/
    "Icky Things",			/*'i'*/
    "Insects",				/*'I'*/
    "Jellies",				/*'j'*/
    "Snakes",				/*'J'*/
    "Kobolds",				/*'k'*/
    "Killer Beetles",			/*'K'*/
    "Lice",				/*'l'*/
    "Lichs",				/*'L'*/
    "Molds",				/*'m'*/
    "Mummies",     	                /*'M'*/
    "Nagas",				/*'n'*/
    /*Unused*/				/*'N'*/
    "Orcs",				/*'o'*/
    "Ogres",				/*'O'*/
    "People/Humans",			/*'p'*/
    "Giant Humanoids",			/*'P'*/
    "Quadrupeds",			/*'q'*/
    "Quylthulgs",			/*'Q'*/
    "Rodents",				/*'r'*/
    "Reptiles/Amphibians",		/*'R'*/
    "Skeletons",			/*'s'*/
    "Spiders/Scorpions/Ticks",		/*'S'*/
    "Townpersons",			/*'t'*/
    "Trolls",				/*'T'*/
    "Minor Demons",			/*'u'*/
    "Major Demons",			/*'U'*/
    "Vortices",				/*'v'*/
    "Vampires",				/*'V'*/
    "Worms/Worm Masses",		/*'w'*/
    "Wight/Wraith/etc",			/*'W'*/
    /*Unused*/				/*'x'*/
    "Xorn/Xaren/etc",			/*'X'*/
    "Yeeks",				/*'y'*/
    "Yetis",				/*'Y'*/
    "Zombies",            		/*'z'*/
    "Zephyr Hounds",			/*'Z'*/
    "Mushroom Patches",			/*','*/
    "Mimics",				/*'$!?=._-*/
    NULL
};

/*
 * Symbols of monsters in each group. Note the "Uniques" group
 * is handled differently.
 */
static cptr monster_group_char[] =
{
  (char *) -1L,
  "a",
  /*"A",  Unused*/
  "b",
  "B",
  "c",
  "C",
  "d",
  "D",
  "e",
  "E",
  "f",
  "F",
  "g",
  "G",
  "h",
  "H",
  "i",
  "I",
  "j",
  "J",
  "k",
  "K",
  "l",
  "L",
  "m",
  "M",
  "n",
  /*"N",  Unused*/
  "o",
  "O",
  "p",
  "P",
  "q",
  "Q",
  "r",
  "R",
  "s",
  "S",
  "t",
  "T",
  "u",
  "U",
  "v",
  "V",
  "w",
  "W",
  /*"x", Unused*/
  "X",
  "y",
  "Y",
  "z",
  "Z",
  ",",
  "$!?=._-",  /*Mimics*/
  NULL
};

/*
 * Build a list of monster indexes in the given group. Return the number
 * of monsters in the group.
 */
static int collect_monsters(int grp_cur, monster_list_entry *mon_idx, int mode)
{
  int i, mon_count = 0;
  
  /* Get a list of x_char in this group */
  cptr group_char = monster_group_char[grp_cur];
  
  /* XXX Hack -- Check if this is the "Uniques" group */
  bool grp_unique = (monster_group_char[grp_cur] == (char *) -1L);
  
  /* Check every race */
  for (i = 0; i < MAX_R_IDX; i++)
    {
      /* Access the race */
      monster_race *r_ptr = &r_info[i];
      monster_lore *l_ptr = &l_list[i];
      
      /* Is this a unique? */
      bool unique = (r_ptr->flags1 & (RF1_UNIQUE));
      
      /* Skip empty race */
      if (!r_ptr->aaf) continue;
      
      /* No Player Ghosts, unless active */
      if ((r_ptr->flags2 & (RF2_PLAYER_GHOST)) && (r_ptr->cur_num == 0)) 
	continue;

      if (grp_unique && !(unique)) continue;
      
      /* Require known monsters */
      if (!(mode & 0x02) && (!cheat_know) && (!(l_ptr->sights))) continue;
      
      /* Check for race in the group */
      if ((grp_unique) || (strchr(group_char, r_ptr->d_char)))
	{
	  /* Add the race */
	  mon_idx[mon_count++].r_idx = i;
	  
	  /* XXX Hack -- Just checking for non-empty group */
	  if (mode & 0x01) break;
	}
    }
  
  /* Terminate the list */
  mon_idx[mon_count].r_idx = 0;
  
  /* Return the number of races */
  return (mon_count);
}

/*
 * Display the object groups.
 */
static void display_group_list(int col, int row, int wid, int per_page,
	int grp_idx[], cptr group_text[], int grp_cur, int grp_top)
{
  int i;
  
  /* Display lines until done */
  for (i = 0; i < per_page && (grp_idx[i] >= 0); i++)
    {
      /* Get the group index */
      int grp = grp_idx[grp_top + i];
      
      /* Choose a color */
      byte attr = (grp_top + i == grp_cur) ? TERM_L_BLUE : TERM_WHITE;
      
      /* Erase the entire line */
      Term_erase(col, row + i, wid);
      
      /* Display the group label */
      c_put_str(attr, group_text[grp], row + i, col);
    }
}

/*
 * Display the monsters in a group.
 */
static void display_monster_list(int col, int row, int per_page, 
				 monster_list_entry *mon_idx,
				 int mon_cur, int mon_top, int grp_cur)
{
  int i;
  int disp_row = 0;
  int mult;
  
  u32b known_uniques, dead_uniques, slay_count;
  
  /* Start with 0 kills */
  known_uniques = dead_uniques = slay_count = 0;
  
  /* Count up monster kill counts */
  for (i = 1; i < MAX_R_IDX - 1; i++)
    {
      monster_race *r_ptr = &r_info[i];
      monster_lore *l_ptr = &l_list[i];
      
      /* Require non-unique monsters */
      if (r_ptr->flags1 & RF1_UNIQUE)
	{
	  /* No active player ghosts */
	  if ((r_ptr->flags2 & (RF2_PLAYER_GHOST)) && (r_ptr->cur_num == 0)) 
	    continue;
	  
	  /* Count if we have seen the unique */
	  if (l_ptr->sights)
	    {
	      known_uniques++;
	      
	      /*Count if the unique is dead*/
	      if (r_ptr->max_num == 0)
		{
		  dead_uniques++;
		  slay_count++;
		}
	    }
	  
	}
      
      /* Collect "appropriate" monsters */
      else slay_count += l_ptr->pkills;
    }

  /* Set the multiplier */
  mult = (use_trptile ? 3 : (use_dbltile ? 2 : 1));
  
  /* Display lines until done */
  for (i = 0; i < per_page && mon_idx[i].r_idx; i++)
    {
      byte attr;
      
      /* Get the race index */
      int r_idx = mon_idx[mon_top + i].r_idx;
      
      /* Access the race */
      monster_race *r_ptr = &r_info[r_idx];
      monster_lore *l_ptr = &l_list[r_idx];
      
      char race_name[80];
      
      /* Handle player chosts differently */
      if (r_ptr->flags2 & (RF2_PLAYER_GHOST))
	{
	  char racial_name[80];
	  
	  /* Get the ghost name. */
	  strcpy(race_name, ghost_name);
	  
	  /* Get the racial name. */
	  strcpy(racial_name, r_name + r_ptr->name);
	  
	  /* Build the ghost name. */
	  strcat(race_name, ", the ");
	  strcat(race_name, racial_name);
	}
      
      /* Get the monster race name (singular)*/
      else monster_desc_race(race_name, sizeof(race_name), r_idx);
      
      /* Choose a color */
      attr = ((i + mon_top == mon_cur) ? TERM_L_BLUE : TERM_WHITE);
      
      /* Get the display row */
      disp_row = mult * i;
      
      /* Display the name */
      c_prt(attr, race_name, row + disp_row, col);
      
      if (cheat_know)
	{
	  c_prt(attr, format ("%d", r_idx), row + disp_row, 
		(small_screen ? 28 : 60));
	}
      
      /* Clear the symbol area */
      Term_erase((small_screen ? 35 : 67), row + disp_row, 20);
      if (use_dbltile || use_trptile) 
	Term_erase((small_screen ? 35 : 67), row + disp_row + 1, 20);
      if (use_trptile)
	Term_erase((small_screen ? 35 : 67), row + disp_row + 2, 20);
  
      /* Display symbol */
      Term_putch((small_screen ? 35 : 67), row + disp_row, 
		  r_ptr->x_attr, r_ptr->x_char);
      
#ifdef USE_TRANSPARENCY
      if (use_bigtile || use_dbltile || use_trptile)
	{
	  big_putch((small_screen ? 35 : 67), row + disp_row, r_ptr->x_attr,
		    r_ptr->x_char);
	}
#endif

      /* Display kills */
      if (r_ptr->flags1 & (RF1_UNIQUE))
	{
	  /* Ignore status of shapechange only monsters */
	  if (!(r_ptr->flags2 & (RF2_NO_PLACE))) 

	    /* use alive/dead for uniques */
	    put_str(format("%5s", (r_ptr->max_num == 0) ? "dead" : "alive"),
		    row + disp_row, (small_screen ? 41 : 73));
	}
      else put_str(format("%5d", l_ptr->pkills), row + disp_row, 
		   (small_screen ? 41 : 73));
      
    }
  
  /* Clear remaining lines */
  for (i = disp_row + mult; i < per_page * mult; i++)
    {
      Term_erase(col, row + i, 255);
    }
  
  /* Clear the monster count line */
  Term_erase(0, 22, 255);
  
  if (monster_group_char[grp_cur] != (char *) -1L)
    {
      
      
      c_put_str(TERM_L_BLUE, format("Total Creatures Slain: %8d.", 
				    slay_count), 22, col);
    }
  else
    {
      c_put_str(TERM_L_BLUE, format("Known Uniques: %3d, Slain Uniques: %3d.", 
				    known_uniques, dead_uniques), 22, col);
    }
}

/*
 * Display known monsters.
 */
static void do_cmd_knowledge_monsters(void)
{
  int i, len, max;
  int grp_cur, grp_top;
  int mon_cur, mon_top;
  int grp_cnt, grp_idx[100];
  monster_list_entry *mon_idx;
  int monster_count;
  
  int column = 0;
  bool flag;
  bool redraw;
  
  /* Allocate the "mon_idx" array */
  C_MAKE(mon_idx, MAX_R_IDX, monster_list_entry);
  
  max = 0;
  grp_cnt = 0;
  
  /* Check every group */
  for (i = 0; monster_group_text[i] != NULL; i++)
    {
      
      /* Measure the label */
      len = strlen(monster_group_text[i]);
      
      /* Save the maximum length */
      if (len > max) max = len;
      
      /* See if any monsters are known */
      if ((monster_group_char[i] == ((char *) -1L)) || 
	  collect_monsters(i, mon_idx, 0x01))
	{
	  /* Build a list of groups with known monsters */
	  grp_idx[grp_cnt++] = i;
	}
      
    }
  
  /* Terminate the list */
  grp_idx[grp_cnt] = -1;
  
  grp_cur = grp_top = 0;
  mon_cur = mon_top = 0;
  
  flag = FALSE;
  redraw = TRUE;
  
  while (!flag)
    {
      key_event ke;
      int mult = 1;

      /* Set the multiplier */
      if (use_dbltile) mult = 2;
      else if (use_trptile) mult = 3;
      
      if (redraw)
	{
	  clear_from(0);
	  
	  prt("Knowledge - Monsters", knowledge_row, 0);
	  prt("ESC to quit", knowledge_row + 1, 0);
	  if (small_screen)
	    {
	      if (!column)
		prt("Group", header_row, 0);
	      else
		{
		  prt("Name", header_row, 3);
		  if (cheat_know) 
		    prt("Idx", header_row, 28);
		  prt("Sym   Kills", header_row, 35);
		}
	    }
	  else
	    {
	      prt("Group", header_row, 0);
	      prt("Name", header_row, max + 3);
	      if (cheat_know) prt("Idx", header_row, 60);
	      prt("Sym   Kills", header_row, 67);
	    }
	  
	  Term_putstr(0, 5, 4, TERM_WHITE, "<^>v");
	  for (i = 4; i < (small_screen ? 46 : 78); i++)
	    {
	      Term_putch(i, 5, TERM_WHITE, '=');
	    }
	  
	  if (small_screen)
	    {
	      for (i = 0; i < browser_rows; i++)
		{
		  clear_from(6 + i);
		}
	    }
	  else
	    {
	      for (i = 0; i < browser_rows; i++)
		{
		  Term_putch(max + 1, 6 + i, TERM_WHITE, '|');
		}
	      
	      redraw = FALSE;
	    }
	}
      
      /* Scroll group list */
      if (grp_cur < grp_top) grp_top = grp_cur;
      if (grp_cur >= grp_top + browser_rows) 
	grp_top = grp_cur - browser_rows + 1;
      
      /* Scroll monster list */
      if (mon_cur < mon_top) mon_top = mon_cur;
      if (mon_cur >= mon_top + browser_rows / mult) 
	{
	  mon_top = mon_cur - browser_rows / mult + 1;
	  if (mon_top < 0) mon_top = 0;
	}
      
      /* Display a list of monster groups */
      if ((!small_screen) || (!column))
	display_group_list(0, 6, (small_screen ? 48 : max), browser_rows, 
			   grp_idx, monster_group_text, grp_cur, grp_top);
      
      /* Get a list of monsters in the current group */
      monster_count = collect_monsters(grp_idx[grp_cur], mon_idx, 0x00);
      
      /* Display a list of monsters in the current group */
      if ((!small_screen) || (column))
	display_monster_list((small_screen ? 0 : max) + 3, 6, 
			     browser_rows / mult, 
			     mon_idx, mon_cur, mon_top, grp_cur);
      
      /* Track selected monster, to enable recall in sub-win*/
      p_ptr->monster_race_idx = mon_idx[mon_cur].r_idx;
      
      /* Prompt */
      prt("<dir>, 'r' to recall, ESC", 23, 0);
      
      /* Hack -- handle stuff */
      handle_stuff();
      
      if ((!column) || (small_screen))
	{
	  Term_gotoxy(0, 6 + (grp_cur - grp_top));
	}
      else
	{
	  Term_gotoxy(max + 3, 6 + mult * (mon_cur - mon_top));
	}
      
      ke = inkey_ex();
      
      /* Mouse input */
      if (ke.key == '\xff') 
	{
	  if ((ke.mousey == knowledge_row + 1) ||
	      ((ke.mousey == 23) && (ke.mousex > 21) && (ke.mousex < 25)))
	    ke.key = ESCAPE;
	  else if ((ke.mousey == 23) && (ke.mousex > 6) && (ke.mousex < 20))
	    ke.key = 'r';
	}

      switch (ke.key)
	{
	case ESCAPE:
	  {
	    flag = TRUE;
	    break;
	  }
	  
	case 'R':
	case 'r':
	  {
	    /* Recall on screen */
	    if (mon_idx[mon_cur].r_idx)
	      {
		screen_roff(mon_idx[mon_cur].r_idx);
		
		(void) inkey_ex();
		
		redraw = TRUE;
	      }
	    break;
	  }
	  
	default:
	  {
	    /* Move the cursor */
	    browser_cursor(ke, &column, max, &grp_cur, grp_cnt, grp_top, 
			   &mon_cur, monster_count, mon_top);
	    
	    /* Update to a new monster */
	    p_ptr->window |= (PW_MONSTER);
	    
	    break;
	  }
	}
    }
  
  /* XXX XXX Free the "mon_idx" array */
  C_KILL(mon_idx, MAX_R_IDX, monster_list_entry);
}

/*
 * Display kill counts
 */
static void do_cmd_knowledge_kills(void)
{
  int n, i;
  
  FILE *fff;
  
  char file_name[1024];
  
  u16b *who;
  u16b why = 4;
  
  
  /* Temporary file */
  fff = my_fopen_temp(file_name, sizeof(file_name));
  
  /* Failure */
  if (!fff) return;
  
  
  /* Allocate the "who" array */
  C_MAKE(who, MAX_R_IDX, u16b);
  
  /* Collect matching monsters */
  for (n = 0, i = 1; i < MAX_R_IDX - 1; i++)
    {
      monster_race *r_ptr = &r_info[i];
      monster_lore *l_ptr = &l_list[i];
      
      /* Require non-unique monsters */
      if (r_ptr->flags1 & RF1_UNIQUE) continue;
      
      /* Collect "appropriate" monsters */
      if (l_ptr->pkills > 0) who[n++] = i;
    }
  
  /* Select the sort method */
  ang_sort_comp = ang_sort_comp_hook;
  ang_sort_swap = ang_sort_swap_hook;
  
  /* Sort by kills (and level) */
  ang_sort(who, &why, n);
  
  /* Print the monsters (highest kill counts first) */
  for (i = n - 1; i >= 0; i--)
    {
      monster_race *r_ptr = &r_info[who[i]];
      monster_lore *l_ptr = &l_list[who[i]];
      
      /* Print a message */
      fprintf(fff, "%-36s  %5d\n",
	      (r_name + r_ptr->name), l_ptr->pkills);
    }
  
  /* Free the "who" array */
  C_FREE(who, MAX_R_IDX, u16b);
  
  /* Close the file */
  my_fclose(fff);
  
  /* Display the file contents */
  show_file(file_name, "Kill counts", 0, 0);
  
  /* Remove the file */
  fd_kill(file_name);
}

/*display the notes file*/
static void do_cmd_knowledge_notes(void)
{
  
  /*close the notes file for writing*/
  my_fclose(notes_file);
  
  show_file(notes_fname, "Notes", 0, 0);
  
  /*re-open for appending*/
  notes_file = my_fopen(notes_fname, "a");
  
}

/*
 * Interact with "knowledge"
 */
void do_cmd_knowledge(void)
{
  key_event ke;
  
  /* File type is "TEXT" */
  FILE_TYPE(FILE_TYPE_TEXT);
  
  /* Save screen */
  screen_save();
  
  /* Interact until done */
  while (1)
    {
      
      store_type *st_ptr;
      int k, which;

      /* Start at the beginning */
      which = 0;
      
      /* Get the store number of the home */
      for (k = 0; k < NUM_TOWNS; k++)
	{
	  /* Found the town */
	  if (p_ptr->home == towns[k])
	    {
	      which += (k < NUM_TOWNS_SMALL ? 3 : STORE_HOME);
	      break;
	    }
	  /* Next town */
	  else
	    which += (k < NUM_TOWNS_SMALL ? MAX_STORES_SMALL : MAX_STORES_BIG);
	}
      
      /* Activate the store */
      st_ptr = &store[which];

      /* Clear screen */
      Term_clear();
      
      /* Ask for a choice */
      prt("Display current knowledge", knowledge_row, 0);
      
      /* Give some choices */
      prt("(1) Display known artifacts", header_row, 5);
      prt("(2) Display known monsters", header_row + 1, 5);
      prt("(3) Display known objects", header_row + 2, 5);
      prt("(4) Display hall of fame", header_row + 3, 5);
      prt("(5) Display kill counts", header_row + 4, 5);
      
      /*allow the player to see the notes taken if that option is selected*/
      c_put_str((adult_take_notes ? TERM_WHITE : TERM_SLATE) ,
		"(6) Display character notes file", header_row + 5, 5);
      
      /*give player option to see home inventory if there is anything there */
      c_put_str((st_ptr->stock_num ? TERM_WHITE : TERM_SLATE) ,
		"(7) Display contents of your home", header_row + 6, 5);

      /* Quit */
      prt("ESC to exit", header_row + 7, 5);
      
      /* Prompt */
      prt("Command: ", header_row + 8, 0);
      
      /* Prompt */
      ke = inkey_ex();

      /* Mouse input */
      if (ke.key == '\xff')
	{
	  if ((ke.mousey > 3) && (ke.mousey < 11))
	    ke.key = I2D(ke.mousey - 3);
	  else
	    ke.key = ESCAPE;
	}
          
      /* Done */
      if (ke.key == ESCAPE) break;
      
      /* Artifacts */
      if (ke.key == '1')
	{
	  do_cmd_knowledge_artifacts();
	}
      
      /* Uniques */
      else if (ke.key == '2')
	{
	  do_cmd_knowledge_monsters();
	}
      
      /* Objects */
      else if (ke.key == '3')
	{
	  do_cmd_knowledge_objects();
	}
      
      /* Scores */
      else if (ke.key == '4')
	{
	  show_scores();
	}
      
      /* Scores */
      else if (ke.key == '5')
	{
	  do_cmd_knowledge_kills();
	}
      
      /* Ntoes file, if one exists */
      else if ((ke.key == '6') && (adult_take_notes))
	{
	  /* Spawn */
	  do_cmd_knowledge_notes();
	}
      
      /* Home inventory, if there is anything in the house */
      else if ((ke.key == '7') && (st_ptr->stock_num))
	{
	  /* Spawn */
	  do_cmd_knowledge_home();
	}
      
      /* Unknown option */
      else
	{
	  bell("Illegal command for knowledge!");
	}
      
      /* Flush messages */
      msg_print(NULL);
    }
  
  
  /* Load screen */
  screen_load();
}



/*
 * Display the time and date
 */
void do_cmd_time(void)
{
  s32b len = 10L * TOWN_DAWN;
  s32b tick = turn % len + len / 4;
  
  int day = turn / len + 1;
  int hour = (24 * tick / len) % 24;
  int min = (1440 * tick / len) % 60;
  
  
  /* Message */
  msg_format("This is day %d. The time is %d:%02d %s.", day,
	     (hour % 12 == 0) ? 12 : (hour % 12), min,
	     (hour < 12) ? "AM" : "PM");
}



