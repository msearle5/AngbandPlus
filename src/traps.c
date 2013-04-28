/* File: traps.c */

/*
 * Trap-handling.  Table of trap kinds, tests for kinds of traps, display
 * traps and graphics, pick a trap and place it.  Disarm and load traps.
 * Monster trap effects.  Character trap effects.
 *
 *
 * Copyright (c) 2002
 * Leon Marrick, Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"


/*
 * A table of trap_kind information.  Move this to an external text file
 * at some point.
 *
 * d_attr, d_char, x_attr, x_char, min_depth, xtra,
 * flags, name
 *
 * A max_depth of 0 means no limit
 * A rarity of 0 means it doesn't get generated normally
 */
trap_kind t_kind_info[TRAP_KIND_MAX] =
{
	{ 'd', '^', 'd', '^', 0, 0, 0, "" },  /* Blank trap */
	{ 'B', ';', 'B', ';', 0, 0,  /* Loose rock -- has special-case code */
	  TRAP_NO_DISARM | TRAP_PLAY, "loose rock" },
	{ 'y', ';', 'y', ';', 0, 0,
	  TRAP_VISIBLE | TRAP_EASY_DISARM | TRAP_KILL, "glyph of warding" },
	{ 'd', '^', 'd', '^', 0, 0, 0, "" },
	{ 'd', '^', 'd', '^', 0, 0, 0, "" },

	{ 'y', '^', 'y', '^', 0, 0,
	  TRAP_VISIBLE | TRAP_EASY_DISARM | TRAP_PLAY | TRAP_KILL, "monster trap" },
	{ 'w', '^', 'w', '^', 3, 0,  /* Trap door -- has special-case code */
	  TRAP_PLAY | TRAP_NASTY, "trap door" },
	{ 'u', '^', 'u', '^', 2, 0,
	  TRAP_PLAY | TRAP_NASTY, "pit" },
	{ 'r', '^', 'r', '^', 6, 0,
	  TRAP_PLAY | TRAP_NASTY, "dart trap" },
	{ 's', '^', 's', '^', 5, 0,
	  TRAP_PLAY | TRAP_NASTY, "discolored spot" },
	{ 'g', '^', 'g', '^', 0, 0,
	  TRAP_PLAY | TRAP_NASTY, "gas trap" },
	{ 'o', '^', 'o', '^', 3, 0,
	  TRAP_PLAY | TRAP_NASTY, "strange rune" },
	{ 'U', '^', 'U', '^', 3, 0,
	  TRAP_PLAY | TRAP_NASTY, "strange rune" },
	{ 'v', '^', 'v', '^', 30, 0,
	  TRAP_PLAY | TRAP_NASTY, "hex" },
	{ 'b', '^', 'b', '^', 0, 0,
	  TRAP_PLAY | TRAP_NASTY, "shimmering portal" },
	{ 'W', '^', 'W', '^', 4, 0,
	  TRAP_PLAY | TRAP_NASTY, "murder hole" },
	{ 'd', '^', 'd', '^', 0, 0, 0, "" },
	{ 'd', '^', 'd', '^', 0, 0, 0, "" },
	{ 'd', '^', 'd', '^', 0, 0, 0, "" },
	{ 'd', '^', 'd', '^', 0, 0, 0, "" }
};




/*
 * Is there a specific kind of trap in this grid?
 */
bool cave_trap_specific(int y, int x, int t_idx)
{
	int i;

	/* First, check the trap marker */
	if (!cave_trap(y, x)) return (FALSE);

	/* Scan the current trap list */
	for (i = 0; i < t_max; i++)
	{
		/* Point to this trap */
		trap_type *t_ptr = &t_list[i];

		/* Find a trap in this position */
		if ((t_ptr->fy == y) && (t_ptr->fx == x))
		{
			/* We found a trap of the right kind */
			if (t_ptr->t_idx == t_idx) return (TRUE);
		}
	}

	/* Report failure */
	return (FALSE);
}

/*
 * Is there a loose rock in this grid?
 */
bool cave_loose_rock(int y, int x)
{
	/* Look for a loose rock */
	return (cave_trap_specific(y, x, TRAP_LOOSE_ROCK));
}

/*
 * Is there a glyph in this grid?
 */
bool cave_glyph(int y, int x)
{
	/* Look for a glyph */
	return (cave_trap_specific(y, x, TRAP_GLYPH));
}

/*
 * Is there a pit trap in this grid?
 */
bool cave_pit_trap(int y, int x)
{
	/* Look for a glyph */
	return (cave_trap_specific(y, x, TRAP_PIT));
}


/*
 * Check for a monster trap trap that affects monsters
 */
bool cave_monster_trap(int y, int x)
{
	/* Look for a monster trap */
	return (cave_trap_specific(y, x, TRAP_MONSTER));
}


/*
 * Determine if a trap actually exists in this grid.
 *
 * Called with vis = 0 to accept any trap, = 1 to accept only visible
 * traps, and = -1 to accept only invisible traps.
 *
 * Clear the CAVE_TRAP flag if none exist.
 */
static bool verify_trap(int y, int x, int vis)
{
	int i;
	bool trap = FALSE;

	/* Scan the current trap list */
	for (i = 0; i < t_max; i++)
	{
		/* Point to this trap */
		trap_type *t_ptr = &t_list[i];

		/* Find a trap in this position */
		if ((t_ptr->fy == y) && (t_ptr->fx == x))
		{
			/* Accept any trap */
			if (!vis) return (TRUE);

			/* Accept traps that match visibility requirements */
			if (vis == 1)
			{
				if (t_ptr->flags & (TRAP_VISIBLE)) return (TRUE);
			}

			if (vis == -1)
			{
				if (!(t_ptr->flags & (TRAP_VISIBLE))) return (TRUE);
			}

			/* Note that a trap does exist */
			trap = TRUE;
		}
	}

	/* No traps in this location. */
	if (!trap)
	{
		/* No traps */
		cave_info[y][x] &= ~(CAVE_TRAP);

		/* No reason to mark this grid, ... */
		cave_info[y][x] &= ~(CAVE_MARK);

		/* ... unless certain conditions apply */
		note_spot(y, x);
	}

	/* Report failure */
	return (FALSE);
}

/*
 * Is there a visible trap in this grid?
 */
bool cave_visible_trap(int y, int x)
{
	/* First, check the trap marker */
	if (!cave_trap(y, x)) return (FALSE);

	/* Verify trap, require that it be visible */
	return (verify_trap(y, x, 1));
}

/*
 * Is there an invisible trap in this grid?
 */
bool cave_invisible_trap(int y, int x)
{
	/* First, check the trap marker */
	if (!cave_trap(y, x)) return (FALSE);

	/* Verify trap, require that it be invisible */
	return (verify_trap(y, x, -1));
}



/*
 * Get the graphics of any trap in this grid.
 *
 * We should probably have better handling of stacked traps, but that can
 * wait until we do, in fact, have stacked traps under normal conditions.
 *
 * Note:  If an error should have occurred, and there is in fact no trap
 * in this grid, we clear the CAVE_TRAP flag and return FALSE.
 */
bool get_trap_graphics(int y, int x, byte *a, char *c, bool require_visible)
{
	int i;
	bool trap = FALSE;

	/* Scan the current trap list */
	for (i = 0; i < t_max; i++)
	{
		/* Point to this trap */
		trap_type *t_ptr = &t_list[i];

		/* Find a trap in this position */
		if ((t_ptr->fy == y) && (t_ptr->fx == x))
		{
			/* Trap is visible, or we don't care */
			if ((!require_visible) || (t_ptr->flags & (TRAP_VISIBLE)))
			{
				/* Get the graphics */
				*a = color_char_to_attr(t_kind_info[t_ptr->t_idx].x_attr);
				*c = t_kind_info[t_ptr->t_idx].x_char;

				/* We found a trap */
				return (TRUE);
			}

			/* Note that a trap does exist */
			trap = TRUE;
		}
	}

	/* We found no traps in this location.  Clear the CAVE_TRAP flag */
	if (!trap) cave_info[y][x] &= ~(CAVE_TRAP);

	/* No traps found */
	return (FALSE);
}


/*
 * Hack -- toggle on TRAP_NO_EXP markers for nearby traps.   XXX XXX
 */
void no_exp_traps(int y, int x)
{
	int d, i;

	/* Look in all directions (and underneath). */
	for (d = 0; d < 9; d++)
	{
		/* Extract adjacent location */
		int yy = y + ddy_ddd[d];
		int xx = x + ddx_ddd[d];

		/* Require a trap */
		if (cave_info[yy][xx] & (CAVE_TRAP))
		{
			/* Scan the current trap list */
			for (i = 0; i < t_max; i++)
			{
				/* Point to this trap */
				trap_type *t_ptr = &t_list[i];

				/* Find any traps in this position */
				if ((t_ptr->fy == yy) && (t_ptr->fx == xx))
				{
					/* Trap gives no exp */
					t_ptr->flags |= (TRAP_NO_EXP);
				}
			}
		}
	}
}


/*
 * Reveal some of the traps in a grid
 */
