/* File: files.c */

/* Code for multiuser machines, Colors for skill descriptions, the char-
 * acter screens (inc. resistance flags for races, etc.), equippy chars, 
 * online-help, extraction of base name (for savefiles), saving, death 
 * (with inventory, equip, etc. display), calculating and displaying 
 * scores, creating tombstones, winners, panic saves, reading a random 
 * line from a file, and signal handling.
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"

/*
 * You may or may not want to use the following "#undef".
 */
/* #undef _POSIX_SAVED_IDS */


/*
 * Hack -- drop permissions
 */
void safe_setuid_drop(void)
{

#ifdef SET_UID

# ifdef SAFE_SETUID

#  ifdef SAFE_SETUID_POSIX

  if (setuid(getuid()) != 0)
    {
      quit("setuid(): cannot set permissions correctly!");
    }
  if (setgid(getgid()) != 0)
    {
      quit("setgid(): cannot set permissions correctly!");
    }
  
#  else
  
  if (setreuid(geteuid(), getuid()) != 0)
    {
      quit("setreuid(): cannot set permissions correctly!");
    }
  if (setregid(getegid(), getgid()) != 0)
    {
      quit("setregid(): cannot set permissions correctly!");
    }
  
#  endif
  
# endif
  
#endif
  
}


/*
 * Hack -- grab permissions
 */
void safe_setuid_grab(void)
{
  
#ifdef SET_UID
  
# ifdef SAFE_SETUID
  
#  ifdef SAFE_SETUID_POSIX
  
  if (setuid(player_euid) != 0)
    {
      quit("setuid(): cannot set permissions correctly!");
    }
  if (setgid(player_egid) != 0)
    {
      quit("setgid(): cannot set permissions correctly!");
    }
  
#  else
  
  if (setreuid(geteuid(), getuid()) != 0)
    {
      quit("setreuid(): cannot set permissions correctly!");
    }
  if (setregid(getegid(), getgid()) != 0)
    {
      quit("setregid(): cannot set permissions correctly!");
    }
  
#  endif
  
# endif
  
#endif
  
}



/*
 * Extract the first few "tokens" from a buffer
 *
 * This function uses "colon" and "slash" as the delimeter characters.
 *
 * We never extract more than "num" tokens.  The "last" token may include
 * "delimeter" characters, allowing the buffer to include a "string" token.
 *
 * We save pointers to the tokens in "tokens", and return the number found.
 *
 * Hack -- Attempt to handle the 'c' character formalism
 *
 * Hack -- An empty buffer, or a final delimeter, yields an "empty" token.
 *
 * Hack -- We will always extract at least one token
 */
s16b tokenize(char *buf, s16b num, char **tokens)
{
  int i = 0;
  
  char *s = buf;
  
  
  /* Process */
  while (i < num - 1)
    {
      char *t;
      
      /* Scan the string */
      for (t = s; *t; t++)
	{
	  /* Found a delimiter */
	  if ((*t == ':') || (*t == '/')) break;
	  
	  /* Handle single quotes */
	  if (*t == '\'')
	    {
	      /* Advance */
	      t++;
	      
	      /* Handle backslash */
	      if (*t == '\\') t++;
	      
	      /* Require a character */
	      if (!*t) break;
	      
	      /* Advance */
	      t++;
	      
	      /* Hack -- Require a close quote */
	      if (*t != '\'') *t = '\'';
	    }
	  
	  /* Handle back-slash */
	  if (*t == '\\') t++;
	}
      
      /* Nothing left */
      if (!*t) break;
      
      /* Nuke and advance */
      *t++ = '\0';
      
      /* Save the token */
      tokens[i++] = s;
      
      /* Advance */
      s = t;
    }
  
  /* Save the token */
  tokens[i++] = s;
  
  /* Number found */
  return (i);
}



/*
 * Parse a sub-file of the "extra info" (format shown below)
 *
 * Each "action" line has an "action symbol" in the first column,
 * followed by a colon, followed by some command specific info,
 * usually in the form of "tokens" separated by colons or slashes.
 *
 * Blank lines, lines starting with white space, and lines starting
 * with pound signs ("#") are ignored (as comments).
 *
 * Note the use of "tokenize()" to allow the use of both colons and
 * slashes as delimeters, while still allowing final tokens which
 * may contain any characters including "delimiters".
 *
 * Note the use of "strtol()" to allow all "integers" to be encoded
 * in decimal, hexidecimal, or octal form.
 *
 * Note that "monster zero" is used for the "player" attr/char, "object
 * zero" will be used for the "stack" attr/char, and "feature zero" is
 * used for the "nothing" attr/char.
 *
 * Specify the attr/char values for "monsters" by race index
 *   R:<num>:<a>/<c>
 *
 * Specify the attr/char values for "objects" by kind index
 *   K:<num>:<a>/<c>
 *
 * Specify the attr/char values for "features" by feature index
 *   F:<num>:<a>/<c>
 *
 * Specify the attr/char values for "flavors" by flavors index.
 *   L:<num>:<a>/<c>
 *
 * Specify the attr/char values for "special" things
 *   S:<num>:<a>/<c>
 *
 * Specify the attr/char values for unaware "objects" by kind tval
 *   U:<tv>:<a>/<c>
 *
 * Specify the attribute values for inventory "objects" by kind tval
 *   E:<tv>:<a>
 *
 * Define a macro action, given an encoded macro action
 *   A:<str>
 *
 * Create a macro, given an encoded macro trigger
 *   P:<str>
 *
 * Create a keymap, given an encoded keymap trigger
 *   C:<num>:<str>
 *
 * Turn an option off, given its name
 *   X:<str>
 *
 * Turn an option on, given its name
 *   Y:<str>
 *
 * Specify visual information, given an index, and some data
 *   V:<num>:<kv>:<rv>:<gv>:<bv>
 */
errr process_pref_file_aux(char *buf)
{
  int i, j, n1, n2;
  
  char *zz[16];
  
  
  /* Skip "empty" lines */
  if (!buf[0]) return (0);
  
  /* Skip "blank" lines */
  if (isspace(buf[0])) return (0);
  
  /* Skip comments */
  if (buf[0] == '#') return (0);
  
  
  /* Paranoia */
  /* if (strlen(buf) >= 1024) return (1); */
  
  
  /* Require "?:*" format */
  if (buf[1] != ':') return (1);
  
  
  /* Process "R:<num>:<a>/<c>" -- attr/char for monster races */
  if (buf[0] == 'R')
    {
      if (tokenize(buf+2, 3, zz) == 3)
	{
	  monster_race *r_ptr;
	  i = (huge)strtol(zz[0], NULL, 0);
	  n1 = strtol(zz[1], NULL, 0);
	  n2 = strtol(zz[2], NULL, 0);
	  if (i >= MAX_R_IDX) return (1);
	  r_ptr = &r_info[i];
	  if (n1) r_ptr->x_attr = n1;
	  if (n2) r_ptr->x_char = n2;
	  return (0);
	}
    }
  
  
  /* Process "K:<num>:<a>/<c>"  -- attr/char for object kinds */
  else if (buf[0] == 'K')
    {
      if (tokenize(buf+2, 3, zz) == 3)
	{
	  object_kind *k_ptr;
	  i = (huge)strtol(zz[0], NULL, 0);
	  n1 = strtol(zz[1], NULL, 0);
	  n2 = strtol(zz[2], NULL, 0);
	  if (i >= MAX_K_IDX) return (1);
	  k_ptr = &k_info[i];
	  if (n1) k_ptr->x_attr = n1;
	  if (n2) k_ptr->x_char = n2;
	  return (0);
	}
    }
  
  
  /* Process "F:<num>:<a>/<c>" -- attr/char for terrain features */
  else if (buf[0] == 'F')
    {
      if (tokenize(buf+2, 3, zz) == 3)
	{
	  feature_type *f_ptr;
	  i = (huge)strtol(zz[0], NULL, 0);
	  n1 = strtol(zz[1], NULL, 0);
	  n2 = strtol(zz[2], NULL, 0);
	  if (i >= MAX_F_IDX) return (1);
	  f_ptr = &f_info[i];
	  if (n1) f_ptr->x_attr = n1;
	  if (n2) f_ptr->x_char = n2;
	  
	  return (0);
	}
    }
  
  
  /* Process "L:<num>:<a>/<c>" -- attr/char for flavors */
  else if (buf[0] == 'L')
    {
      if (tokenize(buf+2, 3, zz) == 3)
	{
	  flavor_type *flavor_ptr;
	  i = (huge)strtol(zz[0], NULL, 0);
	  n1 = strtol(zz[1], NULL, 0);
	  n2 = strtol(zz[2], NULL, 0);
	  if ((i < 0) || (i >= MAX_FL_IDX)) return (1);
	  flavor_ptr = &flavor_info[i];
	  if (n1) flavor_ptr->x_attr = (byte)n1;
	  if (n2) flavor_ptr->x_char = (char)n2;
	  return (0);
	}
    }


  /* Process "S:<num>:<a>/<c>" -- attr/char for special things */
  else if (buf[0] == 'S')
    {
      if (tokenize(buf+2, 3, zz) == 3)
	{
	  j = (byte)strtol(zz[0], NULL, 0);
	  n1 = strtol(zz[1], NULL, 0);
	  n2 = strtol(zz[2], NULL, 0);
	  misc_to_attr[j] = n1;
	  misc_to_char[j] = n2;
	  return (0);
	}
    }
  
  
  /* Process "U:<tv>:<a>/<c>" -- attr/char for unaware items */
  else if (buf[0] == 'U')
    {
      if (tokenize(buf+2, 3, zz) == 3)
	{
	  j = (huge)strtol(zz[0], NULL, 0);
	  n1 = strtol(zz[1], NULL, 0);
	  n2 = strtol(zz[2], NULL, 0);
	  for (i = 1; i < MAX_K_IDX; i++)
	    {
	      object_kind *k_ptr = &k_info[i];
	      if (k_ptr->tval == j)
		{
		  if (n1) k_ptr->d_attr = n1;
		  if (n2) k_ptr->d_char = n2;
		}
	    }
	  return (0);
	}
    }
  
  
	/* Process "E:<tv>:<a>" -- attribute for inventory objects */
  else if (buf[0] == 'E')
    {
      if (tokenize(buf+2, 2, zz) == 2)
	{
	  j = (byte)strtol(zz[0], NULL, 0) % 128;
	  n1 = strtol(zz[1], NULL, 0);
	  if (n1) tval_to_attr[j] = n1;
	  return (0);
	}
    }
  
  
  /* Process "A:<str>" -- save an "action" for later */
  else if (buf[0] == 'A')
    {
      text_to_ascii(macro_buffer, buf+2);
      return (0);
    }
  
  /* Process "P:<str>" -- create macro */
  else if (buf[0] == 'P')
    {
      char tmp[1024];
      text_to_ascii(tmp, buf+2);
      macro_add(tmp, macro_buffer);
      return (0);
    }
  
  /* Process "C:<num>:<str>" -- create keymap */
  else if (buf[0] == 'C')
    {
      int mode;
      
      char tmp[1024];
      
      if (tokenize(buf+2, 2, zz) != 2) return (1);
      
      mode = strtol(zz[0], NULL, 0);
      if ((mode < 0) || (mode >= KEYMAP_MODES)) return (1);
      
      text_to_ascii(tmp, zz[1]);
      if (!tmp[0] || tmp[1]) return (1);
      i = (byte)(tmp[0]);
      
      string_free(keymap_act[mode][i]);
      
      keymap_act[mode][i] = string_make(macro_buffer);
      
      /* XXX Mega-Hack - Let system know a keymap changed */
      angband_keymap_flag = TRUE;
      
      return (0);
    }
  
  
  /* Process "V:<num>:<kv>:<rv>:<gv>:<bv>" -- visual info */
  else if (buf[0] == 'V')
    {
      if (tokenize(buf+2, 5, zz) == 5)
	{
	  i = (byte)strtol(zz[0], NULL, 0);
	  angband_color_table[i][0] = (byte)strtol(zz[1], NULL, 0);
	  angband_color_table[i][1] = (byte)strtol(zz[2], NULL, 0);
	  angband_color_table[i][2] = (byte)strtol(zz[3], NULL, 0);
	  angband_color_table[i][3] = (byte)strtol(zz[4], NULL, 0);
	  return (0);
	}
    }
  
  
  /* Process "X:<str>" -- turn option off */
  else if (buf[0] == 'X')
    {
      for (i = 0; i < OPT_adult_start; i++)
	{
	  if (option_text[i] && streq(option_text[i], buf + 2))
	    {
	      op_ptr->opt[i] = FALSE;
	      return (0);
	    }
	}
    }
  
  /* Process "Y:<str>" -- turn option on */
  else if (buf[0] == 'Y')
    {
      for (i = 0; i < OPT_adult_start; i++)
	{
	  if (option_text[i] && streq(option_text[i], buf + 2))
	    {
	      op_ptr->opt[i] = TRUE;
	      return (0);
	    }
	}
    }
  
  /* Process "W:<win>:<flag>:<value>" -- window flags */
  else if (buf[0] == 'W')
    {
      int win, flag, value;
      
      if (tokenize(buf + 2, 3, zz) == 3)
	{
	  win = strtol(zz[0], NULL, 0);
	  flag = strtol(zz[1], NULL, 0);
	  value = strtol(zz[2], NULL, 0);
	  
	  /* Ignore illegal windows */
	  /* Hack -- Ignore the main window */
	  if ((win <= 0) || (win >= 8)) return (1);
	  
	  /* Ignore illegal flags */
	  if ((flag < 0) || (flag >= 32)) return (1);
	  
	  /* Require a real flag */
	  if (window_flag_desc[flag])
	    {
	      if (value)
		{
		  /* Turn flag on */
		  op_ptr->window_flag[win] |= (1L << flag);
		}
	      else
		{
		  /* Turn flag off */
		  op_ptr->window_flag[win] &= ~(1L << flag);
		}
	    }
	  
	  /* Success */
	  return (0);
	}
    }
  
  /* Process "M:<type>:<attr>" -- colors for message-types */
  else if (buf[0] == 'M')
    {
      if (tokenize(buf+2, 2, zz) == 2)
	{
	  u16b type = (u16b)strtol(zz[0], NULL, 0);
	  int color = color_char_to_attr(zz[1][0]);
	  
	  /* Ignore illegal types */
	  if (type >= MSG_MAX) return (1);
	  
	  /* Store the color */
	  message__color[type] = (byte)color;
	  
	  /* Success */
	  return (0);
	}
    }
  
  /* Failure */
  return (1);
}



/*
 * Helper function for "process_pref_file()"
 *
 * Input:
 *   v: output buffer array
 *   f: final character
 *
 * Output:
 *   result
 */
static cptr process_pref_file_expr(char **sp, char *fp)
{
  cptr v;
  
  char *b;
  char *s;
  
  char b1 = '[';
  char b2 = ']';
  
  char f = ' ';
  
  /* Initial */
  s = (*sp);
  
  /* Skip spaces */
  while (isspace(*s)) s++;
  
  /* Save start */
  b = s;
  
  /* Default */
  v = "?o?o?";
  
  /* Analyze */
  if (*s == b1)
    {
      const char *p;
      const char *t;
      
      /* Skip b1 */
      s++;
      
      /* First */
      t = process_pref_file_expr(&s, &f);
      
      /* Oops */
      if (!*t)
	{
	  /* Nothing */
	}
      
      /* Function: IOR */
      else if (streq(t, "IOR"))
	{
	  v = "0";
	  while (*s && (f != b2))
	    {
	      t = process_pref_file_expr(&s, &f);
	      if (*t && !streq(t, "0")) v = "1";
	    }
	}
      
      /* Function: AND */
      else if (streq(t, "AND"))
	{
	  v = "1";
	  while (*s && (f != b2))
	    {
	      t = process_pref_file_expr(&s, &f);
	      if (*t && streq(t, "0")) v = "0";
	    }
	}
      
      /* Function: NOT */
      else if (streq(t, "NOT"))
	{
	  v = "1";
	  while (*s && (f != b2))
	    {
	      t = process_pref_file_expr(&s, &f);
	      if (*t && !streq(t, "0")) v = "0";
	    }
	}
      
      /* Function: EQU */
      else if (streq(t, "EQU"))
	{
	  v = "1";
	  if (*s && (f != b2))
	    {
	      t = process_pref_file_expr(&s, &f);
	    }
	  while (*s && (f != b2))
	    {
	      p = t;
	      t = process_pref_file_expr(&s, &f);
	      if (*t && !streq(p, t)) v = "0";
	    }
	}
      
      /* Function: LEQ */
      else if (streq(t, "LEQ"))
	{
	  v = "1";
	  if (*s && (f != b2))
	    {
	      t = process_pref_file_expr(&s, &f);
	    }
	  while (*s && (f != b2))
	    {
	      p = t;
	      t = process_pref_file_expr(&s, &f);
	      if (*t && (strcmp(p, t) >= 0)) v = "0";
	    }
	}
      
      /* Function: GEQ */
      else if (streq(t, "GEQ"))
	{
	  v = "1";
	  if (*s && (f != b2))
	    {
	      t = process_pref_file_expr(&s, &f);
	    }
	  while (*s && (f != b2))
	    {
	      p = t;
	      t = process_pref_file_expr(&s, &f);
	      if (*t && (strcmp(p, t) <= 0)) v = "0";
	    }
	}
      
      /* Oops */
      else
	{
	  while (*s && (f != b2))
	    {
	      t = process_pref_file_expr(&s, &f);
	    }
	}
      
      /* Verify ending */
      if (f != b2) v = "?x?x?";
      
      /* Extract final and Terminate */
      if ((f = *s) != '\0') *s++ = '\0';
    }
  
  /* Other */
  else
    {
      /* Accept all printables except spaces and brackets */
      while (isprint(*s) && !strchr(" []", *s)) ++s;
      
      /* Extract final and Terminate */
      if ((f = *s) != '\0') *s++ = '\0';
      
      /* Variable */
      if (*b == '$')
	{
	  /* System */
	  if (streq(b+1, "SYS"))
	    {
	      v = ANGBAND_SYS;
	    }
	  
	  /* Graphics */
	  else if (streq(b+1, "GRAF"))
	    {
	      v = ANGBAND_GRAF;
	    }

	  /* Race */
	  else if (streq(b+1, "RACE"))
	    {
	      v = rp_name + rp_ptr->name;
	    }
	  
	  /* Class */
	  else if (streq(b+1, "CLASS"))
	    {
	      v = cp_name + cp_ptr->name;
	    }
	  
	  /* Player */
	  else if (streq(b+1, "PLAYER"))
	    {
	      v = op_ptr->base_name;
	    }
	}
      
      /* Constant */
      else
	{
	  v = b;
	}
    }
  
  /* Save */
  (*fp) = f;
  
  /* Save */
  (*sp) = s;
  
  /* Result */
  return (v);
}


/*
 * Process the "user pref file" with the given name
 *
 * See the function above for a list of legal "commands".
 *
 * We also accept the special "?" and "%" directives, which
 * allow conditional evaluation and filename inclusion.
 */
errr process_pref_file(cptr name)
{
  FILE *fp;
  
  char buf[1024];
  
  char old[1024];
  
  int num = -1;
  
  errr err = 0;
  
  bool bypass = FALSE;
  
  
  /* Build the filename */
  path_build(buf, 1024, ANGBAND_DIR_PREF, name);
  
  /* Open the file */
  fp = my_fopen(buf, "r");
  
  /* Don't stop at non-existing file */
  if (!fp)
    {
      /* Build the filename */
      path_build(buf, sizeof(buf), ANGBAND_DIR_USER, name);
      
      /* Open the file */
      fp = my_fopen(buf, "r");
    }

  /* No such file */
  if (!fp) return (-1);
  
  /* Process the file */
  while (0 == my_fgets(fp, buf, 1024))
    {
      /* Count lines */
      num++;
      
      
      /* Skip "empty" lines */
      if (!buf[0]) continue;
      
      /* Skip "blank" lines */
      if (isspace(buf[0])) continue;
      
      /* Skip comments */
      if (buf[0] == '#') continue;
      
      
      /* Save a copy */
      strcpy(old, buf);
      
      
      /* Process "?:<expr>" */
      if ((buf[0] == '?') && (buf[1] == ':'))
	{
	  char f;
	  cptr v;
	  char *s;
	  
	  /* Start */
	  s = buf + 2;
	  
	  /* Parse the expr */
	  v = process_pref_file_expr(&s, &f);
	  
	  /* Set flag */
	  bypass = (streq(v, "0") ? TRUE : FALSE);
	  
	  /* Continue */
	  continue;
	}
      
      /* Apply conditionals */
      if (bypass) continue;
      
      
      /* Process "%:<file>" */
      if (buf[0] == '%')
	{
	  /* Process that file if allowed */
	  (void) process_pref_file(buf + 2);
	  
	  /* Continue */
	  continue;
	}
      
      
      /* Process the line */
      err = process_pref_file_aux(buf);
      
      /* Oops */
      if (err) break;
    }
  
  
  /* Error */
  if (err)
    {
      /* Useful error message */
      msg_format("Error %d in line %d of file '%s'.", err, num, name);
      msg_format("Parsing '%s'", old);
      msg_print(NULL);
    }
  
  /* Close the file */
  my_fclose(fp);
  
  /* Result */
  return (err);
}


#ifdef CHECK_TIME

/*
 * Operating hours for ANGBAND (defaults to non-work hours)
 */
static char days[7][29] =
{
  "SUN:XXXXXXXXXXXXXXXXXXXXXXXX",
  "MON:XXXXXXXX.........XXXXXXX",
  "TUE:XXXXXXXX.........XXXXXXX",
  "WED:XXXXXXXX.........XXXXXXX",
  "THU:XXXXXXXX.........XXXXXXX",
  "FRI:XXXXXXXX.........XXXXXXX",
  "SAT:XXXXXXXXXXXXXXXXXXXXXXXX"
};

/*
 * Restict usage (defaults to no restrictions)
 */
static bool check_time_flag = FALSE;

#endif


/*
 * Handle CHECK_TIME
 */