bool reveal_trap(int y, int x, int chance, bool msg, bool see_loose_rocks)
{
	int i;
	int found_trap = 0;
	int found_rock = 0;

	/* Reveal at least one trap */
	int num = 1;

	/* Allow all traps to be revealed */
	if (chance >= 100) num = 0;

	/* Check the trap marker */
	if (!cave_trap(y, x)) return (FALSE);


	/* Scan the current trap list */
	for (i = 0; i < t_max; i++)
	{
		/* Point to this trap */
		trap_type *t_ptr = &t_list[i];

		/* Find a trap in this position */
		if ((t_ptr->fy == y) && (t_ptr->fx == x))
		{
			/* Trap is invisible */
			if (!(t_ptr->flags & (TRAP_VISIBLE)))
			{
				/* Hack -- sometimes ignore loose rocks */
				if ((!see_loose_rocks) && (t_ptr->t_idx == TRAP_LOOSE_ROCK))
					continue;

				/* See the trap */
				t_ptr->flags |= (TRAP_VISIBLE);
				cave_info[y][x] |= (CAVE_MARK);

				/* We found a trap */
				found_trap++;

				/* We found a loose rock */
				if (t_ptr->t_idx == TRAP_LOOSE_ROCK) found_rock++;

				/* We have a limit on the number of traps we can find */
				if (num)
				{
					/* Use up some of the limit */
					num--;

					/* Try to extend the limit */
					if (rand_int(100) < chance) num++;

					/* Stop when we've reached the limit */
					if (num <= 0) break;
				}
			}
		}
	}

	/* We found at least one trap (or loose rock) */
	if (found_trap || found_rock)
	{
		/* We want to talk about it */
		if (msg)
		{
			if (found_rock == found_trap)
			{
				msg_print("You have found a loose rock.");
			}
			else if (found_trap == 1) msg_print("You have found a trap.");
			else msg_format("You have found %d traps.", found_trap);
		}

		/* Memorize */
		cave_info[y][x] |= (CAVE_MARK);

		/* Redraw */
		lite_spot(y, x);
	}

	/* Return TRUE if we found any traps */
	return (found_trap != 0);
}

/*
 * Count the nasty traps in this location.
 *
 * Called with vis = 0 to accept any trap, = 1 to accept only visible
 * traps, and = -1 to accept only invisible traps.
 */
int nasty_traps(int y, int x, int vis)
{
	int i, num;

	/* Scan the current trap list */
	for (num = 0, i = 0; i < t_max; i++)
	{
		/* Point to this trap */
		trap_type *t_ptr = &t_list[i];

		/* Find all traps in this position */
		if ((t_ptr->fy == y) && (t_ptr->fx == x))
		{
			/* Require that trap be capable of affecting the character */
			if (!(t_kind_info[t_ptr->t_idx].flags & (TRAP_PLAY))) continue;

			/* Require that trap be "nasty" */
			if (!(t_kind_info[t_ptr->t_idx].flags & (TRAP_NASTY))) continue;

			/* Require correct visibility */
			if (vis >= 1)
			{
				if (t_ptr->flags & (TRAP_VISIBLE)) num++;
			}
			else if (vis <= -1)
			{
				if (!(t_ptr->flags & (TRAP_VISIBLE))) num++;
			}
			else
			{
				num++;
			}
		}
	}

	/* Return the number of nasty traps */
	return (num);
}

/*
 * Get a random, but appropriate, trap for this depth.
 *
 * Accept only "nasty" traps.
 *
 * Note:  We use current character depth, not the given trap level, for
 * deciding whether trap doors can be placed.
 */
static int pick_trap(int trap_level)
{
	bool trap_is_okay = FALSE;
	int i = 0;
	int trap = -1;

	trap_kind *t_kind_ptr;

	/* Try to create a trap appropriate to the level. */
	while ((!trap_is_okay) && (i++ < 50))
	{
		/* Pick at random. */
		trap = rand_int(TRAP_KIND_MAX);

		/* Get this trap */
		t_kind_ptr = &t_kind_info[trap];

		/* Require that trap be capable of hurting the character */
		if (!(t_kind_ptr->flags & (TRAP_PLAY))) continue;

		/* Require that trap be "nasty" */
		if (!(t_kind_ptr->flags & (TRAP_NASTY))) continue;

		/* Require that trap_level not be too low */
		if (t_kind_ptr->min_depth > trap_level) continue;

		/* Assume legal until proven otherwise. */
		trap_is_okay = TRUE;

		/* Check legality. */
		switch (trap)
		{
			/* Trap doors */
			case TRAP_DOOR:
			{
				/* Hack -- no trap doors on quest levels */
				if (quest_check(p_ptr->depth)) trap_is_okay = FALSE;

				/* Hack -- no trap doors on the deepest level */
				if (p_ptr->depth >= MAX_DEPTH - 1) trap_is_okay = FALSE;

				break;
			}

			/* Pits */
			case TRAP_PIT:
			case TRAP_DART:
			case TRAP_SPOT:
			case TRAP_GAS:
			case TRAP_SUMMON:
			case TRAP_ALTER_DUNGEON:
			case TRAP_HEX:
			case TRAP_PORTAL:
			case TRAP_MURDER_HOLE:
			{
				/* No special restrictions */
				break;
			}

			/* Any other selection is not defined. */
			default:
			{
				trap_is_okay = FALSE;
				break;
			}
		}
	}

	/* Return our chosen trap */
	return (trap);
}



/*
 * Determine if a cave grid is allowed to have traps in it.
 *
 * There are some decisions here that could stand to be reviewed.
 */
bool cave_trap_allowed(int y, int x)
{
	int feat = cave_feat[y][x];
	bool wall = (cave_info[y][x] & (CAVE_WALL)) ? TRUE : FALSE;

	/*
	 * We currently forbid multiple traps in a grid under normal conditions.
	 * If this changes, various bits of code elsewhere will have to change too.
	 */
	if (cave_trap(y, x)) return (FALSE);


	/* Walls */
	if (wall)
	{
		/* Rubble is OK */
		if (feat == FEAT_RUBBLE) return (TRUE);

		/* Trees are OK */
		if (feat == FEAT_TREE) return (TRUE);
	}

	/* Non-wall */
	else
	{
		/* Quick check for floors */
		if (feat == FEAT_FLOOR) return (TRUE);

		/* Lava and water (traps in water are weird) */
		if (feat == FEAT_LAVA) return (FALSE);
		if (feat == FEAT_WATER) return (FALSE);

		/* Stairs */
		if (feat == FEAT_LESS) return (FALSE);
		if (feat == FEAT_MORE) return (FALSE);
		if (feat == FEAT_LESS2) return (FALSE);
		if (feat == FEAT_MORE2) return (FALSE);

		/* Doors */
		if (feat == FEAT_OPEN) return (FALSE);
		if (feat == FEAT_BROKEN) return (FALSE);

		/* Shop entrances */
		if ((feat >= FEAT_SHOP_HEAD) && (feat <= FEAT_SHOP_TAIL))
			return (FALSE);
	}

	/* Assume that walls are not OK and non-walls are */
	return (!wall);
}

/*
 * Make a new trap of the given type.  Return TRUE if successful.
 *
 * We choose a trap at random if the index is not legal.
 *
 * This should be the only function that places traps in the dungeon
 * except the savefile loading code.
 */
bool place_trap(int y, int x, int t_idx, int trap_level)
{
	int i;


	/* Require the correct terrain */
	if (!cave_trap_allowed(y, x)) return (FALSE);

	/* Hack -- don't use up all the trap slots during dungeon generation */
	if (!character_dungeon)
	{
		if (t_max > z_info->t_max - 50) return (FALSE);
	}

	/* We've been called with an illegal index; choose a random trap */
	if ((t_idx < 0) || (t_idx >= TRAP_KIND_MAX))
	{
		t_idx = pick_trap(trap_level);
	}

	/* Note failure */
	if (t_idx < 0) return (FALSE);


	/* Scan the entire trap list */
	for (i = 1; i < z_info->t_max; i++)
	{
		/* Point to this trap */
		trap_type *t_ptr = &t_list[i];

		/* This space is available */
		if (!t_ptr->t_idx)
		{
			/* Fill in the trap index */
			t_ptr->t_idx = t_idx;

			/* Fill in the trap details */
			t_ptr->fy = y;
			t_ptr->fx = x;

			t_ptr->hold_o_idx = 0;
			t_ptr->flags = t_kind_info[t_ptr->t_idx].flags;

			/* Adjust trap count if necessary */
			if (i + 1 > t_max) t_max = i + 1;

			/* We created a glyph */
			if (t_idx == TRAP_GLYPH) num_glyph_on_level++;

			/* We created a monster trap */
			if (t_idx == TRAP_MONSTER) num_trap_on_level++;

			/* Toggle on the trap marker */
			cave_info[y][x] |= (CAVE_TRAP);

			/* Redraw the grid */
			lite_spot(y, x);

			/* Report success */
			return (TRUE);
		}
	}

	/* No luck -- report failure */
	return (FALSE);
}


/*
 * Leave a "glyph of warding" which prevents monster movement
 */
bool warding_glyph(int y, int x)
{
	/* Limit total number of glyphs. -LM- */
	if (num_glyph_on_level >= MAX_GLYPHS)
	{
		/* Messages vary depending on where the glyph is being set */
		if ((y == p_ptr->py) && (x == p_ptr->px))
		{
		msg_print("You cannot set any more glyphs until you desanctify your existing ones.");
		}
		else
		{
			/* This could use a rethink  XXX */
			msg_print("No more glyphs can be created until you desanctify some of the existing ones.");
		}
		return (FALSE);
	}

	/* Trap (or glyph) can't be placed here */
	if (!cave_trap_allowed(y, x))
	{
		if ((y == p_ptr->py) && (x == p_ptr->px))
		{
			msg_print("You cannot place a glyph here.");
		}
		else
		{
			/* This could use a rethink  XXX */
			msg_print("(glyph could not be placed)");
		}
		return (FALSE);
	}

	/* Create a glyph, increment glyph count */
	place_trap(y, x, TRAP_GLYPH, 0);

	/* Warning. */
	if (num_glyph_on_level >= MAX_GLYPHS)
	{
		if ((y == p_ptr->py) && (x == p_ptr->px))
		{
			msg_print("You have now reached your glyph limit.  In order to set more, desanctify some.");
		}
		else
		{
			/* This could use a rethink  XXX */
			msg_print("(glyph limit now reached)");
		}
	}

	/* Success */
	return (TRUE);
}


/*
 * Delete/Remove all the traps when the player leaves the level
 */
void wipe_t_list(void)
{
	int i;

	/* Delete all the traps */
	for (i = t_max - 1; i >= 0; i--)
	{
		trap_type *t_ptr = &t_list[i];

		/* Wipe the trap */
		WIPE(t_ptr, trap_type);
	}

	/* Reset "t_max" */
	t_max = 0;

	/* Reset the number of glyphs on the level. */
	num_glyph_on_level = 0;

	/* Reset the number of monster traps on the level. */
	num_trap_on_level = 0;
}



/*
 * Remove a trap
 */
static void remove_trap_aux(trap_type *t_ptr, int y, int x)
{
	object_type *i_ptr;
	object_type *o_ptr;
	object_type forge;

	/* We are deleting a glyph */
	if (t_ptr->t_idx == TRAP_GLYPH) num_glyph_on_level--;

	/* We are deleting a monster trap */
	if (t_ptr->t_idx == TRAP_MONSTER) num_trap_on_level--;

	/* Trap drops all of its objects */
	while (t_ptr->hold_o_idx)
	{
		/* Get this object */
		o_ptr = &o_list[t_ptr->hold_o_idx];

		/* Get local object */
		i_ptr = &forge;

		/* Wipe the copy */
		object_wipe(i_ptr);

		/* Copy the object */
		object_copy(i_ptr, o_ptr);

		/* Delete the original object */
		delete_object_idx(t_ptr->hold_o_idx);

		/* Drop the copy */
		drop_near(i_ptr, -1, y, x);
	}

	/* Wipe the trap */
	(void)WIPE(t_ptr, trap_type);
}

/*
 * Remove traps.
 *
 * If called with t_idx < 0, will remove all traps in the location given.
 * Otherwise, will remove the trap with the given index.
 *
 * Return TRUE if ny traps now exist in this grid.
 */
bool remove_trap(int y, int x, int t_idx)
{
	int i;
	bool trap;

	/* Called with a specific index */
	if (t_idx >= 0)
	{
		/* Point to this trap */
		trap_type *t_ptr = &t_list[t_idx];

		/* Remove it */
		remove_trap_aux(t_ptr, y, x);

		/* Note when trap list actually gets shorter */
		if (t_idx == t_max - 1) t_max--;
	}

	/* No specific index -- remove all traps here */
	else
	{
		/* Scan the current trap list (backwards) */
		for (i = t_max - 1; i >= 0; i--)
		{
			/* Point to this trap */
			trap_type *t_ptr = &t_list[i];

			/* Find all traps in this position */
			if ((t_ptr->fy == y) && (t_ptr->fx == x))
			{
				/* Remove it */
				remove_trap_aux(t_ptr, y, x);

				/* Note when trap list actually gets shorter */
				if (i == t_max - 1) t_max--;
			}
		}
	}

	/* Refresh grids that the character can see */
	if (player_can_see_bold(y, x)) lite_spot(y, x);

	/* Verify traps (remove marker if appropriate) */
	trap = verify_trap(y, x, 0);

	/* Report whether any traps exist in this grid */
	return (trap);
}

/*
 * Remove all traps of a specific kind from a location.
 */
void remove_trap_kind(int y, int x, int t_idx)
{
	int i;

	/* Scan the current trap list */
	for (i = 0; i < t_max; i++)
	{
		/* Point to this trap */
		trap_type *t_ptr = &t_list[i];

		/* Find a trap in this position */
		if ((t_ptr->fy == y) && (t_ptr->fx == x))
		{
			/* Require that it be of this type */
			if (t_ptr->t_idx == t_idx) (void)remove_trap(y, x, i);
		}
	}
}

/* Convert letters to indexes */
static int trap_indexes[99];

/*
 * Display traps in a list.
 *
 * Remember have many traps were shown, and what their indexes were.
 */
static void display_traps(int y, int x, int *count)
{
	int i;

	/* Display the individual traps */
	for (*count = 0, i = 0; i < t_max; i++)
	{
		/* Point to this trap */
		trap_type *t_ptr = &t_list[i];

		/* Find a trap in this position */
		if ((t_ptr->fy == y) && (t_ptr->fx == x))
		{
			/* Trap must be visible */
			if (!(t_ptr->flags & (TRAP_VISIBLE))) continue;

			/* Display this trap */
			prt(format(" %c) %.30s", index_chars_lower[*count],
				t_kind_info[t_ptr->t_idx].name), *count + 1, Term->wid - 34);

			/* Remember this trap */
			trap_indexes[(*count)++] = i;
		}
	}

	/* Print a blank line below the list of traps */
	prt("                                  ", i, Term->wid - 34);
}


/*
 * Choose a trap.  If there is more than one trap in this grid, we display
 * a menu, and let the player choose among them.
 *
 * Possible enhancements to this code include remembering which trap was
 * chosen last so that command repeats can work correctly on grids with
 * multiple traps.
 */
bool get_trap(int y, int x, int *idx)
{
	int i;
	int count;
	int choice;
	char ch;

	/* Require that grid be legal */
	if (!in_bounds_fully(y, x)) return (FALSE);

	/* Look for the trap marker */
	if (!cave_trap(y, x)) return (FALSE);

	/* Scan the current trap list */
	for (count = 0, i = 0; i < t_max; i++)
	{
		/* Point to this trap */
		trap_type *t_ptr = &t_list[i];

		/* Find a trap in this position */
		if ((t_ptr->fy == y) && (t_ptr->fx == x))
		{
			/* Trap must be visible */
			if (!(t_ptr->flags & (TRAP_VISIBLE))) continue;

			/* Count all traps */
			count++;

			/* Remember last trap index */
			*idx = i;
		}
	}

	/* We have no visible traps */
	if (!count) return (FALSE);

	/* We have one trap (usual case) */
	if (count == 1) return (TRUE);

	/* We have more than one trap */
	else
	{
		/* Save the screen */
		screen_save();

		/* Display traps */
		display_traps(y, x, &count);

		/* Interact */
		while (TRUE)
		{
			/* Ask player to choose a trap, allow escape */
			if (!get_com(format("Choose which trap (%c-%c)?",
				index_chars_lower[0], index_chars_lower[count - 1]), &ch))
			{
				/* Restore the screen */
				screen_load();

				return (FALSE);
			}

			/* Convert command to a choice (squash case) */
			choice = get_index(ch, TRUE);

			/* If choice is legal... */
			if ((choice >= 0) && (choice < count))
			{
				/* Select trap */
				*idx = trap_indexes[choice];

				/* Restore the screen */
				screen_load();

				/* Report success */
				return (TRUE);
			}

			/* Otherwise... */
			else
			{
				/* Be helpful */
				msg_print("Please press a letter to select a trap, or ESC to cancel");

				/* Allow user to try again */
			}
		}
	}

	/* Return */
	return (FALSE);
}


/*
 * Determine if a trap is disarmable
 */
static bool is_disarmable_trap(trap_type *t_ptr)
{
	if (!(t_ptr->flags & (TRAP_VISIBLE))) return (FALSE);
	return (!(t_kind_info[t_ptr->t_idx].flags & (TRAP_NO_DISARM)));
}

/*
 * Determine if a grid contains disarmable traps.
 */
bool has_disarmable_trap(int y, int x)
{
	int i;

	/* First, check the trap marker */
	if (!cave_trap(y, x)) return (FALSE);

	/* Scan the current trap list */
	for (i = 0; i < t_max; i++)
	{
		/* Point to this trap */
		trap_type *t_ptr = &t_list[i];

		/* Find all traps in this position */
		if ((t_ptr->fy == y) && (t_ptr->fx == x))
		{
			/* Accept first disarmable trap */
			if (is_disarmable_trap(t_ptr)) return (TRUE);
		}
	}

	/* No disarmable traps found */
	return (FALSE);
}

/*
 * Attempt to magically disarm traps using magic.
 */
bool magic_disarm(int y, int x, int chance)
{
	int i;
	bool obvious = FALSE;

	/* Scan the current trap list */
	for (i = 0; i < t_max; i++)
	{
		/* Point to this trap */
		trap_type *t_ptr = &t_list[i];

		/* Find a trap in this position */
		if ((t_ptr->fy == y) && (t_ptr->fx == x))
		{
			/* Ignore glyphs, monster traps, and loose rocks */
			if ((t_ptr->t_idx == TRAP_GLYPH) ||
			    (t_ptr->t_idx == TRAP_MONSTER) ||
			    (t_ptr->t_idx == TRAP_LOOSE_ROCK))
			{
				continue;
			}

			/* Attempt to disarm the trap */
			if (rand_int(100) < chance)
			{
				/* Check line of sight */
				if (player_has_los_bold(y, x))
				{
					msg_print("There is a bright flash of light.");
					obvious = TRUE;
				}

				/* Kill the trap */
				remove_trap(y, x, i);
			}

			/* Some chance of setting off the trap. */
			else
			{
				msg_print("Your magic was too weak!");
				(void)hit_trap(-1, y, x);
				obvious = TRUE;
			}
		}
	}

	/* Return whether anything was noticed */
	return (obvious);
}



/*
 * Interact with a monster trap.
 *
 * Return 0 when we did nothing, 1 when we did something, and -1 when we
 * want to disarm the trap.
 */