errr check_time(void)
{

#ifdef CHECK_TIME

  time_t c;
  struct tm *tp;
  
  /* No restrictions */
  if (!check_time_flag) return (0);
  
  /* Check for time violation */
  c = time((time_t *)0);
  tp = localtime(&c);
  
  /* Violation */
  if (days[tp->tm_wday][tp->tm_hour + 4] != 'X') return (1);
  
#endif
  
  /* Success */
  return (0);
}



/*
 * Initialize CHECK_TIME
 */
errr check_time_init(void)
{

#ifdef CHECK_TIME

  FILE *fp;
  
  char buf[1024];
  
  
  /* Build the filename */
  path_build(buf, 1024, ANGBAND_DIR_FILE, "time.txt");
  
  /* Open the file */
  fp = my_fopen(buf, "r");
  
  /* No file, no restrictions */
  if (!fp) return (0);
  
  /* Assume restrictions */
  check_time_flag = TRUE;
  
  /* Parse the file */
  while (0 == my_fgets(fp, buf, 80))
    {
      /* Skip comments and blank lines */
      if (!buf[0] || (buf[0] == '#')) continue;
      
      /* Chop the buffer */
      buf[29] = '\0';
      
      /* Extract the info */
      if (prefix(buf, "SUN:")) strcpy(days[0], buf);
      if (prefix(buf, "MON:")) strcpy(days[1], buf);
      if (prefix(buf, "TUE:")) strcpy(days[2], buf);
      if (prefix(buf, "WED:")) strcpy(days[3], buf);
      if (prefix(buf, "THU:")) strcpy(days[4], buf);
      if (prefix(buf, "FRI:")) strcpy(days[5], buf);
      if (prefix(buf, "SAT:")) strcpy(days[6], buf);
    }
  
  /* Close it */
  my_fclose(fp);
  
#endif
  
  /* Success */
  return (0);
}



#ifdef CHECK_LOAD

#ifndef MAXHOSTNAMELEN
# define MAXHOSTNAMELEN	 64
#endif

typedef struct statstime statstime;

struct statstime
{
  int cp_time[4];
  int dk_xfer[4];
  unsigned int	    v_pgpgin;
  unsigned int	    v_pgpgout;
  unsigned int	    v_pswpin;
  unsigned int	    v_pswpout;
  unsigned int	    v_intr;
  int if_ipackets;
  int if_ierrors;
  int if_opackets;
  int if_oerrors;
  int if_collisions;
  unsigned int	    v_swtch;
  long avenrun[3];
  struct timeval	    boottime;
  struct timeval	    curtime;
};

/*
 * Maximal load (if any).
 */
static int check_load_value = 0;

#endif


/*
 * Handle CHECK_LOAD
 */
errr check_load(void)
{

#ifdef CHECK_LOAD

  struct statstime    st;
  
  /* Success if not checking */
  if (!check_load_value) return (0);
  
  /* Check the load */
  if (0 == rstat("localhost", &st))
    {
      long val1 = (long)(st.avenrun[2]);
      long val2 = (long)(check_load_value) * FSCALE;
      
      /* Check for violation */
      if (val1 >= val2) return (1);
    }
  
#endif
  
  /* Success */
  return (0);
}


/*
 * Initialize CHECK_LOAD
 */
errr check_load_init(void)
{

#ifdef CHECK_LOAD

  FILE *fp;
  
  char buf[1024];

  char temphost[MAXHOSTNAMELEN+1];
  char thishost[MAXHOSTNAMELEN+1];
  
  
  /* Build the filename */
  path_build(buf, 1024, ANGBAND_DIR_FILE, "load.txt");
  
  /* Open the "load" file */
  fp = my_fopen(buf, "r");
  
  /* No file, no restrictions */
  if (!fp) return (0);
  
  /* Default load */
  check_load_value = 100;
  
  /* Get the host name */
  (void)gethostname(thishost, (sizeof thishost) - 1);
  
  /* Parse it */
  while (0 == my_fgets(fp, buf, 1024))
    {
      int value;
      
      /* Skip comments and blank lines */
      if (!buf[0] || (buf[0] == '#')) continue;
      
      /* Parse, or ignore */
      if (sscanf(buf, "%s%d", temphost, &value) != 2) continue;
      
      /* Skip other hosts */
      if (!streq(temphost, thishost) &&
	  !streq(temphost, "localhost")) continue;
      
      /* Use that value */
      check_load_value = value;
      
      /* Done */
      break;
    }
  
  /* Close the file */
  my_fclose(fp);
  
#endif
  
  /* Success */
  return (0);
}

/*
 * Print long number with header at given row, column
 * Use the color for the number, not the header
 */
static void prt_lnum(cptr header, s32b num, int row, int col, byte color)
{
  int len = strlen(header);
  char out_val[32];
  put_str(header, row, col);
  sprintf(out_val, "%9ld", (long)num);
  c_put_str(color, out_val, row, col + len);
}

/*
 * Print number with header at given row, column
 */
static void prt_num(cptr header, int num, int row, int col, byte color)
{
  int len = strlen(header);
  char out_val[32];
  put_str(header, row, col);
  put_str("   ", row, col + len);
  sprintf(out_val, "%6ld", (long)num);
  c_put_str(color, out_val, row, col + len);
}

/*
 * Print decimal number with header at given row, column
 */
static void prt_deci(cptr header, int num, int deci, int row, int col, 
		     byte color)
{
  int len = strlen(header);
  char out_val[32];
  put_str(header, row, col);
  put_str("   ", row, col + len);
  sprintf(out_val, "%8ld", (long)deci);
  c_put_str(color, out_val, row, col + len);
  sprintf(out_val, "%6ld", (long)num);
  c_put_str(color, out_val, row, col + len);
  sprintf(out_val, ".");
  c_put_str(color, out_val, row, col + len + 6);
}



/*
 * Prints the player resistance levels.
 */
static void display_player_resistances(void)
{
  int resist;
  byte color;
  int row_adj, col_adj;

  row_adj = (small_screen ? 3 : 0);
  
  resist = extract_resistance[p_ptr->dis_res_list[P_RES_ACID]];
  color = ((resist < 0) ? TERM_RED :
	   (resist == 0) ? TERM_YELLOW :
	   (resist < 70) ? TERM_WHITE :
	   TERM_L_GREEN);
  prt_num("Res Acid:      ", resist, 10 + row_adj, 1, color);
  
  resist = extract_resistance[p_ptr->dis_res_list[P_RES_ELEC]];
  color = ((resist < 0) ? TERM_RED :
	   (resist == 0) ? TERM_YELLOW :
	   (resist < 70) ? TERM_WHITE :
	   TERM_L_GREEN);
  prt_num("Res Lightning: ", resist, 11 + row_adj, 1, color);
  
  resist = extract_resistance[p_ptr->dis_res_list[P_RES_FIRE]];
  color = ((resist < 0) ? TERM_RED :
	   (resist == 0) ? TERM_YELLOW :
	   (resist < 70) ? TERM_WHITE :
	   TERM_L_GREEN);
  prt_num("Res Fire:      ", resist, 12 + row_adj, 1, color);
  
  resist = extract_resistance[p_ptr->dis_res_list[P_RES_COLD]];
  color = ((resist < 0) ? TERM_RED :
	   (resist == 0) ? TERM_YELLOW :
	   (resist < 70) ? TERM_WHITE :
	   TERM_L_GREEN);
  prt_num("Res Cold:      ", resist, 13 + row_adj, 1, color);
  
  resist = extract_resistance[p_ptr->dis_res_list[P_RES_POIS]];
  color = ((resist < 0) ? TERM_RED :
	   (resist == 0) ? TERM_YELLOW :
	   (resist < 70) ? TERM_WHITE :
	   TERM_L_GREEN);
  prt_num("Res Poison:    ", resist, 14 + row_adj, 1, color);
  
  row_adj = (small_screen ? 8 : 0);
  col_adj = (small_screen ? -25 : 0);
  
  resist = extract_resistance[p_ptr->dis_res_list[P_RES_LITE]];
  color = ((resist < 0) ? TERM_RED :
	   (resist == 0) ? TERM_YELLOW :
	   (resist < 70) ? TERM_WHITE :
	   TERM_L_GREEN);
  prt_num("Res Light:     ", resist, 10 + row_adj, 26 + col_adj, color);
  
  resist = extract_resistance[p_ptr->dis_res_list[P_RES_DARK]];
  color = ((resist < 0) ? TERM_RED :
	   (resist == 0) ? TERM_YELLOW :
	   (resist < 70) ? TERM_WHITE :
	   TERM_L_GREEN);
  prt_num("Res Darkness:  ", resist, 11 + row_adj, 26 + col_adj, color);
  
  row_adj = (small_screen ? 1 : 0);
  
  resist = extract_resistance[p_ptr->dis_res_list[P_RES_CONFU]];
  color = ((resist < 0) ? TERM_RED :
	   (resist == 0) ? TERM_YELLOW :
	   (resist < 70) ? TERM_WHITE :
	   TERM_L_GREEN);
  prt_num("Res Confusion: ", resist, 12 + row_adj, 26, color);
  
  resist = extract_resistance[p_ptr->dis_res_list[P_RES_SOUND]];
  color = ((resist < 0) ? TERM_RED :
	   (resist == 0) ? TERM_YELLOW :
	   (resist < 70) ? TERM_WHITE :
	   TERM_L_GREEN);
  prt_num("Res Sound:     ", resist, 13 + row_adj, 26, color);
  
  resist = extract_resistance[p_ptr->dis_res_list[P_RES_SHARD]];
  color = ((resist < 0) ? TERM_RED :
	   (resist == 0) ? TERM_YELLOW :
	   (resist < 70) ? TERM_WHITE :
	   TERM_L_GREEN);
  prt_num("Res Shards:    ", resist, 14 + row_adj, 26, color);
  
  row_adj = (small_screen ? 6 : 0);
  col_adj = (small_screen ? -25 : 0);
  
  resist = extract_resistance[p_ptr->dis_res_list[P_RES_NEXUS]];
  color = ((resist < 0) ? TERM_RED :
	   (resist == 0) ? TERM_YELLOW :
	   (resist < 70) ? TERM_WHITE :
	   TERM_L_GREEN);
  prt_num("Res Nexus:     ", resist, 10 + row_adj, 51 + col_adj, color);
  
  resist = extract_resistance[p_ptr->dis_res_list[P_RES_NETHR]];
  color = ((resist < 0) ? TERM_RED :
	   (resist == 0) ? TERM_YELLOW :
	   (resist < 70) ? TERM_WHITE :
	   TERM_L_GREEN);
  prt_num("Res Nether:    ", resist, 11 + row_adj, 51 + col_adj, color);
  
  resist = extract_resistance[p_ptr->dis_res_list[P_RES_CHAOS]];
  color = ((resist < 0) ? TERM_RED :
	   (resist == 0) ? TERM_YELLOW :
	   (resist < 70) ? TERM_WHITE :
	   TERM_L_GREEN);
  prt_num("Res Chaos:     ", resist, 12 + row_adj, 51 + col_adj, color);
  
  resist = extract_resistance[p_ptr->dis_res_list[P_RES_DISEN]];
  color = ((resist < 0) ? TERM_RED :
	   (resist == 0) ? TERM_YELLOW :
	   (resist < 70) ? TERM_WHITE :
	   TERM_L_GREEN);
  prt_num("Res Disenchant:", resist, 13 + row_adj, 51 + col_adj, color);
}


/*
 * Prints the following information on the screen.
 *
 * For this to look right, the following should be spaced the
 * same as in the prt_lnum code... -CFT
 */
static void display_player_middle(void)
{
  int show_m_tohit = p_ptr->dis_to_h;
  int show_a_tohit = p_ptr->dis_to_h;
  int show_m_todam = p_ptr->dis_to_d;
  int show_a_todam = p_ptr->dis_to_d;
  
  object_type *o_ptr;
  char tmp[32];
  
  /* Dump the fighting bonuses to hit/dam */
  
  o_ptr = &inventory[INVEN_WIELD];
  
  /* Hack -- add in weapon info if known */
  if (object_known_p(o_ptr)) show_m_tohit += o_ptr->to_h;
  if (object_known_p(o_ptr)) show_m_todam += o_ptr->to_d;
  
  put_str("       (Fighting)    ", (small_screen ? 14 : 12), 1);
  prt_num("Blows/Round      ", p_ptr->num_blow, (small_screen ? 15 : 13), 1, 
	  TERM_L_BLUE);
  prt_num("+ to Skill       ", show_m_tohit, (small_screen ? 16 : 14), 1, 
	  TERM_L_BLUE);
  
  if (show_m_todam >= 0)
    prt_num("Deadliness (%)   ", deadliness_conversion[show_m_todam], 
	    (small_screen ? 17 : 15), 1, TERM_L_BLUE);
  else
    prt_num("Deadliness (%)   ", -deadliness_conversion[-show_m_todam], 
	    (small_screen ? 17 : 15), 1, TERM_L_BLUE);
  
  /* Dump the shooting bonuses to hit/dam */
  
  o_ptr = &inventory[INVEN_BOW];
  
  /* Hack -- add in weapon info if known */
  if (object_known_p(o_ptr)) show_a_tohit += o_ptr->to_h;
  if (object_known_p(o_ptr)) show_a_todam += o_ptr->to_d;
  
  put_str("       (Shooting)    ", (small_screen ? 14 : 12), 
	  (small_screen ? 26 : 53));

  if (small_screen)
    prt_deci("Shots/Round   ", p_ptr->num_fire/10, p_ptr->num_fire%10, 
	     15, 26, TERM_L_BLUE);
  else
    prt_deci("Shots/Round    ", p_ptr->num_fire/10, p_ptr->num_fire%10, 
	   13, 53, TERM_L_BLUE);

  if (small_screen)
    prt_num("+ to Skill      ", show_a_tohit, 16, 26, TERM_L_BLUE);
  else
    prt_num("+ to Skill       ", show_a_tohit, 14, 53, TERM_L_BLUE);

  if (small_screen)
    {
      if (show_a_todam > 0)
	prt_num("Deadliness (%)  ", deadliness_conversion[show_a_todam], 
		17, 26, TERM_L_BLUE);
      else
	prt_num("Deadliness (%)  ", -deadliness_conversion[-show_a_todam], 
		17, 26, TERM_L_BLUE);
    }
  else
    {
      if (show_a_todam > 0)
	prt_num("Deadliness (%)   ", deadliness_conversion[show_a_todam], 
		15, 53, TERM_L_BLUE);
      else
	prt_num("Deadliness (%)   ", -deadliness_conversion[-show_a_todam], 
		15, 53, TERM_L_BLUE);
    }
  /* Dump the base and bonus armor class */
  if (small_screen)
    put_str("AC/+ To AC", (small_screen ? 21 : 15), (small_screen ? 26 : 27));
  else
    put_str("Base AC/+ To AC", (small_screen ? 21 : 15), 
	    (small_screen ? 26 : 27));
  
  sprintf(tmp, "%3d", p_ptr->dis_ac);
  c_put_str(TERM_L_BLUE, tmp, (small_screen ? 21 : 15), 
	    (small_screen ? 41 : 43));
  
  put_str("/", (small_screen ? 21 : 15), (small_screen ? 44 : 46));
  
  sprintf(tmp, "%3d", p_ptr->dis_to_a);
  c_put_str(TERM_L_BLUE, tmp, (small_screen ? 21 : 15), 
	    (small_screen ? 45 : 47));
  
  if (p_ptr->lev >= p_ptr->max_lev)
    prt_num("Level            ", (int)p_ptr->lev, (small_screen ? 19 : 9), 
	    (small_screen ? 1 : 27), TERM_L_GREEN);
  else
    prt_num("Level            ", (int)p_ptr->lev, (small_screen ? 19 : 9), 
	    (small_screen ? 1 : 27), TERM_YELLOW);
  
  if (p_ptr->exp >= p_ptr->max_exp)
    {
      prt_lnum("Experience    ", p_ptr->exp, (small_screen ? 20 : 10), 
	       (small_screen ? 1 : 27), TERM_L_GREEN);
    }
  else
    {
      prt_lnum("Experience    ", p_ptr->exp, (small_screen ? 20 : 10), 
	       (small_screen ? 1 : 27), TERM_YELLOW);
    }
  
  prt_lnum("Max Exp       ", p_ptr->max_exp, (small_screen ? 21 : 11), 
	   (small_screen ? 1 : 27), TERM_L_GREEN);
  
  if (small_screen)
    {
      if (p_ptr->lev >= PY_MAX_LEVEL)
	{
	  put_str("Exp to Adv.  ", 19, 26);
	  c_put_str(TERM_L_GREEN, "       *****", 19, 26 + 11);
	}
      else
	{
	  prt_lnum("Exp to Adv.  ",
		   (s32b)(player_exp[p_ptr->lev - 1]),
		   19, 26, TERM_L_GREEN);
	}
      
      prt_lnum("Gold         ", p_ptr->au, 20, 26, TERM_L_GREEN);
    }
  else
    {
      if (p_ptr->lev >= PY_MAX_LEVEL)
	{
	  put_str("Exp to Adv.   ", 12, 27);
	  c_put_str(TERM_L_GREEN, "       *****", 12, 27 + 11);
	}
      else
	{
	  prt_lnum("Exp to Adv.   ",
		   (s32b)(player_exp[p_ptr->lev - 1]),
		   12, 27, TERM_L_GREEN);
	}
      
      prt_lnum("Gold          ", p_ptr->au, 13, 27, TERM_L_GREEN);
    }

  
  
  
  prt_num("Max Hit Points   ", p_ptr->mhp, (small_screen ? 11 : 9), 1, 
	  TERM_L_GREEN);
  
  if (p_ptr->chp >= p_ptr->mhp)
    {
      prt_num("Cur Hit Points   ", p_ptr->chp, (small_screen ? 12 : 10), 1, 
	      TERM_L_GREEN);
    }
  else if (p_ptr->chp > (p_ptr->mhp * op_ptr->hitpoint_warn) / 10)
    {
      prt_num("Cur Hit Points   ", p_ptr->chp, (small_screen ? 12 : 10), 1, 
	      TERM_YELLOW);
    }
  else
    {
      prt_num("Cur Hit Points   ", p_ptr->chp, (small_screen ? 12 : 10), 1, 
	      TERM_RED);
    }
  
  
  if (small_screen)
    {
      prt_num("Max SP (Mana)   ", p_ptr->msp, 11, 26, TERM_L_GREEN);
  
      if (p_ptr->csp >= p_ptr->msp)
	{
	  prt_num("Cur SP (Mana)   ", p_ptr->csp, 12, 26, TERM_L_GREEN);
	}
      else if (p_ptr->csp > (p_ptr->msp * op_ptr->hitpoint_warn) / 10)
	{
	  prt_num("Cur SP (Mana)   ", p_ptr->csp, 12, 26, TERM_YELLOW);
	}
      else
	{
	  prt_num("Cur SP (Mana)   ", p_ptr->csp, 12, 26, TERM_RED);
	}
    }
  else
    {
      prt_num("Max SP (Mana)    ", p_ptr->msp, 9, 53, TERM_L_GREEN);
  
      if (p_ptr->csp >= p_ptr->msp)
	{
	  prt_num("Cur SP (Mana)    ", p_ptr->csp, 10, 53, TERM_L_GREEN);
	}
      else if (p_ptr->csp > (p_ptr->msp * op_ptr->hitpoint_warn) / 10)
	{
	  prt_num("Cur SP (Mana)    ", p_ptr->csp, 10, 53, TERM_YELLOW);
	}
      else
	{
	  prt_num("Cur SP (Mana)    ", p_ptr->csp, 10, 53, TERM_RED);
	}
    }

}

/*
 * Hack -- pass color info around this file
 */
static byte likert_color = TERM_WHITE;


/*
 * Returns a "rating" of x depending on y
 */
static cptr likert(int x, int y)
{
  /* Paranoia */
  if (y <= 0) y = 1;
  
  /* Negative value */
  if (x < 0)
    {
      likert_color = TERM_L_DARK;
      return ("Awful");
    }
  
  /* Analyze the value */
  switch ((x / y))
    {
    case 0:
      {
	likert_color = TERM_RED;
	return ("Very Bad");
      }
    case 1:
      {
	likert_color = TERM_L_RED;
	return ("Bad");
      }
    case 2:
      {
	likert_color = TERM_ORANGE;
	return ("Poor");
      }
    case 3:
      {
	likert_color = TERM_ORANGE;
	return ("Mediocre");
      }
    case 4:
      {
	likert_color = TERM_YELLOW;
	return ("Fair");
      }
    case 5:
      {
	likert_color = TERM_YELLOW;
	return ("Good");
      }
    case 6:
    case 7:
      {
	likert_color = TERM_YELLOW;
	return ("Very Good");
      }
    case 8:
    case 9:
    case 10:
      {
	likert_color = TERM_L_GREEN;
	return ("Excellent");
      }
    case 11:
    case 12:
    case 13:
      {
	likert_color = TERM_L_GREEN;
	return ("Superb");
      }
    case 14:
    case 15:
    case 16:
    case 17:
      {
	likert_color = TERM_BLUE;
	return ("Heroic");
      }
    default:
      {
	likert_color = TERM_BLUE;
	return ("Legendary");
      }
    }
}

/*
 * Prints ratings on certain abilities
 *
 * This code is "imitated" elsewhere to "dump" a character sheet.
 */