int load_trap(int y, int x)
{
	trap_type *t_ptr;
	int i, idx, item;
	int action = 0;

	char o_name[120];
	char ch;
	object_type *o_ptr;
	object_type *i_ptr;
	object_type object_type_body;

	s16b this_o_idx, next_o_idx = 0;


	/* Scan the current trap list */
	for (idx = 1; idx < t_max; idx++)
	{
		/* Point to this trap */
		t_ptr = &t_list[idx];

		/* Find a trap in this position */
		if ((t_ptr->fy == y) && (t_ptr->fx == x))
		{
			break;
		}
	}

	/* No monster trap in this location */
	if (idx == t_max) return (FALSE);

	/* Point to this trap (again) */
	t_ptr = &t_list[idx];

	/* Save the screen */
	screen_save();


	/* Interact with the trap */
	while (TRUE)
	{
		bool no_objects = FALSE;

		/* List all the items this trap contains */
		for (i = 1, this_o_idx = t_ptr->hold_o_idx; this_o_idx;
			  this_o_idx = next_o_idx, i++)
		{
			/* Get the object */
			o_ptr = &o_list[this_o_idx];

			/* Get the next object */
			next_o_idx = o_ptr->next_o_idx;

			/* Get the object name */
			object_desc(o_name, o_ptr, TRUE, 3);

			/* Print object */
			prt(format("%s", o_name), i, 0);
		}

		/* Print an indication that no objects exist */
		if (i == 1)
		{
			prt("                  (This trap contains no objects yet)", i++, 0);
			no_objects = TRUE;
		}

		/* Print a blank line */
		prt("", i++, 0);

		/* Print some helpful notes */
		move_cursor(i, 0);
		c_roff(TERM_L_BLUE, "     Traps may be loaded with a single melee weapon, a single missile launcher, any number of wands, rods, or staffs, any number of potions or scrolls, or any number of weapons or objects that you wish to be thrown.\n     Traps can usually only contain one type of object.  However, if you have loaded it with a missile weapon, you also need to supply ammunition.  You can also have any number of boulders.\n ", 0, 80);

		/* Instructions (similar to stores, etc.) */
		if (no_objects)
		{
			prt("Press 'd' to drop objects, 'k' to disarm the trap, or ESC to exit.", 0, 0);
		}
		else
		{
			prt("Press 'd' to drop objects, 'g' to get objects, or ESC to exit.", 0, 0);
		}


		/* Get a command */
		ch = inkey();

		/* Blank the message line */
		prt("", 0, 0);

		/* Disarm the trap */
		if ((ch == 'k') || (ch == 'K') || (ch == '+'))
		{
			/* No objects -- ask for disarm */
			if (no_objects)
			{
				/* Restore screen, exit */
				screen_load();
				return (-1);
			}

			/* Ask to empty the trap first */
			else
			{
				msg_print("You need to empty the trap before you can disarm it.");
				message_flush();
			}
		}

		/* Load new objects */
		else if ((ch == 'd') || (ch == 'D'))
		{
			/* Get an item */
			cptr q = "Load which item?";
			cptr s = "You have nothing to load the trap with.";
			if (!get_item(&item, q, s, (USE_INVEN | USE_EQUIP | USE_FLOOR)))
				continue;
			item_to_object(o_ptr, item);


			/* Forbid double-loading with any object other than ammo */
			if ((t_ptr->hold_o_idx) && (!is_missile(o_ptr)) &&
			    (o_ptr->tval != TV_JUNK) && (o_ptr->sval != SV_BOULDER))
			{
				/* Assume no combination possible */
				bool can_combine = FALSE;

				/* See if this item will combine with others already loaded */
				for (this_o_idx = t_ptr->hold_o_idx; this_o_idx;
					  this_o_idx = next_o_idx)
				{
					/* Get the object */
					object_type *j_ptr = &o_list[this_o_idx];

					/* Check for combination */
					if (object_similar(j_ptr, o_ptr)) can_combine = TRUE;
				}

				/* Require combination */
				if (!can_combine)
				{
					msg_print("Traps can usually contain only one type of object.");
					message_flush();
					continue;
				}
			}

			/* Determine how many items we wish to load the trap with */
			if (o_ptr->number > 1)
			{
				i = get_quantity(format("How many items (0-%d)?", o_ptr->number),
				                 o_ptr->number);

				/* Cancel */
				if (!i)	continue;
			}
			else
			{
				i = 1;
			}

			/* Get local object */
			i_ptr = &object_type_body;

			/* Copy the item being loaded */
			object_copy(i_ptr, o_ptr);

			/* Adjust quantity */
			i_ptr->number = i;

			/* Hack -- Adjust charges and timeouts */
			distribute_charges(o_ptr, i_ptr, i_ptr->number);

			/* Reduce (but do not describe) inventory */
			if (item >= 0)
			{
				inven_item_increase(item, -i);
				inven_item_optimize(item);
			}
			else
			{
				floor_item_increase(0 - item, -i);
				floor_item_optimize(0 - item);
			}

			/* See if this item will combine with others already loaded */
			for (this_o_idx = t_ptr->hold_o_idx; this_o_idx;
				  this_o_idx = next_o_idx)
			{
				/* Get the object */
				o_ptr = &o_list[this_o_idx];

				/* Check for combination */
				if (object_similar(o_ptr, i_ptr))
				{
					/* Combine the items */
					object_absorb(o_ptr, i_ptr);

					/* Done */
					i_ptr->number = 0;
					break;
				}

				/* Get the next object */
				next_o_idx = o_ptr->next_o_idx;
			}

			/* Load the trap with a new object */
			if (i_ptr->number)
			{
				/* Make an object */
				int o_idx = o_pop();

				/* Get the object */
				object_type *j_ptr = &o_list[o_idx];

				/* Copy object */
				object_copy(j_ptr, i_ptr);

				/* Forget location */
				j_ptr->iy = j_ptr->ix = 0;

				/* Object will be held by the trap */
				j_ptr->held_m_idx = -(idx);

				/* Object goes to the bottom of the pile */
				j_ptr->next_o_idx = 0;


				/* Trap has no objects yet */
				if (!t_ptr->hold_o_idx)
				{
					/* Link the trap to the object */
					t_ptr->hold_o_idx = o_idx;
				}

				/* Trap has objects */
				else
				{
					/* Scan through the list of objects */
					for (this_o_idx = t_ptr->hold_o_idx;; this_o_idx = next_o_idx)
					{
						/* Get the object */
						o_ptr = &o_list[this_o_idx];

						/* This object is at the bottom of the pile */
						if (!o_ptr->next_o_idx)
						{
							/* Link it to the new object */
							o_ptr->next_o_idx = o_idx;

							/* Done */
							break;
						}

						/* Get the next object */
						next_o_idx = o_ptr->next_o_idx;
					}
				}
			}

			/* We did something */
			action = TRUE;

			/* Object loaded -- await next command */
			continue;
		}

		/* Remove existing objects */
		else if ((ch == 'g') || (ch == 'G'))
		{
			/* Note that no objects exist */
			if (!t_ptr->hold_o_idx)
			{
				msg_print("The trap had no objects to remove.");
				message_flush();
			}

			/* Remove the first object (we could get more sophisticated here) */
			else
			{
				/* Get this object */
				o_ptr = &o_list[t_ptr->hold_o_idx];

				/* Trap is now linked to the second object (if any) */
				t_ptr->hold_o_idx = o_ptr->next_o_idx;

				/* Give the object to the character */
				give_object(o_ptr);

				/* We did something */
				action = TRUE;
			}

			/* Object removed -- await next command */
			continue;
		}

		/* Confirm (or escape) */
		else if ((ch == '\r') || (ch == '\n') || (ch == ESCAPE)) break;

		/* Error */
		else
		{
			msg_print("Unknown command.");
			message_flush();
		}
	}

	/* Restore the screen */
	screen_load();

	/* Return "anything done" */
	return (action);
}



/*
 * Trap attacks in melee, with missile weapons, or by throwing objects.
 *
 * We use a somewhat simplified (and bastardized) version of the code in
 * "attack.c".
 */
static void trap_combat(int mode, int y, int x, object_type *o_ptr,
	object_type *i_ptr, int power)
{
	int i, tmp;
	int dice;
	long die_average, temp, sides;
	int total_deadliness, chance;
	long mult = 100L;
	int damage;

	/* Assume one attack */
	int attacks = 1;

	bool fear = FALSE;

	u32b f1, f2, f3;

	monster_race *r_ptr;
	monster_type *m_ptr;

	int m_idx;

	/* Get the index of the creature in this grid */
	m_idx = cave_m_idx[y][x];

	/* Require that a monster (not a character) be present */
	if (m_idx <= 0) return;

	/* Get the monster in this grid */
	m_ptr = &m_list[m_idx];

	/* Get the monster race */
	r_ptr = &r_info[m_ptr->r_idx];


	/* Using melee */
	if (mode == 1)
	{
		/* Require a valid melee weapon */
		if (!is_melee_weapon(o_ptr)) return;

		total_deadliness = o_ptr->to_d;
		chance = (BTH_PLUS_ADJ * o_ptr->to_h) + power;
	}

	/* Using archery (no accuracy penalty for point-blank range) */
	else if (mode == 2)
	{
		/* Require a valid missile launcher */
		if (i_ptr->tval != TV_BOW) return;

		/* Require valid missiles */
		if (!is_missile(o_ptr)) return;

		total_deadliness = i_ptr->to_d + o_ptr->to_d;
		chance = (BTH_PLUS_ADJ * (i_ptr->to_h + o_ptr->to_h)) + power;
	}

	/* Throwing the object (no penalty for point-blank range) */
	else
	{
		total_deadliness = o_ptr->to_d;
		chance = (BTH_PLUS_ADJ * o_ptr->to_h) + power;
	}

	/* Using archery */
	if (mode == 2)
	{
		/* Hack -- transfer launcher attributes to the missile */
		transfer_attributes_to_missile(i_ptr, o_ptr);
	}

	/* Get weapon, missile, or thrown object flags */
	object_flags(o_ptr, &f1, &f2, &f3);

	/* Base damage dice */
	dice = o_ptr->dd;

	/* We do not allow for critical hits (see trap power multiplier) */

	/* Get the average value of a single damage die. (x10 inflation) */
	die_average = (10 * (o_ptr->ds + 1)) / 2;

	/* Get deadliness from object (100x inflation) */
	apply_deadliness(&mult, total_deadliness);

	/* Deflate multiplier */
	mult = div_round(mult, 100);

	/* Apply trap power multiplier (+20% - +180%) */
	mult += deadliness_conversion[power / 2];

	/* Traps can get multiple melee blows */
	if (mode == 1)
	{
		/* Heavy weapons get fewer blows  XXX */
		int divisor = 10 + (o_ptr->weight / 12);

		/* More powerful traps get more blows */
		attacks = 1 + div_round(power, divisor);

		/* Limit blows */
		if (attacks > 6) attacks = 6;

		/* Apply extra blows */
		attacks += get_object_pval(o_ptr, TR_PVAL_BLOWS);
	}

	/* Using archery */
	else if (mode == 2)
	{
		/* Apply missile weapon multiplier (200% - 500%) */
		mult *= get_object_pval(i_ptr, TR_PVAL_MIGHT);

		/* Hack -- handle triple crossbows  XXX XXX */
		if (o_ptr->sval == SV_TRIPLE_XBOW) mult += 2;

		/* Calculate shooting speed (110% - ~400%) */
		tmp = 100 + (get_object_pval(i_ptr, TR_PVAL_SHOTS) * 50) + power;

		/* Translate into attacks */
		attacks = MAX(1, div_round(tmp, 100));
	}

	/* Throwing */
	else
	{
		/* We are throwing a weapon designed for such use (200% - 1265%) */
		if (f1 & (TR1_THROWING))
		{
			mult += 40 + (6 * power);

			/* We are throwing a perfectly balanced weapon */
			if (f1 & (TR1_PERFECT_BALANCE)) mult += mult / 2;
		}
	}


	/* Apply multiplier to die average (100x inflation) */
	die_average *= mult;

	/* Convert die average to die sides. */
	temp = (2 * die_average) - 1000;

	/* Calculate the actual number of sides to each die. */
	sides = div_round(temp, 1000);


	/* Always have at least one attack */
	if (attacks < 1) attacks = 1;

	/* Use one attack after another */
	for (damage = 0, i = 0; i < attacks; i++)
	{
		/* Check for hit */
		if (!test_hit_combat(chance, r_ptr->ac, TRUE)) continue;

		/* Roll for damage */
		tmp = damroll(dice, (s16b)sides);

		/* Adjust damage for slays, brands, resists. */
		adjust_dam(&tmp, o_ptr, m_ptr, TRUE);

		/* Add to total */
		damage += tmp;
	}

	/* Monster gets hit (note that we give experience for trap kills) */
	if (mon_take_hit(m_idx, -1, (s16b)damage, &fear, NULL)) return;


	/* Take note of fear */
	if (fear && m_ptr->ml)
	{
		char m_name[80];

		/* Sound */
		sound(SOUND_FLEE);

		/* Get "the monster" or "it" */
		monster_desc(m_name, m_ptr, 0);

		/* Message */
		msg_format("%^s flees in terror!", m_name);
	}

	/* Monster is not visible, but is nearby */
	else if ((!m_ptr->ml) && (m_ptr->cdis <= 10) && (damage > 0))
	{
		/* Nice messages, but not always appropriate  XXX */
		if (player_has_los_bold(y, x))
			msg_print("You hear moaning.");
		else
			msg_print("You hear anguished yells.");
	}
}


/*
 * I thought traps only affected players!  Unfair!
 * -LM-
 *
 * Rogues may set traps for monsters.  They can be fairly deadly, but
 * monsters can also sometimes disarm, smash, or fly over them.
 *
 * Traps need objects to do damage; damage depends partly on object quality
 * and partly on burglary skill.  The relative importance of these two
 * factors varies greatly; this may cause balance problems.  XXX XXX
 *
 * Traps almost always give experience for killed monsters.
 *
 * Note that skilled burglars can set up traps to use magical devices that
 * he cannot understand, wield weapons that he cannot lift, and fire
 * missiles with deadly accuracy regardless of combat skill.  All of this
 * is deliberate.
 */
static void hit_monster_trap(int who, int y, int x, int t_idx)
{
	monster_type *m_ptr = &m_list[who];
	monster_race *r_ptr = &r_info[m_ptr->r_idx];
	monster_lore *l_ptr = &l_list[m_ptr->r_idx];
	char m_name[80];
	char o_name[80];

	trap_type *t_ptr = &t_list[t_idx];

	object_type *o_ptr;
	object_type *i_ptr;
	object_type *j_ptr;
	object_type object_type_body;
	s16b this_o_idx, next_o_idx = 0;

	bool do_throw = FALSE;
	bool kill_trap = FALSE;

	bool ident = FALSE;

	int skill = get_skill(S_BURGLARY, 0, 100);
	int power;


	/* Get the monster name */
	monster_desc(m_name, m_ptr, 0);

	/* Ghosts can usually avoid traps entirely.  XXX XXX */
	if ((r_ptr->flags2 & (RF2_PASS_WALL)) && (!one_in_(4)))
	{
		/* Message */
		if (m_ptr->ml) msg_format("%^s floats over your trap.", m_name);

		return;
	}

	/* Evasive monsters can usually avoid traps entirely. */
	if ((r_ptr->flags2 & (RF2_EVASIVE)) && (!one_in_(3)))
	{
		if (m_ptr->ml)
		{
			/* Message */
			msg_format("%^s dodges your trap.", m_name);

			/* Note that monster is evasive */
			l_ptr->flags2 |= (RF2_EVASIVE);
		}

		return;
	}


	/* Traps can sometimes be disarmed */
	if ((r_ptr->flags2 & (RF2_SMART)) && (!m_ptr->confused))
	{
		int chance = 6;
		if (m_ptr->stunned) chance = 10;

		if (one_in_(chance))
		{
			/* Messages */
			if (m_ptr->ml)
			{
				msg_format("%^s finds your trap and disarms it.", m_name);
			}
			else if (distance(y, x, p_ptr->py, p_ptr->px) < 12)
			{
				msg_print("You hear a click.");
			}

			kill_trap = TRUE;
		}
	}

	/* Any non-ghostly monster can sometimes smash traps */
	else if (!(r_ptr->flags2 & (RF2_PASS_WALL)))
	{
		int chance = 15;
		if (m_ptr->stunned) chance += 5;
		if (m_ptr->confused) chance += 15;

		if (one_in_(chance))
		{
			/* Messages */
			if (m_ptr->ml)
			{
				msg_format("%^s finds your trap and smashes it.", m_name);
			}
			else if (distance(y, x, p_ptr->py, p_ptr->px) < 25)
			{
				msg_print("You hear smashing noises.");
			}

			kill_trap = TRUE;
		}
	}

	/* Kill the trap */
	if (kill_trap)
	{
		/* Delete the trap */
		remove_trap(y, x, t_idx);

		/* Return */
		return;
	}


	/* Trap has no objects to use */
	if (!t_ptr->hold_o_idx) return;


	/* Power goes up with skill (ranging from 10 to 136) */
	power = skill;

	/* High-level burglars get extra bonuses */
	if (skill > 70) power += (skill - 70) / 2;
	if (skill > 85) power += (skill - 85);
	if (skill > 94) power += (skill - 94);


	/* Practice the burglary skill */
	skill_being_used = S_BURGLARY;


	/* Player is in line of sight */
	if (player_has_los_bold(y, x))
	{
		/* Acquire the monster name/poss */
		if (m_ptr->ml) monster_desc(m_name, m_ptr, 0);

		/* Default name */
		else strcpy(m_name, "Something");

		msg_format("%^s sets off your cunning trap!", m_name);
	}


	/* Get this trap's first object */
	o_ptr = &o_list[t_ptr->hold_o_idx];


	/* Object is a melee weapon */
	if (is_melee_weapon(o_ptr))
	{
		u32b f[4];

		/* Get object attributes */
		object_flags(o_ptr, &f[1], &f[2], &f[3]);

		/* Throw multiple melee weapons */
		if (o_ptr->number > 1) do_throw = TRUE;

		/* Throw anything that's perfectly balanced (not just throwing) */
		else if (f[1] & (TR1_PERFECT_BALANCE)) do_throw = TRUE;

		/* Fight with anything else */
		if (!do_throw)
		{
			/* Engage in combat (type 1 = melee) */
			trap_combat(1, y, x, o_ptr, o_ptr, power);
		}
	}

	/* Object is a missile weapon */
	else if (o_ptr->tval == TV_BOW)
	{
		/* Scan the trap's objects, use the first stack of ammo */
		for (this_o_idx = o_ptr->next_o_idx; next_o_idx;
		     this_o_idx = next_o_idx)
		{
			/* Get the object */
			i_ptr = &o_list[this_o_idx];

			/* Is this object ammunition? */
			if (is_missile(i_ptr))
			{
				/* Is this object the right sort of ammunition?  XXX XXX */
				if (((o_ptr->sval == SV_SLING) && (i_ptr->tval == TV_SHOT)) ||
				    (((o_ptr->sval == SV_SHORT_BOW) ||
				     (o_ptr->sval == SV_LONG_BOW)) && (i_ptr->tval == TV_ARROW)) ||
				    (((o_ptr->sval == SV_LIGHT_XBOW) ||
				     (o_ptr->sval == SV_HEAVY_XBOW)) && (i_ptr->tval == TV_BOLT)))
				{
					/* Engage in combat (type 2 = archery) */
					trap_combat(2, y, x, i_ptr, o_ptr, power);

					/* Get local object */
					j_ptr = &object_type_body;

					/* Copy the missiles */
					object_copy(j_ptr, i_ptr);

					/* Set quantity of new object to 1 */
					j_ptr->number = 1;

					/* Drop the fired object */
					drop_near(j_ptr, -1, y, x);

					/* Delete one object from the trap */
					i_ptr->number--;

					/* Delete this stack of objects from the trap */
					if (!i_ptr->number)
					{
						/* Trap is now linked to the second object (if any) */
						t_ptr->hold_o_idx = i_ptr->next_o_idx;

						/* Wipe the object */
						object_wipe(i_ptr);
					}
				}
			}

			/* Get the next object */
			next_o_idx = i_ptr->next_o_idx;
		}
	}

	/* Object is a potion */
	else if (o_ptr->tval == TV_POTION)
	{
		/* Potion explodes (special case of character = -2) */
		ident = potion_smash_effect(-2, y, x, o_ptr);

		/* An identification was made */
		if ((ident) && (!object_aware_p(o_ptr)))
		{
			/* Learn about the objects, do not gain experience */
			object_aware(o_ptr);

			/* Describe the object */
			strip_name(o_name, o_ptr->k_idx);

			/* Message */
			msg_format("That was a Potion of %s that just exploded.", o_name);
		}


		/* Remove one potion */
		o_ptr->number--;

		/* Remove the stack of potions */
		if (!o_ptr->number)
		{
			/* Trap is now linked to the second object (if any) */
			t_ptr->hold_o_idx = o_ptr->next_o_idx;

			/* Wipe the object */
			object_wipe(o_ptr);
		}
	}

	/* Object is a scroll */
	else if (o_ptr->tval == TV_SCROLL)
	{
		/* Read the scroll */
		int read = scroll_read_effect(-1, y, x, o_ptr);

		/* The scroll was read */
		if (read)
		{
			/* An identification was made */
			if ((read == 1) && (!object_aware_p(o_ptr)))
			{
				/* Learn about the objects, do not gain experience */
				object_aware(o_ptr);

				/* Describe the object */
				strip_name(o_name, o_ptr->k_idx);

				/* Message */
				msg_format("That was a Scroll of %s that just activated.", o_name);
			}

			/* Remove one scroll */
			o_ptr->number--;

			/* Remove the stack of scrolls */
			if (!o_ptr->number)
			{
				/* Trap is now linked to the second object (if any) */
				t_ptr->hold_o_idx = o_ptr->next_o_idx;

				/* Wipe the object */
				object_wipe(o_ptr);
			}
		}
	}

	/* Object is a magical device */
	else if (is_magical_device(o_ptr))
	{
		/* Use the magical device */
		int notice = device_use_effect(-1, power, y, x, o_ptr);

		/* An identification was made */
		if ((notice) && (!object_aware_p(o_ptr)))
		{
			cptr tval_desc = "Staff";
			if (o_ptr->tval == TV_WAND) tval_desc = "Wand";
			if (o_ptr->tval == TV_ROD) tval_desc = "Rod";

			/* Learn about the objects, do not gain experience */
			object_aware(o_ptr);

			/* Describe the object */
			strip_name(o_name, o_ptr->k_idx);

			/* Message */
			msg_format("The trap appears to be using a %s of %s.", o_name);
		}
	}

	/* Object is anything else */
	else
	{
		/* Throw it */
		do_throw = TRUE;
	}

	/* Throw an object if requested */
	if (do_throw)
	{
		/* Get local object */
		i_ptr = &object_type_body;

		/* Copy the thrown object */
		object_copy(i_ptr, o_ptr);

		/* Set quantity of new object to 1 */
		i_ptr->number = 1;

		/* Engage in combat (type 3 = throwing) */
		trap_combat(3, y, x, o_ptr, o_ptr, power);

		/* Drop the thrown object */
		drop_near(i_ptr, -1, y, x);

		/* Delete one object from the trap */
		o_ptr->number--;

		/* Delete this stack of objects from the trap */
		if (!o_ptr->number)
		{
			/* Trap is now linked to the second object (if any) */
			t_ptr->hold_o_idx = o_ptr->next_o_idx;

			/* Wipe the object */
			object_wipe(o_ptr);
		}
	}

	/* Practice no special skill */
	skill_being_used = S_NOSKILL;


	/* Note that objects or the trap itself may be destroyed at this point. */
}