static void display_player_various(void)
{
  int tmp;
  int xthn, xthb, xfos, xsrh;
  int xdis, xdev, xsav, xstl;
  cptr desc;
  
  object_type *o_ptr;
  
  
  /* Fighting Skill (with current weapon) */
  o_ptr = &inventory[INVEN_WIELD];
  tmp = p_ptr->to_h + o_ptr->to_h;
  xthn = p_ptr->skill_thn + (tmp * BTH_PLUS_ADJ);
  
  /* Shooting Skill (with current bow and normal missile) */
  o_ptr = &inventory[INVEN_BOW];
  tmp = p_ptr->to_h + o_ptr->to_h;
  xthb = p_ptr->skill_thb + (tmp * BTH_PLUS_ADJ);
  
  /* Basic abilities */
  xdis = p_ptr->skill_dis;
  xdev = p_ptr->skill_dev;
  xsav = p_ptr->skill_sav;
  xstl = p_ptr->skill_stl;
  xsrh = p_ptr->skill_srh;
  xfos = p_ptr->skill_fos;
  
  
  put_str("Fighting    :", (small_screen ? 6 : 18), 1);
  desc = likert(xthn, 10);
  c_put_str(likert_color, desc, (small_screen ? 6 : 18), 15);
  
  put_str("Bows/Throw  :", (small_screen ? 7 : 19), 1);
  desc = likert(xthb, 10);
  c_put_str(likert_color, desc, (small_screen ? 7 : 19), 15);
  
  put_str("Saving Throw:", (small_screen ? 8 : 20), 1);
  desc = likert(xsav, 7);
  c_put_str(likert_color, desc, (small_screen ? 8 : 20), 15);
  
  
  put_str("Stealth     :", (small_screen ? 6 : 18), (small_screen ? 25 : 27));
  desc = likert(xstl, 1);
  c_put_str(likert_color, desc, (small_screen ? 6 : 18), 
	    (small_screen ? 39 : 41));
  
  put_str("Perception  :", (small_screen ? 7 : 19), (small_screen ? 25 : 27));
  desc = likert(xfos, 6);
  c_put_str(likert_color, desc, (small_screen ? 7 : 19), 
	    (small_screen ? 39 : 41));
  
  put_str("Searching   :", (small_screen ? 8 : 20), (small_screen ? 25 : 27));
  desc = likert(xsrh, 6);
  c_put_str(likert_color, desc, (small_screen ? 8 : 20), 
	    (small_screen ? 39 : 41));
  
  
  put_str("Disarming   :", (small_screen ? 9 : 18), (small_screen ? 1 : 53));
  desc = likert(xdis, 8);
  c_put_str(likert_color, desc, (small_screen ? 9 : 18), 
	    (small_screen ? 15 : 67));
  
  put_str("Magic Device:", (small_screen ? 10 : 19), (small_screen ? 1 : 53));
  desc = likert(xdev, 8);
  c_put_str(likert_color, desc, (small_screen ? 10 : 19), 
	    (small_screen ? 15 : 67));
  
  put_str("Infra-Vision:", (small_screen ? 9 : 20), (small_screen ? 25 : 53));
  
  if (use_metric) put_str(format("%d meters", p_ptr->see_infra * 3), 
			  (small_screen ? 9 : 20), (small_screen ? 39 : 67));
  else put_str(format("%d feet", p_ptr->see_infra * 10), 
	       (small_screen ? 9 : 20), (small_screen ? 39 : 67));
}



/*
 * Obtain the "flags" for the player as if he was an item.  Currently includes 
 * race, class, and shapechange (optionally). -LM-
 *
 * Mega - Hack
 * 'shape' should be set on when calling this function for display purposes, 
 * but off when actually applying 'intrinsic flags' in xtra1.c.
 *
 * Shapeshift flags are displayed like race/class flags, but actually 
 * applied differently.
 */

void player_flags(u32b *f1, u32b *f2, u32b *f3, bool shape)
{
  /* Clear */
  (*f1) = (*f2) = (*f3) = 0L;
  
  /* Add racial flags */
  (*f1) |= rp_ptr->flags1;
  (*f2) |= rp_ptr->flags2;
  (*f3) |= rp_ptr->flags3;
  
  /* Warrior. */
  if (check_ability(SP_RELENTLESS))
    {
      if (p_ptr->lev >= 30) (*f2) |= (TR2_RES_FEAR);
      if (p_ptr->lev >= 40) (*f3) |= (TR3_REGEN);
    }
  
  /* Specialty ability Holy Light */
  if (check_ability(SP_HOLY_LIGHT))
    {
      (*f2) |= (TR2_RES_LITE);
    }
  
  /* Specialty ability Unlight */
  if (check_ability(SP_UNLIGHT))
    {
      (*f2) |= (TR2_RES_DARK);
    }
  
  if (shape)
    {
      
      /* Shapechange, if any. */
      switch (p_ptr->schange)
	{
	case SHAPE_BEAR:
	case SHAPE_NORMAL:
	  {
	    break;
	  }
	case SHAPE_MOUSE:
	  {
	    (*f1) |= (TR1_STEALTH);
	    (*f1) |= (TR1_INFRA);
	    break;
	  }
	case SHAPE_FERRET:
	  {
	    (*f1) |= (TR1_INFRA);
	    (*f3) |= (TR3_REGEN);
	    (*f1) |= (TR1_SPEED);
	    (*f1) |= (TR1_SEARCH);
	    break;
	  }
	case SHAPE_HOUND:
	  {
	    (*f1) |= (TR1_INFRA);
	    (*f3) |= (TR3_TELEPATHY);
	    break;
	  }
	case SHAPE_GAZELLE:
	  {
	    (*f1) |= (TR1_SPEED);
	    break;
	  }
	case SHAPE_LION:
	  {
	    (*f1) |= (TR1_INFRA);
	    (*f2) |= (TR2_RES_FEAR);
	    (*f3) |= (TR3_REGEN);
	    (*f1) |= (TR1_SPEED);
	    break;
	  }
	case SHAPE_ENT:
	  {
	    (*f2) |= (TR2_RES_COLD);
	    (*f2) |= (TR2_RES_POIS);
	    (*f1) |= (TR2_RES_FEAR);
	    (*f3) |= (TR3_SEE_INVIS);
	    (*f3) |= (TR3_FREE_ACT);
	    (*f1) |= (TR1_TUNNEL);
	    break;
	  }
	case SHAPE_BAT:
	  {
	    (*f1) |= (TR1_INFRA);
	    (*f2) |= (TR2_RES_BLIND);
	    (*f3) |= (TR3_FEATHER);
	    (*f1) |= (TR1_SPEED);
	    break;
	  }
	case SHAPE_WEREWOLF:
	  {
	    (*f1) |= (TR1_INFRA);
	    (*f3) |= (TR3_REGEN);
	    (*f3) |= (TR3_AGGRAVATE);
	    break;	
	  }
	case SHAPE_VAMPIRE:
	  {
	    (*f1) |= (TR1_INFRA);
	    (*f3) |= (TR3_SEE_INVIS);
	    (*f3) |= (TR3_HOLD_LIFE);
	    (*f2) |= (TR2_RES_COLD);
	    (*f3) |= (TR3_REGEN);
	    (*f1) |= (TR1_STEALTH);
	    (*f1) |= (TR1_MAGIC_MASTERY);
	    break;
	  }
	case SHAPE_WYRM:
	  {
	    object_type *o_ptr = &inventory[INVEN_BODY];
	    (*f1) |= (TR1_STEALTH);
	    (*f1) |= (TR1_MAGIC_MASTERY);
	    
	    /* Paranoia */
	    if (o_ptr->tval != TV_DRAG_ARMOR) break;
	    
	    /* Add 'extra' power if any */
	    switch (o_ptr->sval)
	      {
		
	      case (SV_DRAGON_BLACK):
		{
		  (*f2) |= (TR2_IM_ACID);
		  break;
		}
	      case (SV_DRAGON_BLUE):
		{
		  (*f2) |= (TR2_IM_ELEC);
		  break;
		}
	      case (SV_DRAGON_WHITE):
		{
		  (*f2) |= (TR2_IM_COLD);
		  break;
		}
	      case (SV_DRAGON_RED):
		{
		  (*f2) |= (TR2_IM_FIRE);
		  break;
		}
	      case (SV_DRAGON_GREEN):
		{
		  (*f3) |= (TR3_REGEN);
		  break;
		}
	      case (SV_DRAGON_SHINING):
		{
		  (*f3) |= (TR3_SEE_INVIS);
		  break;
		}
	      case (SV_DRAGON_LAW):
	      case (SV_DRAGON_CHAOS):
		{
		  (*f3) |= (TR3_HOLD_LIFE);
		  break;
		}
	      case (SV_DRAGON_BRONZE):
	      case (SV_DRAGON_GOLD):
		{
		  (*f3) |= (TR3_FREE_ACT);
		  break;
		}
		
	      }
	    break;
	  }
	}
    }
}

/*
 * Obtain information about player negative mods.
 * Currently includes shapechange and race effects.
 *
 * We do not include AGGRAVATE, which is inherantly bad.  We only use
 * 'reversed' effects.
 *
 * The only effects that we *do* include are those which either totally
 * negate a resist/ability or those which have a negatively effective
 * pval.
 *
 * Based on player_flags, but for display purposes only.
 */
void player_weakness_dis(u32b *f1, u32b *f2, u32b *f3)
{
  /* Clear */
  (*f1) = (*f2) = (*f3) = 0L;
  
  /* HACK - add weakness of some races */
  if (check_ability(SP_WOODEN)) 
    {	
      (*f3) |= (TR3_FEATHER);
      (*f2) |= (TR2_RES_FIRE);
    }
  
  if (check_ability(SP_SHADOW)) 
    {	
      (*f2) |= (TR2_RES_LITE);
    }

  /* Shapechange, if any. */
  switch (p_ptr->schange)
    {
    case SHAPE_NORMAL:
      {
	break;
      }
    case SHAPE_MOUSE:
    case SHAPE_FERRET:
    case SHAPE_HOUND:
    case SHAPE_GAZELLE:
    case SHAPE_LION:
    case SHAPE_BAT:
    case SHAPE_WEREWOLF:
    case SHAPE_BEAR:
      {
	(*f1) |= (TR1_MAGIC_MASTERY);
	break;
      }
    case SHAPE_ENT:
      {
	(*f2) |= (TR2_RES_FIRE);
	(*f3) |= (TR3_FEATHER);
	break;
      }
    case SHAPE_VAMPIRE:
      {
	(*f2) |= (TR2_RES_LITE);
	(*f3) |= (TR3_LITE);
	break;
      }
    case SHAPE_WYRM:
      {
	(*f1) |= (TR1_STEALTH);
	break;
      }
    }
  
}


/*
 * Equippy chars
 */
static void display_player_equippy(int y, int x)
{
  int i;
  
  byte a;
  char c;
  
  object_type *o_ptr;
  object_kind *k_ptr;
	  
  
  /* Dump equippy chars */
  for (i=INVEN_WIELD; i<INVEN_SUBTOTAL; i++)
    {
      /* Object */
      o_ptr = &inventory[i];
      k_ptr = &k_info[o_ptr->k_idx];
      
      /* Skip empty objects */
      if (!o_ptr->k_idx) continue;
      
      /* Get attr/char for display */
      a = k_ptr->d_attr;
      c = k_ptr->d_char;
      
      /* Dump */
      Term_putch(x + i - INVEN_WIELD, y, a, c);
    }
}


/*
 * Hack -- see below
 */
static byte display_player_flag_set[4] =
{
  2,
  2,
  3,
  1
};

/*
 * Hack -- see below
 */
static u32b display_player_flag_head[4] =
{
  TR2_RES_ACID,
  TR2_RES_BLIND,
  TR3_SLOW_DIGEST,
  TR1_MAGIC_MASTERY
};

/*
 * Hack -- see below
 */
static cptr display_player_flag_names[4][8] =
{
  {
    " Acid:",	/* TR2_RES_ACID */
    " Elec:",	/* TR2_RES_ELEC */
    " Fire:",	/* TR2_RES_FIRE */
    " Cold:",	/* TR2_RES_COLD */
    " Pois:",	/* TR2_RES_POIS */
    " Fear:",	/* TR2_RES_FEAR */
    " Lite:",	/* TR2_RES_LITE */
    " Dark:"	/* TR2_RES_DARK */
  },
  
  {
    "Blind:",	/* TR2_RES_BLIND */
    "Confu:",	/* TR2_RES_CONFU */
    "Sound:",	/* TR2_RES_SOUND */
    "Shard:",	/* TR2_RES_SHARD */
    "Nexus:",	/* TR2_RES_NEXUS */
    "Nethr:",	/* TR2_RES_NETHR */
    "Chaos:",	/* TR2_RES_CHAOS */
    "Disen:"	/* TR2_RES_DISEN */
  },
  
  {
    "S.Dig:",	/* TR3_SLOW_DIGEST */
    "Feath:",	/* TR3_FEATHER */
    "PLite:",	/* TR3_LITE */
    "Regen:",	/* TR3_REGEN */
    "Telep:",	/* TR3_TELEPATHY */
    "Invis:",	/* TR3_SEE_INVIS */
    "FrAct:",	/* TR3_FREE_ACT */
    "HLife:"	/* TR3_HOLD_LIFE */
  },
  
  {
    "M-Mas:",	/* TR1_MAGIC_MASTERY */
    "Stea.:",	/* TR1_STEALTH */
    "Sear.:",	/* TR1_SEARCH */
    "Infra:",	/* TR1_INFRA */
    "Tunn.:",	/* TR1_TUNNEL */
    "Speed:",	/* TR1_SPEED */
    "Might:",	/* TR1_MIGHT2.  Hack -- also TR1_MIGHT1 */
    "Shots:",	/* TR1_SHOTS */
  }
};


/*
 * Special display, part 1
 */
static void display_player_flag_info(void)
{
  int x, y, i, n;
  
  int row, col;
  
  int set;
  u32b head;
  u32b flag;
  cptr name;
  
  u32b f[4];
  
  
  /* Four columns */
  for (x = 0; x < 4; x++)
    {
      /* Reset */
      row = (small_screen ? 5 : 10);
      col = 20 * (small_screen ? (x % 2) : x);
      
      /* Extract set */
      set = display_player_flag_set[x];
      
      /* Extract head */
      head = display_player_flag_head[x];
      
      /* Header */
      if ((!small_screen) || (x < 2))
	{
	  display_player_equippy(row++, col+6);
	  c_put_str(TERM_WHITE, "abcdefghijkl@", row++, col+6);
	}
      else
	{
	  row += 10;
	}

      /* Eight rows */
      for (y = 0; y < 8; y++)
	{
	  /* Extract flag */
	  flag = (head << y);
	  
	  /* Extract name */
	  name = display_player_flag_names[x][y];
	  
	  /* Header */
	  c_put_str(TERM_WHITE, name, row, col);
	  
	  /* Check equipment */
	  for (n=6, i=INVEN_WIELD; i<INVEN_SUBTOTAL; i++,n++)
	    {
	      byte attr = TERM_SLATE;
	      
	      object_type *o_ptr;
	      
	      /* Object */
	      o_ptr = &inventory[i];
	      
	      /* Known flags */
	      object_flags_known(o_ptr, &f[1], &f[2], &f[3]);
	      
	      /* Color columns by parity */
	      if (i % 2) attr = TERM_L_WHITE;

	      /* Non-existant objects */
	      if (!o_ptr->k_idx) attr = TERM_L_DARK;
	      
	      /* Hack -- Check immunities */
	      if ((x == 0) && (y < 4) &&
		  (f[set] & ((TR2_IM_ACID) << y)))
		{
		  c_put_str(TERM_WHITE, "*", row, col+n);
		}
	      
	      /* Check flags */
	      else if (f[set] & flag)
		{
		  c_put_str(TERM_WHITE, "+", row, col+n);
		}
	      
	      /* Default */
	      else
		{
		  c_put_str(attr, ".", row, col+n);
		}
	      
	      /* Hack -- "Might" covers both might1 and might2. */
	      if ((y == 6) && (x == 3))
		{
		  if (f[set] & TR1_MIGHT2) 
		    c_put_str(TERM_WHITE, "*", row, col+n);
		  else if (f[set] & TR1_MIGHT1) 
		    c_put_str(TERM_WHITE, "+", row, col+n);
		}
	      
	    }
	  
	  /* Player flags */
	  player_flags(&f[1], &f[2], &f[3], TRUE);
	  
	  /* Default */
	  c_put_str(TERM_SLATE, ".", row, col+n);
	  
	  /* Hack -- Check immunities */
	  if ((x == 0) && (y < 4) &&
	      (f[set] & ((TR2_IM_ACID) << y)))
	    {
	      c_put_str(TERM_WHITE, "*", row, col+n);
	    }
	  
	  /* Check flags */
	  else if (f[set] & flag) c_put_str(TERM_WHITE, "+", row, col+n);
	  
	  /* Player 'reversed' flags */
	  player_weakness_dis(&f[1], &f[2], &f[3]);
	  
	  /* Check flags */
	  if (f[set] & flag) c_put_str(TERM_RED, "X", row, col+n);
	  
	  /* Advance */
	  row++;
	}
      /* No footer for small screen */
      continue;
      
      /* Footer */
      c_put_str(TERM_WHITE, "abcdefghijkl@", row++, col+6);
      display_player_equippy(row++, col+6);
    }
}


/*
 * Special display, part 2a
 */
static void display_player_misc_info(void)
{
  char buf[80];
  
  /* Display basics */
  put_str("Name :", 2, 1);
  put_str("Sex  :", 3, 1);
  put_str("Race :", 4, 1);
  put_str("Class:", 5, 1);
  
  c_put_str(TERM_L_BLUE, op_ptr->full_name, 2, 8);
  c_put_str(TERM_L_BLUE, sp_ptr->title, 3, 8);
  c_put_str(TERM_L_BLUE, rp_name + rp_ptr->name, 4, 8);
  
  c_put_str(TERM_L_BLUE, cp_name + cp_ptr->name, 5, 8);
  
  /* Display extras */
  put_str("Level:", 6, 1);
  put_str("HP   :", 7, 1);
  put_str("SP   :", 8, 1);
  
  sprintf(buf, "%d", p_ptr->lev);
  c_put_str(TERM_L_BLUE, buf, 6, 8);
  sprintf(buf, "%d/%d", p_ptr->chp, p_ptr->mhp);
  c_put_str(TERM_L_BLUE, buf, 7, 8);
  sprintf(buf, "%d/%d", p_ptr->csp, p_ptr->msp);
  c_put_str(TERM_L_BLUE, buf, 8, 8);
}


/*
 * Special display, part 2b
 *
 * How to print out the modifications and sustains.
 * Positive mods with no sustain will be light green.
 * Positive mods with a sustain will be dark green.
 * Sustains (with no modification) will be a dark green 's'.
 * Negative mods (from a curse) will be red.
 * Huge mods (>9), like from MICoMorgoth, will be a '*'
 * No mod, no sustain, will be a slate '.'
 */
static void display_player_stat_info(void)
{
  int i, row, col;
  int stat_col, stat;
  
  object_type *o_ptr;
  u32b f1, f2, f3;
  u32b ignore_f2, ignore_f3;
  s16b k_idx;
  
  byte a;
  char c;
  
  char buf[80];
  
  
  /* Column */
  stat_col = (small_screen ? 7 : 24);
  
  /* Row */
  row = (small_screen ? 6 : 3);
  
  /* Print out the labels for the columns */
  c_put_str(TERM_WHITE, "Stat", row-1, stat_col);
  c_put_str(TERM_BLUE, "Intrnl", row-1, stat_col+5);
  c_put_str(TERM_L_BLUE, "Rce Cls Oth", row-1, stat_col+12);
  c_put_str(TERM_L_GREEN, "Actual", row-1, stat_col+24);
  c_put_str(TERM_YELLOW, "Currnt", row-1, stat_col+31);
  
  /* Display the stats */
  for (i = 0; i < A_MAX; i++)
    {
      /* Reduced name of stat */
      c_put_str(TERM_WHITE, stat_names_reduced[i], row+i, stat_col);
      
      /* Internal "natural" maximum value */
      cnv_stat(p_ptr->stat_max[i], buf);
      c_put_str(TERM_BLUE, buf, row+i, stat_col+5);
      
      /* Race, class, and equipment modifiers */
      sprintf(buf, "%3d", rp_ptr->r_adj[i]);
      c_put_str(TERM_L_BLUE, buf, row+i, stat_col+12);
      sprintf(buf, "%3d", cp_ptr->c_adj[i]);
      c_put_str(TERM_L_BLUE, buf, row+i, stat_col+16);
      sprintf(buf, "%3d", p_ptr->stat_add[i]);
      c_put_str(TERM_L_BLUE, buf, row+i, stat_col+20);
      
      /* Resulting "modified" maximum value */
      cnv_stat(p_ptr->stat_top[i], buf);
      c_put_str(TERM_L_GREEN, buf, row+i, stat_col+24);
      
      /* Only display stat_use if not maximal */
      if (p_ptr->stat_use[i] < p_ptr->stat_top[i])
	{
	  cnv_stat(p_ptr->stat_use[i], buf);
	  c_put_str(TERM_YELLOW, buf, row+i, stat_col+31);
	}
    }

  /* Done if small screen */
  if (small_screen) return;

  /* Column */
  col = 60+6;
  
  /* Header and Footer */
  c_put_str(TERM_WHITE, "abcdefghijkl@", row-1, col);
  
  /* Process equipment */
  for (i=INVEN_WIELD; i<INVEN_SUBTOTAL; i++)
    {
      /* Access object */
      o_ptr = &inventory[i];
      
      /* Object kind */
      k_idx = o_ptr->k_idx;
      
      /* Acquire "known" flags */
      object_flags_known(o_ptr, &f1, &f2, &f3);
      
      /* Hack -- assume stat modifiers are known */
      object_flags(o_ptr, &f1, &ignore_f2, &ignore_f3);
      
      /* Initialize color based of sign of pval. */
      for (stat=0; stat<A_MAX; stat++)
	{
	  /* Default */
	  a = TERM_SLATE;
	  c = '.';
	  
	  /* Boost */
	  if (f1 & 1<<stat)
	    {
	      /* Default */
	      c = '*';
	      
	      /* Good */
	      if (o_ptr->pval > 0)
		{
		  /* Good */
		  a = TERM_L_GREEN;
		  
		  /* Label boost */
		  if (o_ptr->pval < 10) c = '0' + o_ptr->pval;
		}
	      
	      /* Bad */
	      if (o_ptr->pval < 0)
		{
		  /* Bad */
		  a = TERM_RED;
		  
		  /* Label boost */
		  if (o_ptr->pval < 10) c = '0' - o_ptr->pval;
		}
	    }
	  
	  /* Sustain */
	  if (f2 & 1<<stat)
	    {
	      /* Dark green, "s" if no stat bonus. */
	      a = TERM_GREEN;
	      if (c == '.') c = 's';
	    }
	  
	  /* Dump proper character */
	  Term_putch(col, row+stat, a, c);
	}
      
      /* Advance */
      col++;
    }
  
  /* Player flags */
  player_flags(&f1, &f2, &f3, TRUE);
  
  /* Check stats */
  for (stat=0; stat<A_MAX; stat++)
    {
      /* Default */
      a = TERM_SLATE;
      c = '.';
      
      /* Sustain */
      if (f2 & 1<<stat)
	{
	  /* Dark green "s" */
	  a = TERM_GREEN;
	  c = 's';
	}
      
      /* Dump */
      Term_putch(col, row+stat, a, c);
    }
}