/*
 * Determine if a trap affects the character.
 * Match trap power against player dodging ability, but never hit more than
 * 95% of the time or hit less than 5% of the time.
 */
static int check_trap_hit(int power)
{
	int percent = 0;

	/* Power competes against armour */
	if (power)
	{
		percent = div_round(100 * (power - dodging_ability(100)), power);
	}

	/* Minimum of 5% chance to hit */
	if (percent < 5) percent = 5;

	/* Maximum of 95% chance to hit */
	if (percent > 95) percent = 95;


	/*
	 * This function is called from the trap attack code, which generally
	 * uses the simple RNG.   We temporarily switch over to the complex
	 * RNG for true randomness.
	 */
	Rand_quick = FALSE;

	/* Return hit or miss */
	return (rand_int(100) < percent);
}


/*
 * Handle hitting a real trap.
 *
 * Rewritten in Oangband to allow a greater variety of traps, with
 * effects controlled by dungeon level.
 * To allow a trap to choose one of a variety of effects consistently,
 * the quick RNG is often used, and trap location input as a seed value.
 */
static bool hit_trap_aux(int who, int y, int x, int t_idx)
{
	trap_type *t_ptr = &t_list[t_idx];

	int i, num;
	int dam = 0;

	/* Assume trap actually does go off */
	bool flag = (who < 0);

	int nastiness, selection;

	/* Use the "simple" RNG to insure that traps are consistent. */
	Rand_quick = TRUE;

	/* Hack -- Use the coordinates of the trap to seed the RNG. */
	Rand_value = GRID(t_ptr->fy, t_ptr->fx);

	/* Disturb the player */
	if (who < 0) disturb(0, 0);


	/* Analyze */
	switch (t_ptr->t_idx)
	{
		/* Loose rock */
		case TRAP_LOOSE_ROCK:
		{
			/* Lift the rock... */
			remove_trap(y, x, t_idx);

			/* ... and peek inside */
			if (TRUE)
			{
				object_type *i_ptr;
				object_type forge;

				/* Get local object */
				i_ptr = &forge;

				/* Make the object */
				if (make_object(i_ptr, FALSE, FALSE, FALSE))
				{
					char o_name[80];

					/* Coolness */
					msg_print("Hmm.  There was something under this rock.");

					/* See the object */
					if (player_can_see_bold(y, x))
					{
						/* Describe object (briefly) */
						object_desc(o_name, i_ptr, TRUE, 0);

						/* Message */
						msg_format("You see %s.", o_name);
					}

					/* Place the object (pretend object was dropped) */
					drop_near(i_ptr, 0, y, x);
				}
				else
				{
					/* Aw shucks */
					if (!p_ptr->blind)
						msg_print("There was nothing under this rock.");
				}
			}

			/* Delete the trap */
			remove_trap(y, x, t_idx);

			break;
		}

		/* Glyph */
		case TRAP_GLYPH:
		{
			/* Nothing -- monsters currently can't enter them */
			flag = FALSE;
			break;
		}

		/* Burglar's monster trap */
		case TRAP_MONSTER:
		{
			if (who < 0)
			{
				msg_print("You inspect your cunning trap.");
				flag = FALSE;
			}

			/* Attack monsters */
			else if (who > 0)
			{
				/* Truly random */
				Rand_quick = FALSE;

				hit_monster_trap(who, y, x, t_idx);
				flag = TRUE;

				Rand_quick = TRUE;
			}
			break;
		}

		/* Trap door */
		case TRAP_DOOR:
		{
			if (p_ptr->ffall)
			{
				bool confirmed;

				msg_print("You float over a trap door.");
				confirmed = get_check("Go down to the next level?");

				if (confirmed)
				{
					msg_print("You deliberately jump through the trap door.");
				}
				else break;
			}
			else
			{
				msg_print("You fall through a trap door!");
				dam = damroll(2, 8);
				take_hit(dam, 0, NULL, "falling through a trap door");
			}

			/* New depth */
			p_ptr->depth++;

			/* Leaving */
			p_ptr->leaving = TRUE;

			break;
		}

		/* Pits */
		case TRAP_PIT:
		{
			/* Determine how dangerous the trap is allowed to be */
			nastiness = randint(p_ptr->depth);
			if (one_in_(20)) nastiness += 20;
			else if (one_in_(5)) nastiness += 10;

			/* Player is now in pit */
			if (who < 0) monster_swap(p_ptr->py, p_ptr->px, y, x);

			/* Center on player */
			y = p_ptr->py;
			x = p_ptr->px;

			/* Pit of daggers */
			if ((nastiness > 70) && (!one_in_(3)))
			{
				msg_print("You fall into a pit of daggers!");

				if (p_ptr->ffall)
				{
					msg_print("You float gently to the floor of the pit.");
					msg_print("You carefully avoid setting off the daggers.");
				}

				else
				{
					/* A trap of Morgul */
					if (one_in_(6))
					{
						Rand_quick = FALSE;

						/* Lots of damage */
						dam = damroll(20, 20);

						/* Take the damage */
						take_hit(dam, 0,
						   "A single coldly gleaming dagger pierces you deeply!",
							"a Blade of Morgul");

						/* Character is still alive */
						if (!p_ptr->is_dead)
						{
							/* Activate the Black Breath */
							p_ptr->black_breath = TRUE;
							msg_print("You feel a deadly chill slowly withering your soul.");

							/* Undead may be attracted */
							if (one_in_(2))
							{
								msg_print("Undead suddenly appear and call you to them!");
								for (i = 0; i < rand_range(3, 5); i++)
								{
									summon_specific(y, x, FALSE, p_ptr->depth, SUMMON_UNDEAD);
								}
							}
						}

						/* Morgul-traps are one-time only */
						remove_trap(y, x, t_idx);
					}

					/* Dagger trap */
					else
					{
						Rand_quick = FALSE;

						/* Activate the ordinary daggers */
						dam += damroll(8, 8 + p_ptr->depth / 2);

						/* Take the damage. */
						take_hit(dam, 0, "Daggers pierce you everywhere!",
							"a pit of daggers");

						Rand_quick = TRUE;
					}

					/* Cut the character */
					(void)set_cut(p_ptr->cut + randint(dam));
				}
			}

			/* Poisoned spiked pit */
			else if ((nastiness > 40) && (!one_in_(3)))
			{
				msg_print("You fall into a spiked pit!");

				if (p_ptr->ffall)
				{
					msg_print("You float gently to the floor of the pit.");
					msg_print("You carefully avoid touching the spikes.");
				}

				/* Disease pit */
				else if (one_in_(4))
				{
					Rand_quick = FALSE;

					/* Base damage */
					dam = damroll(5, 5 + p_ptr->depth / 2);

					/* Cause disease, adjust damage */
					disease(&dam);

					/* Take the damage */
					take_hit(dam, 0, "You are impaled on disease-bearing spikes!",
					   "a disease pit");

					/* Cause cuts */
					(void)set_cut(p_ptr->cut + randint(dam));

					Rand_quick = TRUE;
				}

				else
				{
					Rand_quick = FALSE;

					/* Base damage */
					dam = damroll(4, 4 + p_ptr->depth / 2);

					if (!p_ptr->resist_pois && !p_ptr->oppose_pois)
					{
						dam = 3 * dam / 2;
						(void)set_poisoned(p_ptr->poisoned + randint(dam));
					}

					/* Take the damage */
					take_hit(dam, 0, "You are impaled on venomous spikes!",
					   "a poison spiked pit");

					/* Cause cuts */
					(void)set_cut(p_ptr->cut + randint(dam));

					if (p_ptr->resist_pois || p_ptr->oppose_pois)
					{
						if (!p_ptr->is_dead)
							msg_print("The poison does not affect you!");
					}

					Rand_quick = TRUE;
				}
			}

			/* Spiked pit */
			else if ((nastiness > 15) && (!one_in_(3)))
			{
				msg_print("You fall into a spiked pit!");

				if (p_ptr->ffall)
				{
					msg_print("You float gently to the floor of the pit.");
					msg_print("You carefully avoid touching the spikes.");
				}
				else
				{
					Rand_quick = FALSE;

					/* Base damage */
					dam = damroll(3, 3 + p_ptr->depth / 4);

					/* Take the damage */
					take_hit(dam, 0, "You are impaled!", "a spiked pit");

					/* Cause cuts */
					(void)set_cut(p_ptr->cut + randint(dam));

					Rand_quick = TRUE;
				}
			}

			/* Ordinary pit in all other cases */
			else
			{
				msg_print("You fall into a pit!");
				if (p_ptr->ffall)
				{
					msg_print("You float gently to the bottom of the pit.");
				}
				else
				{
					Rand_quick = FALSE;

					dam = damroll(2, 6);
					take_hit(dam, 0, NULL, "a pit trap");

					Rand_quick = TRUE;
				}
			}

			break;
		}

		/* Stat-reducing dart traps */
		case TRAP_DART:
		{
			cptr msg = "";
			bool sust = FALSE;

			/* Decide if the dart hits */
			if (check_trap_hit(50 + p_ptr->depth))
			{
				/* Select a stat to drain */
				selection = rand_int(6);

				/* Note sustain, otherwise colorful message */
				if (selection == A_STR)
				{
					if (p_ptr->sustain_str) sust = TRUE;
					else msg = "A small dart saps your strength!";
				}
				else if (selection == A_INT)
				{
					if (p_ptr->sustain_int) sust = TRUE;
					else msg = "A small dart attacks your mind!";
				}
				else if (selection == A_WIS)
				{
					if (p_ptr->sustain_wis) sust = TRUE;
					else msg = "A small dart hits you!  You feel more naive!";
				}
				else if (selection == A_DEX)
				{
					if (p_ptr->sustain_int) sust = TRUE;
					else msg = "A small dart hits you; you suddenly feel clumsy!";
				}
				else if (selection == A_CON)
				{
					if (p_ptr->sustain_int) sust = TRUE;
					else msg = "A small dart drains away your health!";
				}
				else if (selection == A_CHR)
				{
					if (p_ptr->sustain_int) sust = TRUE;
					else msg = "A small dart twists your features!";
				}

				Rand_quick = FALSE;

				/* Note sustain */
				if (sust)
				{
					msg = "A small dart hits you.";
				}

				/* Hit the character */
				dam = damroll(1, 4);
				take_hit(dam, 0, msg, "a dart trap");

				/* Drain the stat */
				if ((!p_ptr->is_dead) && (!sust))
				{
					/* Determine how dangerous the trap is allowed to be */
					nastiness = randint(p_ptr->depth);

					/* Decide how much to drain the stat by */
					if ((nastiness > 25) && (one_in_(4)))
					{
						num = randint(5);
					}
					else num = 1;

					/* Drain the stat */
					(void)dec_stat(selection, num, FALSE);
				}

				Rand_quick = TRUE;
			}
			else
			{
				msg_print("A small dart barely misses you.");
			}
			break;
		}

		/* Discolored spots */
		case TRAP_SPOT:
		{
			cptr msg = "";

			/* Determine how dangerous the trap is allowed to be. */
			nastiness = randint(p_ptr->depth);
			if (one_in_(5)) nastiness += 10;

			/* Pick an elemental attack type. */
			selection = randint(4);


			/* Electricity trap */
			if (selection == 1)
			{
				if ((nastiness >= 40) && (one_in_(2)))
				{
					Rand_quick = FALSE;

					msg = "You are struck by lightning!";
					dam = damroll(6, 30);

					Rand_quick = TRUE;
				}
				else
				{
					Rand_quick = FALSE;

					msg = "You get zapped!";
					dam = damroll(4, 8);

					Rand_quick = TRUE;
				}
				Rand_quick = FALSE;
				elec_dam(dam, 0, msg, "an electricity trap");
				Rand_quick = TRUE;

			}

			/* Frost trap */
			if (selection == 2)
			{
				if ((nastiness >= 40) && (one_in_(2)))
				{
					Rand_quick = FALSE;

					msg = "You are lost within a blizzard!";
					dam = damroll(6, 30);

					Rand_quick = TRUE;
				}
				else
				{
					Rand_quick = FALSE;

					msg = "You are coated in frost!";
					dam = damroll(4, 8);

					Rand_quick = TRUE;
				}
				Rand_quick = FALSE;
				cold_dam(dam, 0, msg, "a frost trap");
				Rand_quick = TRUE;
			}

			/* Fire trap */
			if (selection == 3)
			{
				if ((nastiness >= 40) && (one_in_(2)))
				{
					Rand_quick = FALSE;

					msg = "You are enveloped in a column of fire!";
					dam = damroll(6, 30);

					Rand_quick = TRUE;
				}
				else
				{
					Rand_quick = FALSE;

					msg = "You are surrounded by flames!";
					dam = damroll(4, 8);

					Rand_quick = TRUE;
				}
				Rand_quick = FALSE;
				fire_dam(dam, 0, msg, "a fire trap");
				Rand_quick = TRUE;
			}

			/* Acid trap */
			if (selection == 4)
			{
				if ((nastiness >= 40) && (one_in_(2)))
				{
					Rand_quick = FALSE;

					msg = "A cauldron of acid is tipped over your head!";
					dam = damroll(6, 30);

					Rand_quick = TRUE;
				}
				else
				{
					Rand_quick = FALSE;

					msg = "You are splashed with acid!";
					dam = damroll(4, 8);

					Rand_quick = TRUE;
				}
				Rand_quick = FALSE;
				acid_dam(dam, 0, msg, "an acid trap");
				Rand_quick = TRUE;
			}

			break;
		}

		/* Gas traps */
		case TRAP_GAS:
		{
			selection = randint(4);

			/* blinding trap */
			if (selection == 1)
			{
				msg_print("You are surrounded by a black gas!");
				if (!p_ptr->resist_blind)
				{
					Rand_quick = FALSE;

					(void)set_blind(p_ptr->blind + rand_range(30, 45),
						"It gets into your eyes!");

					Rand_quick = TRUE;
				}
			}

			/* Confusing trap */
			if (selection == 2)
			{
				msg_print("You are surrounded by a gas of scintillating colors!");
				if (!p_ptr->resist_confu)
				{
					Rand_quick = FALSE;

					(void)set_confused(p_ptr->confused + rand_range(10, 30));

					Rand_quick = TRUE;
				}
			}

			/* Poisoning trap */
			if (selection == 3)
			{
				msg_print("You are surrounded by a pungent green gas!");
				if (!p_ptr->resist_pois || !p_ptr->oppose_pois)
				{
					if (!p_ptr->resist_pois && !p_ptr->oppose_pois)
					{
						Rand_quick = FALSE;

						(void)set_poisoned(p_ptr->poisoned + rand_range(10, 30));

						Rand_quick = TRUE;
					}
					else
					{
						Rand_quick = FALSE;

						(void)set_poisoned(p_ptr->poisoned + rand_range(5, 10));

						Rand_quick = TRUE;
					}
				}
			}

			/* Sleeping trap */
			if (selection == 4)
			{
				msg_print("You are surrounded by a strange white mist!");
				if (!p_ptr->free_act)
				{
					(void)set_paralyzed(p_ptr->paralyzed + rand_range(5, 10));
				}
			}

			break;
		}

		/* Summoning traps */
		case TRAP_SUMMON:
		{
			/* Sometimes summon thieves */
			if ((p_ptr->depth > 8) && (one_in_(5)))
			{
				msg_print("You have aroused a den of thieves!");

				Rand_quick = FALSE;

				num = rand_range(3, 5);
				for (i = 0; i < num; i++)
				{
					(void)summon_specific(y, x, FALSE, p_ptr->depth + 2,
						SUMMON_THIEF);
				}

				Rand_quick = TRUE;
			}

			/* Sometimes summon a nasty unique */
			else if (one_in_(8))
			{
				msg_print("You are enveloped in a cloud of smoke!");

				Rand_quick = FALSE;

				(void)summon_specific(y, x, FALSE, p_ptr->depth + 5, SUMMON_UNIQUE);

				Rand_quick = TRUE;
			}

			/* Otherwise, the ordinary summon monsters */
			else
			{
				msg_print("You are enveloped in a cloud of smoke!");

				Rand_quick = FALSE;

				num = rand_range(3, 5);
				for (i = 0; i < num; i++)
				{
					(void)summon_specific(y, x, FALSE, p_ptr->depth + 2, 0);
				}

				Rand_quick = TRUE;
			}

			/* These are all one-time traps */
			remove_trap(y, x, t_idx);

			break;
		}

		/* Dungeon alteration traps */
		case TRAP_ALTER_DUNGEON:
		{
			/* Determine how dangerous the trap is allowed to be. */
			nastiness = randint(p_ptr->depth);
			if (one_in_(5)) nastiness += 10;

			/* Make room for alterations */
			remove_trap(y, x, t_idx);

			/* Everything truly random from here on */
			Rand_quick = FALSE;

			/* Dungeon destruction trap */
			if ((nastiness > 60) && (one_in_(12)))
			{
				msg_print("A ear-splitting howl shatters your mind as the dungeon is smashed by hammer blows!");

				(void)destroy_level(FALSE);

				/* the player is hard-hit */
				(void)set_confused(p_ptr->confused + rand_range(20, 30));
				(void)set_blind(p_ptr->blind + rand_range(30, 45), NULL);
				(void)set_stun(p_ptr->stun + rand_range(50, 100));
				dam = damroll(15, 15);
				take_hit(dam, 0, NULL, "a dungeon destruction trap");
			}

			/* Collapse ceiling trap */
			if ((nastiness > 35) && (one_in_(3)))
			{
				if (collapse_ceiling(y, x, p_ptr->depth))
					msg_print("The ceiling collapses!");
			}

			/* Earthquake trap */
			else if ((nastiness > 20) && (one_in_(4)))
			{
				msg_print("A tremor shakes the dungeon around you.");
				earthquake(y, x, 10);
			}

			/* Falling rock trap */
			else if ((nastiness > 4) && (one_in_(2)))
			{
				dam = damroll(2, 10);
				take_hit(dam, 0, "A rock falls on your head.", "falling rock");
				(void)set_stun(p_ptr->stun + rand_range(10, 20));
			}

			/* A few pebbles */
			else
			{
				dam = damroll(1, 8);
				take_hit(dam, 0, "A bunch of pebbles rain down on you.",
					"falling rock");
			}

			Rand_quick = TRUE;

			break;
		}

		/*
		 * Various char and equipment-alteration traps, lumped together
		 * to avoid any one effect being too common (some of them can be
		 * rather nasty).
		 */
		case TRAP_HEX:
		{
			/* Determine how dangerous the trap is allowed to be. */
			int choice = rand_int(100);

			/* These are all one-time traps */
			remove_trap(y, x, t_idx);

			/* Everything truly random from here on */
			Rand_quick = FALSE;

			/* Trap of fire off magical devices */
			if (choice < 25)
			{
				/* Fire off some charges */
				fire_off_devices(20 + p_ptr->depth / 2);
			}

			/* Trap of forgetting */
			else if (choice < 40)
			{
				if (rand_int(100) < p_ptr->skill_sav)
				{
					msg_print("You hang on to your memories!");
				}
				else
				{
					(void)lose_all_info("Your memories fade away.");
				}
			}

			/* Trap of alter reality */
			else if (choice < 55)
			{
				msg_print("The world changes!");

				/* Leaving */
				p_ptr->leaving = TRUE;
			}

			/* Trap of remold player */
			else if (choice < 85)
			{
				/* Message */
				msg_print("You feel yourself being twisted by wild magic!");

				/* Resist */
				if ((p_ptr->resist_nexus) || (rand_int(100) < p_ptr->skill_sav))
				{
					msg_print("You resist the effects!");
				}

				/* Shuffle a pair of stats */
				else
				{
					shuffle_stats(1);
				}
			}

			/* Time ball trap */
			else
			{
				msg_print("You feel time itself assault you!");

				/* Target the player with a radius 1 ball attack. */
				(void)project_ball(0, 1, y, x, y,
				                   x, p_ptr->depth, GF_TIME, 0L, 20);
			}

			Rand_quick = TRUE;

			break;
		}

		/* Teleport trap */
		case TRAP_PORTAL:
		{
			/* Determine how dangerous the trap is allowed to be. */
			nastiness = randint(p_ptr->depth);

			if ((nastiness < 10) || (!one_in_(3)) || (p_ptr->resist_nexus))
			{
				msg_print("You teleport across the dungeon.");

				teleport_player(150, FALSE);
			}
			else
			{
				(void)set_dancing_feet(p_ptr->dancing_feet +
						rand_range(10, 20),
						"You start to blink around uncontrollably!", FALSE);
			}

			break;
		}

		/* Murder holes */
		case TRAP_MURDER_HOLE:
		{
			/* hold the object info. */
			object_type *o_ptr;
			object_type object_type_body;

			/* hold the missile type and name. */
			int sval = 0;
			int tval = 0;
			cptr missile_name = "";

			/* Determine how dangerous the trap is allowed to be. */
			nastiness = rand_range(p_ptr->depth / 2, p_ptr->depth);

			/* Determine the missile type and base damage. */
			if (nastiness < 15)
			{
				missile_name = "shot";
				dam = damroll(3, 3 * nastiness / 4);
				tval = TV_SHOT;
				sval = SV_AMMO_NORMAL;
			}
			else if (nastiness < 30)
			{
				missile_name = "arrow";
				dam = damroll(3, nastiness);
				tval = TV_ARROW;
				sval = SV_AMMO_NORMAL;
			}
			else if (nastiness < 45)
			{
				missile_name = "bolt";
				dam = damroll(3, 5 * nastiness / 4);
				tval = TV_BOLT;
				sval = SV_AMMO_NORMAL;
			}
			else if (nastiness < 60)
			{
				missile_name = "seeker shot";
				dam = damroll(3, 3 * nastiness / 2);
				tval = TV_SHOT;
				sval = SV_AMMO_HEAVY;
			}
			else if (nastiness < 75)
			{
				missile_name = "seeker arrow";
				dam = damroll(3, 7 * nastiness / 4);
				tval = TV_ARROW;
				sval = SV_AMMO_HEAVY;
			}
			else
			{
				missile_name = "seeker bolt";
				dam = damroll(3, 2 * nastiness);
				tval = TV_BOLT;
				sval = SV_AMMO_HEAVY;
			}

			/* Determine if the missile hits */
			if (check_trap_hit(75 + p_ptr->depth))
			{
				msg_format("A %s hits you from above.", missile_name);

				Rand_quick = FALSE;

				/* Critical hits */
				if (one_in_(3))
				{
					msg_print("It was well-aimed!");
					dam *= 2;
				}
				else if (one_in_(2))
				{
					msg_print("It gouges you!");
					dam = 3 * dam / 2;

					/* Cut the player */
					(void)set_cut(p_ptr->cut + randint(dam));
				}

				/* Player armour reduces damage */
				dam = 1 + dam * 110 / (110 + p_ptr->ac + p_ptr->to_a);

				Rand_quick = TRUE;

				take_hit(dam, 0, NULL, format("a %s trap", missile_name));
			}

			/* Explain what just happened. */
			else msg_format("A %s whizzes by your head.", missile_name);

			/* Get local object */
			o_ptr = &object_type_body;

			/* Make a missile, identify it, and drop it near the player. */
			object_prep(o_ptr, lookup_kind(tval, sval));
			object_aware(o_ptr);
			object_known(o_ptr);
			drop_near(o_ptr, -1, y, x);

			/* These are one-time traps */
			remove_trap(y, x, t_idx);

			break;
		}

		/* Undefined trap */
		default:
		{
			if (who < 0)
			{
				dam = damroll(3, 4);
				take_hit(dam, 0, "A dagger is thrown at you from the shadows!",
					"a dagger trap");
			}

			break;
		}
	}

	/* Revert to usage of the complex RNG. */
	Rand_quick = FALSE;

	/* Return "something happened" */
	return (flag);
}

/*
 * Hit a trap.  Who of 0 fires off all traps.  Who of less than 0 fires off
 * traps that affect characters.  Who of greater than zero fires off traps
 * that affect monsters.
 *
 * Return TRUE if a trap did actually go off.
 */
bool hit_trap(int who, int y, int x)
{
	int i;
	bool trap_went_off = FALSE;

	/* Character stumbles into invisible, nasty traps */
	if (who < 0)
	{
		/* Count the nasty, hidden traps here */
		int num = nasty_traps(y, x, -1);

		/* Oops.  We've walked right into trouble. */
		if      (num == 1) msg_print("You stumble upon a trap!");
		else if (num >  1) msg_print("You stumble upon some traps!");
	}

	/* Scan the current trap list */
	for (i = 0; i < t_max; i++)
	{
		/* Point to this trap */
		trap_type *t_ptr = &t_list[i];

		/* Find all traps in this position */
		if ((t_ptr->fy == y) && (t_ptr->fx == x))
		{
			/* Sometimes require a trap that does something to the character */
			if ((who < 0) &&
				(!(t_kind_info[t_ptr->t_idx].flags & (TRAP_PLAY)))) continue;

			/* Sometimes require a trap that does something to monsters */
			if ((who > 0) &&
				(!(t_kind_info[t_ptr->t_idx].flags & (TRAP_KILL)))) continue;

			/* Fire off the trap */
			if (hit_trap_aux(who, y, x, i)) trap_went_off = TRUE;

			/* Trap becomes visible (always XXX) */
			if (who < 0)
			{
				t_ptr->flags |= (TRAP_VISIBLE);
				cave_info[y][x] |= (CAVE_MARK);
			}
		}
	}

	/* Verify traps (remove marker if appropriate) */
	(void)verify_trap(y, x, 0);

	/* Return */
	return (trap_went_off);
}