/*
 * Display the character on the screen (various modes)
 *
 * The top two and bottom two lines are left blank.
 *
 * Mode 0 = standard display with skills
 * Mode 1 = standard display with history
 * Mode 2 = special display with flags
 *
 * New, compact display for small screen -NRM-
 *
 */
void display_player(int mode)
{
  int i;
  
  int stat;
  
  object_type *o_ptr;
  u32b f1, f2, f3;
  u32b ignore_f2, ignore_f3;
  s16b k_idx;
  
  byte a;
  char c;
  
  char buf[80];
  
  /* Hack - small screen first */
  if (small_screen)
    {
      /* XXX XXX XXX */
      mode = (mode % 4);
      
      
      /* Erase screen */
      clear_from(0);

      /* First screen */
      if (mode == 0)
	{
	  /* Name, Sex, Race, Class */
	  put_str("Name    :", 0, 1);
	  put_str("Sex     :", 1, 1);
	  put_str("Race    :", 0, 27);
	  put_str("Class   :", 1, 27);
	  if (p_ptr->total_winner) put_str("***WINNER***", 2, 1);
	  
	  c_put_str(TERM_L_BLUE, op_ptr->full_name, 0, 11);
	  c_put_str(TERM_L_BLUE, sp_ptr->title, 1, 11);
	  
	  c_put_str(TERM_L_BLUE, rp_name + rp_ptr->name, 0, 37);
	  c_put_str(TERM_L_BLUE, cp_name + cp_ptr->name, 1, 37);
	  
	  /* Header and Footer */
	  put_str("abcdefghijkl@", 3, 25);
	  
	  /* Display the stats */
	  for (i = 0; i < A_MAX; i++)
	    {
	      /* Special treatment of "injured" stats */
	      if (p_ptr->stat_cur[i] < p_ptr->stat_max[i])
		{
		  int value;
		  
		  /* Use lowercase stat name */
		  put_str(stat_names_reduced[i], 4 + i, 0);
		  
		  /* Get the current stat */
		  value = p_ptr->stat_use[i];
		  
		  /* Obtain the current stat (modified) */
		  cnv_stat(value, buf);
		  
		  /* Display the current stat (modified) */
		  c_put_str(TERM_YELLOW, buf, 4 + i, 8);
		  
		  /* Acquire the max stat */
		  value = p_ptr->stat_top[i];
		  
		  /* Obtain the maximum stat (modified) */
		  cnv_stat(value, buf);
		  
		  /* Display the maximum stat (modified) */
		  c_put_str(TERM_L_GREEN, buf, 4 + i, 17);
		}
	      
	      /* Normal treatment of "normal" stats */
	      else
		{
		  /* Assume uppercase stat name */
		  put_str(stat_names[i], 4 + i, 0);
		  
		  /* Indicate natural maximum */
		  if (p_ptr->stat_max[i] == 18+100)
		    {
		      put_str("!", 4 + i, 3);
		    }
		  
		  /* Obtain the current stat (modified) */
		  cnv_stat(p_ptr->stat_use[i], buf);
		  
		  /* Display the current stat (modified) */
		  c_put_str(TERM_L_GREEN, buf, 4 + i, 8);
		}
	    }
	  /* Process equipment */
	  for (i = INVEN_WIELD; i < INVEN_SUBTOTAL; i++)
	    {
	      /* Access object */
	      o_ptr = &inventory[i];
      
	      /* Object kind */
	      k_idx = o_ptr->k_idx;
	      
	      /* Acquire "known" flags */
	      object_flags_known(o_ptr, &f1, &f2, &f3);
	      
	      /* Hack -- assume stat modifiers are known */
	      object_flags(o_ptr, &f1, &ignore_f2, &ignore_f3);
	      
	      /* Initialize color based of sign of pval. */
	      for (stat = 0; stat < A_MAX; stat++)
		{
		  /* Default */
		  a = TERM_SLATE;
		  c = '.';
	  
		  /* Boost */
		  if (f1 & 1<<stat)
		    {
		      /* Default */
		      c = '*';
		      
		      /* Good */
		      if (o_ptr->pval > 0)
			{
			  /* Good */
			  a = TERM_L_GREEN;
			  
			  /* Label boost */
			  if (o_ptr->pval < 10) c = '0' + o_ptr->pval;
			}
		      
		      /* Bad */
		      if (o_ptr->pval < 0)
			{
			  /* Bad */
			  a = TERM_RED;
			  
			  /* Label boost */
			  if (o_ptr->pval < 10) c = '0' - o_ptr->pval;
			}
		    }
		  
		  /* Sustain */
		  if (f2 & 1<<stat)
		    {
		      /* Dark green, "s" if no stat bonus. */
		      a = TERM_GREEN;
		      if (c == '.') c = 's';
		    }
		  
		  /* Dump proper character */
		  Term_putch(i + 1, 4 + stat, a, c);
		}

	      /* Player flags */
	      player_flags(&f1, &f2, &f3, TRUE);
  
	      /* Check stats */
	      for (stat=0; stat<A_MAX; stat++)
		{
		  /* Default */
		  a = TERM_SLATE;
		  c = '.';
		  
		  /* Sustain */
		  if (f2 & 1<<stat)
		    {
		      /* Dark green "s" */
		      a = TERM_GREEN;
		      c = 's';
		    }
		  
		  /* Dump */
		  Term_putch(37, 4 + stat, a, c);
		}
	    }
	  display_player_middle();
	      
	}

      /* Later screens */
      if ((mode == 1) || (mode == 2) || (mode == 3))
	{
	  /* Name, Sex, Race, Class */
	  put_str("Name    :", 0, 1);
	  put_str("Sex     :", 1, 1);
	  put_str("Race    :", 2, 1);
	  put_str("Class   :", 3, 1);
	  if (p_ptr->total_winner) put_str("***WINNER***", 4, 1);
	  
	  c_put_str(TERM_L_BLUE, op_ptr->full_name, 0, 11);
	  c_put_str(TERM_L_BLUE, sp_ptr->title, 1, 11);
      
	  c_put_str(TERM_L_BLUE, rp_name + rp_ptr->name, 2, 11);
	  c_put_str(TERM_L_BLUE, cp_name + cp_ptr->name, 3, 11);
	  
	  /* Age, Height, Weight, Social */
	  prt_num("Age              ", (int)p_ptr->age, 0, 25, TERM_L_BLUE);
	  
	  if (use_metric)	/* A GSNband idea. */
	    {
	      prt_num("Height           ", (int)p_ptr->ht * 254 / 100, 
		      1, 25, TERM_L_BLUE);
	      prt_num("Weight           ", (int)p_ptr->wt * 10 / 22, 
		      2, 25, TERM_L_BLUE);
	    }
	  else
	    {
	      prt_num("Height           ", (int)p_ptr->ht, 1, 25, TERM_L_BLUE);
	      prt_num("Weight           ", (int)p_ptr->wt, 2, 25, TERM_L_BLUE);
	    }
	  
	  prt_num("Social Class     ", (int)p_ptr->sc, 3, 25, TERM_L_BLUE);
	  
	  /* History and Abilities Screen */
	  if (mode == 1)
	    {
	      int adj = 0;
	      char buf1[60];
	      char buf2[30];

	      put_str("(Character Abilities)", 5, 16);
	      
	      display_player_various();

	      put_str("(Character Background)", 12, 16);
	      
	      for (i = 0; i < 4; i++)
		{
		  int k = 0, j, m;

		  if (!p_ptr->history[i]) break;

		  strcpy(buf1, p_ptr->history[i]);

		  /* Check if it finishes */
		  for (j = 0; j < 30; j++)
		    if (buf1[j] == '\0') break;

		  /* Print the line... */
		  if (buf1[j] == '\0') 
		    put_str(buf1, i + 13 + adj, 10);
		  else
		    {
		      /* ...or split the line... */
		      for (j = 30; buf1[j] != ' '; j++) ;
		      m = j;
		      while (buf1[j++] != '\0')
			buf2[k++] = buf1[j];
		      buf2[k] = '\0';
		      buf1[m] = '\0';
		      
		      /* ...and print the two bits */
		      put_str(buf1, i + 13 + adj++, 10);
		      put_str(buf2, i + 13 + adj, 10);
		    }
		}
	    }
	  
	  /* Flags screen */
	  if (mode == 2)
	    display_player_flag_info();

	  /* Stat breakdown and resistances screen */
	  if (mode == 3)
	    {
	      display_player_stat_info();
	      display_player_resistances();
	    }
	}
      
      /* Special */
      else if (mode == 2)
	{
	  /* See "http://www.cs.berkeley.edu/~davidb/angband.html" */
	  
	  /* Dump the info */
	  display_player_misc_info();
	  display_player_stat_info();
	  display_player_flag_info();
	}
    }
  else
    {
      /* XXX XXX XXX */
      mode = (mode % 3);
      
      
      /* Erase screen */
      clear_from(0);
      
      /* Standard */
      if ((mode == 0) || (mode == 1))
	{
	  /* Name, Sex, Race, Class */
	  put_str("Name    :", 2, 1);
	  put_str("Sex     :", 3, 1);
	  put_str("Race    :", 4, 1);
	  put_str("Class   :", 5, 1);
	  if (p_ptr->total_winner) put_str("***WINNER***", 6, 1);
	  
	  c_put_str(TERM_L_BLUE, op_ptr->full_name, 2, 11);
	  c_put_str(TERM_L_BLUE, sp_ptr->title, 3, 11);
      
	  c_put_str(TERM_L_BLUE, rp_name + rp_ptr->name, 4, 11);
	  c_put_str(TERM_L_BLUE, cp_name + cp_ptr->name, 5, 11);
	  
	  /* Age, Height, Weight, Social */
	  prt_num("Age              ", (int)p_ptr->age, 2, 27, TERM_L_BLUE);
	  
	  if (use_metric)	/* A GSNband idea. */
	    {
	      prt_num("Height           ", (int)p_ptr->ht * 254 / 100, 
		      3, 27, TERM_L_BLUE);
	      prt_num("Weight           ", (int)p_ptr->wt * 10 / 22, 
		      4, 27, TERM_L_BLUE);
	    }
	  else
	    {
	      prt_num("Height           ", (int)p_ptr->ht, 3, 27, TERM_L_BLUE);
	      prt_num("Weight           ", (int)p_ptr->wt, 4, 27, TERM_L_BLUE);
	    }
	  
	  prt_num("Social Class     ", (int)p_ptr->sc, 5, 27, TERM_L_BLUE);
	  
	  /* Display the stats */
	  for (i = 0; i < A_MAX; i++)
	    {
	      /* Special treatment of "injured" stats */
	      if (p_ptr->stat_cur[i] < p_ptr->stat_max[i])
		{
		  int value;
		  
		  /* Use lowercase stat name */
		  put_str(stat_names_reduced[i], 2 + i, 53);
		  
		  /* Get the current stat */
		  value = p_ptr->stat_use[i];
		  
		  /* Obtain the current stat (modified) */
		  cnv_stat(value, buf);
		  
		  /* Display the current stat (modified) */
		  c_put_str(TERM_YELLOW, buf, 2 + i, 61);
		  
		  /* Acquire the max stat */
		  value = p_ptr->stat_top[i];
		  
		  /* Obtain the maximum stat (modified) */
		  cnv_stat(value, buf);
		  
		  /* Display the maximum stat (modified) */
		  c_put_str(TERM_L_GREEN, buf, 2 + i, 70);
		}
	      
	      /* Normal treatment of "normal" stats */
	      else
		{
		  /* Assume uppercase stat name */
		  put_str(stat_names[i], 2 + i, 53);
		  
		  /* Indicate natural maximum */
		  if (p_ptr->stat_max[i] == 18+100)
		    {
		      put_str("!", 2 + i, 56);
		    }
		  
		  /* Obtain the current stat (modified) */
		  cnv_stat(p_ptr->stat_use[i], buf);
		  
		  /* Display the current stat (modified) */
		  c_put_str(TERM_L_GREEN, buf, 2 + i, 61);
		}
	    }
	  
	  /* History and Resistance Screen */
	  if (mode == 1)
	    {
	      display_player_resistances();
	      
	      put_str("(Character Background)", 17, 28);
	      
	      for (i = 0; i < 4; i++)
		{
		  put_str(p_ptr->history[i], i + 18, 10);
		}
	    }
	  
	  /* Initial Screen */
	  else
	    {
	      put_str("(Character Abilities)", 17, 28);
	      
	      /* Extra info */
	      display_player_middle();
	      
	      display_player_various();
	    }
	}
      
      /* Special */
      else if (mode == 2)
	{
	  /* See "http://www.cs.berkeley.edu/~davidb/angband.html" */
	  
	  /* Dump the info */
	  display_player_misc_info();
	  display_player_stat_info();
	  display_player_flag_info();
	}
    }
}


/*
 * Hack -- Dump a character description file
 *
 * XXX XXX XXX Allow the "full" flag to dump additional info,
 * and trigger its usage from various places in the code.
 */
errr file_character(cptr name, bool full)
{
  int i, j, x, y;
  bool quiver_empty = TRUE;
  
  cptr paren = ")";
  
  int fd = -1;
  
  FILE *fff = NULL;
  
  int k, which = 0;  

  store_type *st_ptr;
  
  char o_name[120];
  
  char buf[1024];
  char buf1[10];
  char buf2[10];

  bool red;

  int show_m_tohit = p_ptr->dis_to_h;
  int show_a_tohit = p_ptr->dis_to_h;
  int show_m_todam = p_ptr->dis_to_d;
  int show_a_todam = p_ptr->dis_to_d;
  
  object_type *o_ptr;
  int value;

  int tmp1;
  int xthn, xthb, xfos, xsrh;
  int xdis, xdev, xsav, xstl;
  cptr desc;

  int n;
  
  int set1, set2;
  u32b head1, head2;
  u32b flag1, flag2;
  cptr name1, name2;
  
  u32b f[4];

  /* Drop priv's */
  safe_setuid_drop();
  
  /* Build the filename */
  path_build(buf, 1024, ANGBAND_DIR_USER, name);
  
  /* File type is "TEXT" */
  FILE_TYPE(FILE_TYPE_TEXT);
  
  /* Check for existing file */
  fd = fd_open(buf, O_RDONLY);
  
  /* Existing file */
#ifdef _WIN32_WCE
  if (fd != -1)
#else
  if (fd >= 0)
#endif
    {
      char out_val[160];
      
      /* Close the file */
      fd_close(fd);
      
      /* Build query */
      sprintf(out_val, "Replace existing file %s? ", buf);
      
      /* Ask */
      if (get_check(out_val)) fd = -1;
    }
  
  /* Open the non-existing file */
  if (fd < 0) fff = my_fopen(buf, "w");
  
  /* Grab priv's */
  safe_setuid_grab();
  
  
  /* Invalid file */
  if (!fff)
    {
      /* Message */
      msg_format("Character dump failed!");
      msg_print(NULL);
      
      /* Error */
      return (-1);
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
  
  /* Begin dump */
  fprintf(fff, "  [FAangband %d.%d.%d Character Dump]\n\n",
	  VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
  
  

  /* Name, Sex, Race, Class */
  red = (p_ptr->stat_cur[0] < p_ptr->stat_max[0]);
  value = p_ptr->stat_use[0];
  cnv_stat(value, buf1);
  value = p_ptr->stat_top[0];
  cnv_stat(value, buf2);
  fprintf(fff, " Name    : %-14s  Age          %10d%6s%s%10s%10s\n", 
	  op_ptr->full_name, (int)p_ptr->age, (red ? "Str" : "STR"), 
	  ((p_ptr->stat_cur[0] == 18 + 100) ? "!" : " "), buf1, 
	  (red ? buf2 : " "));
  red = (p_ptr->stat_cur[1] < p_ptr->stat_max[1]);
  value = p_ptr->stat_use[1];
  cnv_stat(value, buf1);
  value = p_ptr->stat_top[1];
  cnv_stat(value, buf2);
  fprintf(fff, " Sex     : %-14s  Height       %10d%6s%s%10s%10s\n", 
	  sp_ptr->title,
	  (use_metric ? ((int)p_ptr->ht) * 254 / 100 : (int)p_ptr->ht), 
	  (red ? "Int" : "INT"), 
	  ((p_ptr->stat_cur[1] == 18 + 100) ? "!" : " "), buf1, 
	  (red ? buf2 : " "));
  red = (p_ptr->stat_cur[2] < p_ptr->stat_max[2]);
  value = p_ptr->stat_use[2];
  cnv_stat(value, buf1);
  value = p_ptr->stat_top[2];
  cnv_stat(value, buf2);
  fprintf(fff, " Race    : %-14s  Weight       %10d%6s%s%10s%10s\n", 
	  rp_name + rp_ptr->name,
	  (use_metric ? ((int)p_ptr->wt) * 10 / 22 : (int)p_ptr->wt), 
	  (red ? "Wis" : "WIS"), 
	  ((p_ptr->stat_cur[2] == 18 + 100) ? "!" : " "), buf1, 
	  (red ? buf2 : " "));
  red = (p_ptr->stat_cur[3] < p_ptr->stat_max[3]);
  value = p_ptr->stat_use[3];
  cnv_stat(value, buf1);
  value = p_ptr->stat_top[3];
  cnv_stat(value, buf2);
  fprintf(fff, " Class   : %-14s  Social Class %10d%6s%s%10s%10s\n", 
	  cp_name + cp_ptr->name,
	  (int)p_ptr->sc, (red ? "Dex" : "DEX"), 
	  ((p_ptr->stat_cur[3] == 18 + 100) ? "!" : " "), buf1, 
	  (red ? buf2 : " "));
  red = (p_ptr->stat_cur[4] < p_ptr->stat_max[4]);
  value = p_ptr->stat_use[4];
  cnv_stat(value, buf1);
  value = p_ptr->stat_top[4];
  cnv_stat(value, buf2);
  fprintf(fff, "%s%34s%10s%s%10s%10s\n",
	  (p_ptr->total_winner ? "***WINNER***" : "            "), " ",
	  (red ? "Con" : "CON"), 
	  ((p_ptr->stat_cur[4] == 18 + 100) ? "!" : " "), buf1, 
	  (red ? buf2 : " "));
  red = (p_ptr->stat_cur[5] < p_ptr->stat_max[5]);
  value = p_ptr->stat_use[5];
  cnv_stat(value, buf1);
  value = p_ptr->stat_top[5];
  cnv_stat(value, buf2);
  fprintf(fff, "%46s%10s%s%10s%10s\n", " ", (red ? "Chr" : "CHR"), 
	  ((p_ptr->stat_cur[5] == 18 + 100) ? "!" : " "), buf1, 
	  (red ? buf2 : " "));

  fprintf(fff, "\n");
	  
  /* Get the bonuses to hit/dam */
  
  o_ptr = &inventory[INVEN_WIELD];
  if (object_known_p(o_ptr)) show_m_tohit += o_ptr->to_h;
  if (object_known_p(o_ptr)) show_m_todam += o_ptr->to_d;

  o_ptr = &inventory[INVEN_BOW];
  if (object_known_p(o_ptr)) show_a_tohit += o_ptr->to_h;
  if (object_known_p(o_ptr)) show_a_todam += o_ptr->to_d;
  
  fprintf(fff, " Max Hit Points%9d   Level%18d   Max SP (Mana)%10d\n", 
	  p_ptr->mhp, (int)p_ptr->lev, p_ptr->msp);
  
  fprintf(fff," Cur Hit Points%9d   Experience%13ld   Cur SP (Mana)%10d\n", 
	  p_ptr->chp, p_ptr->exp, p_ptr->csp);
  
  fprintf(fff, "                           Max Exp%16ld\n", p_ptr->max_exp);

  if (p_ptr->lev >= PY_MAX_LEVEL)
    fprintf(fff, "        (Fighting)         Exp to Adv.          *****          (Shooting)    \n");
  else
    fprintf(fff, "        (Fighting)         Exp to Adv.%12ld          (Shooting)    \n", (s32b)(player_exp[p_ptr->lev - 1]));
  
  fprintf(fff, " Blows/Round%12d   Gold%19d   Shots/Round%10d.%1d\n", 
	  p_ptr->num_blow, (int) p_ptr->au, (int) p_ptr->num_fire/10, 
	  p_ptr->num_fire%10);
	  
  fprintf(fff, " + to Skill%13d%29s+ to Skill%13d\n", 
	  show_m_tohit, " ", show_a_tohit);
  
  if (show_m_todam >= 0)
    fprintf(fff, " Deadliness (%%)%9d", deadliness_conversion[show_m_todam]);
  else
    fprintf(fff," Deadliness (%%)%9d", -deadliness_conversion[-show_m_todam]);
  fprintf(fff, "   Base AC/+ To AC %3d/%3d", p_ptr->dis_ac, p_ptr->dis_to_a);
  
  if (show_a_todam > 0)
    fprintf(fff, "   Deadliness (%%)%9d", deadliness_conversion[show_a_todam]);
  else
	fprintf(fff, "   Deadliness (%%)%9d", -deadliness_conversion[-show_a_todam]);
    
  fprintf(fff, "\n\n                            (Character Abilities)\n");
	      
  /* Fighting Skill (with current weapon) */
  o_ptr = &inventory[INVEN_WIELD];
  tmp1 = p_ptr->to_h + o_ptr->to_h;
  xthn = p_ptr->skill_thn + (tmp1 * BTH_PLUS_ADJ);
  
  /* Shooting Skill (with current bow and normal missile) */
  o_ptr = &inventory[INVEN_BOW];
  tmp1 = p_ptr->to_h + o_ptr->to_h;
  xthb = p_ptr->skill_thb + (tmp1 * BTH_PLUS_ADJ);
  
  /* Basic abilities */
  xdis = p_ptr->skill_dis;
  xdev = p_ptr->skill_dev;
  xsav = p_ptr->skill_sav;
  xstl = p_ptr->skill_stl;
  xsrh = p_ptr->skill_srh;
  xfos = p_ptr->skill_fos;
  
  
  desc = likert(xthn, 10);
  fprintf(fff, " Fighting    : %-9s   ", desc);
  
  desc = likert(xstl, 1);
  fprintf(fff, "Stealth     : %-9s   ", desc);
  
  desc = likert(xdis, 8);
  fprintf(fff, "Disarming   : %-9s\n", desc);
  
  desc = likert(xthb, 10);
  fprintf(fff, " Bows/Throw  : %-9s   ", desc);
  
  desc = likert(xfos, 6);
  fprintf(fff, "Perception  : %-9s   ", desc);
  
  desc = likert(xdev, 8);
  fprintf(fff, "Magic Device: %-9s\n", desc);
  
  desc = likert(xsav, 7);
  fprintf(fff, " Saving Throw: %-9s   ", desc);
   
  desc = likert(xsrh, 6);
  fprintf(fff, "Searching   : %-9s   ", desc);
  
  if (use_metric)
    fprintf(fff, "Infra-Vision: %d meters\n\n", p_ptr->see_infra * 3);
  else  
    fprintf(fff, "Infra-Vision: %d feet\n\n", p_ptr->see_infra * 10);

  
  /* Display history */
  fprintf(fff, "                            (Character Background)\n");

  for (i = 0; i < 4; i++)
    {
      fprintf(fff, "          %s\n", p_ptr->history[i]);
    }
  
  /* Skip some lines */
  fprintf(fff, "\n\n");
  
  /* Current, recent and recall points */
  fprintf(fff, "  [Recent locations]\n\n");
  
  /* Current, previous */
  fprintf(fff,"Current Location : %s Level %d\n",
	  locality_name[stage_map[p_ptr->stage][LOCALITY]], p_ptr->depth);
  if (p_ptr->last_stage != 0)
    fprintf(fff,"Previous Location: %s Level %d\n",
	    locality_name[stage_map[p_ptr->last_stage][LOCALITY]], 
	    stage_map[p_ptr->last_stage][DEPTH]);
  
  /* Recall points */
  for (i = 0; i < 4; i++)
    {
      if (p_ptr->recall[i] == 0) continue;
      fprintf(fff,"Recall Point %d   : %s Level %d\n", i + 1,
	      locality_name[stage_map[p_ptr->recall[i]][LOCALITY]], 
	      stage_map[p_ptr->recall[i]][DEPTH]);
    }
  
  /* Skip some lines */
  fprintf(fff, "\n\n");
  
  /* Heading */
  fprintf(fff, "  [Resistances and Powers]\n\n");
  
  /* Two tables */
  for (x = 0; x < 2; x++)
    {
      /* Header */
      fprintf(fff, "      abcdefghijkl@       abcdefghijkl@\n");
      
      /* Extract set */
      set1 = display_player_flag_set[2 * x];
      set2 = display_player_flag_set[2 * x + 1];
      
      /* Extract head */
      head1 = display_player_flag_head[2 * x];
      head2 = display_player_flag_head[2 * x + 1];
      
      /* Eight rows */
      for (y = 0; y < 8; y++)
	{
	  /* Extract flag */
	  flag1 = (head1 << y);
	  flag2 = (head2 << y);
	  
	  /* Extract name */
	  name1 = display_player_flag_names[2 * x][y];
	  name2 = display_player_flag_names[2 * x + 1][y];
	  
	  /* Name */
	  fprintf(fff, "%5s", name1);
	  
	  /* Check equipment */
	  for (n = 6, i = INVEN_WIELD; i < INVEN_SUBTOTAL; i++, n++)
	    {
	      object_type *o_ptr;
	      
	      /* Object */
	      o_ptr = &inventory[i];
	      
	      /* Known flags */
	      object_flags_known(o_ptr, &f[1], &f[2], &f[3]);
	      
	      /* Hack -- Check immunities */
	      if ((x == 0) && (y < 4) && (f[set1] & ((TR2_IM_ACID) << y)))
		{
		  fprintf(fff, "*");
		}
	      
	      /* Check flags */
	      else if (f[set1] & flag1)
		{
		  fprintf(fff, "+");
		}
	      
	      /* Default */
	      else
		{
		  fprintf(fff, ".");
		}
	    }
	  
	  /* Player flags */
	  player_flags(&f[1], &f[2], &f[3], TRUE);
	  
	  /* Check flags */
	  if (f[set1] & flag1) 
	    {
	      fprintf(fff, "+");
	    }
	  
	  else
	    {
	      /* Player 'reversed' flags */
	      player_weakness_dis(&f[1], &f[2], &f[3]);
	  
	      /* Check flags */
	      if (f[set1] & flag1) 
		{
		  fprintf(fff, "X");
		}
	  
	      else
		/* Default */
		fprintf(fff, ".");
	    }
	  
	  /* Name */
	  fprintf(fff, "%7s", name2);
	  
	  /* Check equipment */
	  for (n = 6, i = INVEN_WIELD; i < INVEN_SUBTOTAL; i++, n++)
	    {
	      object_type *o_ptr;
	      
	      /* Object */
	      o_ptr = &inventory[i];
	      
	      /* Known flags */
	      object_flags_known(o_ptr, &f[1], &f[2], &f[3]);
	      
	      /* Hack -- "Might" covers both might1 and might2. */
	      if ((y == 6) && (x == 1))
		{
		  if (f[set2] & TR1_MIGHT2) 
		    fprintf(fff, "*");
		  else if (f[set2] & TR1_MIGHT1) 
		    fprintf(fff, "+");
		  else fprintf(fff, ".");
		  continue;
		}
	      
	      /* Check flags */
	      if (f[set2] & flag2)
		{
		  fprintf(fff, "+");
		}
	      
	      /* Default */
	      else
		{
		  fprintf(fff, ".");
		}
	    }
	  
	  /* Player flags */
	  player_flags(&f[1], &f[2], &f[3], TRUE);
	  
	  /* Check flags */
	  if (f[set2] & flag2) 
	    {
	      fprintf(fff, "+\n");
	    }
	  
	  else
	    {
	      /* Player 'reversed' flags */
	      player_weakness_dis(&f[1], &f[2], &f[3]);
	  
	      /* Check flags */
	      if (f[set2] & flag2) 
		{
		  fprintf(fff, "X\n");
		}

	      else
	  
		/* Default */
		fprintf(fff, ".\n");
	    }
	}
      fprintf(fff, "\n\n");
    }
  
  /* Skip some lines */
  fprintf(fff, "\n\n");
  
  /* Dump specialties if any */
  if (p_ptr->specialty_order[0] != SP_NO_SPECIALTY)
    {
      fprintf(fff, "  [Specialty Abilities]\n\n");
      for (i = 0; i < 10; i++)
	{
	  if (p_ptr->specialty_order[i] != SP_NO_SPECIALTY)
	    {
	      fprintf(fff, "%s %s\n", 
		      specialty_names[p_ptr->specialty_order[i]],
		      (i >= p_ptr->specialties_allowed) ? "(forgotten)" : ""); 
	    }
	}
      fprintf(fff, "\n\n");
    }
  
  /* If dead, dump last messages -- Prfnoff */
  if (p_ptr->is_dead)
    {
      i = message_num();
      if (i > 15) i = 15;
      fprintf(fff, "  [Last Messages]\n\n");
      while (i-- > 0)
	{
	  fprintf(fff, "> %s\n", message_str((s16b)i));
	}
      fprintf(fff, "\n\n");
    }
  
  /* Dump the equipment */
  if (p_ptr->equip_cnt)
    {
      fprintf(fff, "  [Character Equipment]\n\n");
      for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
	{
	  if (i == INVEN_BLANK) 
	    {
	      for (j = 0; j < 10; j++)
		{
		  object_desc(o_name, &inventory[i + j + 1], TRUE, 4);
		  if (!streq(o_name, "(nothing)")) 
		    quiver_empty = FALSE;
		}
	      if (!quiver_empty) fprintf(fff, "\n         [Quiver]\n");
	    }	      
	  
	  else
	    {
	      object_desc(o_name, &inventory[i], TRUE, 4);
	      if (streq(o_name,"(nothing)")) continue;
	      fprintf(fff, "%c%s %s\n", index_to_label(i), paren, o_name);
	    }
	}
      fprintf(fff, "\n\n");
    }
  
  /* Dump the inventory */
  fprintf(fff, "  [Character Inventory]\n\n");
  for (i = 0; i < INVEN_PACK; i++)
    {
      if (!inventory[i].k_idx) break;
      
      object_desc(o_name, &inventory[i], TRUE, 4);
      fprintf(fff, "%c%s %s\n",
	      index_to_label(i), paren, o_name);
    }
  fprintf(fff, "\n\n");
  
  
  /* Dump the Home -- if anything there */
  if (st_ptr->stock_num)
    {
      /* Header */
      fprintf(fff, "  [Home Inventory]\n\n");
      
      /* Dump all available items */
      for (i = 0; i < st_ptr->stock_num; i++)
	{
	  object_desc(o_name, &st_ptr->stock[i], TRUE, 4);
	  fprintf(fff, "%c) %s\n", I2A(i), o_name);
	  
	}
      
      /* Add an empty line */
      fprintf(fff, "\n\n");
    }
  
  
  /* Dump notes */
  if (adult_take_notes)
    {
      int holder;
      
      /*close the notes file for writing*/
      my_fclose(notes_file);
      
      /*get the path for the notes file*/
      notes_file = my_fopen(notes_fname, "r");
      
      do
	{
	  
	  /*get a character from the notes file*/
	  holder = getc(notes_file);
	  
	  /*output it to the character dump, unless end of file char*/
	  if (holder != EOF) fprintf(fff, "%c", holder);
	  
	}
      while (holder != EOF);
      
      /*aesthetics*/
      fprintf(fff, "============================================================\n");

      fprintf(fff, "\n\n");
      
      /*close it for reading*/
      my_fclose(notes_file);
      
      /*re-open for appending*/
      notes_file = my_fopen(notes_fname, "a");

    }
  
  /* Dump options */
  fprintf(fff, "  [Birth and Cheat Options]\n\n");

  /* Dump options */
  for (i = OPT_adult_start; i < OPT_score_end; i++)
    {
      if (option_desc[i])
	{
	  fprintf(fff, "%-49s: %s (%s)\n",
		  option_desc[i],
		  op_ptr->opt[i] ? "yes" : "no ",
		  option_text[i]);
	}
    }
  
  /* Close it */
  my_fclose(fff);
  
  /* Message */
  msg_print("Character dump successful.");
  msg_print(NULL);
  
  /* Success */
  return (0);
}

/*
 * Make a string lower case.
 */
static void string_lower(char *buf)
{
  cptr buf_ptr;
  
  /* No string */
  if (!buf) return;
  
  /* Lower the string */
  for (buf_ptr = buf; *buf_ptr != 0; buf_ptr++)
    {
      buf[buf_ptr - buf] = tolower(*buf_ptr);
    }
}

/*
 * Recursive file perusal.
 *
 * Return FALSE on "ESCAPE", otherwise TRUE.
 *
 * Process various special text in the input file, including
 * the "menu" structures used by the "help file" system.
 *
 * XXX XXX XXX Consider using a temporary file.
 *
 * XXX XXX XXX Allow the user to "save" the current file.
 */
bool show_file(cptr name, cptr what, int line, int mode)
{
  int i, k;
  int wid, hgt;
  key_event ke;
  
  /* Number of "real" lines passed by */
  int next = 0;
  
  /* Number of "real" lines in the file */
  int size = 0;
  
  /* Backup value for "line" */
  int back = 0;
  
  /* This screen has sub-screens */
  bool menu = FALSE;

  /* Case sensitive search */
  bool case_sensitive = FALSE;
  
  /* Current help file */
  FILE *fff = NULL;
  
  /* Find this string (if any) */
  cptr find = NULL;
  
  /* Hold a string to find */
  char finder[81];
  
  /* Hold a string to show */
  char shower[81];
  
  /* Describe this thing */
  char caption[128];

  /* Path buffer */
  char path[1024];
  
  /* General buffer */
  char buf[1024];
  
  /* Small screen back up buffer */
  char buf2[1024];
  
  /* Lower case version of the buffer, for searching */
  char lc_buf[1024];
  
  /* Sub-menu information */
  char hook[10][32];

  /* Sub-menu mouse position */
  int mouse[24];
  
  /* Handle second half of screen */
  bool second_half = FALSE;
  bool line_finished = FALSE;
 
  /* Get size */
  Term_get_size(&wid, &hgt);
  
  /* Wipe finder */
  strcpy(finder, "");

  /* Wipe shower */
  strcpy(shower, "");
  
  /* Wipe caption */
  strcpy(caption, "");
  
  /* Wipe the hooks */
  for (i = 0; i < 10; i++) 
    {
      hook[i][0] = '\0';
    }

  /* Wipe the mouse menu */
  for (i = 0; i < 24; i++)
    {
      mouse[i] = 0;
    }
  
  /* Hack XXX XXX XXX */
  if (what)
    {
      /* Caption */
      strcpy(caption, what);
      
      /* Access the "file" */
      strcpy(path, name);
      
      /* Open */
      fff = my_fopen(path, "r");
    }
  
  /* Look in "help" */
  if (!fff)
    {
      /* Caption */
      sprintf(caption, "Help file '%s'", name);
      
      /* Build the filename */
      path_build(path, 1024, ANGBAND_DIR_HELP, name);
      
      /* Open the file */
      fff = my_fopen(path, "r");
    }

  /* Look in "info" */
  if (!fff)
    {
      /* Caption */
      sprintf(caption, "Info file '%s'", name);
      
      /* Build the filename */
      path_build(path, 1024, ANGBAND_DIR_INFO, name);
      
      /* Open the file */
      fff = my_fopen(path, "r");
    }
  
  /* Oops */
  if (!fff)
    {
      /* Message */
      msg_format("Cannot open '%s'.", name);
      msg_print(NULL);
      
      /* Oops */
      return (TRUE);
    }
  
  
  /* Pre-Parse the file */
  while (TRUE)
    {
      /* Read a line or stop */
      if (my_fgets(fff, buf, 1024)) break;

      /* Check for a mouseable line (note hex parentheses) */
      if ((buf[4] == 0x28) && (isdigit(buf[5])) && (buf[6] == 0x29))
	{
	  mouse[next + 2] = D2I(buf[5]);
	}
      
      /* XXX Parse "menu" items */
      if (prefix(buf, "***** "))
	{
	  char b1 = '[', b2 = ']';
	  
	  /* Notice "menu" requests */
	  if ((buf[6] == b1) && isdigit(buf[7]) &&
	      (buf[8] == b2) && (buf[9] == ' '))
	    {
	      /* This is a menu file */
	      menu = TRUE;
	      
	      /* Extract the menu item */
	      k = D2I(buf[7]);
	      
	      /* Extract the menu item */
	      strcpy(hook[k], buf + 10);
	    }
	  
	  /* Skip this */
	  continue;
	}
      
      /* Count the "real" lines */
      next++;
    }
  
  /* Save the number of "real" lines */
  size = next;
  
  
  
  /* Display the file */
  while (TRUE)
    {
      /* Mouse variables */
      int qstart = 80;
      int estart = 80;
      int hstart = 80;
      int sstart = 80;

      /* Clear screen */
      Term_clear();
      
      
      /* Restart when necessary */
      if (line >= size) line = 0;
      
      
      /* Re-open the file if needed */
      if (next > line)
	{
	  /* Close it */
	  my_fclose(fff);
	  
	  /* Hack -- Re-Open the file */
	  fff = my_fopen(path, "r");
	  
	  /* Oops */
	  if (!fff) return (FALSE);
	  
	  /* File has been restarted */
	  next = 0;
	}
      
      /* Skip lines if needed */
      for (; next < line; next++)
	{
	  /* Skip a line */
	  if (my_fgets(fff, buf, 1024)) break;
	}
      
      
      /* Dump the next hgt - 4 lines of the file */
      for (i = 0; i < hgt - 4; )
	{
	  /* Hack -- track the "first" line */
	  if (!i) line = next;
	  
	  /* Get a line of the file or stop */
	  if (my_fgets(fff, buf, 1024)) break;
	  
	  /* Hack -- skip "special" lines */
	  if (prefix(buf, "***** ")) continue;
	  
	  /* Count the "real" lines */
	  next++;
	  
	  /* Make a copy of the current line for searching */
	  strcpy(lc_buf, buf);
	  
	  /* Make the line lower case */
	  if (!case_sensitive)
	    string_lower(lc_buf);
	  
	  /* Hack -- keep searching */
	  if (find && !i && !strstr(lc_buf, find)) continue;
	  
	  /* Hack -- stop searching */
	  find = NULL;
	  
	  /* Check if the line is finished */
	  for (k = 0; k < 32; k++)
	    if (!buf[k]) line_finished = TRUE;

	  /* Dump the line */
	  if ((small_screen) && (second_half))
	    {
	      if (!line_finished)
		{
		  for (k = 0; k < strlen(buf); k++)
		    {
		      buf2[k] = buf[k + 32];
		    }
		  Term_putstr(0, i+2, -1, TERM_WHITE, buf2);
		}
	    }
	  else
	    Term_putstr(0, i+2, -1, TERM_WHITE, buf);
	  
	  /* Reset line */
	  line_finished = FALSE;

	  /* Hilite "shower" */
	  if (shower[0])
	    {
	      cptr str = lc_buf;
	      
	      /* Display matches */
	      while ((str = strstr(str, shower)) != NULL)
		{
		  int len = strlen(shower);
		  
		  /* Display the match */
		  Term_putstr(str-lc_buf, i+2, len, TERM_YELLOW, 
			      &buf[str-lc_buf]);
		  
		  /* Advance */
		  str += len;
		}
	    }
	  
	  /* Count the printed lines */
	  i++;
	}
      
      /* Hack -- failed search */
      if (find)
	{
	  bell("Search string not found!");
	  line = back;
	  find = NULL;
	  continue;
	}
      
      
      /* Show a general "title" */
      prt(format("[FAangband %d.%d.%d, %s, Line %d/%d]",
		 VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH,
		 caption, line, size), 0, 0);
      
      
      /* Prompt -- menu screen */
      if (menu)
	{
	  /* Wait for it */
	  if (small_screen)
	    {
	      prt("[Number, 'h':other half, ESC:previous, ?:exit]", 
		  hgt - 1, 0);
	      hstart = 7;
	      estart = 23;
	      qstart = 37;
	    }
	  else
	    {
	      prt("[Press a number, ESC for previous file or ? to exit]", 
		  hgt - 1, 0);
	      estart = 15;
	      qstart = 38;
	    }
	}
      
      /* Prompt -- small files */
      else if (size <= hgt - 4)
	{
	  /* Wait for it */
	  if (small_screen)
	    {
	      prt("['h' other half, ESC previous file, ? exit]", 
		  hgt - 1, 0);
	      hstart = 0;
	      estart = 15;
	      qstart = 34;
	    }
	  else
	    {
	      prt("[Press ESC for previous file, ? to exit.]", hgt - 1, 0);
	      estart = 6;
	      qstart = 28;
	    }
	}
      
      /* Prompt -- large files */
      else
	{
	  /* Wait for it */
	  if (small_screen)
	    {
	      prt("['h':other half,Space:advance,ESC:last,?:exit]", 
		  hgt - 1, 0);
	      hstart = 0;
	      sstart = 15;
	      estart = 29;
	      qstart = 38;
	    }
	  else
	    {
	      prt("[Press Space to advance, ESC to return to the previous file, or ? to exit.]", hgt - 1, 0);
	      sstart = 6;
	      estart = 23;
	      qstart = 59;
	    }
	}
      
      /* Get a keypress */
      ke = inkey_ex();

      /* Mouse input - control line */
      if ((ke.key == '\xff') && (ke.mousey == hgt - 1))
	{
	  if (ke.mousex > hstart) 
	    ke.key = 'h';
	  if (ke.mousex > sstart) 
	    ke.key = ' ';
	  if (ke.mousex > estart)
	    ke.key = ESCAPE;
	  if (ke.mousex > qstart)
	    ke.key = '?';
	}

      /* Mouse input - menus */
      if ((ke.key == '\xff') && (menu) && (mouse[ke.mousey]))
	{
	  /* Recurse on that file */
	  if (!show_file(hook[mouse[ke.mousey]], NULL, 0, mode)) ke.key = '?';
	}
      
      /* Hack -- return to last screen on escape */
      if (ke.key == ESCAPE) break;
      
      
      /* Toggle case sensitive on/off */
      if (ke.key == '!')
	{
	  case_sensitive = !case_sensitive;
	}
      
      /* Hack -- try showing */
      if (ke.key == '=')
	{
	  /* Get "shower" */
	  prt("Show: ", hgt - 1, 0);
	  (void)askfor_aux(shower, 80);
	  
	  /* Make the "shower" lowercase */
	  if (!case_sensitive)
	    string_lower(shower);
	}
      
      /* Hack -- try finding */
      if (ke.key == '/')
	{
	  /* Get "finder" */
	  prt("Find: ", hgt - 1, 0);
	  if (askfor_aux(finder, 80))
	    {
	      /* Find it */
	      find = finder;
	      back = line;
	      line = line + 1;
	      
	      /* Make the "finder" lowercase */
	      if (!case_sensitive)
		string_lower(finder);
	      
	      /* Show it */
	      strcpy(shower, finder);
	    }
	}
      
      /* Hack -- go to a specific line */
      if (ke.key == '#')
	{
	  char tmp[81];
	  prt("Goto Line: ", hgt - 1, 0);
	  strcpy(tmp, "0");
	  if (askfor_aux(tmp, 80))
	    {
	      line = atoi(tmp);
	    }
	}
      
      /* Hack -- go to a specific file */
      if (ke.key == '%')
	{
	  char tmp[81];
	  prt("Goto File: ", hgt - 1, 0);
	  strcpy(tmp, "help.hlp");
	  if (askfor_aux(tmp, 80))
	    {
	      if (!show_file(tmp, NULL, 0, mode)) ke.key = '?';
	    }
	}
      
      /* Hack -- Allow backing up */
      if (ke.key == '-')
	{
	  line = line - 10;
	  if (line < 0) line = 0;
	}
      
      /* Hack -- Advance a single line */
      if ((ke.key == '\n') || (ke.key == '\r'))
	{
	  line = line + 1;
	}
      
      /* Switch to other page half */
      if ((small_screen) && (ke.key == 'h'))
	{
	  second_half = !second_half;
	}
      
      /* Advance one page */
      if (ke.key == ' ')
	{
	  line = line + hgt - 4;
	}
      
      /* Recurse on numbers */
      if (menu && isdigit(ke.key) && hook[D2I(ke.key)][0])
	{
	  /* Recurse on that file */
	  if (!show_file(hook[D2I(ke.key)], NULL, 0, mode)) ke.key = '?';
	}
      
      /* Exit on '?' */
      if (ke.key == '?') break;
    }
  
  /* Close the file */
  my_fclose(fff);
  
  /* Exit on '?' */
  if (ke.key == '?') return (FALSE);
  
  /* Normal return */
  return (TRUE);
}


/*
 * Peruse the On-Line-Help
 */
void do_cmd_help(void)
{
  /* Save screen */
  screen_save();
  
  /* Peruse the main help file */
  (void)show_file("help.hlp", NULL, 0, 0);
  
  /* Load screen */
  screen_load();
}


/*
 * Process the player name.
 * Extract a clean "base name".
 * Build the savefile name if needed.
 */
void process_player_name(bool sf)
{
  int i, k = 0;
  
  
  /* Cannot be too long */
  if (strlen(op_ptr->full_name) > 15)
    {
      /* Name too long */
      quit_fmt("The name '%s' is too long!", op_ptr->full_name);
    }
  
  /* Cannot contain "icky" characters */
  for (i = 0; op_ptr->full_name[i]; i++)
    {
      /* No control characters */
      if (iscntrl(op_ptr->full_name[i]))
	{
	  /* Illegal characters */
	  quit_fmt("The name '%s' contains control chars!", op_ptr->full_name);
	}
    }
  
  
#ifdef MACINTOSH
  
  /* Extract "useful" letters */
  for (i = 0; op_ptr->full_name[i]; i++)
    {
      char c = op_ptr->full_name[i];
      
      /* Convert "colon" and "period" */
      if ((c == ':') || (c == '.')) c = '_';
      
      /* Accept all the letters */
      op_ptr->base_name[k++] = c;
    }
  
#else
  
  /* Extract "useful" letters */
  for (i = 0; op_ptr->full_name[i]; i++)
    {
      char c = op_ptr->full_name[i];
      
      /* Accept some letters */
      if (isalpha(c) || isdigit(c)) op_ptr->base_name[k++] = c;
      
      /* Convert space, dot, and underscore to underscore */
      else if (strchr(". _", c)) op_ptr->base_name[k++] = '_';
    }
  
#endif
  
  
#if defined(WINDOWS) || defined(MSDOS)
  
  /* Hack -- max length */
  if (k > 8) k = 8;
  
#endif
  
  /* Terminate */
  op_ptr->base_name[k] = '\0';
  
  /* Require a "base" name */
  if (!op_ptr->base_name[0]) strcpy(op_ptr->base_name, "PLAYER");
  
  
#ifdef SAVEFILE_MUTABLE
  
  /* Accept */
  sf = TRUE;
  
#endif
  
  /* Change the savefile name */
  if (sf)
    {
      char temp[128];
      
#ifdef SAVEFILE_USE_UID
      /* Rename the savefile, using the player_uid and base_name */
      sprintf(temp, "%d.%s", player_uid, op_ptr->base_name);
#else
      /* Rename the savefile, using the base name */
      sprintf(temp, "%s", op_ptr->base_name);
#endif
      
#ifdef VM
      /* Hack -- support "flat directory" usage on VM/ESA */
      sprintf(temp, "%s.sv", op_ptr->base_name);
#endif /* VM */
      
      /* Build the filename */
#ifdef _WIN32_WCE
      /* SJG */
      /* Rename the savefile, using the base name + .faa */
      sprintf(temp, "%s.faa", op_ptr->base_name);
      
      // The common open file dialog doesn't like
      // anything being farther up than one directory!
      // For now hard code it. I should probably roll my
      // own open file dailog.
      path_build(savefile, 1024, "\\My Documents\\FA", temp);
#else
      path_build(savefile, 1024, ANGBAND_DIR_SAVE, temp);
#endif
    }
}


/*
 * Gets a name for the character, reacting to name changes.
 *
 * Assumes that "display_player(0)" has just been called
 *
 * Perhaps we should NOT ask for a name (at "birth()") on
 * Unix machines?  XXX XXX
 *
 * What a horrible name for a global function.  XXX XXX XXX
 */
void get_name(void)
{
  char tmp[32];

  int row;

  /* Set the row */
  row = (small_screen ? 0 : 2);
  
  /* Clear last line */
  clear_from(22);
  
  /* Prompt and ask */
  prt("[Enter your player's name above, or hit ESCAPE]", 23, 2);
  
  /* Ask until happy */
  while (1)
    {
      /* Go to the "name" field */
      move_cursor(row, 11);
      
      /* Save the player name */
      strcpy(tmp, op_ptr->full_name);
      
      /* Get an input, ignore "Escape" */
      if (askfor_aux(tmp, 15)) strcpy(op_ptr->full_name, tmp);
      
      /* Process the player name */
      process_player_name(FALSE);
      
      /* All done */
      break;
    }

  /* Pad the name (to clear junk) */
  sprintf(tmp, "%-15.15s", op_ptr->full_name);
  
  /* Re-Draw the name (in light blue) */
  c_put_str(TERM_L_BLUE, tmp, row, 11);
  
  /* Erase the prompt, etc */
  clear_from(22);
}



/*
 * Hack -- commit suicide
 */
void do_cmd_suicide(void)
{
  int i;
  
  /* Flush input */
  flush();
  
  /* Verify Retirement */
  if (p_ptr->total_winner)
    {
      /* Verify */
      if (!get_check("Do you want to retire? ")) return;
    }
  
  /* Verify Suicide */
  else
    {
      /* Verify */
      if (!get_check("Do you really want to suicide? ")) return;
      
      /* Special Verification for suicide */
      prt("Please verify SUICIDE by typing the '@' sign: ", 0, 0);
      flush();
      i = inkey();
      prt("", 0, 0);
      if (i != '@') return;
    }
  
  /* Commit suicide */
  p_ptr->is_dead = TRUE;
  
  /* Stop playing */
  p_ptr->playing = FALSE;
  
  /* Leaving */
  p_ptr->leaving = TRUE;
  
  /* Cause of death */
  strcpy(p_ptr->died_from, "Quitting");
}



/*
 * Save the game
 */
void do_cmd_save_game(bool autosave)
{
  /* Disturb the player, unless autosaving. */
  if (!autosave) disturb(1, 0);
  
  /* Clear messages */
  msg_print(NULL);
  
  /* Handle stuff */
  handle_stuff();
  
  /* Message */
  prt("Saving game...", 0, 0);
  
  /* Refresh */
  Term_fresh();
  
  /* The player is not dead */
  strcpy(p_ptr->died_from, "(saved)");
  
  /* Forbid suspend */
  signals_ignore_tstp();
  
  /* Save the player */
  if (save_player())
    {
      prt("Saving game... done.", 0, 0);
    }
  
  /* Save failed (oops) */
  else
    {
      prt("Saving game... failed!", 0, 0);
    }
  
  /* Allow suspend again */
  signals_handle_tstp();
  
  /* Refresh */
  Term_fresh();
  
  /* Clear messages */
  msg_print(NULL);
  
  /* Note that the player is not dead */
  strcpy(p_ptr->died_from, "(alive and well)");
}



/*
 * Hack -- Calculates the total number of points earned
 */
long total_points(void)
{
  long score = (p_ptr->max_exp + (100 * p_ptr->recall[0]));
  
  return (score);
}


/*
 * Gets a personalized string for ghosts.  Code originally from get_name. -LM-
 */
static char *get_personalized_string(byte choice)
{
  static char tmp[80], info[80];
  byte n, i;
  
  /* Clear last line */
  clear_from(15);
  
  /* Prompt and ask */
  if (choice == 1)
    { 
      prt("Enter a message for your character's ghost", 15, 0);
      prt("above, or hit ESCAPE.", 16, 0);
    }
  else if (choice == 2) 
    {
      prt("Enter an addition to your character ghost's", 15, 0);
      prt("description above, or hit ESCAPE.", 16, 0);
    }
  else return NULL;

  sprintf(info, "(%d characters maximum.  Entry will be used as", 
	  (small_screen ? 47 : 79));

  prt(info, 17, 0);
  prt("(a) sentence(s).)", 18, 0);
  
  /* Ask until happy */
  while (1)
    {
      /* Start at beginning of field. */
      move_cursor(14, 0);
      
      /* Get an input */
      (void)askfor_aux(tmp, (small_screen ? 47 : 79));
      
      /* All done */
      break;
    }
  
  /* Pad the string (to clear junk and allow room for a ending) */
  if (small_screen)
    sprintf(tmp, "%-47.47s", tmp);
  else
    sprintf(tmp, "%-79.79s", tmp);
  
  /* Ensure that strings end like a sentence, and neatly clip the string. */
  for (n = (small_screen ? 47 : 79); ; n--)
    {
      if ((tmp[n] == ' ') || (tmp[n] == '\0')) continue;
      else
	{
	  if ((tmp[n] == '!') || (tmp[n] == '.') || (tmp[n] == '?'))
	    {
	      tmp[n + 1] = '\0';
	      for (i = n + 2; i < (small_screen ? 48 : 80); i++) tmp[i] = '\0';
	      break;
	    }
	  else 
	    {
	      tmp[n + 1] = '.';
	      tmp[n + 2] = '\0';
	      for (i = n + 3; i < (small_screen ? 48 : 80); i++) tmp[i] = '\0';
	      break;
	    }
	}
    }
  
  /* Start the sentence with a capital letter. */
  if (islower(tmp[0])) tmp[0] = toupper(tmp[0]);
  
  /* Return the string */
  return tmp;
  
}

/*
 * Save a "bones" file for a dead character.  Now activated and (slightly) 
 * altered.  Allows the inclusion of personalized strings. 
 */
static void make_bones(void)
{
  FILE *fp;
  
  char str[1024];
  key_event answer;
  byte choice=0;
  
  int i;
  
  /* Ignore wizards and borgs */
  if (!(p_ptr->noscore & 0x00FF))
    {
      /* Ignore people who die in town */
      if (p_ptr->depth)
	{
	  int level;
	  char tmp[128];
	  
	  /* Slightly more tenacious saving routine. */
	  for (i = 0; i < 5; i++)
	    {
	      /* Ghost hovers near level of death. */
	      if (i == 0) level = p_ptr->depth;
	      else level = p_ptr->depth + 5 - damroll(2, 4);
	      if (level < 1) level = randint(4);
	      
	      /* XXX XXX XXX "Bones" name */
	      sprintf(tmp, "bone.%03d", level);
	      
	      /* Build the filename */
	      path_build(str, 1024, ANGBAND_DIR_BONE, tmp);
	      
	      /* Attempt to open the bones file */
	      fp = my_fopen(str, "r");
	      
	      /* Close it right away */
	      if (fp) my_fclose(fp);
	      
	      /* Do not over-write a previous ghost */
	      if (fp) continue;
	      
	      /* If no file by that name exists, we can make a new one. */
	      if (!(fp)) break;
	    }
	  
	  /* Failure */
	  if (fp) return;
	  
	  /* File type is "TEXT" */
	  FILE_TYPE(FILE_TYPE_TEXT);
	  
	  /* Try to write a new "Bones File" */
	  fp = my_fopen(str, "w");
	  
	  /* Not allowed to write it?  Weird. */
	  if (!fp) return;
	  
	  /* Save the info */
	  if (op_ptr->full_name[0] != '\0') 
	    fprintf(fp, "%s\n", op_ptr->full_name);
	  else fprintf(fp, "Anonymous\n");
	  
	  
	  
	  fprintf(fp, "%d\n", p_ptr->psex);
	  fprintf(fp, "%d\n", p_ptr->prace);
	  fprintf(fp, "%d\n", p_ptr->pclass);
	  
	  /* Clear screen */
	  Term_clear();
	  
	  while(1)
	    {
	      /* Ask the player if he wants to 
	       * add a personalized string. 
	       */
	      prt("Information about your character has been saved", 15, 0);
	      prt("in a bones file.  Would you like to give the", 16, 0);
	      prt("ghost a special message or description? (yes/no)", 17, 0);
	      
	      answer = inkey_ex();
	      if ((answer.key == '\xff') && (answer.mousey == 17) &&
		  (answer.mousex > 40) && (answer.mousex < 44))
		answer.key = 'y';
	      
	      /* Clear last line */
	      clear_from(15);
	      clear_from(16);
	      
	      /* Determine what the personalized string will be used for.  */
	      if ((answer.key == 'Y') || (answer.key == 'y'))
		{
		  prt("Will you add something for your ghost to say,", 15, 0);
		  prt("or add to the monster description?", 16, 0);
		  prt("((M)essage/(D)escription)", 17, 0);
		  
		  while(1)
		    {
		      answer = inkey_ex();
		      
		      if ((answer.key == '\xff') && (answer.mousey == 17))
			{
			  if ((answer.mousex > 0) && (answer.mousex < 10))
			    answer.key = 'M';
			  else if ((answer.mousex > 10) && 
				   (answer.mousex < 24))
			    answer.key = 'D';
			}

		      clear_from(15);
		      clear_from(16);
		      
		      if ((answer.key == 'M') || (answer.key == 'm'))
			{
			  choice = 1;
			  break;
			}
		      else if ((answer.key == 'D') || (answer.key == 'd'))
			{
			  choice = 2;
			  break;
			}
		      else
			{
			  choice = 0;
			  break;
			}
		    }
		}
	      else if ((answer.key == 'N') || (answer.key == 'n') || 
		       (answer.key == ESCAPE)) 
		{
		  choice = 0;
		  break;
		}
	      
	      
	      /* If requested, get the personalized string, and write it and 
	       * info on how it should be used in the bones file.  Otherwise, 
	       * indicate the absence of such a string.
	       */
	      if (choice) fprintf(fp, "%d:%s\n", 
				  choice, get_personalized_string(choice));
	      else fprintf(fp, "0: \n");
	      
	      /* Close and save the Bones file */
	      my_fclose(fp);
	      
	      return;
	    }
	}
    }
}


/*
 * Centers a string within a 31 character string
 */
static void center_string(char *buf, cptr str)
{
  int i, j;
  
  /* Total length */
  i = strlen(str);
  
  /* Necessary border */
  j = 15 - i / 2;
  
  /* Mega-Hack */
  sprintf(buf, "%*s%s%*s", j, "", str, 31 - i - j, "");
}

/*
 * Hack - save the time of death
 */
static time_t death_time = (time_t)0;


/*
 * Encode the screen colors for the closing screen
 */
static char hack[17] = "dwsorgbuDWvyRGBU";

/*
 * Display a "tomb-stone"
 */
static void print_tomb(void)
{
  cptr p;

  int offset = 12;
  
  char tmp[160];
  
  char buf[1024];
  
  FILE *fp;
  
#ifdef _WIN32_WCE
  time_t ct = fake_time((time_t)0);
#else
  time_t ct = time((time_t)0);
#endif

  bool boat = ((p_ptr->total_winner) && (check_ability(SP_ELVEN)));
  bool tree = ((p_ptr->total_winner) && 
	       (check_ability(SP_WOODEN) || check_ability(SP_DIVINE))); 
  
  /* Clear screen */
  Term_clear();
  
  /* Build the filename */
  if (tree)
   path_build(buf, 1024, ANGBAND_DIR_FILE, 
	      (small_screen ? "tree_s.txt" : "tree.txt"));
  else if (boat)
    path_build(buf, 1024, ANGBAND_DIR_FILE, 
	       (small_screen ? "boat_s.txt" : "boat.txt"));
  else
    path_build(buf, 1024, ANGBAND_DIR_FILE, 
	       (small_screen ? "dead_s.txt" : "dead.txt"));
	       
  /* Open the News file */
  fp = my_fopen(buf, "r");
  
  /* Dump */
  if (fp)
    {
      int i, y, x;
      
      byte a = 0;
      char c = ' ';
      
      bool okay = TRUE;
      
      int len;
      
      
      /* Load the screen */
      for (y = 0; okay; y++)
	{
	  /* Get a line of data */
	  if (my_fgets(fp, buf, 1024)) okay = FALSE;
	  
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
      /* if (my_fgets(fp, buf, 1024)) okay = FALSE; */
      
      
      /* Load the screen */
      for (y = 0; okay; y++)
	{
	  /* Get a line of data */
	  if (my_fgets(fp, buf, 1024)) okay = FALSE;
	  
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

	  /* Place the cursor */
	  move_cursor(y, x);
	  
	}
      
      
      /* Get the blank line */
      /* if (my_fgets(fp, buf, 1024)) okay = FALSE; */
      
      
      /* Close it */
      my_fclose(fp);
    }
  
  /* King or Queen */
  if (p_ptr->total_winner || (p_ptr->lev > PY_MAX_LEVEL))
    {
      p = "Magnificent";
    }
  
  /* Normal */
  else
    {
      p = cp_text + cp_ptr->title[(p_ptr->lev - 1) / 5];
    }

  /* Set offset */
  if (small_screen && (!tree)) offset = 3;
  else offset = 11;
  
  center_string(buf, op_ptr->full_name);
  put_str(buf, 6, offset);
  
  center_string(buf, "the");
  put_str(buf, 7, offset);
  
  center_string(buf, p);
  put_str(buf, 8, offset);
  
  
  center_string(buf, cp_name + cp_ptr->name);
  put_str(buf, 10, offset);
  
  sprintf(tmp, "Level: %d", (int)p_ptr->lev);
  center_string(buf, tmp);
  put_str(buf, 11, offset);
  
  sprintf(tmp, "Exp: %ld", (long)p_ptr->exp);
  center_string(buf, tmp);
  put_str(buf, 12, offset);
  
  sprintf(tmp, "AU: %ld", (long)p_ptr->au);
  center_string(buf, tmp);
  put_str(buf, 13, offset);
  
  if (p_ptr->depth)
    sprintf(tmp, "Killed in %s level %d", 
	    locality_name[stage_map[p_ptr->stage][LOCALITY]], p_ptr->depth);
  else if (boat)
    sprintf(tmp, "Sailed victorious to Aman.");
  else if (tree)
    sprintf(tmp, "Retired to Fangorn Forest.");
  else
    sprintf(tmp, "Killed in %s town", 
	    locality_name[stage_map[p_ptr->stage][LOCALITY]]);  
  center_string(buf, tmp);
  put_str(buf, 14, offset);
  
  if (!(boat || tree))
    {
      sprintf(tmp, "by %s.", p_ptr->died_from);
      center_string(buf, tmp);
      put_str(buf, 15, offset);
    }
  
#ifdef _WIN32_WCE
  {	
    char* fake_ctime(const unsigned long* fake_time_t);
    sprintf(tmp, "%-.24s", fake_ctime(&ct));
  }
#else
  sprintf(tmp, "%-.24s", ctime(&ct));
#endif
  center_string(buf, tmp);
  put_str(buf, 17, offset);
}


/*
 * Hack - Know inventory and home items upon death
 */
static void death_knowledge(void)
{
  int i, which = 0;
  
  object_type *o_ptr;
  
  store_type *st_ptr;
  
  /* Get the store number of the home */
  for (i = 0; i < NUM_TOWNS; i++)
    {
      /* Found the town */
      if (p_ptr->home == towns[i])
	{
	  which += (i < NUM_TOWNS_SMALL ? 3 : STORE_HOME);
	  break;
	}
      /* Next town */
      else
	which += (i < NUM_TOWNS_SMALL ? MAX_STORES_SMALL : MAX_STORES_BIG);
    }

  /* Activate the store */
  st_ptr = &store[which];

  
  /* Hack -- Know everything in the inven/equip */
  for (i = 0; i < INVEN_TOTAL; i++)
    {
      o_ptr = &inventory[i];
      
      /* Skip non-objects */
      if (!o_ptr->k_idx) continue;
      
      /* Aware and Known */
      object_aware(o_ptr);
      object_known(o_ptr);
      
      /* Fully known */
      o_ptr->ident |= (IDENT_MENTAL);
    }
  
  /* Hack -- Know everything in the home */
  for (i = 0; i < st_ptr->stock_num; i++)
    {
      o_ptr = &st_ptr->stock[i];
      
      /* Skip non-objects */
      if (!o_ptr->k_idx) continue;
      
      /* Aware and Known */
      object_aware(o_ptr);
      object_known(o_ptr);
      
      /* Fully known */
      o_ptr->ident |= (IDENT_MENTAL);
    }
  
  /* Hack -- Recalculate bonuses */
  p_ptr->update |= (PU_BONUS);
  
  /* Handle stuff */
  handle_stuff();
}

/*
 * Display some character info
 */
static void show_info(void)
{
  int i, j, k, which = 0;
  
  object_type *o_ptr;
  
  store_type *st_ptr;
  
  /* Get the store number of the home */
  for (i = 0; i < NUM_TOWNS; i++)
    {
      /* Found the town */
      if (p_ptr->home == towns[i])
	{
	  which += (i < NUM_TOWNS_SMALL ? 3 : STORE_HOME);
	  break;
	}
      /* Next town */
      else
	which += (i < NUM_TOWNS_SMALL ? MAX_STORES_SMALL : MAX_STORES_BIG);
    }

  /* Activate the store */
  st_ptr = &store[which];
  
  /* Display player */
  display_player(0);
  
  /* Prompt for inventory */
  prt("Hit any key to see more information (ESC to abort): ", 23, 0);
  
  /* Allow abort at this point */
  if (inkey() == ESCAPE) return;
  
  /* Show equipment and inventory */
  
  /* Equipment -- if any */
  if (p_ptr->equip_cnt)
    {
      Term_clear();
      item_tester_full = TRUE;
      show_equip();
      prt("You are using: -more-", 0, 0);
      if (inkey() == ESCAPE) return;
    }

  /* Inventory -- if any */
  if (p_ptr->inven_cnt)
    {
      Term_clear();
      item_tester_full = TRUE;
      show_inven();
      prt("You are carrying: -more-", 0, 0);
      if (inkey() == ESCAPE) return;
    }
  
  
  
  /* Home -- if anything there */
  if (st_ptr->stock_num)
    {
      /* Display contents of the home */
      for (k = 0, i = 0; i < st_ptr->stock_num; k++)
	{
	  /* Clear screen */
	  Term_clear();
	  
	  /* Show 12 items */
	  for (j = 0; (j < 12) && (i < st_ptr->stock_num); j++, i++)
	    {
	      byte attr;
	      
	      char o_name[80];
	      char tmp_val[80];
	      
	      /* Get the object */
	      o_ptr = &st_ptr->stock[i];
	      
	      /* Print header, clear line */
	      sprintf(tmp_val, "%c) ", I2A(j));
	      prt(tmp_val, j+2, 4);
	      
	      /* Get the object description */
	      object_desc(o_name, o_ptr, TRUE, 4);
	      
	      /* Get the inventory color */
	      attr = tval_to_attr[o_ptr->tval & 0x7F];
	      
	      /* Display the object */
	      c_put_str(attr, o_name, j+2, 7);
	    }
	  
	  /* Caption */
	  prt(format("Your home contains (page %d): -more-", k+1), 0, 0);
	  
	  /* Wait for it */
	  if (inkey() == ESCAPE) return;
	}
    }
}



/*
 * Special version of 'do_cmd_examine'
 */
static void death_examine(void)
{
  int item;
  
  object_type *o_ptr;
  
  char o_name[80];
  
  cptr q, s;
  
  
  /* Start out in "display" mode */
  p_ptr->command_see = TRUE;
  
  /* Get an item */
  q = "Examine which item? ";
  s = "You have nothing to examine.";
  if (!get_item(&item, q, s, (USE_INVEN | USE_EQUIP))) return;
  
  /* Get the item (in the pack) */
  if (item >= 0)
    {
      o_ptr = &inventory[item];
    }
  
  /* Get the item (on the floor) */
  else
    {
      o_ptr = &o_list[0 - item];
    }
  
  /* Fully known */
  o_ptr->ident |= (IDENT_MENTAL);
  
  /* Description */
  object_desc(o_name, o_ptr, TRUE, 3);
  
  /* Describe */
  msg_format("Examining %s...", o_name);
  
  /* Describe it fully */
  do_cmd_observe(o_ptr, FALSE);
}

/*
 * Seek score 'i' in the highscore file
 */
static int highscore_seek(int i)
{
  /* Seek for the requested record */
  return (fd_seek(highscore_fd, (huge)(i) * sizeof(high_score)));
}


/*
 * Read one score from the highscore file
 */
static errr highscore_read(high_score *score)
{
  /* Read the record, note failure */
  return (fd_read(highscore_fd, (char*)(score), sizeof(high_score)));
}


/*
 * Write one score to the highscore file
 */
static int highscore_write(high_score *score)
{
  /* Write the record, note failure */
  return (fd_write(highscore_fd, (char*)(score), sizeof(high_score)));
}




/*
 * Just determine where a new score *would* be placed
 * Return the location (0 is best) or -1 on failure
 */
static int highscore_where(high_score *score)
{
  int i;
  
  high_score the_score;
  
  /* Paranoia -- it may not have opened */
#ifdef _WIN32_WCE
  if (highscore_fd == -1) return (-1);
#else
  if (highscore_fd < 0) return (-1);
#endif

  /* Go to the start of the highscore file */
  if (highscore_seek(0)) return (-1);
  
  /* Read until we get to a higher score */
  for (i = 0; i < MAX_HISCORES; i++)
    {
      if (highscore_read(&the_score)) return (i);
      if (strcmp(the_score.pts, score->pts) < 0) return (i);
    }
  
  /* The "last" entry is always usable */
  return (MAX_HISCORES - 1);
}


/*
 * Actually place an entry into the high score file
 * Return the location (0 is best) or -1 on "failure"
 */
static int highscore_add(high_score *score)
{
  int i, slot;
  bool done = FALSE;
  
  high_score the_score, tmpscore;
  
  
  /* Paranoia -- it may not have opened */
#ifdef _WIN32_WCE
  if (highscore_fd == -1) return (-1);
#else
  if (highscore_fd < 0) return (-1);
#endif
  
  /* Determine where the score should go */
  slot = highscore_where(score);
  
  /* Hack -- Not on the list */
  if (slot < 0) return (-1);
  
  /* Hack -- prepare to dump the new score */
  the_score = (*score);
  
  /* Slide all the scores down one */
  for (i = slot; !done && (i < MAX_HISCORES); i++)
    {
      /* Read the old guy, note errors */
      if (highscore_seek(i)) return (-1);
      if (highscore_read(&tmpscore)) done = TRUE;
      
      /* Back up and dump the score we were holding */
      if (highscore_seek(i)) return (-1);
      if (highscore_write(&the_score)) return (-1);
      
      /* Hack -- Save the old score, for the next pass */
      the_score = tmpscore;
    }
  
  /* Return location used */
  return (slot);
}



/*
 * Display the scores in a given range.
 * Assumes the high score list is already open.
 * Only five entries per line, too much info.
 *
 * Mega-Hack -- allow "fake" entry at the given position.
 */
void display_scores_aux(int from, int to, int note, high_score *score)
{
  int i, j, k, n, place;
  byte attr;
  int wid, hgt;
  key_event ke;
  
  high_score the_score;
  
  char out_val[256];
  char tmp_val[160];
  
  int per_screen;
  
  /* Get size */
  Term_get_size(&wid, &hgt);
  
  per_screen = (hgt - 4) / (small_screen ? 7 : 4);
  
  
  /* Paranoia -- it may not have opened */
#ifdef _WIN32_WCE
  if (highscore_fd == -1) return (-1);
#else
  if (highscore_fd < 0) return;
#endif
  
  
  /* Assume we will show the first 10 */
  if (from < 0) from = 0;
  if (to < 0) to = 10;
  if (to > MAX_HISCORES) to = MAX_HISCORES;
  
  
  /* Seek to the beginning */
  if (highscore_seek(0)) return;
  
  /* Hack -- Count the high scores */
  for (i = 0; i < MAX_HISCORES; i++)
    {
      if (highscore_read(&the_score)) break;
    }
  
  /* Hack -- allow "fake" entry to be last */
  if ((note == i) && score) i++;
  
  /* Forget about the last entries */
  if (i > to) i = to;
  
  
  /* Show per_screen per page, until "done" */
  for (k = from, place = k+1; k < i; k += per_screen)
    {
      /* Clear screen */
      Term_clear();
      
      /* Title */
      put_str_center("FAangband Hall of Fame", 0);
      
#if 0
      /* Indicate non-top scores */
      if (k > 0)
	{
	  sprintf(tmp_val, "(from position %d)", k + 1);
	  put_str(tmp_val, 0, 40);
	}
#endif
      
      /* Dump per_screen entries */
      for (j = k, n = 0; j < i && n < per_screen; place++, j++, n++)
	{
	  int pr, pc, clev, mlev;
	  
	  cptr user, gold, when, aged, cdun;
	  
	  /* Hack -- indicate death in yellow */
	  attr = (j == note) ? TERM_YELLOW : TERM_WHITE;
	  
	  /* Mega-Hack -- insert a "fake" record */
	  if ((note == j) && score)
	    {
	      the_score = (*score);
	      attr = TERM_L_GREEN;
	      score = NULL;
	      note = -1;
	      j--;
	    }
	  
	  /* Read a normal record */
	  else
	    {
	      /* Read the proper record */
	      if (highscore_seek(j)) break;
	      if (highscore_read(&the_score)) break;
	    }
	  
	  /* Extract the race/class */
	  pr = atoi(the_score.p_r);
	  pc = atoi(the_score.p_c);
	  
	  /* Extract the level info */
	  clev = atoi(the_score.cur_lev);
	  mlev = atoi(the_score.max_lev);
	  cdun = the_score.cur_dun;
	  
	  /* Hack -- extract the gold and such */
	  for (user = the_score.uid; isspace(*user); user++) /* loop */;
	  for (when = the_score.day; isspace(*when); when++) /* loop */;
	  for (gold = the_score.gold; isspace(*gold); gold++) /* loop */;
	  for (aged = the_score.turns; isspace(*aged); aged++) /* loop */;
	  
	  /* Clean up standard encoded form of "when" */
	  if ((*when == '@') && strlen(when) == 9)
	    {
	      sprintf(tmp_val, "%.4s-%.2s-%.2s",
		      when + 1, when + 5, when + 7);
	      when = tmp_val;
	    }
	  
	  if (small_screen)
	    {
	      /* Dump some info */
	      sprintf(out_val, "%3d.%9s  %s the",
		      place, the_score.pts, the_score.who);
	      
	      /* Dump the first line */
	      c_put_str(attr, out_val, n * 7 + 2, 0);
	      
	      /* More info */
	      sprintf(out_val, "               %s %s, Level %d",
		      rp_name + rp_info[pr].name, cp_name + cp_info[pc].name,
		      clev);
	      
	      /* Dump the next line */
	      c_put_str(attr, out_val, n * 7 + 3, 0);
	      
	      /* Line of info */
	      sprintf(out_val, "               Killed by %s", the_score.how);

	      /* Dump the info */
	      c_put_str(attr, out_val, n * 7 + 4, 0);

	      /* Line of info */
	      sprintf(out_val, "               in %s", cdun);

	      /* Dump the info */
	      c_put_str(attr, out_val, n * 7 + 5, 0);
	  
	      /* Line of info */
	      sprintf(out_val,
		      "               (User %s, Date %s,", user, when);

	      /* Dump the info */
	      c_put_str(attr, out_val, n * 7 + 6, 0);
	  
	      /* Line of info */
	      sprintf(out_val,
		      "               Gold %s, Turn %s).", gold, aged);

	      /* Dump the info */
	      c_put_str(attr, out_val, n * 7 + 7, 0);
	    }
	  else
	    {
	      /* Dump some info */
	      sprintf(out_val, "%3d.%9s  %s the %s %s, Level %d",
		      place, the_score.pts, the_score.who,
		      rp_name + rp_info[pr].name, cp_name + cp_info[pc].name,
		      clev);
	  
	      /* Dump the first line */
	      c_put_str(attr, out_val, n * 4 + 2, 0);
	  
	      /* Another line of info */
	      sprintf(out_val, "               Killed by %s in %s",
		      the_score.how, cdun);
	  
	      /* Dump the info */
	      c_put_str(attr, out_val, n * 4 + 3, 0);
	  
	      /* And still another line of info */
	      sprintf(out_val,
		      "               (User %s, Date %s, Gold %s, Turn %s).",
		      user, when, gold, aged);
	      c_put_str(attr, out_val, n * 4 + 4, 0);
	    }
	}
      
      
      /* Wait for response */
      prt_center("[Press ESC to quit, any other key to continue.]", hgt - 1);
      ke = inkey_ex();
      prt("", hgt - 1, 0);
      
      /* Hack -- notice Escape */
      if (j == ESCAPE) break;
    }
}


/*
 * Hack -- Display the scores in a given range and quit.
 *
 * This function is only called from "main.c" when the user asks
 * to see the "high scores".
 */
void display_scores(int from, int to)
{
  char buf[1024];
  
  int wid, hgt;
  
  /* Get size */
  Term_get_size(&wid, &hgt);
  
  /* Build the filename */
  path_build(buf, 1024, ANGBAND_DIR_APEX, "scores.raw");
  
  /* Open the binary high score file, for reading */
  highscore_fd = fd_open(buf, O_RDONLY);
  
  /* Paranoia -- No score file */
#ifdef _WIN32_WCE
  if (highscore_fd == -1) quit("Score file unavailable.");
#else
  if (highscore_fd < 0) quit("Score file unavailable.");
#endif
  
  /* Clear screen */
  Term_clear();
  
  /* Display the scores */
  display_scores_aux(from, to, -1, NULL);
  
  /* Shut the high score file */
  fd_close(highscore_fd);
  
  /* Forget the high score fd */
  highscore_fd = -1;
  
  /* Quit */
  quit(NULL);
}


/*
 * Hack - save index of player's high score
 */
static int score_idx = -1;



/*
 * Enters a players name on a hi-score table, if "legal".
 *
 * Assumes "signals_ignore_tstp()" has been called.
 */
static errr enter_score(void)
{
  int j;
  
  high_score the_score;
  
  
  /* No score file */
  if (highscore_fd < 0)
    {
      return (0);
    }
  
#ifndef SCORE_WIZARDS
  
  /* Wizard-mode pre-empts scoring */
  if (p_ptr->noscore & 0x000F)
    {
      msg_print("Score not registered for wizards.");
      msg_print(NULL);
      score_idx = -1;
      return (0);
    }
  
#endif

#ifndef SCORE_BORGS

  /* Borg-mode pre-empts scoring */
  if (p_ptr->noscore & 0x00F0)
    {
      msg_print("Score not registered for borgs.");
      msg_print(NULL);
      score_idx = -1;
      return (0);
    }
#endif
  
#ifndef SCORE_CHEATERS
  
  /* Cheaters are not scored */
  for (j = OPT_cheat_start; j < OPT_cheat_end+1; ++j)
    {
      if (!op_ptr->opt[j]) continue;
      
      msg_print("Score not registered for cheaters.");
      msg_print(NULL);
      score_idx = -1;
      return (0);
    }
  
#endif
  
  /* Hack -- Interupted */
  if (!p_ptr->total_winner && streq(p_ptr->died_from, "Interrupting"))
    {
      msg_print("Score not registered due to interruption.");
      msg_print(NULL);
      score_idx = -1;
      return (0);
    }
  
  /* Hack -- Quitter */
  if (!p_ptr->total_winner && streq(p_ptr->died_from, "Quitting"))
    {
      msg_print("Score not registered due to quitting.");
      msg_print(NULL);
      score_idx = -1;
      return (0);
    }
  
  
  /* Clear the record */
  (void)WIPE(&the_score, high_score);
  
  /* Save the version */
  sprintf(the_score.what, "%u.%u.%u",
	  VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
  
  /* Calculate and save the points */
  sprintf(the_score.pts, "%9lu", (long)total_points());
  the_score.pts[9] = '\0';
  
  /* Save the current gold */
  sprintf(the_score.gold, "%9lu", (long)p_ptr->au);
  the_score.gold[9] = '\0';
  
  /* Save the current turn */
  sprintf(the_score.turns, "%9lu", (long)turn);
  the_score.turns[9] = '\0';
  
#ifdef HIGHSCORE_DATE_HACK
  /* Save the date in a hacked up form (9 chars) */
  sprintf(the_score.day, "%-.6s %-.2s",
	  ctime(&death_time) + 4, ctime(&death_time) + 22);
#else
  /* Save the date in standard encoded form (9 chars) */
#ifdef _WIN32_WCE
  {
    char* fake_ctime(const unsigned long* fake_time_t);
    sprintf(the_score.day, "%-.6s %-.2s",
	    fake_ctime(&death_time) + 4, fake_ctime(&death_time) + 22);
  }
#else
  strftime(the_score.day, 10, "@%Y%m%d", localtime(&death_time));
#endif /* _WIN32_WCE */
#endif
  
  /* Save the player name (15 chars) */
  sprintf(the_score.who, "%-.15s", op_ptr->full_name);
  
  /* Save the player info XXX XXX XXX */
  sprintf(the_score.uid, "%7u", player_uid);
  sprintf(the_score.sex, "%c", (p_ptr->psex ? 'm' : 'f'));
  sprintf(the_score.p_r, "%2d", p_ptr->prace);
  sprintf(the_score.p_c, "%2d", p_ptr->pclass);
  
  /* Save the level and such */
  sprintf(the_score.cur_lev, "%3d", p_ptr->lev);
  if (p_ptr->depth)
    sprintf(the_score.cur_dun, "%s level %d", 
	    locality_name[stage_map[p_ptr->stage][LOCALITY]], p_ptr->depth);
  else
    sprintf(the_score.cur_dun, "%s Town", 
	    locality_name[stage_map[p_ptr->stage][LOCALITY]]);
  sprintf(the_score.max_lev, "%3d", p_ptr->max_lev);
  
  /* Save the cause of death (31 chars) */
  sprintf(the_score.how, "%-.31s", p_ptr->died_from);
  
  
  /* Lock (for writing) the highscore file, or fail */
  if (fd_lock(highscore_fd, F_WRLCK)) return (1);
  
  /* Add a new entry to the score list, see where it went */
  score_idx = highscore_add(&the_score);
  
  /* Unlock the highscore file, or fail */
  if (fd_lock(highscore_fd, F_UNLCK)) return (1);
  
  
  /* Success */
  return (0);
}



/*
 * Enters a players name on a hi-score table, if "legal", and in any
 * case, displays some relevant portion of the high score list.
 *
 * Assumes "signals_ignore_tstp()" has been called.
 */
static void top_twenty(void)
{
  /* Clear screen */
  Term_clear();
  
  /* No score file */
  if (highscore_fd < 0)
    {
      msg_print("Score file unavailable.");
      msg_print(NULL);
      return;
    }
  
  /* Player's score unavailable */
  if (score_idx == -1)
    {
      display_scores_aux(0, 10, -1, NULL);
      return;
    }
  
  /* Hack -- Display the top fifteen scores */
  else if (score_idx < 10)
    {
      display_scores_aux(0, (small_screen ? 8 : 15), score_idx, NULL);
    }
  
  /* Display the scores surrounding the player */
  else
    {
      display_scores_aux(0, (small_screen ? 4 : 5), score_idx, NULL);
      display_scores_aux(score_idx - 2, score_idx + (small_screen ? 3 : 7), 
			 score_idx, NULL);
    }
  
  
  /* Success */
  return;
}



/*
 * Predict the players location, and display it.
 */
errr predict_score(void)
{
  int j;
  
  high_score the_score;
  
  
  /* No score file */
#ifdef _WIN32_WCE
  if (highscore_fd == -1)
#else
  if (highscore_fd < 0)
#endif
    {
      msg_print("Score file unavailable.");
      msg_print(NULL);
      return (0);
    }
  
  
  /* Save the version */
  sprintf(the_score.what, "%u.%u.%u",
	  VERSION_MAJOR, VERSION_MINOR, VERSION_PATCH);
  
  /* Calculate and save the points */
  sprintf(the_score.pts, "%9lu", (long)total_points());
  
  /* Save the current gold */
  sprintf(the_score.gold, "%9lu", (long)p_ptr->au);

  /* Save the current turn */
  sprintf(the_score.turns, "%9lu", (long)turn);
  
  /* Hack -- no time needed */
  strcpy(the_score.day, "TODAY");
  
  /* Save the player name (15 chars) */
  sprintf(the_score.who, "%-.15s", op_ptr->full_name);
  
  /* Save the player info XXX XXX XXX */
  sprintf(the_score.uid, "%7u", player_uid);
  sprintf(the_score.sex, "%c", (p_ptr->psex ? 'm' : 'f'));
  sprintf(the_score.p_r, "%2d", p_ptr->prace);
  sprintf(the_score.p_c, "%2d", p_ptr->pclass);
  
  /* Save the level and such */
  sprintf(the_score.cur_lev, "%3d", p_ptr->lev);
  sprintf(the_score.cur_dun, "%s level %d", 
	  locality_name[stage_map[p_ptr->stage][LOCALITY]],p_ptr->depth);
  sprintf(the_score.max_lev, "%3d", p_ptr->max_lev);
  
  /* Hack -- no cause of death */
  strcpy(the_score.how, "nobody (yet!)");
  
  
  /* See where the entry would be placed */
  j = highscore_where(&the_score);
  
  
  /* Hack -- Display the top fifteen scores */
  if (j < 10)
    {
      display_scores_aux(0, (small_screen ? 10 : 15), j, &the_score);
    }
  
  /* Display some "useful" scores */
  else
    {
      display_scores_aux(0, (small_screen ? 4 : 5), -1, NULL);
      display_scores_aux(j - 2, j + (small_screen ? 2 : 7), j, &the_score);
    }
  
  
  /* Success */
  return (0);
}



void show_scores(void)
{
  char buf[1024];
  
  /* Build the filename */
  path_build(buf, sizeof(buf), ANGBAND_DIR_APEX, "scores.raw");
  
  /* Open the binary high score file, for reading */
  highscore_fd = fd_open(buf, O_RDONLY);

  /* Paranoia -- No score file */
#ifdef _WIN32_WCE
  if (highscore_fd == -1)
#else
  if (highscore_fd < 0)
#endif
    {
      msg_print("Score file unavailable.");
    }
  else
    {
      /* Save Screen */
      screen_save();
      
      /* Clear screen */
      Term_clear();
      
      /* Display the scores */
      if (character_generated)
	predict_score();
      else
	display_scores_aux(0, MAX_HISCORES, -1, NULL);
      
      /* Shut the high score file */
      (void)fd_close(highscore_fd);
      
      /* Forget the high score fd */
      highscore_fd = -1;
      
      /* Load screen */
      screen_load();
      
      /* Hack - Flush it */
      Term_fresh();
    }
}



/*
 * Change the player into a Winner
 */
static void kingly(void)
{
  /* Hack -- retire in town */
  p_ptr->depth = 0;
  
  /* Fake death */
  strcpy(p_ptr->died_from, "Ripe Old Age");
  
  /* Restore the experience */
  p_ptr->exp = p_ptr->max_exp;
  
  /* Restore the level */
  p_ptr->lev = p_ptr->max_lev;
  
  /* Hack -- Instant Gold */
  p_ptr->au += 10000000L;
  
  /* Clear screen */
  Term_clear();
  
  /* Display a crown */
  put_str("#", 1, 34);
  put_str("#####", 2, 32);
  put_str("#", 3, 34);
  put_str(",,,  $$$  ,,,", 4, 28);
  put_str(",,=$   \"$$$$$\"   $=,,", 5, 24);
  put_str(",$$        $$$        $$,", 6, 22);
  put_str("*>         <*>         <*", 7, 22);
  put_str("$$         $$$         $$", 8, 22);
  put_str("\"$$        $$$        $$\"", 9, 22);
  put_str("\"$$       $$$       $$\"", 10, 23);
  put_str("*#########*#########*", 11, 24);
  put_str("*#########*#########*", 12, 24);
  
  /* Display a message */
  put_str("Veni, Vidi, Vici!", 15, 26);
  put_str("I came, I saw, I conquered!", 16, 21);
  put_str(format("All Hail the Mighty %s!", sp_ptr->winner), 17, 22);
  
  /* Flush input */
  flush();
  
  /* Wait for response */
  pause_line(23);
}

/*
 * Handle character death
 */
static void close_game_aux(void)
{
  int ch, adj = 0;
  key_event ke;
  
  cptr p, q;
  
  /* Flush all input keys */
  flush();

  /* Dump bones file */
  make_bones();

  /* Prompt */
  if (adult_take_notes) 
    {
      if (small_screen)
	{
	  q = "['a' add to notes file,'c' chardump,'t' scores]";
	  adj = 22;
	}
      else
	{
	  q = "['a' to add a notes file comment, 'c' for a character dump, 't' for scores]";
	  adj = 41;
	}
    }
  else
    if (small_screen)
      q = "['c' chardump, 't' scores]";
    else
      q = "['c' for a character dump, 't' for scores]";
  if (small_screen)
    p = "['i' info, 'm' messages, 'x' items, or ESC]";
  else
    p = "['i' for character info, 'm' for messages, 'x' to examine items, or ESC]";

  /* Handle retirement */
  if (p_ptr->total_winner) kingly();

  /* Get time of death */
#ifdef _WIN32_WCE
  {
    unsigned long fake_time(unsigned long* fake_time_t);
    fake_time(&death_time);
  }
#else
  (void)time(&death_time);
#endif

  /* You are dead */
  print_tomb();

  /* Hack - Know everything upon death */
  death_knowledge();

  /* Enter player in high score list */
  enter_score();

  /* Flush all input keys */
  flush();

  /* Flush messages */
  msg_print(NULL);

  /* Forever */
  while (1)
    {
      /* Describe options */
      Term_putstr((small_screen ? 0 : 2), 22, -1, TERM_WHITE, q);
      Term_putstr((small_screen ? 0 : 2), 23, -1, TERM_WHITE, p);
      
      /* Query */
      ke = inkey_ex();
      ch = ke.key;
      
      /* Mouse input */
      if (ch == '\xff') 
	{
	  if (ke.mousey == 22)
	    {
	      if ((adj) && (ke.mousex > (small_screen ? 0 : 2)) 
		  && (ke.mousex < adj))
		ch = 'a';
	      else if ((ke.mousex > adj + 1) 
		       && (ke.mousex < adj + (small_screen ? 13 : 26)))
		ch = 'c';
	      else if ((ke.mousex > adj + (small_screen ? 14 : 27)) && 
		       (ke.mousex < adj + (small_screen ? 25 : 42)))
		ch = 't';
	    }
	  if (ke.mousey == 23)
	    {
	      if ((ke.mousex > (small_screen ? 0 : 2)) && 
		  (ke.mousex < (small_screen ? 9 : 25))) ch = 'i';
	      if ((ke.mousex > (small_screen ? 10 : 26)) && 
		  (ke.mousex < (small_screen ? 23 : 43))) ch = 'm';
	      if ((ke.mousex > (small_screen ? 24 : 44)) && 
		  (ke.mousex < (small_screen ? 34 : 65))) ch = 'x';
	      if ((ke.mousex > (small_screen ? 38 : 69)) && 
		  (ke.mousex < (small_screen ? 42 : 73))) ch = ESCAPE;
	    }
	}
      
      /* Exit */
      if (ch == ESCAPE)
	{
	  if (get_check("Do you want to quit? ")) break;
	}

      /* File dump */
      else if (ch == 'c')
	{
	  char ftmp[80];
	  
	  sprintf(ftmp, "%s.txt", op_ptr->base_name);
	  
	  if (get_string("File name: ", ftmp, 80))
	    {
	      if (ftmp[0] && (ftmp[0] != ' '))
		{
		  errr err;
		  
		  /* Save screen */
		  screen_save();

		  /* Dump a character file */
		  err = file_character(ftmp, FALSE);

		  /* Load screen */
		  screen_load();

		  /* Check result */
		  if (err)
		    {
		      msg_print("Character dump failed!");
		    }
		  else
		    {
		      msg_print("Character dump successful.");
		    }
		  
		  /* Flush messages */
		  msg_print(NULL);
		}
	    }
	}

      /* Show more info */
      else if (ch == 'i')
	{
	  /* Save screen */
	  screen_save();

	  /* Show the character */
	  show_info();

	  /* Load screen */
	  screen_load();
	}

      /* Show top scores */
      else if (ch == 't')
	{
	  /* Save screen */
	  screen_save();

	  /* Show the scores */
	  top_twenty();


	  /* Load screen */
	  screen_load();
	}

      /* Show top scores */
      else if (ch == 'm')
	{
	  /* Save screen */
	  screen_save();

	  /* Show the scores */
	  do_cmd_messages();


	  /* Load screen */
	  screen_load();
	}

      /* Examine an item */
      else if (ch == 'x')
	{
	  death_examine();
	}
      
      /* Add last words to notes file */
      else if (ch == 'a')
	{
	  if (adult_take_notes)
	    {
	      do_cmd_note("",  p_ptr->stage);
	    }
	  
	}
      
    }


  /* Save dead player */
  if (!save_player())
    {
      msg_print("death save failed!");
      msg_print(NULL);
    }
  
}

/*
 * Close up the current game (player may or may not be dead)
 *
 * This function is called only from "main.c" and "signals.c".
 *
 * Note that the savefile is not saved until the tombstone is
 * actually displayed and the player has a chance to examine
 * the inventory and such.  This allows cheating if the game
 * is equipped with a "quit without save" method.  XXX XXX XXX
 */
void close_game(void)
{
  char buf[1024];
  
  
  /* Handle stuff */
  handle_stuff();
  
  /* Flush the messages */
  msg_print(NULL);
  
  /* Flush the input */
  flush();
  
  
  /* No suspending now */
  signals_ignore_tstp();
  
  
  /* Hack -- Character is now "icky" */
  character_icky = TRUE;
  
  
  /* Build the filename */
  path_build(buf, 1024, ANGBAND_DIR_APEX, "scores.raw");
  
  /* Open the high score file, for reading/writing */
  highscore_fd = fd_open(buf, O_RDWR);
  
  
  /* Handle death */
  if (p_ptr->is_dead)
    {
      /* Auxiliary routine */
      close_game_aux();
    }
  
  /* Still alive */
  else
    {
      /* Save the game */
      do_cmd_save_game(FALSE);
      
      /* Prompt for scores XXX XXX XXX */
      prt("Press Return (or Escape).", 0, 40);
      
      /* Predict score (or ESCAPE) */
      if (inkey() != ESCAPE) predict_score();
    }
  
  
  /* Shut the high score file */
  fd_close(highscore_fd);
  
  /* Forget the high score fd */
  highscore_fd = -1;
  
  
  /* Close the notes file */
  if (adult_take_notes)
    {
      my_fclose(notes_file);
      
      /* Delete the notes file */
      fd_kill(notes_fname);
    }
  
  /* Allow suspending now */
  signals_handle_tstp();
}


/*
 * Handle abrupt death of the visual system
 *
 * This routine is called only in very rare situations, and only
 * by certain visual systems, when they experience fatal errors.
 *
 * XXX XXX Hack -- clear the death flag when creating a HANGUP
 * save file so that player can see tombstone when restart.
 */
void exit_game_panic(void)
{
  /* If nothing important has happened, just quit */
  if (!character_generated || character_saved) quit("panic");
  
  /* Mega-Hack -- see "msg_print()" */
  msg_flag = FALSE;
  
  /* Clear the top line */
  prt("", 0, 0);
  
  /* Hack -- turn off some things */
  disturb(1, 0);
  
  /* Hack -- Delay death XXX XXX XXX */
  if (p_ptr->chp < 0) p_ptr->is_dead = FALSE;
  
  /* Hardcode panic save */
  p_ptr->panic_save = 1;
  
  /* Forbid suspend */
  signals_ignore_tstp();
  
  /* Indicate panic save */
  strcpy(p_ptr->died_from, "(panic save)");
  
  /* Panic save, or get worried */
  if (!save_player()) quit("panic save failed!");
  
  /* Successful panic save */
  quit("panic save succeeded!");
}


/*  Taken from Zangband.  What a good idea! */
errr get_rnd_line(char *file_name, char *output)
{
  FILE	    *fp;
  char	buf[1024];
  int lines=0, line, counter;
  
  /* Build the filename */
  path_build(buf, 1024, ANGBAND_DIR_FILE, file_name);
  
  /* Open the file */
  fp = my_fopen(buf, "r");
  
  /* Failed */
  if (!fp) return (-1);
  
  /* Parse the file */
  if (0 == my_fgets(fp, buf, 80))
    lines = atoi(buf);
  else return (1);
  
  /* choose a random line */
  line = randint(lines);
  
  for (counter = 0; counter <= line; counter++)
    {
      if (!(0 == my_fgets(fp, buf, 80)))
	return (1);
      else if (counter == line)
	break;
    }
  
  strcpy (output, buf);
  
  /* Close the file */
  my_fclose(fp);
  
  return (0);
}



#ifdef HANDLE_SIGNALS


#include <signal.h>


/*
 * Handle signals -- suspend
 *
 * Actually suspend the game, and then resume cleanly
 */
static void handle_signal_suspend(int sig)
{
  /* Disable handler */
  (void)signal(sig, SIG_IGN);
  
#ifdef SIGSTOP

  /* Flush output */
  Term_fresh();
  
  /* Suspend the "Term" */
  Term_xtra(TERM_XTRA_ALIVE, 0);
  
  /* Suspend ourself */
  (void)kill(0, SIGSTOP);
  
  /* Resume the "Term" */
  Term_xtra(TERM_XTRA_ALIVE, 1);
  
  /* Redraw the term */
  Term_redraw();
  
  /* Flush the term */
  Term_fresh();
  
#endif

  /* Restore handler */
  (void)signal(sig, handle_signal_suspend);
}


/*
 * Handle signals -- simple (interrupt and quit)
 *
 * This function was causing a *huge* number of problems, so it has
 * been simplified greatly.  We keep a global variable which counts
 * the number of times the user attempts to kill the process, and
 * we commit suicide if the user does this a certain number of times.
 *
 * We attempt to give "feedback" to the user as he approaches the
 * suicide thresh-hold, but without penalizing accidental keypresses.
 *
 * To prevent messy accidents, we should reset this global variable
 * whenever the user enters a keypress, or something like that.
 */
static void handle_signal_simple(int sig)
{
  /* Disable handler */
  (void)signal(sig, SIG_IGN);
  
  
  /* Nothing to save, just quit */
  if (!character_generated || character_saved) quit(NULL);
  
  
  /* Count the signals */
  signal_count++;
  
  
  /* Terminate dead characters */
  if (p_ptr->is_dead)
    {
      /* Mark the savefile */
      strcpy(p_ptr->died_from, "Abortion");
      
      /* Close stuff */
      close_game();
      
      /* Quit */
      quit("interrupt");
    }
  
  /* Allow suicide (after 5) */
  else if (signal_count >= 5)
    {
      /* Cause of "death" */
      strcpy(p_ptr->died_from, "Interrupting");
      
      /* Commit suicide */
      p_ptr->is_dead = TRUE;
      
      /* Stop playing */
      p_ptr->playing = FALSE;
      
      /* Leaving */
      p_ptr->leaving = TRUE;
      
      /* Close stuff */
      close_game();
      
      /* Quit */
      quit("interrupt");
    }
  
  /* Give warning (after 4) */
  else if (signal_count >= 4)
    {
      /* Make a noise */
      Term_xtra(TERM_XTRA_NOISE, 0);
      
      /* Clear the top line */
      Term_erase(0, 0, 255);
      
      /* Display the cause */
      Term_putstr(0, 0, -1, TERM_WHITE, "Contemplating suicide!");
      
      /* Flush */
      Term_fresh();
    }
  
  /* Give warning (after 2) */
  else if (signal_count >= 2)
    {
      /* Make a noise */
      Term_xtra(TERM_XTRA_NOISE, 0);
    }
  
  /* Restore handler */
  (void)signal(sig, handle_signal_simple);
}


/*
 * Handle signal -- abort, kill, etc
 */
static void handle_signal_abort(int sig)
{
  /* Disable handler */
  (void)signal(sig, SIG_IGN);
  
  
  /* Nothing to save, just quit */
  if (!character_generated || character_saved) quit(NULL);
  
  
  /* Clear the bottom line */
  Term_erase(0, 23, 255);
  
  /* Give a warning */
  Term_putstr(0, 23, -1, TERM_RED,
	      "A gruesome software bug LEAPS out at you!");
  
  /* Message */
  Term_putstr(45, 23, -1, TERM_RED, "Panic save...");
  
  /* Flush output */
  Term_fresh();
  
  /* Panic Save */
  p_ptr->panic_save = 1;
  
  /* Panic save */
  strcpy(p_ptr->died_from, "(panic save)");
  
  /* Forbid suspend */
  signals_ignore_tstp();
  
  /* Attempt to save */
  if (save_player())
    {
      Term_putstr(45, 23, -1, TERM_RED, "Panic save succeeded!");
    }
  
  /* Save failed */
  else
    {
      Term_putstr(45, 23, -1, TERM_RED, "Panic save failed!");
    }
  
  /* Flush output */
  Term_fresh();
  
  /* Quit */
  quit("software bug");
}




/*
 * Ignore SIGTSTP signals (keyboard suspend)
 */
void signals_ignore_tstp(void)
{

#ifdef SIGTSTP
  (void)signal(SIGTSTP, SIG_IGN);
#endif

}

/*
 * Handle SIGTSTP signals (keyboard suspend)
 */
void signals_handle_tstp(void)
{

#ifdef SIGTSTP
  (void)signal(SIGTSTP, handle_signal_suspend);
#endif

}


/*
 * Prepare to handle the relevant signals
 */
void signals_init(void)
{

#ifdef SIGHUP
  (void)signal(SIGHUP, SIG_IGN);
#endif


#ifdef SIGTSTP
  (void)signal(SIGTSTP, handle_signal_suspend);
#endif


#ifdef SIGINT
  (void)signal(SIGINT, handle_signal_simple);
#endif

#ifdef SIGQUIT
  (void)signal(SIGQUIT, handle_signal_simple);
#endif


#ifdef SIGFPE
  (void)signal(SIGFPE, handle_signal_abort);
#endif
  
#ifdef SIGILL
  (void)signal(SIGILL, handle_signal_abort);
#endif

#ifdef SIGTRAP
  (void)signal(SIGTRAP, handle_signal_abort);
#endif

#ifdef SIGIOT
  (void)signal(SIGIOT, handle_signal_abort);
#endif

#ifdef SIGKILL
  (void)signal(SIGKILL, handle_signal_abort);
#endif

#ifdef SIGBUS
  (void)signal(SIGBUS, handle_signal_abort);
#endif

#ifdef SIGSEGV
  (void)signal(SIGSEGV, handle_signal_abort);
#endif

#ifdef SIGTERM
  (void)signal(SIGTERM, handle_signal_abort);
#endif

#ifdef SIGPIPE
  (void)signal(SIGPIPE, handle_signal_abort);
#endif

#ifdef SIGEMT
  (void)signal(SIGEMT, handle_signal_abort);
#endif

#ifdef SIGDANGER
  (void)signal(SIGDANGER, handle_signal_abort);
#endif

#ifdef SIGSYS
  (void)signal(SIGSYS, handle_signal_abort);
#endif

#ifdef SIGXCPU
  (void)signal(SIGXCPU, handle_signal_abort);
#endif

#ifdef SIGPWR
  (void)signal(SIGPWR, handle_signal_abort);
#endif

}


#else	/* HANDLE_SIGNALS */


/*
 * Do nothing
 */
void signals_ignore_tstp(void)
{
}

/*
 * Do nothing
 */
void signals_handle_tstp(void)
{
}

/*
 * Do nothing
 */
void signals_init(void)
{
}


#endif	/* HANDLE_SIGNALS */


static void write_html_escape_char(FILE *htm, char c)
{
  switch (c)
    {
    case '<':
      fprintf(htm, "&lt;");
      break;
    case '>':
      fprintf(htm, "&gt;");
      break;
    case '&':
      fprintf(htm, "&amp;");
      break;
    default:
      fprintf(htm, "%c", c);
      break;
    }
}

bool write_char(int row, int col)
{
  if (use_trptile && ((row % 3) || (col % 3) || ((col % 6) && use_bigtile))) 
    return (FALSE);
  if (use_dbltile && ((row % 2) || (col % 2) || ((col % 4) && use_bigtile))) 
    return (FALSE);
  if (use_bigtile && (col % 2)) return (FALSE);
  return (TRUE);
}
      

/*
 * Get the default (ASCII) tile for a given screen location
 */
static void get_default_tile(int row, int col, byte *a_def, char *c_def)
{
  byte a;
  char c;
  
  int wid, hgt;
  int screen_wid, screen_hgt;
  int x, y, col_factor, row_factor;

  col_factor = (use_trptile ? (use_bigtile ? 6 : 3) : 
		(use_dbltile ? (use_bigtile ? 4 : 2) : 
		 (use_bigtile ? 2 : 1)));
  row_factor = (use_trptile ? 3 : (use_dbltile ? 2 : 1));
  
  x = (col - COL_MAP)/col_factor + panel_col_min;
  y = (row - ROW_MAP)/row_factor + panel_row_min;

  /* Retrieve current screen size */
  Term_get_size(&wid, &hgt);
  
  /* Calculate the size of dungeon map area (ignoring bigscreen) */
  screen_wid = wid - (COL_MAP + 1);
  screen_hgt = hgt - (ROW_MAP + 1);
  
  /* Get the tile from the screen */
  a = Term->scr->a[row][col];
  c = Term->scr->c[row][col];
  
  /* Convert the map display to the default characters */
  if (!character_icky &&
      ((col - COL_MAP) >= 0) && ((col - COL_MAP) < SCREEN_WID * col_factor) &&
      ((row - ROW_MAP) >= 0) && ((row - ROW_MAP) < SCREEN_HGT * row_factor))
    {
      /* Convert dungeon map into default attr/chars */
      if (in_bounds(y, x) && write_char(row - ROW_MAP, col - COL_MAP))
	{
	  /* Retrieve default attr/char */
	  map_info_default(y, x, &a, &c);
	}
      else
	{
	  /* "Out of bounds" is empty */
	  a = TERM_WHITE;
	  c = ' ';
	}
      
      if (c == '\0') c = ' ';
    }
  
  /* Filter out remaining graphics */
  if (a & 0xf0)
    {
      /* Replace with "white space" */
      a = TERM_WHITE;
      c = ' ';
    }
  
  /* Return the default tile */
  *a_def = a;
  *c_def = c;
}



/* Take an html screenshot */
void html_screenshot(cptr name)
{
  int y, x;
  int wid, hgt;
  
  byte a;
  byte oa = TERM_WHITE;
  char c = ' ';
  
  FILE *htm;
  
  char buf[1024];
  
  /* Build the filename */
  path_build(buf, sizeof(buf), ANGBAND_DIR_USER, name);
  
  /* File type is "TEXT" */
  FILE_TYPE(FILE_TYPE_TEXT);
  
  /* Append to the file */
  htm = my_fopen(buf, "w");

  /* Oops */
  if (!htm)
    {
      plog_fmt("Cannot write the '%s' file!", buf);
      return;
    }
  
  /* Retrieve current screen size */
  Term_get_size(&wid, &hgt);
  
  fprintf(htm, "<HTML>\n");
  fprintf(htm, "<HEAD>\n");
  fprintf(htm, "<META NAME=\"GENERATOR\" Content=\"FAAngband %s\">\n", 
	  VERSION_STRING);
  fprintf(htm, "<TITLE>%s</TITLE>\n", name);
  fprintf(htm, "</HEAD>\n");
  fprintf(htm, "<BODY TEXT=\"#FFFFFF\" BGCOLOR=\"#000000\">");
  fprintf(htm, "<FONT COLOR=\"#%02X%02X%02X\">\n<PRE><TT>",
	  angband_color_table[TERM_WHITE][1],
	  angband_color_table[TERM_WHITE][2],
	  angband_color_table[TERM_WHITE][3]);
  
  /* Dump the screen */
  for (y = 0; y < hgt; y++)
    {
      for (x = 0; x < wid; x++)
	{
	  
	  /* Get the ASCII tile */
	  get_default_tile(y, x, &a, &c);
	  
	  /* Hack - show base color in dump */
	  a &= 0x0F;

	  /* Color change */
	  if (oa != a)
	    {
	      /* From the default white to another color */
	      if (oa == TERM_WHITE)
		{
		  fprintf(htm, "<FONT COLOR=\"#%02X%02X%02X\">",
			  angband_color_table[a][1],
			  angband_color_table[a][2],
			  angband_color_table[a][3]);
		}
	      /* From another color to the default white */
	      else if (a == TERM_WHITE)
		{
		  fprintf(htm, "</FONT>");
		}
	      /* Change colors */
	      else
		{
		  fprintf(htm, "</FONT><FONT COLOR=\"#%02X%02X%02X\">",
			  angband_color_table[a][1],
			  angband_color_table[a][2],
			  angband_color_table[a][3]);
		}
	      
	      /* Remember the last color */
	      oa = a;
	    }
	  
	  /* Write the character and escape special HTML characters */
	  write_html_escape_char(htm, c);
	}
      
      /* End the row */
      fprintf(htm, "\n");
    }
  
  /* Close the last <font> tag if necessary */
  if (a != TERM_WHITE) fprintf(htm, "</FONT>");
  
  fprintf(htm, "</TT></PRE>\n");
  
  fprintf(htm, "</BODY>\n");
  fprintf(htm, "</HTML>\n");
  
  /* Close it */
  my_fclose(htm);
}
