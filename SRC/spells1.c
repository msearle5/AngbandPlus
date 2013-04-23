/* File: spells1.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"




/*
 * Helper function -- return a "nearby" race for polymorphing
 *
 * Note that this function is one of the more "dangerous" ones...
 */
s16b poly_r_idx(int r_idx)
{
	monster_race *r_ptr = &r_info[r_idx];

	int i, r, lev1, lev2;

	/* Hack -- Uniques never polymorph */
	if (r_ptr->flags1 & (RF1_UNIQUE)) return (r_idx);

	/* Allowable range of "levels" for resulting monster */
	lev1 = r_ptr->level - ((randint(20)/randint(9))+1);
	lev2 = r_ptr->level + ((randint(20)/randint(9))+1);

	/* Pick a (possibly new) non-unique race */
	for (i = 0; i < 1000; i++)
	{
		/* Pick a new race, using a level calculation */
		r = get_mon_num((p_ptr->depth + r_ptr->level) / 2 + 5);

		/* Handle failure */
		if (!r) break;

		/* Obtain race */
		r_ptr = &r_info[r];

		/* Ignore unique monsters */
		if (r_ptr->flags1 & (RF1_UNIQUE)) continue;

		/* Ignore monsters with incompatible levels */
		if ((r_ptr->level < lev1) || (r_ptr->level > lev2)) continue;

		/* Use that index */
		r_idx = r;

		/* Done */
		break;
	}

	/* Result */
	return (r_idx);
}


/*
 * Teleport a monster, normally up to "dis" grids away.
 *
 * Attempt to move the monster at least "dis/2" grids away.
 *
 * But allow variation to prevent infinite loops.
 */
void teleport_away(int m_idx, int dis)
{
	int ny, nx, oy, ox, d, i, min, bored;

	bool look = TRUE;

	monster_type *m_ptr = &m_list[m_idx];


	/* Paranoia */
	if (!m_ptr->r_idx) return;

	/* Save the old location */
	oy = m_ptr->fy;
	ox = m_ptr->fx;

	/* Minimum distance */
	min = dis / 2;

	/* Not bored yet */
	bored = 0;

	/* Look until done */
	while (look)
	{
		/* Verify max distance */
		if (dis > 200) dis = 200;

		/* Try several locations */
		for (i = 0; i < 500; i++)
		{
			/* Pick a (possibly illegal) location */
			while (1)
			{
				ny = rand_spread(oy, dis);
				nx = rand_spread(ox, dis);
				d = distance(oy, ox, ny, nx);
				if ((d >= min) && (d <= dis)) break;
			}

			/* Ignore illegal locations */
			if (!in_bounds_fully(ny, nx)) continue;

			/* Become more bored */
			if (cave_floor_bold(ny, nx)) bored++;

			/* Require "empty" floor space */
			if (!cave_empty_bold(ny, nx)) continue;

			/* Hack -- no teleport onto glyph of warding */
			if (cave_feat[ny][nx] == FEAT_GLYPH) continue;

			/* No teleporting into vaults and such */
			if (cave_info[ny][nx] & (CAVE_ICKY)) continue;

			/* This grid looks good */
			look = FALSE;

			/* Stop looking */
			break;
		}

		/* Stop when bored */
		if (bored > 2000) return;

		/* Increase the maximum distance */
		dis = dis * 2;

		/* Decrease the minimum distance */
		min = min / 2;
	}

	/* Sound */
	sound(SOUND_TPOTHER);

	/* Swap the monsters */
	monster_swap(oy, ox, ny, nx);
}


/*
 * Teleport the player to a location up to "dis" grids away.
 *
 * If no such spaces are readily available, the distance may increase.
 * Try very hard to move the player at least a quarter that distance.
 */
void teleport_player(int dis)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int d, i, min, y, x, bored;

	bool look = TRUE;


	/* Initialize */
	y = py;
	x = px;

	/* Minimum distance */
	min = dis / 2;

	/* Not bored yet */
	bored = 0;

	/* Look until done */
	while (look)
	{
		/* Verify max distance */
		if (dis > 200) dis = 200;

		/* Try several locations */
		for (i = 0; i < 500; i++)
		{
			/* Pick a (possibly illegal) location */
			while (1)
			{
				y = rand_spread(py, dis);
				x = rand_spread(px, dis);
				d = distance(py, px, y, x);
				if ((d >= min) && (d <= dis)) break;
			}

			/* Ignore illegal locations */
			if (!in_bounds_fully(y, x)) continue;

			/* Become more bored */
			if (cave_floor_bold(y, x)) bored++;

			/* Require "empty" floor space */
			if (!cave_empty_bold(y, x)) continue;

			/* No teleporting into vaults and such */
			if (cave_info[y][x] & (CAVE_ICKY)) continue;

			/* This grid looks good */
			look = FALSE;

			/* Stop looking */
			break;
		}

		/* Stop when bored */
		if (bored > 2000) return;

		/* Increase the maximum distance */
		dis = dis * 2;

		/* Decrease the minimum distance */
		min = min / 2;
	}

	/* Sound */
	sound(SOUND_TELEPORT);

	/* Move player */
	monster_swap(py, px, y, x);

	/* Handle stuff XXX XXX XXX */
	handle_stuff();
}



/*
 * Teleport player to a grid near the given location
 *
 * This function is slightly obsessive about correctness.
 * This function allows teleporting into vaults (!)
 */
void teleport_player_to(int ny, int nx)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int y, x;

	int dis = 0, ctr = 0;

	/* Initialize */
	y = py;
	x = px;

	/* Find a usable location */
	while (1)
	{
		/* Pick a nearby legal location */
		while (1)
		{
			y = rand_spread(ny, dis);
			x = rand_spread(nx, dis);
			if (in_bounds_fully(y, x)) break;
		}

		/* Accept "naked" floor grids */
		if (cave_naked_bold(y, x)) break;

		/* Occasionally advance the distance */
		if (++ctr > (4 * dis * dis + 4 * dis + 1))
		{
			ctr = 0;
			dis++;
		}
	}

	/* Sound */
	sound(SOUND_TELEPORT);

	/* Move player */
	monster_swap(py, px, y, x);

	/* Handle stuff XXX XXX XXX */
	handle_stuff();
}



/*
 * Teleport the player one level up or down (random when legal)
 */
void teleport_player_level(void)
{
	if (!p_ptr->depth)
	{
		msg_print("You sink through the floor.");

		/* New depth */
		p_ptr->depth++;

		/* Leaving */
		p_ptr->leaving = TRUE;
	}

	else if (is_quest(p_ptr->depth) || (p_ptr->depth >= MAX_DEPTH-1))
	{
		msg_print("You rise up through the ceiling.");

		/* New depth */
		p_ptr->depth--;

		/* Leaving */
		p_ptr->leaving = TRUE;
	}

	else if (rand_int(100) < 50)
	{
		msg_print("You rise up through the ceiling.");

		/* New depth */
		p_ptr->depth--;

		/* Leaving */
		p_ptr->leaving = TRUE;
	}

	else
	{
		msg_print("You sink through the floor.");

		/* New depth */
		p_ptr->depth++;

		/* Leaving */
		p_ptr->leaving = TRUE;
	}

	/* Sound */
	sound(SOUND_TPLEVEL);
}


static byte elec_color()
{
	switch rand_int(3)
	{
		case 0: case 1: return TERM_WHITE;
		case 2: return TERM_L_BLUE;
	}
	/* Oops */
	return TERM_WHITE;
}


static byte fire_color()
{
	switch rand_int(3)
	{
		case 0: return TERM_RED;
		case 1: return TERM_ORANGE;
		case 2: return TERM_YELLOW;
	}
	/* Oops */
	return TERM_WHITE;
}


static byte pois_color()
{
	switch rand_int(4)
	{
		case 0: case 1: case 2: return TERM_GREEN;
		case 3: return TERM_L_GREEN;
	}
	/* Oops */
	return TERM_WHITE;
}


static byte orb_color()
{
	switch rand_int(4)
	{
		case 0: case 1: case 2: return TERM_L_DARK;
		case 3: return TERM_SLATE;
	}
	/* Oops */
	return TERM_WHITE;
}


static byte mana_color()
{
	switch rand_int(6)
	{
		case 0: case 1: case 2: case 3: return TERM_WHITE;
		case 4: return TERM_GREEN;
		case 5: return TERM_L_GREEN;
	}
	/* Oops */
	return TERM_WHITE;
}


static byte grav_color()
{
	switch rand_int(4)
	{
		case 0: case 1: return TERM_DARK;
		case 2: return TERM_L_DARK;
		case 3: return TERM_SLATE;
	}
	/* Oops */
	return TERM_WHITE;
}


static byte lite_color()
{
	switch rand_int(4)
	{
		case 0: case 1: case 2: return TERM_WHITE;
		case 3: return TERM_YELLOW;
	}
	/* Oops */
	return TERM_WHITE;
}


static byte plasma_color()
{
	switch rand_int(4)
	{
		case 0: case 1: return TERM_WHITE;
		case 2: return TERM_BLUE;
		case 3: return TERM_L_BLUE;
	}
	/* Oops */
	return TERM_WHITE;
}


static byte meteor_color()
{
	switch rand_int(6)
	{
		case 0: case 1: case 2: return TERM_L_DARK;
		case 3: return TERM_RED;
		case 4: return TERM_ORANGE;
		case 5: return TERM_YELLOW;
	}
	/* Oops */
	return TERM_WHITE;
}


static byte random_color()
{
	return randint(15);
}


/*
 * Return a special color
 */
byte spell_color(int type)
{
	/* Analyze */
	switch (type)
	{
		case GF_MISSILE:		return orb_color();
		case GF_ACID:			return (TERM_SLATE);
		case GF_ELEC:			return elec_color();
		case GF_FIRE:			return fire_color();
		case GF_COLD:			return (TERM_WHITE);
		case GF_POIS:			return pois_color();
		case GF_HOLY_ORB:		return orb_color();
		case GF_MANA:			return mana_color();
		case GF_ARROW:			return (TERM_L_UMBER);
		case GF_WATER:			return (TERM_SLATE);
		case GF_NETHER:		return (TERM_L_GREEN);
		case GF_CHAOS:			return random_color();
		case GF_DISENCHANT:	return random_color();
		case GF_NEXUS:			return (TERM_L_RED);
		case GF_CONFUSION:	return (TERM_L_UMBER);
		case GF_SOUND:			return (TERM_YELLOW);
		case GF_SHARD:			return orb_color();
		case GF_FORCE:			return (TERM_UMBER);
		case GF_INERTIA:		return (TERM_L_WHITE);
		case GF_GRAVITY:		return grav_color();
		case GF_TIME:			return (TERM_L_BLUE);
		case GF_LITE_WEAK:	return lite_color();
		case GF_LITE:			return lite_color();
		case GF_DARK_WEAK:	return (TERM_L_DARK);
		case GF_DARK:			return (TERM_L_DARK);
		case GF_PLASMA:		return plasma_color();
		case GF_METEOR:		return meteor_color();
		case GF_ICE:			return (TERM_WHITE);
		case GF_MAKE_SANC:	return (TERM_L_GREEN);
	}

	/* Standard "color" */
	return (TERM_WHITE);
}






/*
 * Decreases players hit points and sets death flag if necessary
 *
 * Invulnerability needs to be changed into a "shield" XXX XXX XXX
 *
 * Hack -- this function allows the user to save (or quit) the game
 * when he dies, since the "You die." message is shown before setting
 * the player to "dead".  XXX XXX XXX
 */
void take_hit(int dam, cptr kb_str)
{
	int old_chp = p_ptr->chp;

	int warning = (p_ptr->mhp * op_ptr->hitpoint_warn / 10);

	/* Disturb */
	disturb(1, 0);

	if (dam <= 0) return;

	if (p_ptr->chp < -30000) return;

	/* Mega-Hack -- Apply "invulnerability" */
	if (p_ptr->invuln && (dam < 9000)) return;

	/* Bleed */
	if (!character_icky && !p_ptr->poisoned && (p_ptr->food < PY_FOOD_PAIN))
	{
		bleed(-1, dam);
	}

	/* Hurt the player */
	p_ptr->chp -= dam;

	/* Display the hitpoints */
	p_ptr->redraw |= (PR_HP);

	/* Window stuff */
	p_ptr->window |= (PW_SPELL | PW_PLAYER);

	/* Dead player */
	if (p_ptr->chp < 0 && !p_ptr->is_dead)
	{
		/* No longer a winner */
		p_ptr->total_winner = FALSE;

		/* Note death */
		p_ptr->is_dead = TRUE;

		/* Leaving */
		p_ptr->leaving = TRUE;

		/* Sound */
		sound(SOUND_DEATH);

		/* Hack -- Note death */
		msg_print("You die.");
		msg_print(NULL);

		/* Note cause of death */
		strcpy(p_ptr->died_from, kb_str);
	}

	if (p_ptr->is_dead) return;

	/* Hitpoint warning */
	if (alert_hitpoint && (p_ptr->chp < warning))
	{
		/* Hack -- bell on first notice */
		if (old_chp > warning) bell();

		/* Message */
		msg_print("*** LOW HITPOINT WARNING! ***");
		msg_print(NULL);
	}
}





/*
 * Does a given class of objects (usually) hate acid?
 * Note that acid can either melt or corrode something.
 */
static bool hates_acid(object_type *o_ptr)
{
	/* Analyze the type */
	switch (o_ptr->tval)
	{
		/* Wearable items */
		case TV_ARROW:
		case TV_BOLT:
		case TV_BOW:
		case TV_SWORD:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_HELM:
		case TV_CROWN:
		case TV_SHIELD:
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_CLOAK:
		case TV_SOFT_ARMOR:
		case TV_HARD_ARMOR:
		case TV_DRAG_ARMOR:
		{
			return (TRUE);
		}

		/* Staffs/Scrolls are wood/paper */
		case TV_STAFF:
		case TV_SCROLL:
		{
			return (TRUE);
		}

		/* Ouch */
		case TV_CHEST:
		{
			return (TRUE);
		}

		/* Junk is useless */
		case TV_SKELETON:
		case TV_JUNK:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/*
 * Does a given object (usually) hate electricity?
 */
static bool hates_elec(object_type *o_ptr)
{
	switch (o_ptr->tval)
	{
		case TV_RING:
		case TV_WAND:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/*
 * Does a given object (usually) hate fire?
 * Hafted/Polearm weapons have wooden shafts.
 * Arrows/Bows are mostly wooden.
 */
static bool hates_fire(object_type *o_ptr)
{
	/* Analyze the type */
	switch (o_ptr->tval)
	{
		/* Oil */
		case TV_FLASK:
		{
			return (TRUE);
		}

		/* Wearable */
		case TV_LITE:
		case TV_ARROW:
		case TV_BOW:
		case TV_HAFTED:
		case TV_POLEARM:
		case TV_BOOTS:
		case TV_GLOVES:
		case TV_CLOAK:
		case TV_SOFT_ARMOR:
		{
			return (TRUE);
		}

		/* Books */
		case TV_MAGIC_BOOK:
		case TV_PRAYER_BOOK:
		{
			return (TRUE);
		}

		/* Chests */
		case TV_CHEST:
		{
			return (TRUE);
		}

		/* Staffs/Scrolls burn */
		case TV_STAFF:
		case TV_SCROLL:
		{
			return (TRUE);
		}

		/* Corpses burn */
		case TV_SKELETON:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/*
 * Does a given object (usually) hate cold?
 */
static bool hates_cold(object_type *o_ptr)
{
	switch (o_ptr->tval)
	{
		case TV_POTION:
		{
			return (TRUE);
		}
	}

	return (FALSE);
}


/*
 * Melt something
 */
static int set_acid_destroy(object_type *o_ptr)
{
	u32b f1, f2, f3;
	if (!hates_acid(o_ptr)) return (FALSE);
	object_flags(o_ptr, &f1, &f2, &f3);
	if (f3 & (TR3_IGNORE_ACID)) return (FALSE);
	return (TRUE);
}


/*
 * Electrical damage
 */
static int set_elec_destroy(object_type *o_ptr)
{
	u32b f1, f2, f3;
	if (!hates_elec(o_ptr)) return (FALSE);
	object_flags(o_ptr, &f1, &f2, &f3);
	if (f3 & (TR3_IGNORE_ELEC)) return (FALSE);
	return (TRUE);
}


/*
 * Burn something
 */
static int set_fire_destroy(object_type *o_ptr)
{
	u32b f1, f2, f3;
	if (!hates_fire(o_ptr)) return (FALSE);
	object_flags(o_ptr, &f1, &f2, &f3);
	if (f3 & (TR3_IGNORE_FIRE)) return (FALSE);
	return (TRUE);
}


/*
 * Freeze things
 */
static int set_cold_destroy(object_type *o_ptr)
{
	u32b f1, f2, f3;
	if (!hates_cold(o_ptr)) return (FALSE);
	object_flags(o_ptr, &f1, &f2, &f3);
	if (f3 & (TR3_IGNORE_COLD)) return (FALSE);
	return (TRUE);
}




/*
 * This seems like a pretty standard "typedef"
 */
typedef int (*inven_func)(object_type *);

/*
 * Destroys a type of item on a given percent chance
 * Note that missiles are no longer necessarily all destroyed
 * Destruction taken from "melee.c" code for "stealing".
 * Returns number of items destroyed.
 */
static int inven_damage(inven_func typ, int perc)
{
	int i, j, k, amt;

	s16b explode, explode_dam;

	object_type *o_ptr;

	char o_name[80];


	/* Count the casualties */
	k = 0;

	/* Scan through the slots backwards */
	for (i = 0; i < INVEN_PACK; i++)
	{
		o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Hack -- for now, skip artifacts */
		if (artifact_p(o_ptr)) continue;

		/* Give this item slot a shot at death */
		if ((*typ)(o_ptr))
		{
			/* Count the casualties */
			for (amt = j = 0; j < o_ptr->number; ++j)
			{
				if (rand_int(100) < perc) amt++;
			}

			/* Some casualities */
			if (amt)
			{
				/* Get a description */
				object_desc(o_name, o_ptr, FALSE, 3);

				/* Object might explode */
				explode = hates_dam(o_ptr);

				/* Damage from explosion */
				explode_dam = object_force(o_ptr) * amt;

				/* Message */
				msg_format("%sour %s (%c) %s destroyed!",
							  ((o_ptr->number > 1) ?
							  ((amt == o_ptr->number) ? "All of y" :
							  (amt > 1 ? "Some of y" : "One of y")) : "Y"),
							  o_name, index_to_label(i),
							  ((amt > 1) ? "were" : "was"));

				/* Destroy "amt" items */
				inven_item_increase(i, -amt);
				inven_item_optimize(i);

				/* Explode it if needed */
				if (explode && explode_dam)
				{
					int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_JUMP;
					project(0, 1, p_ptr->py, p_ptr->px, explode_dam, explode, flg);
				}

				/* Count the casualties */
				k += amt;
			}
		}
	}

	/* Return the casualty count */
	return (k);
}




/*
 * Acid has hit the player, attempt to affect some armor.
 *
 * Note that the "base armor" of an object never changes.
 *
 * If any armor is damaged (or resists), the player takes less damage.
 */
static int minus_ac(void)
{
	object_type *o_ptr = NULL;

	int i;

	u32b f1, f2, f3;

	char o_name[80];


	/* Pick a (possibly empty) inventory slot */
	switch (randint(6))
	{
		case 1: i = INVEN_BODY; break;
		case 2: i = INVEN_ARM; break;
		case 3: i = INVEN_OUTER; break;
		case 4: i = INVEN_HANDS; break;
		case 5: i = INVEN_HEAD; break;
		case 6: i = INVEN_FEET; break;
	}

	/* Acquire the item */
	o_ptr = &inventory[i];

	/* Nothing to damage */
	if (!o_ptr->k_idx) return (FALSE);

	/* Describe */
	object_desc(o_name, o_ptr, FALSE, 0);

	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3);

	/* Object resists */
	if (f3 & (TR3_IGNORE_ACID))
	{
		msg_format("Your %s is unaffected!", o_name);

		return (TRUE);
	}

	/* Damage the item */
	o_ptr->to_a--;

	/* No armor remains */
	if (o_ptr->ac + o_ptr->to_a <= 0)
	{
		/* Message */
		msg_format("Your %s is destroyed!", o_name);

		/* Destroy it */
		inven_item_increase(i, 0 - 1);
		inven_item_optimize(i);
	}
	else
	{
		/* Message */
		msg_format("Your %s is damaged!", o_name);
	}

	/* Calculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Window stuff */
	p_ptr->window |= (PW_EQUIP | PW_SPELL | PW_PLAYER);

	/* Item was damaged */
	return (TRUE);
}


/*
 * Hurt the player with Acid
 */
void acid_dam(int dam, cptr kb_str)
{
	int inv = (dam < 30) ? 1 : (dam < 60) ? 2 : 3;

	/* Total Immunity */
	if (p_ptr->immune_acid || (dam <= 0)) return;

	/* Resist the damage */
	if (p_ptr->resist_acid) dam = (dam + 2) / 3;
	if (p_ptr->oppose_acid) dam = (dam + 2) / 3;

	/* If any armor gets hit, defend the player */
	if (minus_ac()) dam = (dam + 1) / 2;

	/* Take damage */
	take_hit(dam, kb_str);

	/* Inventory damage */
	inven_damage(set_acid_destroy, inv);
}


/*
 * Hurt the player with electricity
 */
void elec_dam(int dam, cptr kb_str)
{
	int inv = (dam < 30) ? 1 : (dam < 60) ? 2 : 3;

	/* Total immunity */
	if (p_ptr->immune_elec || (dam <= 0)) return;

	/* Resist the damage */
	if (p_ptr->oppose_elec) dam = (dam + 2) / 3;
	if (p_ptr->resist_elec) dam = (dam + 2) / 3;

	/* Take damage */
	take_hit(dam, kb_str);

	/* Inventory damage */
	inven_damage(set_elec_destroy, inv);
}




/*
 * Hurt the player with Fire
 */
void fire_dam(int dam, cptr kb_str)
{
	int inv = (dam < 30) ? 1 : (dam < 60) ? 2 : 3;

	/* Totally immune */
	if (p_ptr->immune_fire || (dam <= 0)) return;

	/* Resist the damage */
	if (p_ptr->resist_fire) dam = (dam + 2) / 3;
	if (p_ptr->oppose_fire) dam = (dam + 2) / 3;

	/* Take damage */
	take_hit(dam, kb_str);

	/* Inventory damage */
	inven_damage(set_fire_destroy, inv);
}


/*
 * Hurt the player with Cold
 */
void cold_dam(int dam, cptr kb_str)
{
	int inv = (dam < 30) ? 1 : (dam < 60) ? 2 : 3;

	/* Total immunity */
	if (p_ptr->immune_cold || (dam <= 0)) return;

	/* Resist the damage */
	if (p_ptr->resist_cold) dam = (dam + 2) / 3;
	if (p_ptr->oppose_cold) dam = (dam + 2) / 3;

	/* Take damage */
	take_hit(dam, kb_str);

	/* Inventory damage */
	inven_damage(set_cold_destroy, inv);
}



/*
 * Increase a stat by one randomized level
 *
 * Most code will "restore" a stat before calling this function,
 * in particular, stat potions will always restore the stat and
 * then increase the fully restored value.
 */
bool inc_stat(int stat)
{
	int value, gain;

	/* Then augment the current/max stat */
	value = p_ptr->stat_cur[stat];

	/* Cannot go above 18/100 */
	if (value < 18+100)
	{
		/* Gain one (sometimes two) points */
		if (value < 18)
		{
			gain = ((rand_int(100) < 75) ? 1 : 2);
			value += gain;
		}

		/* Gain 1/6 to 1/3 of distance to 18/100 */
		else if (value < 18+98)
		{
			/* Approximate gain value */
			gain = (((18+100) - value) / 2 + 3) / 2;

			/* Paranoia */
			if (gain < 1) gain = 1;

			/* Apply the bonus */
			value += randint(gain) + gain / 2;

			/* Maximal value */
			if (value > 18+99) value = 18 + 99;
		}

		/* Gain one point at a time */
		else
		{
			value++;
		}

		/* Save the new value */
		p_ptr->stat_cur[stat] = value;

		/* Bring up the maximum too */
		if (value > p_ptr->stat_max[stat])
		{
			p_ptr->stat_max[stat] = value;
		}

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Success */
		return (TRUE);
	}

	/* Nothing to gain */
	return (FALSE);
}



/*
 * Decreases a stat by an amount indended to vary from 0 to 100 percent.
 *
 * Note that "permanent" means that the *given* amount is permanent,
 * not that the new value becomes permanent.  This may not work exactly
 * as expected, due to "weirdness" in the algorithm, but in general,
 * if your stat is already drained, the "max" value will not drop all
 * the way down to the "cur" value.
 */
bool dec_stat(int stat, int amount, int permanent)
{
	int cur, max, loss, same, res = FALSE;


	/* Acquire current value */
	cur = p_ptr->stat_cur[stat];
	max = p_ptr->stat_max[stat];

	/* Note when the values are identical */
	same = (cur == max);

	/* Damage "current" value */
	if (cur > 3)
	{
		/* Handle "low" values */
		if (cur <= 18)
		{
			if (amount > 90) cur--;
			if (amount > 50) cur--;
			if (amount > 20) cur--;
			cur--;
		}

		/* Handle "high" values */
		else
		{
			/* Hack -- Decrement by a random amount between one-quarter */
			/* and one-half of the stat bonus times the percentage, with a */
			/* minimum damage of half the percentage. -CWS */
			loss = (((cur-18) / 2 + 1) / 2 + 1);

			/* Paranoia */
			if (loss < 1) loss = 1;

			/* Randomize the loss */
			loss = ((randint(loss) + loss) * amount) / 100;

			/* Maximal loss */
			if (loss < amount/2) loss = amount/2;

			/* Lose some points */
			cur = cur - loss;

			/* Hack -- Only reduce stat to 17 sometimes */
			if (cur < 18) cur = (amount <= 20) ? 18 : 17;
		}

		/* Prevent illegal values */
		if (cur < 3) cur = 3;

		/* Something happened */
		if (cur != p_ptr->stat_cur[stat]) res = TRUE;
	}

	/* Damage "max" value */
	if (permanent && (max > 3))
	{
		/* Handle "low" values */
		if (max <= 18)
		{
			if (amount > 90) max--;
			if (amount > 50) max--;
			if (amount > 20) max--;
			max--;
		}

		/* Handle "high" values */
		else
		{
			/* Hack -- Decrement by a random amount between one-quarter */
			/* and one-half of the stat bonus times the percentage, with a */
			/* minimum damage of half the percentage. -CWS */
			loss = (((max-18) / 2 + 1) / 2 + 1);
			loss = ((randint(loss) + loss) * amount) / 100;
			if (loss < amount/2) loss = amount/2;

			/* Lose some points */
			max = max - loss;

			/* Hack -- Only reduce stat to 17 sometimes */
			if (max < 18) max = (amount <= 20) ? 18 : 17;
		}

		/* Hack -- keep it clean */
		if (same || (max < cur)) max = cur;

		/* Something happened */
		if (max != p_ptr->stat_max[stat]) res = TRUE;
	}

	/* Apply changes */
	if (res)
	{
		/* Actually set the stat to its new value. */
		p_ptr->stat_cur[stat] = cur;
		p_ptr->stat_max[stat] = max;

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);
	}

	/* Done */
	return (res);
}


/*
 * Restore a stat.  Return TRUE only if this actually makes a difference.
 */
bool res_stat(int stat)
{
	/* Restore if needed */
	if (p_ptr->stat_cur[stat] != p_ptr->stat_max[stat])
	{
		/* Restore */
		p_ptr->stat_cur[stat] = p_ptr->stat_max[stat];

		/* Recalculate bonuses */
		p_ptr->update |= (PU_BONUS);

		/* Success */
		return (TRUE);
	}

	/* Nothing to restore */
	return (FALSE);
}




/*
 * Apply disenchantment to the player's stuff
 *
 * XXX XXX XXX This function is also called from the "melee" code
 *
 * The "mode" is currently unused.
 *
 * Return "TRUE" if the player notices anything
 */
bool apply_disenchant(int mode)
{
	int t = 0;

	object_type *o_ptr;

	char o_name[80];

	bool done = TRUE;


	/* Unused */
	mode = mode;


	/* Pick a random slot */
	switch (randint(8))
	{
		case 1: t = INVEN_WIELD; break;
		case 2: t = INVEN_BOW; break;
		case 3: t = INVEN_BODY; break;
		case 4: t = INVEN_OUTER; break;
		case 5: t = INVEN_ARM; break;
		case 6: t = INVEN_HEAD; break;
		case 7: t = INVEN_HANDS; break;
		case 8: t = INVEN_FEET; break;
	}

	/* Get the item */
	o_ptr = &inventory[t];

	/* No item, nothing happens */
	if (!o_ptr->k_idx) return (FALSE);


	/* Nothing to disenchant */
	if ((o_ptr->to_h > 0) || (o_ptr->to_d > 0) || (o_ptr->to_a > 0))
	{
		/* Describe the object */
		object_desc(o_name, o_ptr, FALSE, 0);

		/* Artifacts have 60% chance to resist */
		if (artifact_p(o_ptr) && (rand_int(100) < 60))
		{
			/* Message */
			msg_format("Your %s (%c) resist%s disenchantment!",
						  o_name, index_to_label(t),
						  ((o_ptr->number != 1) ? "" : "s"));

			/* Notice */
			return (TRUE);
		}


		/* Disenchant tohit */
		if (o_ptr->to_h > 0) o_ptr->to_h--;
		if ((o_ptr->to_h > 5) && (rand_int(100) < 20)) o_ptr->to_h--;

		/* Disenchant todam */
		if (o_ptr->to_d > 0) o_ptr->to_d--;
		if ((o_ptr->to_d > 5) && (rand_int(100) < 20)) o_ptr->to_d--;

		/* Disenchant toac */
		if (o_ptr->to_a > 0) o_ptr->to_a--;
		if ((o_ptr->to_a > 5) && (rand_int(100) < 20)) o_ptr->to_a--;

		/* Message */
		msg_format("Your %s (%c) %s disenchanted!",
					  o_name, index_to_label(t),
					  ((o_ptr->number != 1) ? "were" : "was"));

		done = FALSE;
	}

	/* It gets worse... */
	if (!rand_int(10))
	{
		if (t < INVEN_BODY) curse_weapon(t);
		else curse_armor(t);
	}
	/* Nothing happened */
	else if (done) return (FALSE);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Window stuff */
	p_ptr->window |= (PW_EQUIP | PW_SPELL | PW_PLAYER);

	/* Notice */
	return (TRUE);
}


/*
 * Apply Nexus
 */
static void apply_nexus(monster_type *m_ptr)
{
	int max1, cur1, max2, cur2, ii, jj;

	switch (randint(7))
	{
		case 1: case 2: case 3:
		{
			teleport_player(200);
			break;
		}

		case 4: case 5:
		{
			teleport_player_to(m_ptr->fy, m_ptr->fx);
			break;
		}

		case 6:
		{
			if (rand_int(100) < p_ptr->skill_sav)
			{
				msg_print("You resist the effects!");
				break;
			}

			/* Teleport Level */
			teleport_player_level();
			break;
		}

		case 7:
		{
			if (rand_int(100) < p_ptr->skill_sav)
			{
				msg_print("You resist the effects!");
				break;
			}

			msg_print("Your body starts to scramble...");

			/* Pick a pair of stats */
			ii = rand_int(6);
			for (jj = ii; jj == ii; jj = rand_int(6)) /* loop */;

			max1 = p_ptr->stat_max[ii];
			cur1 = p_ptr->stat_cur[ii];
			max2 = p_ptr->stat_max[jj];
			cur2 = p_ptr->stat_cur[jj];

			p_ptr->stat_max[ii] = max2;
			p_ptr->stat_cur[ii] = cur2;
			p_ptr->stat_max[jj] = max1;
			p_ptr->stat_cur[jj] = cur1;

			p_ptr->update |= (PU_BONUS);

			break;
		}
	}
}





/*
 * Mega-Hack -- track "affected" monsters (see "project()" comments)
 */
static int project_m_n;
static int project_m_x;
static int project_m_y;



/*
 * We are called from "project()" to "damage" terrain features
 *
 * We are called both for "beam" effects and "ball" effects.
 *
 * The "r" parameter is the "distance from ground zero".
 *
 * Note that we determine if the player can "see" anything that happens
 * by taking into account: blindness, line-of-sight, and illumination.
 *
 * We return "TRUE" if the effect of the projection is "obvious".
 *
 * Hack -- We also "see" grids which are "memorized".
 *
 * Perhaps we should affect doors and/or walls. XXX XXX
 */
static bool project_f(int who, int r, int y, int x, int dam, int typ)
{
	bool obvious = FALSE;

	int feat = cave_feat[y][x];

	who = who;

	/* Reduce damage by distance */
	dam = (dam + r) / (r + 1);


	/* Analyze the type */
	switch (typ)
	{
		/* Ignore most effects */
		case GF_ACID:
		case GF_ELEC:
		case GF_FIRE:
		case GF_COLD:
		case GF_ICE:
		case GF_SHARD:
		case GF_SOUND:
		case GF_HOLY_ORB:
		{
			break;
		}


		/* Damage walls, leave rubble, and break doors */
		case GF_METEOR:
		{
			/* Leave permanent rock alone */
			if (is_hard_wall(feat)) break;

			/* Destroy rubble */
			if ((feat == FEAT_RUBBLE) && 
				 (rand_int(100) < 30))
			{
				/* Check line of sight */
				if (player_has_los_bold(y, x))
				{
					/* Message */
					msg_print("The rubble is scattered.");
					obvious = TRUE;

					/* Update some things */
					p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);
				}

				/* Forget the rubble */
				cave_info[y][x] &= ~(CAVE_MARK);

				/* Destroy the feature */
				cave_set_feat(y, x, FEAT_FLOOR);
			}

			/* Reduce walls to rubble */
			else if ((feat >= FEAT_MAGMA) && 
						(rand_int(100) < 10))
			{
				/* Check line of sight */
				if (player_has_los_bold(y, x))
				{
					/* Message */
					msg_print("The rock crumbles.");
					obvious = TRUE;
				}

				/* Turn to rubble */
				cave_set_feat(y, x, FEAT_RUBBLE);
			}

			/* Intentional omission of "break". Doors should be broken. */
		}	
		/* Break doors */
		case GF_FORCE:
		{
			if (is_door(feat) || (feat == FEAT_SECRET))
			{
				/* Check line of sight */
				if (player_has_los_bold(y, x))
				{
					/* Message */
					msg_print("The door is shattered!");
					obvious = TRUE;

					/* Update some things */
					p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);
				}

				/* Forget the door */
				cave_info[y][x] &= ~(CAVE_MARK);
					
				/* Break the door */
				cave_set_feat(y, x, FEAT_BROKEN);
			}

			break;
		}
		

		/* Melt doors */
		case GF_PLASMA:
		{
			if (is_door(feat) || (feat == FEAT_SECRET))
			{
				/* Check line of sight */
				if (player_has_los_bold(y, x) && 
					 (feat != FEAT_SECRET))
				{
					/* Message */
					msg_print("The door is fused shut!");
					obvious = TRUE;
				}

				/* Make a wall */
				cave_set_feat(y, x, FEAT_WALL_EXTRA);
			}

			break;
		}


		/* Time -- do strange things */
		case GF_TIME:
		{
			/* Permanent walls - always there */
			if (is_hard_wall(feat)) break;

			if (feat == FEAT_GLYPH)
			{
				if (rand_int(3))
				{
					obvious = TRUE;
					cave_set_feat(y, x, FEAT_FLOOR);
				}

				/* Check line of sight */
				if (player_has_los_bold(y, x))
				{
					/* Message */
					if (obvious) msg_print("The rune of protection is broken!");
					else msg_print ("The rune of protection hisses furiously!");

					obvious = FALSE;

					/* Update some things */
					p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);
				}
			}

			/* Close doors */
			if (feat == FEAT_OPEN)
			{
				cave_set_feat(y, x, rand_int(8) + FEAT_DOOR_HEAD);

				/* Check line of sight */
				if (player_has_los_bold(y, x))
				{
					/* Message */
					msg_print("The door closes!");

					/* Update some things */
					p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);
				}
			}

			/* Jam doors */
			if (feat == FEAT_BROKEN)
			{
				cave_set_feat(y, x, 8 + rand_int(8) + FEAT_DOOR_HEAD);

				/* Check line of sight */
				if (player_has_los_bold(y, x))
				{
					/* Message */
					msg_print("The door slams shut!");

					/* Update some things */
					p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);
				}
			}

			/* Destroy doors and traps sometimes*/
			if (((feat == FEAT_INVIS) || (is_trap(feat)) ||
				 (is_door(feat))) && !rand_int(10))
			{
				/* Forget the door */
				cave_info[y][x] &= ~(CAVE_MARK);

				obvious = TRUE;

				/* Destroy the feature */
				cave_set_feat(y, x, FEAT_FLOOR);
			}

			/* Restore the original rock */
			if ((feat == FEAT_FLOOR) && !rand_int(10))
			{
				/* Make rubble */
				cave_set_feat(y, x, FEAT_RUBBLE);

				obvious = TRUE;
			}
         /* A monster was there */
			else if ((feat == FEAT_FLOOR) && !rand_int(100))
			{
				summon_specific(y, x, p_ptr->depth, 0);
			}

			/* Restore the original rock */
			if ((feat == FEAT_RUBBLE) && !rand_int(10))
			{
				/* Make rubble */
				cave_set_feat(y, x, FEAT_WALL_EXTRA);

				obvious = TRUE;
			}

			/* Disperse treasure */
			if ((feat == FEAT_MAGMA_H) ||
				 (feat == FEAT_MAGMA_K))
			{
				/* Make magma */
				cave_set_feat(y, x, FEAT_MAGMA);
			}

			/* Disperse treasure */
			if ((feat == FEAT_QUARTZ_H) ||
				 (feat == FEAT_QUARTZ_K))
			{
				/* Make magma */
				cave_set_feat(y, x, FEAT_QUARTZ);
			}

			/* Check line of sight */
			if (player_has_los_bold(y, x))
			{
				/* Message */
				if (obvious) msg_print("Part of the cave seems to be missing...");
				obvious = TRUE;

				/* Update some things */
				p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);
			}

			break;
		}


		/* Do lots of damage */
		case GF_MANA:
		{
			/* Leave permanent rock alone */
			if (is_hard_wall(feat)) break;

			/* Destroy all doors and traps */
			if ((feat == FEAT_OPEN) ||
				 (feat == FEAT_BROKEN) ||
				 (feat == FEAT_INVIS) || (feat == FEAT_SECRET) ||
				 is_trap(feat) || is_door(feat))
			{
				/* Check line of sight */
				if (player_has_los_bold(y, x))
				{
					/* Message */
					msg_print("There is a bright flash of light!");
					obvious = TRUE;

					/* Visibility change */
					if (is_door(feat) || (feat == FEAT_SECRET))
					{
						/* Update some things */
						p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);
					}
				}

				/* Forget the door */
				cave_info[y][x] &= ~(CAVE_MARK);

				/* Destroy the feature */
				cave_set_feat(y, x, FEAT_FLOOR);
			}

			/* Destroy rubble */
			else if (feat == FEAT_RUBBLE)
			{
				/* Check line of sight */
				if (player_has_los_bold(y, x))
				{
					/* Message */
					msg_print("The rubble turns to dust.");
					obvious = TRUE;

					/* Update some things */
					p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);
				}

				/* Forget the rubble */
				cave_info[y][x] &= ~(CAVE_MARK);

				/* Destroy the feature */
				cave_set_feat(y, x, FEAT_FLOOR);
			}

			/* Reduce walls to rubble */
			else if (is_soft_wall(feat))
			{
				/* Check line of sight */
				if (player_has_los_bold(y, x))
				{
					/* Message */
					msg_print("The rock crumbles.");
					obvious = TRUE;

					obvious = TRUE;

					/* Update some things */
					p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);
				}

				/* Forget the wall */
				cave_info[y][x] &= ~(CAVE_MARK);

				if (rand_int(100) < 60)
				{
					/* Turn to rubble */
					cave_set_feat(y, x, FEAT_RUBBLE);
				}
				else
				{
					/* Turn to dust */
					cave_set_feat(y, x, FEAT_FLOOR);
				}
			}

			break;
		}


		/* Destroy Traps (and Locks) */
		case GF_KILL_TRAP:
		{
			/* Destroy traps */
			if ((feat == FEAT_INVIS) || is_trap(feat))
			{
				/* Check line of sight */
				if (player_has_los_bold(y, x))
				{
					msg_print("There is a bright flash of light!");
					obvious = TRUE;
				}

				/* Forget the trap */
				cave_info[y][x] &= ~(CAVE_MARK);

				/* Destroy the trap */
				cave_set_feat(y, x, FEAT_FLOOR);
			}

			/* Secret / Locked doors are found and unlocked */
			else if ((feat == FEAT_SECRET) ||
						((feat >= FEAT_DOOR_HEAD + 0x01) &&
						 (feat <= FEAT_DOOR_HEAD + 0x07)))
			{
				/* Unlock the door */
				cave_set_feat(y, x, FEAT_DOOR_HEAD + 0x00);

				/* Check line of sound */
				if (player_has_los_bold(y, x))
				{
					msg_print("Click!");
					obvious = TRUE;
				}
			}

			break;
		}

		/* Destroy Doors (and traps) */
		case GF_KILL_DOOR:
		{
			/* Destroy all doors and traps */
			if ((feat == FEAT_OPEN) ||
				 (feat == FEAT_BROKEN) ||
				 (feat == FEAT_INVIS) ||
				 is_trap(feat) || is_door(feat))
			{
				/* Check line of sight */
				if (player_has_los_bold(y, x))
				{
					/* Message */
					msg_print("There is a bright flash of light!");
					obvious = TRUE;

					/* Visibility change */
					if (is_door(feat))
					{
						/* Update some things */
						p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);
					}
				}

				/* Forget the door */
				cave_info[y][x] &= ~(CAVE_MARK);

				/* Destroy the feature */
				cave_set_feat(y, x, FEAT_FLOOR);
			}

			break;
		}

		/* Destroy anything */
		case GF_MAKE_SANC:
		{
			int t;

			/* Permanent walls */
			if (is_hard_wall(feat)) break;

			/* Turn on the light */
			cave_info[y][x] |= (CAVE_GLOW);

			/* Notice */
			note_spot(y, x);

			/* Redraw */
			lite_spot(y, x);

			/* Player */
			if ((x == p_ptr->px) && (y == p_ptr->py))
			{
				cave_set_feat(y, x, FEAT_FLOOR);
				break;
			}

			/* Glyph of warding */
			if (feat == FEAT_GLYPH) break;

			/* Wall (or floor) type */
			t = rand_int(100);

			/* Granite */
			if (t < 20)
			{
				/* Create granite wall */
				cave_set_feat(y, x, FEAT_WALL_EXTRA);
			}

			/* Quartz */
			else if (t < 70)
			{
				/* Create quartz vein */
				cave_set_feat(y, x, FEAT_QUARTZ);
			}

			/* Magma */
			else if (t < 100)
			{
				/* Create magma vein */
				cave_set_feat(y, x, FEAT_MAGMA);
			}

			/* Observe */
			if (cave_info[y][x] & (CAVE_MARK)) obvious = TRUE;

			/* Update some things */
			p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);

			break;
		}

		/* Destroy walls (and doors) */
		case GF_KILL_WALL:
		{
			/* Non-walls (etc) */
			if (cave_floor_bold(y, x)) break;

			/* Permanent walls */
			if (is_hard_wall(feat)) break;

			/* Granite */
			if (is_granite(feat))
			{
				/* Message */
				if (cave_info[y][x] & (CAVE_MARK))
				{
					msg_print("The wall turns into mud!");
					obvious = TRUE;
				}

				/* Forget the wall */
				cave_info[y][x] &= ~(CAVE_MARK);

				/* Destroy the wall */
				cave_set_feat(y, x, FEAT_FLOOR);
			}

			/* Quartz / Magma with treasure */
			else if (is_treasure(feat))
			{
				/* Message */
				if (cave_info[y][x] & (CAVE_MARK))
				{
					msg_print("The vein turns into mud!");
					msg_print("You have found something!");
					obvious = TRUE;
				}

				/* Forget the wall */
				cave_info[y][x] &= ~(CAVE_MARK);

				/* Destroy the wall */
				cave_set_feat(y, x, FEAT_FLOOR);

				/* Place some gold */
				place_gold(y, x);
			}

			/* Quartz / Magma */
			else if (is_vein(feat))
			{
				/* Message */
				if (cave_info[y][x] & (CAVE_MARK))
				{
					msg_print("The vein turns into mud!");
					obvious = TRUE;
				}

				/* Forget the wall */
				cave_info[y][x] &= ~(CAVE_MARK);

				/* Destroy the wall */
				cave_set_feat(y, x, FEAT_FLOOR);
			}

			/* Rubble */
			else if (feat == FEAT_RUBBLE)
			{
				/* Message */
				if (cave_info[y][x] & (CAVE_MARK))
				{
					msg_print("The rubble turns into mud!");
					obvious = TRUE;
				}

				/* Forget the wall */
				cave_info[y][x] &= ~(CAVE_MARK);

				/* Destroy the rubble */
				cave_set_feat(y, x, FEAT_FLOOR);

				/* Hack -- place an object */
				if (rand_int(100) < 10)
				{
					/* Found something */
					if (player_can_see_bold(y, x))
					{
						msg_print("There was something buried in the rubble!");
						obvious = TRUE;
					}

					/* Place gold */
					place_object(y, x, FALSE, FALSE);
				}
			}

			/* Destroy doors (and secret doors) */
			else
			{
				/* Hack -- special message */
				if (cave_info[y][x] & (CAVE_MARK))
				{
					msg_print("The door turns into mud!");
					obvious = TRUE;
				}

				/* Forget the wall */
				cave_info[y][x] &= ~(CAVE_MARK);

				/* Destroy the feature */
				cave_set_feat(y, x, FEAT_FLOOR);
			}

			/* Update some things */
			p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);

			break;
		}

		/* Make doors */
		case GF_MAKE_DOOR:
		{
			/* Require a "naked" floor grid */
			if (!cave_naked_bold(y, x)) break;

			/* Create closed door */
			cave_set_feat(y, x, FEAT_DOOR_HEAD + 0x00);

			/* Observe */
			if (cave_info[y][x] & (CAVE_MARK)) obvious = TRUE;

			/* Update some things */
			p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);

			break;
		}

		/* Make traps */
		case GF_MAKE_TRAP:
		{
			/* Require a "naked" floor grid */
			if (!cave_naked_bold(y, x)) break;

			/* Place a trap */
			place_trap(y, x);

			break;
		}

		/* Lite up the grid */
		case GF_LITE_WEAK:
		case GF_LITE:
		{
			/* Turn on the light */
			cave_info[y][x] |= (CAVE_GLOW);

			/* Notice */
			note_spot(y, x);

			/* Redraw */
			lite_spot(y, x);

			/* Observe */
			if (player_can_see_bold(y, x)) obvious = TRUE;

			/* Mega-Hack -- Update the monster in the affected grid */
			/* This allows "spear of light" (etc) to work "correctly" */
			if (cave_m_idx[y][x] > 0) update_mon(cave_m_idx[y][x], FALSE);

			break;
		}

		/* Darken the grid */
		case GF_DARK_WEAK:
		case GF_DARK:
		{
			/* Notice */
			if (player_can_see_bold(y, x)) obvious = TRUE;

			/* Turn off the light. */
			cave_info[y][x] &= ~(CAVE_GLOW);

			/* Hack -- Forget "boring" grids */
			if (is_floor(feat))
			{
				/* Forget */
				cave_info[y][x] &= ~(CAVE_MARK);

				/* Notice */
				note_spot(y, x);
			}

			/* Redraw */
			lite_spot(y, x);

			/* Mega-Hack -- Update the monster in the affected grid */
			/* This allows "spear of light" (etc) to work "correctly" */
			if (cave_m_idx[y][x] > 0) update_mon(cave_m_idx[y][x], FALSE);

			/* All done */
			break;
		}
	}

	/* Return "Anything seen?" */
	return (obvious);
}



/*
 * We are called from "project()" to "damage" objects
 *
 * We are called both for "beam" effects and "ball" effects.
 *
 * Perhaps we should only SOMETIMES damage things on the ground.
 *
 * The "r" parameter is the "distance from ground zero".
 *
 * Note that we determine if the player can "see" anything that happens
 * by taking into account: blindness, line-of-sight, and illumination.
 *
 * Hack -- We also "see" objects which are "memorized".
 *
 * We return "TRUE" if the effect of the projection is "obvious".
 */
static bool project_o(int who, int r, int y, int x, int dam, int typ)
{
	s16b this_o_idx, next_o_idx = 0;

	bool obvious = FALSE;

	bool seen = FALSE;
	
	s16b explode, explode_dam;

	u32b f1, f2, f3;

	char o_name[80];

	who = who;

	/* Reduce damage by distance */
	dam = (dam + r) / (r + 1);

	if (player_can_see_bold(y, x)) seen = TRUE;

	/* Scan all objects in the grid */
	for (this_o_idx = cave_o_idx[y][x]; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type *o_ptr;

		bool is_art = FALSE;
		bool ignore = FALSE;
		bool plural = FALSE;
		bool do_kill = FALSE;

		/* Explode by default */
		bool no_explode = FALSE;

		cptr note_kill = NULL;

		/* Acquire object */
		o_ptr = &o_list[this_o_idx];

		/* Acquire next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Extract the flags */
		object_flags(o_ptr, &f1, &f2, &f3);

		/* Get the "plural"-ness */
		if (o_ptr->number > 1) plural = TRUE;

		/* Check for artifact */
		if (artifact_p(o_ptr)) is_art = TRUE;

		/* Analyze the type */
		switch (typ)
		{
			/* Acid -- Lots of things */
			case GF_ACID:
			{
				if (hates_acid(o_ptr))
				{
					do_kill = TRUE;
					note_kill = (plural ? " dissolve!" : " dissolves!");
					if (f3 & (TR3_IGNORE_ACID)) ignore = TRUE;
				}
				break;
			}

			/* Elec -- Rings and Wands */
			case GF_ELEC:
			{
				if (hates_elec(o_ptr))
				{
					do_kill = TRUE;
					note_kill = (plural ? " are destroyed!" : " is destroyed!");
					if (f3 & (TR3_IGNORE_ELEC)) ignore = TRUE;
				}
				break;
			}

			/* Fire -- Flammable objects */
			case GF_FIRE:
			{
				if (hates_fire(o_ptr))
				{
					do_kill = TRUE;
					note_kill = (plural ? " burn up!" : " burns up!");
					if (f3 & (TR3_IGNORE_FIRE)) ignore = TRUE;
				}
				break;
			}

			/* Cold -- potions and flasks */
			case GF_COLD:
			{
				if (hates_cold(o_ptr))
				{
					note_kill = (plural ? " shatter!" : " shatters!");
					do_kill = TRUE;
					if (f3 & (TR3_IGNORE_COLD)) ignore = TRUE;
				}
				break;
			}

			/* Fire + Elec */
			case GF_PLASMA:
			{
				if (hates_fire(o_ptr))
				{
					do_kill = TRUE;
					note_kill = (plural ? " burn up!" : " burns up!");
					if (f3 & (TR3_IGNORE_FIRE)) ignore = TRUE;
				}
				if (hates_elec(o_ptr))
				{
					ignore = FALSE;
					do_kill = TRUE;
					note_kill = (plural ? " are destroyed!" : " is destroyed!");
					if (f3 & (TR3_IGNORE_ELEC)) ignore = TRUE;
				}
				break;
			}

			/* Fire + Cold */
			case GF_METEOR:
			{
				if (hates_fire(o_ptr))
				{
					do_kill = TRUE;
					note_kill = (plural ? " burn up!" : " burns up!");
					if (f3 & (TR3_IGNORE_FIRE)) ignore = TRUE;
				}
				if (hates_cold(o_ptr))
				{
					ignore = FALSE;
					do_kill = TRUE;
					note_kill = (plural ? " shatter!" : " shatters!");
					if (f3 & (TR3_IGNORE_COLD)) ignore = TRUE;
				}
				break;
			}

			/* Hack -- break potions and such */
			case GF_ICE:
			case GF_FORCE:
			case GF_SOUND:
			{
				if (hates_cold(o_ptr))
				{
					note_kill = (plural ? " shatter!" : " shatters!");
					do_kill = TRUE;
				}
				break;
			}

			/* Time -- destroy totally */
			case GF_TIME:
			{
				/* No longer here */
				note_kill = (plural ? " have ceased to exist!" : " has ceased to exist!");

				/*** DOOM ***/
				is_art = FALSE;

				do_kill = TRUE;
				no_explode = TRUE;

				break;
			}

			/* Mana -- destroys everything */
			case GF_MANA:
			{
				do_kill = TRUE;
				note_kill = (plural ? " are destroyed!" : " is destroyed!");
			}

			/* Holy Orb -- destroys cursed non-artifacts */
			case GF_HOLY_ORB:
			{
				if (cursed_p(o_ptr))
				{
					do_kill = TRUE;
					note_kill = (plural ? " are destroyed!" : " is destroyed!");
				}
				break;
			}

			/* Rock -- destroys everything but artifacts */
			case GF_MAKE_SANC:
			{
				if (!is_art) 
				{
					/* Prevent explosion */
					no_explode = TRUE;

					do_kill = TRUE;
				}
				else
				{
					/* Forget the wall */
					cave_info[y][x] &= ~(CAVE_MARK);

					/* Destroy the feature */
					cave_set_feat(y, x, FEAT_FLOOR);

					/* Update some things */
					p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);
				}
				
				note_kill = (plural ? " are crushed!" : " is crushed!");
			}

			/* Unlock chests */
			case GF_KILL_TRAP:
			case GF_KILL_DOOR:
			{
				/* Chests are noticed only if trapped or locked */
				if (o_ptr->tval == TV_CHEST)
				{
					/* Disarm/Unlock traps */
					if (o_ptr->pval > 0)
					{
						/* Disarm or Unlock */
						o_ptr->pval = (0 - o_ptr->pval);

						/* Identify */
						object_known(o_ptr);

						/* Notice */
						if (o_ptr->marked)
						{
							msg_print("Click!");
							obvious = TRUE;
						}
					}
				}

				break;
			}
		}


		/* Attempt to destroy the object */
		if (do_kill)
		{
			/* Effect "observed" */
			if (o_ptr->marked)
			{
				obvious = TRUE;

				object_desc(o_name, o_ptr, FALSE, 0);
			}

			/* Artifacts, and other objects, get to resist */
			if (is_art || ignore)
			{
				/* Observe the resist */
				if (o_ptr->marked && seen)
				{
					msg_format("The %s %s unaffected!",
								  o_name, (plural ? "are" : "is"));
				}
			}

			/* Kill it */
			else
			{
				/* Object might explode */
				explode = hates_dam(o_ptr);

				/* Damage from explosion */
				explode_dam = object_force(o_ptr) * o_ptr->number;

				/* Message */
				if (o_ptr->marked && note_kill && seen)
				{
					msg_format("The %s%s", o_name, note_kill);
				}

				/* Delete the object */
				delete_object_idx(this_o_idx);

				/* Explode it if needed */
				if (!no_explode && explode && explode_dam)
				{
					int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL | PROJECT_JUMP;
					project(0, 1, y, x, explode_dam, explode, flg);
				}

				/* Redraw */
				lite_spot(y, x);
			}
		}
	}

	/* Return "Anything seen?" */
	return (obvious && seen);
}



/*
 * Helper function for "project()" below.
 *
 * Handle a beam/bolt/ball causing damage to a monster.
 *
 * This routine takes a "source monster" (by index) which is mostly used to
 * determine if the player is causing the damage, and a "radius" (see below),
 * which is used to decrease the power of explosions with distance, and a
 * location, via integers which are modified by certain types of attacks
 * (polymorph and teleport being the obvious ones), a default damage, which
 * is modified as needed based on various properties, and finally a "damage
 * type" (see below).
 *
 * Note that this routine can handle "no damage" attacks (like teleport) by
 * taking a "zero" damage, and can even take "parameters" to attacks (like
 * confuse) by accepting a "damage", using it to calculate the effect, and
 * then setting the damage to zero.  Note that the "damage" parameter is
 * divided by the radius, so monsters not at the "epicenter" will not take
 * as much damage (or whatever)...
 *
 * Note that "polymorph" is dangerous, since a failure in "place_monster()"'
 * may result in a dereference of an invalid pointer.  XXX XXX XXX
 *
 * Various messages are produced, and damage is applied.
 *
 * Just "casting" a substance (i.e. plasma) does not make you immune, you must
 * actually be "made" of that substance, or "breathe" big balls of it.
 *
 * We assume that "Plasma" monsters, and "Plasma" breathers, are immune
 * to plasma.
 *
 * We assume "Nether" is an evil, necromantic force, so it doesn't hurt undead,
 * and hurts evil less.  If can breath nether, then it resists it as well.
 *
 * Damage reductions use the following formulas:
 *   Note that "dam = dam * 6 / (randint(6) + 6);"
 *     gives avg damage of .655, ranging from .858 to .500
 *   Note that "dam = dam * 5 / (randint(6) + 6);"
 *     gives avg damage of .544, ranging from .714 to .417
 *   Note that "dam = dam * 4 / (randint(6) + 6);"
 *     gives avg damage of .444, ranging from .556 to .333
 *   Note that "dam = dam * 3 / (randint(6) + 6);"
 *     gives avg damage of .327, ranging from .427 to .250
 *   Note that "dam = dam * 2 / (randint(6) + 6);"
 *     gives something simple.
 *
 * In this function, "result" messages are postponed until the end, where
 * the "note" string is appended to the monster name, if not NULL.  So,
 * to make a spell have "no effect" just set "note" to NULL.  You should
 * also set "notice" to FALSE, or the player will learn what the spell does.
 *
 * We attempt to return "TRUE" if the player saw anything "useful" happen.
 */
static bool project_m(int who, int r, int y, int x, int dam, int typ)
{
	int tmp;

	monster_type *m_ptr;
	monster_race *r_ptr;
	monster_lore *l_ptr;

	cptr name;

	/* Is the monster "seen"? */
	bool seen = FALSE;

	/* Were the effects "obvious" (if seen)? */
	bool obvious = FALSE;

	/* Were the effects "irrelevant"? */
	bool skipped = FALSE;


	/* Polymorph setting (true or false) */
	int do_poly = 0;

	/* Time setting (true or false) */
	int do_time = 0;

	/* Teleport setting (max distance) */
	int do_dist = 0;

	/* Confusion setting (amount to confuse) */
	int do_conf = 0;

	/* Stunning setting (amount to stun) */
	int do_stun = 0;

	/* Sleep amount (amount to sleep) */
	int do_sleep = 0;

	/* Fear amount (amount to fear) */
	int do_fear = 0;


	/* Hold the monster name */
	char m_name[80];

	/* Assume no note */
	cptr note = NULL;

	/* Assume a default death */
	cptr note_dies = " dies.";

	/* Walls protect monsters unless they're moving */
	if (!cave_floor_bold(y,x) &&
		 (typ != GF_MAKE_WALL) &&
		 (typ != GF_MAKE_SANC) &&
		 (typ != GF_TIME))
	{
		return (FALSE);
	}

	/* No monster here */
	if (!(cave_m_idx[y][x] > 0)) return (FALSE);

	/* Never affect projector */
	if (cave_m_idx[y][x] == who) return (FALSE);


	/* Obtain monster info */
	m_ptr = &m_list[cave_m_idx[y][x]];
	r_ptr = &r_info[m_ptr->r_idx];
	l_ptr = &l_info[m_ptr->r_idx];
	name = (r_name + r_ptr->name);
	if (m_ptr->ml) seen = TRUE;


	/* Reduce damage by distance */
	dam = (dam + r) / (r + 1);


	/* Get the monster name (BEFORE polymorphing) */
	monster_desc(m_name, m_ptr, 0);



	/* Some monsters get "destroyed" */
	if ((r_ptr->flags3 & (RF3_DEMON)) ||
		 (r_ptr->flags3 & (RF3_UNDEAD)) ||
		 (r_ptr->flags2 & (RF2_STUPID)) ||
		 (strchr("Evg", r_ptr->d_char)))
	{
		/* Special note at death */
		note_dies = " is destroyed.";
	}


	/* Analyze the damage type */
	switch (typ)
	{
		/* Magic Missile -- pure damage */
		case GF_MISSILE:
		{
			if (seen) obvious = TRUE;
			break;
		}

		/* Acid */
		case GF_ACID:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_IM_ACID))
			{
				note = " resists a lot.";
				dam /= 9;
				if (seen) l_ptr->flags3 |= (RF3_IM_ACID);
			}
			break;
		}

		/* Electricity */
		case GF_ELEC:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_IM_ELEC))
			{
				note = " resists a lot.";
				dam /= 9;
				if (seen) l_ptr->flags3 |= (RF3_IM_ELEC);
			}
			break;
		}

		/* Fire damage */
		case GF_FIRE:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_IM_FIRE))
			{
				note = " resists a lot.";
				dam /= 9;
				if (seen) l_ptr->flags3 |= (RF3_IM_FIRE);
			}
			break;
		}

		/* Cold */
		case GF_COLD:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_IM_COLD))
			{
				note = " resists a lot.";
				dam /= 9;
				if (seen) l_ptr->flags3 |= (RF3_IM_COLD);
			}
			break;
		}

		/* Poison */
		case GF_POIS:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_IM_POIS))
			{
				note = " resists a lot.";
				dam /= 9;
				if (seen) l_ptr->flags3 |= (RF3_IM_POIS);
			}
			break;
		}

		/* Holy Orb -- hurts Evil */
		case GF_HOLY_ORB:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_EVIL))
			{
				dam *= 2;
				note = " is hit hard.";
				if (seen) l_ptr->flags3 |= (RF3_EVIL);
			}
			break;
		}

		/* Arrow -- XXX no defense */
		case GF_ARROW:
		{
			if (seen) obvious = TRUE;
			break;
		}

		/* Plasma -- XXX perhaps check ELEC or FIRE */
		case GF_PLASMA:
		{
			if (seen) obvious = TRUE;
			if (prefix(name, "Plasma") ||
				 (r_ptr->flags4 & (RF4_BR_PLAS)) ||
				 (r_ptr->flags3 & (RF3_RES_PLAS)))
			{
				note = " resists.";
				dam *= 3; dam /= (randint(6)+6);
			}
			break;
		}

		/* Nether -- see above */
		case GF_NETHER:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags3 & (RF3_UNDEAD))
			{
				note = " is immune.";
				dam = 0;
				if (seen) l_ptr->flags3 |= (RF3_UNDEAD);
			}
			else if (r_ptr->flags4 & (RF4_BR_NETH) || 
						r_ptr->flags3 & (RF3_RES_NETH))
			{
				note = " resists.";
				dam *= 3; dam /= (randint(6)+6);
			}
			else if (r_ptr->flags3 & (RF3_EVIL))
			{
				dam /= 2;
				note = " resists somewhat.";
				if (seen) l_ptr->flags3 |= (RF3_EVIL);
			}
			break;
		}

		/* Water (acid) damage -- Water spirits/elementals are immune */
		case GF_WATER:
		{
			if (seen) obvious = TRUE;
			if ((r_ptr->d_char == 'E') && prefix(name, "W"))
			{
				note = " is immune.";
				dam = 0;
			}
			else if (r_ptr->flags3 & (RF3_RES_WATE))
			{
				note = " resists.";
				dam *= 3; dam /= (randint(6)+6);
			}
			break;
		}

		/* Chaos -- Chaos breathers resist */
		case GF_CHAOS:
		{
			if (seen) obvious = TRUE;
			do_poly = TRUE;
			do_conf = (5 + randint(11) + r) / (r + 1);
			if (r_ptr->flags4 & (RF4_BR_CHAO))
			{
				note = " resists.";
				dam *= 3; dam /= (randint(6)+6);
				do_poly = FALSE;
			}
			break;
		}

		/* Shards -- Shard breathers resist */
		case GF_SHARD:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags4 & (RF4_BR_SHAR))
			{
				note = " resists.";
				dam *= 3; dam /= (randint(6)+6);
			}
			break;
		}

		/* Sound -- Sound breathers resist */
		case GF_SOUND:
		{
			if (seen) obvious = TRUE;
			do_stun = (10 + randint(15) + r) / (r + 1);
			if (r_ptr->flags4 & (RF4_BR_SOUN))
			{
				note = " resists.";
				dam *= 2; dam /= (randint(6)+6);
			}
			break;
		}

		/* Confusion */
		case GF_CONFUSION:
		{
			if (seen) obvious = TRUE;
			do_conf = (10 + randint(15) + r) / (r + 1);
			if (r_ptr->flags4 & (RF4_BR_CONF))
			{
				note = " resists.";
				dam *= 2; dam /= (randint(6)+6);
			}
			else if (r_ptr->flags3 & (RF3_NO_CONF))
			{
				note = " resists somewhat.";
				dam /= 2;
			}
			break;
		}

		/* Disenchantment -- Breathers and Disenchanters resist */
		case GF_DISENCHANT:
		{
			if (seen) obvious = TRUE;
			if ((r_ptr->flags4 & (RF4_BR_DISE)) ||
				 prefix(name, "Disen") || 
				 (r_ptr->flags3 & (RF3_RES_DISE)))
			{
				note = " resists.";
				dam *= 3; dam /= (randint(6)+6);
			}
			break;
		}

		/* Nexus -- Breathers and Existers resist */
		case GF_NEXUS:
		{
			if (seen) obvious = TRUE;
			if ((r_ptr->flags4 & (RF4_BR_NEXU)) ||
				 prefix(name, "Nexus") ||
				 (r_ptr->flags3 & (RF3_RES_NEXU)))
			{
				note = " resists.";
				dam *= 3; dam /= (randint(6)+6);
			}
			break;
		}

		/* Force */
		case GF_FORCE:
		{
			if (seen) obvious = TRUE;

			do_stun = (randint(15) + r) / (r + 1);
			do_dist = 1;

			if (r_ptr->flags4 & (RF4_BR_WALL))
			{
				note = " resists.";
				dam *= 3; dam /= (randint(6)+6);
			}
			break;
		}

		/* Inertia -- breathers resist */
		case GF_INERTIA:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags4 & (RF4_BR_INER))
			{
				note = " resists.";
				dam *= 3; dam /= (randint(6)+6);
			}
			break;
		}

		/* Time -- breathers resist */
		case GF_TIME:
		{
			if (seen) obvious = TRUE;

			do_time = TRUE;

			if (r_ptr->flags4 & (RF4_BR_TIME))
			{
				note = " resists.";
				dam *= 3; dam /= (randint(6)+6);
			}

			if (!cave_floor_bold(y, x) && !(r_ptr->flags2 & (RF2_PASS_WALL)))
			{
				/* Monster is certainly awake */
				m_ptr->csleep = 0;

				/* Immense damage */
				dam += 300;
			}

			break;
		}

		/* Gravity -- breathers resist */
		case GF_GRAVITY:
		{
			if (seen) obvious = TRUE;
			do_dist = 5;
			if (r_ptr->flags4 & (RF4_BR_GRAV))
			{
				note = " resists.";
				dam *= 3; dam /= (randint(6)+6);
			}
			break;
		}

		/* Pure damage */
		case GF_MANA:
		{
			if (seen) obvious = TRUE;
			break;
		}

		/* Meteor -- powerful magic missile */
		case GF_METEOR:
		{
			if (seen) obvious = TRUE;
			break;
		}

		/* Ice -- Cold + Cuts + Stun */
		case GF_ICE:
		{
			if (seen) obvious = TRUE;
			do_stun = (randint(15) + 1) / (r + 1);
			if (r_ptr->flags3 & (RF3_IM_COLD))
			{
				note = " resists a lot.";
				dam /= 9;
				if (seen) l_ptr->flags3 |= (RF3_IM_COLD);
			}
			break;
		}


		/* Drain Life */
		case GF_OLD_DRAIN:
		{
			if (seen) obvious = TRUE;
			if ((r_ptr->flags3 & (RF3_UNDEAD)) ||
				 (r_ptr->flags3 & (RF3_DEMON)) ||
				 (strchr("Egv", r_ptr->d_char)))
			{
				if (r_ptr->flags3 & (RF3_UNDEAD))
				{
					if (seen) l_ptr->flags3 |= (RF3_UNDEAD);
				}
				if (r_ptr->flags3 & (RF3_DEMON))
				{
					if (seen) l_ptr->flags3 |= (RF3_DEMON);
				}

				note = " is unaffected!";
				obvious = FALSE;
				dam = 0;
			}

			break;
		}

		/* Polymorph monster (Use "dam" as "power") */
		case GF_OLD_POLY:
		{
			if (seen) obvious = TRUE;

			/* Attempt to polymorph (see below) */
			do_poly = TRUE;

			/* Powerful monsters can resist */
			if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
				 (r_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				note = " is unaffected!";
				do_poly = FALSE;
				obvious = FALSE;
			}

			/* No "real" damage */
			dam = 0;

			break;
		}


		/* Clone monsters (Ignore "dam") */
		case GF_OLD_CLONE:
		{
			if (seen) obvious = TRUE;

			/* Heal fully */
			m_ptr->hp = m_ptr->maxhp;

			/* Speed up */
			if (m_ptr->mspeed < 150) m_ptr->mspeed += 10;

			/* Attempt to clone. */
			if (multiply_monster(cave_m_idx[y][x]))
			{
				note = " spawns!";
			}

			/* No "real" damage */
			dam = 0;

			break;
		}


		/* Heal Monster (use "dam" as amount of healing) */
		case GF_OLD_HEAL:
		{
			if (seen) obvious = TRUE;

			/* Wake up */
			m_ptr->csleep = 0;

			/* Heal */
			m_ptr->hp += dam;

			/* No overflow */
			if (m_ptr->hp > m_ptr->maxhp) m_ptr->hp = m_ptr->maxhp;

			/* Redraw (later) if needed */
			if (p_ptr->health_who == cave_m_idx[y][x]) p_ptr->redraw |= (PR_HEALTH);

			/* Message */
			note = " looks healthier.";

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Speed Monster (Ignore "dam") */
		case GF_OLD_SPEED:
		{
			if (seen) obvious = TRUE;

			/* Speed up */
			if (m_ptr->mspeed < 150) m_ptr->mspeed += 10;
			note = " starts moving faster.";

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Slow Monster (Use "dam" as "power") */
		case GF_OLD_SLOW:
		{
			if (seen) obvious = TRUE;

			/* Powerful monsters can resist */
			if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
			    (r_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				note = " is unaffected!";
				obvious = FALSE;
			}

			/* Normal monsters slow down */
			else
			{
				if (m_ptr->mspeed > 60) m_ptr->mspeed -= 10;
				note = " starts moving slower.";
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Sleep (Use "dam" as "power") */
		case GF_OLD_SLEEP:
		{
			if (seen) obvious = TRUE;

			/* Attempt a saving throw */
			if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
				 (r_ptr->flags3 & (RF3_NO_SLEEP)) ||
			    (r_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				/* Memorize a flag */
				if (r_ptr->flags3 & (RF3_NO_SLEEP))
				{
					if (seen) l_ptr->flags3 |= (RF3_NO_SLEEP);
				}

				/* No obvious effect */
				note = " is unaffected!";
				obvious = FALSE;
			}
			else
			{
				/* Go to sleep (much) later */
				note = " falls asleep!";
				do_sleep = 500;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Confusion (Use "dam" as "power") */
		case GF_OLD_CONF:
		{
			if (seen) obvious = TRUE;

			/* Get confused later */
			do_conf = damroll(3, (dam / 2)) + 1;

			/* Attempt a saving throw */
			if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
				 (r_ptr->flags3 & (RF3_NO_CONF)) ||
				 (r_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				/* Memorize a flag */
				if (r_ptr->flags3 & (RF3_NO_CONF))
				{
					if (seen) l_ptr->flags3 |= (RF3_NO_CONF);
				}

				/* Resist */
				do_conf = 0;

				/* No obvious effect */
				note = " is unaffected!";
				obvious = FALSE;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}



		/* Lite, but only hurts susceptible creatures */
		case GF_LITE_WEAK:
		{
			/* Hurt by light */
			if (r_ptr->flags3 & (RF3_HURT_LITE))
			{
				/* Obvious effect */
				if (seen) obvious = TRUE;

				/* Memorize the effects */
				if (seen) l_ptr->flags3 |= (RF3_HURT_LITE);

				/* Special effect */
				note = " cringes from the light!";
				note_dies = " shrivels away in the light!";
			}

			/* Normally no damage */
			else
			{
				/* No damage */
				dam = 0;
			}

			break;
		}



		/* Lite -- opposite of Dark */
		case GF_LITE:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags4 & (RF4_BR_LITE))
			{
				note = " resists.";
				dam *= 2; dam /= (randint(6)+6);
			}
			else if (r_ptr->flags3 & (RF3_HURT_LITE))
			{
				if (seen) l_ptr->flags3 |= (RF3_HURT_LITE);
				note = " cringes from the light!";
				note_dies = " shrivels away in the light!";
				dam *= 2;
			}
			break;
		}


		/* Dark -- opposite of Lite */
		case GF_DARK:
		{
			if (seen) obvious = TRUE;
			if (r_ptr->flags4 & (RF4_BR_DARK))
			{
				note = " resists.";
				dam *= 2; dam /= (randint(6)+6);
			}
			break;
		}


		/* Mages' Sanctuary */
		case GF_MAKE_SANC:
		{
			if (!(r_ptr->flags2 & (RF2_PASS_WALL)))
			{
				/* Monster is certainly awake */
				m_ptr->csleep = 0;

				/* Force monster away from player */
				do_dist = p_ptr->lev;
			}
			break;
		}


		/* Stone to Mud */
		case GF_KILL_WALL:
		{
			/* Hurt by rock remover */
			if (r_ptr->flags3 & (RF3_HURT_ROCK))
			{
				/* Notice effect */
				if (seen) obvious = TRUE;

				/* Memorize the effects */
				if (seen) l_ptr->flags3 |= (RF3_HURT_ROCK);

				/* Cute little message */
				note = " loses some skin!";
				note_dies = " dissolves!";
			}

			/* Usually, ignore the effects */
			else
			{
				/* No damage */
				dam = 0;
			}

			break;
		}


		/* Teleport undead (Use "dam" as "power") */
		case GF_AWAY_UNDEAD:
		{
			/* Only affect undead */
			if (r_ptr->flags3 & (RF3_UNDEAD))
			{
				if (seen) obvious = TRUE;
				if (seen) l_ptr->flags3 |= (RF3_UNDEAD);
				do_dist = dam;
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Teleport evil (Use "dam" as "power") */
		case GF_AWAY_EVIL:
		{
			/* Only affect undead */
			if (r_ptr->flags3 & (RF3_EVIL))
			{
				if (seen) obvious = TRUE;
				if (seen) l_ptr->flags3 |= (RF3_EVIL);
				do_dist = dam;
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Teleport monster (Use "dam" as "power") */
		case GF_AWAY_ALL:
		{
			/* Obvious */
			if (seen) obvious = TRUE;

			/* Prepare to teleport */
			do_dist = dam;

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Turn undead (Use "dam" as "power") */
		case GF_TURN_UNDEAD:
		{
			/* Only affect undead */
			if (r_ptr->flags3 & (RF3_UNDEAD))
			{
				/* Learn about type */
				if (seen) l_ptr->flags3 |= (RF3_UNDEAD);

				/* Obvious */
				if (seen) obvious = TRUE;

				/* Apply some fear */
				do_fear = damroll(3, (dam / 2)) + 1;

				/* Attempt a saving throw */
				if (r_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10)
				{
					/* No obvious effect */
					note = " is unaffected!";
					obvious = FALSE;
					do_fear = 0;
				}
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Turn evil (Use "dam" as "power") */
		case GF_TURN_EVIL:
		{
			/* Only affect evil */
			if (r_ptr->flags3 & (RF3_EVIL))
			{
				/* Learn about type */
				if (seen) l_ptr->flags3 |= (RF3_EVIL);

				/* Obvious */
				if (seen) obvious = TRUE;

				/* Apply some fear */
				do_fear = damroll(3, (dam / 2)) + 1;

				/* Attempt a saving throw */
				if (r_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10)
				{
					/* No obvious effect */
					note = " is unaffected!";
					obvious = FALSE;
					do_fear = 0;
				}
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Turn monster (Use "dam" as "power") */
		case GF_TURN_ALL:
		{
			/* Obvious */
			if (seen) obvious = TRUE;

			/* Apply some fear */
			do_fear = damroll(3, (dam / 2)) + 1;

			/* Attempt a saving throw */
			if ((r_ptr->flags1 & (RF1_UNIQUE)) ||
			    (r_ptr->flags3 & (RF3_NO_FEAR)) ||
			    (r_ptr->level > randint((dam - 10) < 1 ? 1 : (dam - 10)) + 10))
			{
				/* No obvious effect */
				note = " is unaffected!";
				obvious = FALSE;
				do_fear = 0;
			}

			/* No "real" damage */
			dam = 0;
			break;
		}


		/* Dispel undead */
		case GF_DISP_UNDEAD:
		{
			/* Only affect undead */
			if (r_ptr->flags3 & (RF3_UNDEAD))
			{
				/* Learn about type */
				if (seen) l_ptr->flags3 |= (RF3_UNDEAD);

				/* Obvious */
				if (seen) obvious = TRUE;

				/* Message */
				note = " shudders.";
				note_dies = " dissolves!";
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;

				/* No damage */
				dam = 0;
			}

			break;
		}


		/* Dispel evil */
		case GF_DISP_EVIL:
		{
			/* Only affect evil */
			if (r_ptr->flags3 & (RF3_EVIL))
			{
				/* Learn about type */
				if (seen) l_ptr->flags3 |= (RF3_EVIL);

				/* Obvious */
				if (seen) obvious = TRUE;

				/* Message */
				note = " shudders.";
				note_dies = " dissolves!";
			}

			/* Others ignore */
			else
			{
				/* Irrelevant */
				skipped = TRUE;

				/* No damage */
				dam = 0;
			}

			break;
		}


		/* Dispel monster */
		case GF_DISP_ALL:
		{
			/* Obvious */
			if (seen) obvious = TRUE;

			/* Message */
			note = " shudders.";
			note_dies = " dissolves!";

			break;
		}


		/* Default */
		default:
		{
			/* Irrelevant */
			skipped = TRUE;

			/* No damage */
			dam = 0;

			break;
		}
	}


	/* Absolutely no effect */
	if (skipped) return (FALSE);


	/* "Unique" monsters cannot be polymorphed */
	if (r_ptr->flags1 & (RF1_UNIQUE)) do_poly = FALSE;


	/* "Unique" monsters can only be "killed" by the player */
	if (r_ptr->flags1 & (RF1_UNIQUE))
	{
		/* Uniques may only be killed by the player */
		if ((who >= 0) && (dam > m_ptr->hp)) dam = m_ptr->hp;
	}

	/* Handle "time" */
	if ((dam > m_ptr->hp) && do_time)
	{
		/* Total existence failure */
		note = " has ceased to exist!";
		if (seen) msg_format("%^s%s", m_name, note);

		/* Kill the monster */
		delete_monster_idx(cave_m_idx[y][x]);

		/* Redraw the monster grid */
		lite_spot(y, x);

		/* Anything seen? */
		return (obvious);
	}

	/* Check for death */
	else if (dam > m_ptr->hp)
	{
		/* Extract method of death */
		note = note_dies;
	}

	/* Mega-Hack -- Handle "polymorph" -- monsters get a saving throw */
	else if (do_poly && (randint(90) > r_ptr->level))
	{
		/* Default -- assume no polymorph */
		note = " is unaffected!";

		/* Pick a "new" monster race */
		tmp = poly_r_idx(m_ptr->r_idx);

		/* Handle polymorph */
		if (tmp != m_ptr->r_idx)
		{
			/* Obvious */
			if (seen) obvious = TRUE;

			/* Monster polymorphs */
			note = " changes!";

			/* Get the health fraction */
			dam = m_ptr->maxhp / m_ptr->hp;

			/* "Kill" the "old" monster */
			delete_monster_idx(cave_m_idx[y][x]);

			/* Create a new monster (no groups) */
			(void)place_monster_aux(y, x, tmp, FALSE, FALSE);

			/* XXX XXX XXX Hack -- Assume success */

			/* Hack -- Get new monster */
			m_ptr = &m_list[cave_m_idx[y][x]];

			/* Correct hitpoints */
			m_ptr->hp = m_ptr->maxhp / dam;

			dam = 0;

			/* Hack -- Get new race */
			r_ptr = &r_info[m_ptr->r_idx];
		}
	}

	/* Handle "teleport" */
	else if (do_dist)
	{
		/* Obvious */
		if (seen) obvious = TRUE;

		/* Teleport */
		teleport_away(cave_m_idx[y][x], do_dist);

		/* No movement */
		if ((y == m_ptr->fy) && (x == m_ptr->fx))
		{
			/* Message */
			note = " stumbles.";
		}
		/* Visible */
		else if (projectable(p_ptr->py, p_ptr->px, m_ptr->fy, m_ptr->fx))
		{
			/* Message */
			note = " bounces.";
		}
		else
		{
			/* Message */
			note = " disappears!";
		}

		/* Hack -- get new location */
		y = m_ptr->fy;
		x = m_ptr->fx;
	}

	/* Sound and Impact breathers never stun */
	else if (do_stun &&
				!(r_ptr->flags4 & (RF4_BR_SOUN)) &&
	         !(r_ptr->flags4 & (RF4_BR_WALL)))
	{
		/* Obvious */
		if (seen) obvious = TRUE;

		/* Get confused */
		if (m_ptr->stunned)
		{
			note = " is more dazed.";
			tmp = m_ptr->stunned + (do_stun / 2);
		}
		else
		{
			note = " is dazed.";
			tmp = do_stun;
		}

		/* Apply stun */
		m_ptr->stunned = (tmp < 200) ? tmp : 200;
	}

	/* Confusion and Chaos breathers (and sleepers) never confuse */
	else if (do_conf &&
	         !(r_ptr->flags3 & (RF3_NO_CONF)) &&
				!(r_ptr->flags4 & (RF4_BR_CONF)) &&
	         !(r_ptr->flags4 & (RF4_BR_CHAO)))
	{
		/* Obvious */
		if (seen) obvious = TRUE;

		/* Already partially confused */
		if (m_ptr->confused)
		{
			note = " looks more confused.";
			tmp = m_ptr->confused + (do_conf / 2);
		}

		/* Was not confused */
		else
		{
			note = " looks confused.";
			tmp = do_conf;
		}

		/* Apply confusion */
		m_ptr->confused = (tmp < 200) ? tmp : 200;
	}


	/* Fear */
	if (do_fear)
	{
		/* Increase fear */
		tmp = m_ptr->monfear + do_fear;

		/* Set fear */
		m_ptr->monfear = (tmp < 200) ? tmp : 200;
	}


	/* If another monster did the damage, hurt the monster by hand */
	if (who >= 0)
	{
		/* Redraw (later) if needed */
		if (p_ptr->health_who == cave_m_idx[y][x]) p_ptr->redraw |= (PR_HEALTH);

		/* Wake the monster up */
		m_ptr->csleep = 0;

		/* Hurt the monster */
		m_ptr->hp -= dam;

		/* Dead monster */
		if (m_ptr->hp < 0)
		{
			/* Generate treasure, etc */
			monster_death(cave_m_idx[y][x]);

			/* Delete the monster */
			delete_monster_idx(cave_m_idx[y][x]);

			/* Give detailed messages if destroyed */
			if (note && seen) msg_format("%^s%s", m_name, note);
		}

		/* Damaged monster */
		else
		{
			/* Give detailed messages if visible or destroyed */
			if (note && seen) msg_format("%^s%s", m_name, note);

			/* Hack -- Pain message */
			else if (dam > 0 && seen) message_pain(cave_m_idx[y][x], dam);

			/* Hack -- handle sleep */
			if (do_sleep) m_ptr->csleep = do_sleep;
		}
	}

	/* If the player did it, give him experience, check fear */
	else
	{
		bool fear = FALSE;

		/* Hurt the monster, check for fear and death */
		if (mon_take_hit(cave_m_idx[y][x], dam, &fear, note_dies))
		{
			/* Dead monster */
		}

		/* Damaged monster */
		else
		{
			/* Give detailed messages if visible or destroyed */
			if (note && seen) msg_format("%^s%s", m_name, note);

			/* Hack -- Pain message */
			else if (dam > 0) message_pain(cave_m_idx[y][x], dam);

			/* Take note */
			if ((fear || do_fear) && (m_ptr->ml))
			{
				/* Sound */
				sound(SOUND_FLEE);

				/* Message */
				msg_format("%^s flees in terror!", m_name);
			}

			/* Hack -- handle sleep */
			if (do_sleep) m_ptr->csleep = do_sleep;
		}
	}


	/* Verify this code XXX XXX XXX */

	/* Update the monster */
	update_mon(cave_m_idx[y][x], FALSE);

	/* Redraw the monster grid */
	lite_spot(y, x);


	/* Update monster recall window */
	if (p_ptr->monster_race_idx == m_ptr->r_idx)
	{
		/* Window stuff */
		p_ptr->window |= (PW_MONSTER);
	}


	/* Track it */
	project_m_n++;
	project_m_x = x;
	project_m_y = y;


	/* Return "Anything seen?" */
	return (obvious);
}






/*
 * Helper function for "project()" below.
 *
 * Handle a beam/bolt/ball causing damage to the player.
 *
 * This routine takes a "source monster" (by index), a "distance", a default
 * "damage", and a "damage type".  See "project_m()" above.
 *
 * If "rad" is non-zero, then the blast was centered elsewhere, and the damage
 * is reduced (see "project_m()" above).  This can happen if a monster breathes
 * at the player and hits a wall instead.
 *
 * We return "TRUE" if any "obvious" effects were observed.  XXX XXX Actually,
 * we just assume that the effects were obvious, for historical reasons.
 */
static bool project_p(int who, int r, int y, int x, int dam, int typ)
{
	int k = 0;

	/* Hack -- assume obvious */
	bool obvious = TRUE;

	/* Player blind-ness */
	bool blind = (p_ptr->blind ? TRUE : FALSE);

	/* Player needs a "description" (he is blind) */
	bool fuzzy = FALSE;

	/* Explain the player's death if no source monster */
	bool no_killer = FALSE;

	/* Source monster */
	monster_type *m_ptr;

	/* Monster name (for attacks) */
	char m_name[80];

	/* Monster name (for damage) */
	char killer[80];

	/* Hack -- messages */
	cptr act = NULL;


	/* No player here */
	if (!(cave_m_idx[y][x] < 0)) return (FALSE);

	/* Never affect projector */
	if (cave_m_idx[y][x] == who) return (FALSE);


	/* XXX XXX XXX */
	/* Limit maximum damage */
	if (dam > 1600) dam = 1600;

	/* Reduce damage by distance */
	dam = (dam + r) / (r + 1);


	/* If the player is blind, be more descriptive */
	if (blind) fuzzy = TRUE;

   if (!who) no_killer = TRUE;

	/* Monster may be caster */
	if (who > 0)
	{
		/* Get the source monster */
		m_ptr = &m_list[who];

		/* Get the monster name */
		monster_desc(m_name, m_ptr, 0);

		/* Get the monster's real name */
		monster_desc(killer, m_ptr, 0x88);
	}

	/* Analyze the damage */
	switch (typ)
	{
		/* Standard damage -- hurts inventory too */
		case GF_ACID:
		{
			if (fuzzy) msg_print("You are hit by acid!");

			if (no_killer) strcpy(killer, "a spray of acid");

			acid_dam(dam, killer);
			break;
		}

		/* Standard damage -- hurts inventory too */
		case GF_FIRE:
		{
			if (fuzzy) msg_print("You are hit by fire!");

			if (no_killer) strcpy(killer, "a fireball");

			fire_dam(dam, killer);
			break;
		}

		/* Standard damage -- hurts inventory too */
		case GF_COLD:
		{
			if (fuzzy) msg_print("You are hit by cold!");

			if (no_killer) strcpy(killer, "a freezing blast");

			cold_dam(dam, killer);
			break;
		}

		/* Standard damage -- hurts inventory too */
		case GF_ELEC:
		{
			if (fuzzy) msg_print("You are hit by lightning!");

			if (no_killer) strcpy(killer, "ball lightning");

			elec_dam(dam, killer);
			break;
		}

		/* Standard damage -- also poisons player */
		case GF_POIS:
		{
			if (fuzzy) msg_print("You are hit by poison!");

			if (no_killer) strcpy(killer, "a cloud of poison");

			if (p_ptr->resist_pois) dam = (dam + 2) / 3;
			if (p_ptr->oppose_pois) dam = (dam + 2) / 3;
			take_hit(dam, killer);
			if (!(p_ptr->resist_pois || p_ptr->oppose_pois))
			{
				(void)set_poisoned(p_ptr->poisoned + rand_int(dam) + 10);
			}
			break;
		}

		/* Standard damage */
		case GF_MISSILE:
		{
			if (fuzzy) msg_print("You are hit by something!");

			if (no_killer) strcpy(killer, "a magic missile");

			take_hit(dam, killer);
			break;
		}

		/* Holy Orb -- Player only takes partial damage */
		case GF_HOLY_ORB:
		{
			if (fuzzy) msg_print("You are hit by something!");

			if (no_killer) strcpy(killer, "an act of God");

			dam /= 2;
			take_hit(dam, killer);
			break;
		}

		/* Arrow -- XXX no dodging */
		case GF_ARROW:
		{
			if (fuzzy) msg_print("You are hit by something sharp!");

			if (no_killer) strcpy(killer, "an arrow");

			take_hit(dam, killer);
			break;
		}

		/* Plasma -- XXX No resist */
		case GF_PLASMA:
		{
			if (fuzzy) msg_print("You are hit by a fiery blast!");

			if (no_killer) strcpy(killer, "a fiery blast");

			take_hit(dam, killer);
			if (!p_ptr->resist_sound)
			{
				int k = (randint((dam > 40) ? 35 : (dam * 3 / 4 + 5)));
				(void)set_stun(p_ptr->stun + k);
			}
			break;
		}

		/* Nether -- drain experience */
		case GF_NETHER:
		{
			if (fuzzy) msg_print("You are hit by something cold!");

			if (no_killer) strcpy(killer, "an unholy blast");

			if (p_ptr->resist_nethr)
			{
				dam *= 6; dam /= (randint(6) + 6);
			}
			else
			{
				if (p_ptr->hold_life && (rand_int(100) < 75))
				{
					msg_print("You keep hold of your life force!");
				}
				else if (p_ptr->hold_life)
				{
					msg_print("You feel your life slipping away!");
					lose_exp(200 + (p_ptr->exp/1000) * MON_DRAIN_LIFE);
				}
				else
				{
					msg_print("You feel your life draining away!");
					lose_exp(200 + (p_ptr->exp/100) * MON_DRAIN_LIFE);
				}
			}
			take_hit(dam, killer);
			break;
		}

		/* Water -- stun/confuse */
		case GF_WATER:
		{
			if (fuzzy) msg_print("You are hit by something wet!");

			if (no_killer) strcpy(killer, "a whirlpool");

			if (!p_ptr->resist_sound)
			{
				(void)set_stun(p_ptr->stun + randint(40));
			}
			if (!p_ptr->resist_confu)
			{
				(void)set_confused(p_ptr->confused + randint(5) + 5);
			}
			take_hit(dam, killer);
			break;
		}

		/* Chaos -- many effects */
		case GF_CHAOS:
		{
			if (fuzzy) msg_print("You are hit by something strange!");

			if (no_killer) strcpy(killer, "chaotic forces");

			if (p_ptr->resist_chaos)
			{
				dam *= 6; dam /= (randint(6) + 6);
			}
			if (!p_ptr->resist_confu)
			{
				(void)set_confused(p_ptr->confused + rand_int(20) + 10);
			}
			if (!p_ptr->resist_chaos)
			{
				(void)set_image(p_ptr->image + randint(10));
			}
			if (!p_ptr->resist_nethr && !p_ptr->resist_chaos)
			{
				if (p_ptr->hold_life && (rand_int(100) < 75))
				{
					msg_print("You keep hold of your life force!");
				}
				else if (p_ptr->hold_life)
				{
					msg_print("You feel your life slipping away!");
					lose_exp(500 + (p_ptr->exp/1000) * MON_DRAIN_LIFE);
				}
				else
				{
					msg_print("You feel your life draining away!");
					lose_exp(5000 + (p_ptr->exp/100) * MON_DRAIN_LIFE);
				}
			}
			take_hit(dam, killer);
			break;
		}

		/* Shards -- mostly cutting */
		case GF_SHARD:
		{
			if (fuzzy) msg_print("You are hit by something sharp!");

			if (no_killer) strcpy(killer, "flying debris");

			if (p_ptr->resist_shard)
			{
				dam *= 6; dam /= (randint(6) + 6);
			}
			else
			{
				(void)set_cut(p_ptr->cut + dam);
			}
			take_hit(dam, killer);
			break;
		}

		/* Sound -- mostly stunning */
		case GF_SOUND:
		{
			if (fuzzy) msg_print("You are deafened by a loud noise!");

			if (no_killer) strcpy(killer, "a shock wave");

			if (p_ptr->resist_sound)
			{
				dam *= 5; dam /= (randint(6) + 6);
			}
			else
			{
				int k = (randint((dam > 90) ? 35 : (dam / 3 + 5)));
				(void)set_stun(p_ptr->stun + k);
			}
			take_hit(dam, killer);
			break;
		}

		/* Pure confusion */
		case GF_CONFUSION:
		{
			if (fuzzy) msg_print("You are hit by something very strange!");

			if (no_killer) strcpy(killer, "neurologic shock");

			if (p_ptr->resist_confu)
			{
				dam *= 5; dam /= (randint(6) + 6);
			}
			if (!p_ptr->resist_confu)
			{
				(void)set_confused(p_ptr->confused + randint(20) + 10);
			}
			take_hit(dam, killer);
			break;
		}

		/* Disenchantment -- see above */
		case GF_DISENCHANT:
		{
			if (fuzzy) msg_print("You are hit by something strange!");

			if (no_killer) strcpy(killer, "negative energies");

			if (p_ptr->resist_disen)
			{
				dam *= 6; dam /= (randint(6) + 6);
			}
			else
			{
				(void)apply_disenchant(0);
			}
			take_hit(dam, killer);
			break;
		}

		/* Nexus -- see above */
		case GF_NEXUS:
		{
			if (fuzzy) msg_print("You are hit by something strange!");

			if (no_killer) strcpy(killer, "a fatal metamorphosis");

			if (p_ptr->resist_nexus)
			{
				dam *= 6; dam /= (randint(6) + 6);
			}
			else
			{
				apply_nexus(m_ptr);
			}
			take_hit(dam, killer);
			break;
		}

		/* Force -- mostly stun */
		case GF_FORCE:
		{
			if (fuzzy) msg_print("You are hit hard by something!");

			if (no_killer) strcpy(killer, "a shock wave");

			teleport_player(1);

			if (!p_ptr->resist_sound)
			{
				(void)set_stun(p_ptr->stun + randint(20));
			}
			take_hit(dam, killer);
			break;
		}

		/* Inertia -- slowness */
		case GF_INERTIA:
		{
			if (fuzzy) msg_print("You are hit by something strange!");

			if (no_killer) strcpy(killer, "an abrupt change of speed");

			(void)set_slow(p_ptr->slow + rand_int(4) + 4);
			take_hit(dam, killer);
			break;
		}

		/* Lite -- blinding */
		case GF_LITE:
		{
			if (fuzzy) msg_print("You are hit by something!");

			if (no_killer) strcpy(killer, "a blinding flash");

			if (p_ptr->resist_lite)
			{
				dam *= 4; dam /= (randint(6) + 6);
			}
			else if (!blind && !p_ptr->resist_blind)
			{
				(void)set_blind(p_ptr->blind + randint(5) + 2);
			}
			take_hit(dam, killer);
			break;
		}

		/* Dark -- blinding */
		case GF_DARK:
		{
			if (fuzzy) msg_print("You are hit by something!");

			if (no_killer) strcpy(killer, "a grue");

			if (p_ptr->resist_dark)
			{
				dam *= 4; dam /= (randint(6) + 6);
			}
			else if (!blind && !p_ptr->resist_blind)
			{
				(void)set_blind(p_ptr->blind + randint(5) + 2);
			}
			take_hit(dam, killer);
			break;
		}

		/* Time -- bolt fewer effects XXX */
		case GF_TIME:
		{
			if (fuzzy) msg_print("You feel very strange!");

			if (no_killer) strcpy(killer, "ceasing to exist");

			switch (randint(10))
			{
				case 1: case 2: case 3: case 4: case 5:
				{
					msg_print("You feel life has clocked back.");
					lose_exp(100 + (p_ptr->exp / 100) * MON_DRAIN_LIFE);
					break;
				}

				case 6: case 7: case 8: case 9:
				{
					switch (randint(6))
					{
						case 1: k = A_STR; act = "strong"; break;
						case 2: k = A_INT; act = "bright"; break;
						case 3: k = A_WIS; act = "wise"; break;
						case 4: k = A_DEX; act = "agile"; break;
						case 5: k = A_CON; act = "hale"; break;
						case 6: k = A_CHR; act = "beautiful"; break;
					}

					msg_format("You're not as %s as you used to be...", act);

					p_ptr->stat_cur[k] = (p_ptr->stat_cur[k] * 3) / 4;
					if (p_ptr->stat_cur[k] < 3) p_ptr->stat_cur[k] = 3;
					p_ptr->update |= (PU_BONUS);
					break;
				}

				case 10:
				{
					msg_print("You're not as powerful as you used to be...");
					for (k = 0; k < 6; k++)
					{
						p_ptr->stat_cur[k] = (p_ptr->stat_cur[k] * 3) / 4;
						if (p_ptr->stat_cur[k] < 3) p_ptr->stat_cur[k] = 3;
					}
					p_ptr->update |= (PU_BONUS);
					break;
				}
			}

			/* Some rock has been restored */
			if (!cave_floor_bold(p_ptr->py, p_ptr->px))
			{
				msg_print("The cave is no longer empty here!");

				/* Immense damage */
				dam += 300;

				if (dam > p_ptr->chp)
				{
					strcpy(killer, "ceasing to exist");

					/* Make it look like the player ceased to exist */
					dam = 5000 - p_ptr->chp;
				}
			}

			take_hit(dam, killer);
			break;
		}

		/* Gravity -- stun plus slowness plus teleport */
		case GF_GRAVITY:
		{
			if (fuzzy) msg_print("You slam against the ceiling!");

			if (no_killer) strcpy(killer, "an abrupt change in gravity");

			teleport_player(5);
			(void)set_slow(p_ptr->slow + rand_int(4) + 4);
			if (!p_ptr->resist_sound)
			{
				int k = (randint((dam > 90) ? 35 : (dam / 3 + 5)));
				(void)set_stun(p_ptr->stun + k);
			}
			take_hit(dam, killer);
			break;
		}

		/* Pure damage */
		case GF_MANA:
		{
			if (fuzzy) msg_print("You are hit by something energetic!");

			if (no_killer) strcpy(killer, "a burst of raw mana");

			take_hit(dam, killer);
			break;
		}

		/* Pure damage */
		case GF_METEOR:
		{
			/* Need a message */
			if (fuzzy) msg_print("You are hit by something hot!");
			else msg_print("You are hit by a meteor!");

			if (no_killer) strcpy(killer, "a meteor");

			take_hit(dam, killer);
			break;
		}

		/* Ice -- cold plus stun plus cuts */
		case GF_ICE:
		{
			if (fuzzy) msg_print("You are hit by something sharp!");

			if (no_killer) strcpy(killer, "a spray of ice");

			cold_dam(dam, killer);
			if (!p_ptr->resist_shard)
			{
				(void)set_cut(p_ptr->cut + damroll(5, 8));
			}
			if (!p_ptr->resist_sound)
			{
				(void)set_stun(p_ptr->stun + randint(15));
			}
			break;
		}


		/* Default */
		default:
		{
			/* No damage */
			dam = 0;

			break;
		}
	}


	/* Disturb */
	disturb(1, 0);


	/* Return "Anything seen?" */
	return (obvious);
}


/*
 * Find the distance from (x, y) to a line.
 */
static int dist_to_line(int y, int x, int y1, int x1, int y2, int x2)
{
	int d = distance(y, x, y1, x1);
	int cy = y1;
	int cx = x1;

	/* Simple "find the closest point on the line" routine */
	while (d >= distance(y, x, cy, cx))
	{
		d = distance(y, x, cy, cx);

		mmove2(&cy, &cx, y1, x1, y2, x2);

		/* Stop at end */
		if ((cy == y2) && (cx == x2))
		{
			d = MIN(d, distance(y, x, y2, x2));
			break;
		}
	}

	return d;
}



/*
 * Find the char to use to draw a moving bolt
 * It is moving (or has moved) from (x,y) to (nx,ny).
 * If the distance is not "one", we (may) return "*".
 */
static char bolt_char(int y, int x, int ny, int nx)
{
	if ((ny == y) && (nx == x)) return '*';
	if (ny == y) return '-';
	if (nx == x) return '|';
	if ((ny-y) == (x-nx)) return '/';
	if ((ny-y) == (nx-x)) return '\\';
	return '*';
}



/*
 * Generic "beam"/"bolt"/"ball" projection routine.
 *
 * Input:
 *   who: Index of "source" monster (negative for "player")
 *   rad: Radius of explosion (0 = beam/bolt, 1 to 9 = ball)
 *			 Negative values create a conical blast.
 *   y,x: Target location (or location to travel "towards")
 *   dam: Base damage roll to apply to affected monsters (or player)
 *   typ: Type of damage to apply to monsters (and objects)
 *   flg: Extra bit flags (see PROJECT_xxxx in "defines.h")
 *
 * Return:
 *   TRUE if any "effects" of the projection were observed, else FALSE
 *
 * Allows a monster (or player) to project a beam/bolt/ball of a given kind
 * towards a given location (optionally passing over the heads of interposing
 * monsters), and have it do a given amount of damage to the monsters (and
 * optionally objects) within the given radius of the final location.
 *
 * A "bolt" travels from source to target and affects only the target grid.
 * A "beam" travels from source to target, affecting all grids passed through.
 * A "ball" travels from source to the target, exploding at the target, and
 *   affecting everything within the given radius of the target location.
 *
 * Traditionally, a "bolt" does not affect anything on the ground, and does
 * not pass over the heads of interposing monsters, much like a traditional
 * missile, and will "stop" abruptly at the "target" even if no monster is
 * positioned there, while a "ball", on the other hand, passes over the heads
 * of monsters between the source and target, and affects everything except
 * the source monster which lies within the final radius, while a "beam"
 * affects every monster between the source and target, except for the casting
 * monster (or player), and rarely affects things on the ground.
 *
 * Two special flags allow us to use this function in special ways, the
 * "PROJECT_HIDE" flag allows us to perform "invisible" projections, while
 * the "PROJECT_JUMP" flag allows us to affect a specific grid, without
 * actually projecting from the source monster (or player).
 *
 * The player will only get "experience" for monsters killed by himself
 * Unique monsters can only be destroyed by attacks from the player
 *
 * Only 256 grids can be affected per projection, limiting the effective
 * "radius" of standard ball attacks to nine units (diameter nineteen).
 *
 * One can project in a given "direction" by combining PROJECT_THRU with small
 * offsets to the initial location (see "line_spell()"), or by calculating
 * "virtual targets" far away from the player.
 *
 * One can also use PROJECT_THRU to send a beam/bolt along an angled path,
 * continuing until it actually hits somethings (useful for "stone to mud").
 *
 * Bolts and Beams explode INSIDE walls, so that they can destroy doors.
 *
 * Balls must explode BEFORE hitting walls, or they would affect monsters
 * on both sides of a wall.  Some bug reports indicate that this is still
 * happening in 2.7.8 for Windows, though it appears to be impossible.
 *
 * We "pre-calculate" the blast area only in part for efficiency.
 * More importantly, this lets us do "explosions" from the "inside" out.
 * This results in a more logical distribution of "blast" treasure.
 * It also produces a better (in my opinion) animation of the explosion.
 * It could be (but is not) used to have the treasure dropped by monsters
 * in the middle of the explosion fall "outwards", and then be damaged by
 * the blast as it spreads outwards towards the treasure drop location.
 *
 * Walls and doors are included in the blast area, so that they can be
 * "burned" or "melted" in later versions.
 *
 * This algorithm is intended to maximize simplicity, not necessarily
 * efficiency, since this function is not a bottleneck in the code.
 *
 * We apply the blast effect from ground zero outwards, in several passes,
 * first affecting features, then objects, then monsters, then the player.
 * This allows walls to be removed before checking the object or monster
 * in the wall, and protects objects which are dropped by monsters killed
 * in the blast, and allows the player to see all affects before he is
 * killed or teleported away.  The semantics of this method are open to
 * various interpretations, but they seem to work well in practice.
 *
 * We process the blast area from ground-zero outwards to allow for better
 * distribution of treasure dropped by monsters, and because it provides a
 * pleasing visual effect at low cost.
 *
 * Note that the damage done by "ball" explosions decreases with distance.
 * This decrease is rapid, grids at radius "dist" take "1/dist" damage.
 *
 * Notice the "napalm" effect of "beam" weapons.  First they "project" to
 * the target, and then the damage "flows" along this beam of destruction.
 * The damage at every grid is the same as at the "center" of a "ball"
 * explosion, since the "beam" grids are treated as if they ARE at the
 * center of a "ball" explosion.
 *
 * Currently, specifying "beam" plus "ball" means that locations which are
 * covered by the initial "beam", and also covered by the final "ball", except
 * for the final grid (the epicenter of the ball), will be "hit twice", once
 * by the initial beam, and once by the exploding ball.  For the grid right
 * next to the epicenter, this results in 150% damage being done.  The center
 * does not have this problem, for the same reason the final grid in a "beam"
 * plus "bolt" does not -- it is explicitly removed.  Simply removing "beam"
 * grids which are covered by the "ball" will NOT work, as then they will
 * receive LESS damage than they should.  Do not combine "beam" with "ball".
 *
 * The array "gy[],gx[]" with current size "grids" is used to hold the
 * collected locations of all grids in the "blast area" plus "beam path".
 *
 * Note the rather complex usage of the "gm[]" array.  First, gm[0] is always
 * zero.  Second, for N>1, gm[N] is always the index (in gy[],gx[]) of the
 * first blast grid (see above) with radius "N" from the blast center.  Note
 * that only the first gm[1] grids in the blast area thus take full damage.
 * Also, note that gm[rad+1] is always equal to "grids", which is the total
 * number of blast grids.
 *
 * Note that once the projection is complete, (y2,x2) holds the final location
 * of bolts/beams, and the "epicenter" of balls.
 *
 * Note also that "rad" specifies the "inclusive" radius of projection blast,
 * so that a "rad" of "one" actually covers 5 or 9 grids, depending on the
 * implementation of the "distance" function.  Also, a bolt can be properly
 * viewed as a "ball" with a "rad" of "zero".
 *
 * Some people have requested an "auto-explode ball attacks at max range"
 * option, which is now handled by this function.  XXX XXX XXX
 *
 * Note that for consistency, we "pretend" that the bolt actually takes "time"
 * to move from point A to point B, even if the player cannot see part of the
 * projection path.  Note that in general, the player will *always* see part
 * of the path, since it either starts at the player or ends on the player.
 *
 * Hack -- we assume that every "projection" is "self-illuminating".
 *
 * Mega-Hack -- when only a single monster is affected, we automatically track
 * (and recall) that monster, unless "PROJECT_JUMP" is used.  XXX XXX XXX
 */
bool project(int who, int rad, int y, int x, int dam, int typ, int flg)
{
	int py = p_ptr->py;
	int px = p_ptr->px;

	int i, t, dist;
	int y9, x9;
	int y1, x1;
	int y2, x2;

	int msec = op_ptr->delay_factor * op_ptr->delay_factor;

	/* Assume the player sees nothing */
	bool notice = FALSE;

	/* Assume the player has seen nothing */
	bool visual = FALSE;

	/* Assume the player has seen no blast grids */
	bool drawn = FALSE;

	/* Assume to be a normal ball spell */
	bool breath = FALSE;

	/* Is the player blind? */
	bool blind = (p_ptr->blind ? TRUE : FALSE);

	/* Number of grids in the "blast area" (including the "beam" path) */
	int grids = 0;

	/* Coordinates of the affected grids */
	byte gx[256], gy[256];

	/* Encoded "radius" info (see above) */
	byte gm[32];

	/* Actual radius encoded in gm[] */
	int gm_rad = rad;


	/* Hack -- Jump to target */
	if (flg & (PROJECT_JUMP))
	{
		x1 = x;
		y1 = y;
	}

	/* Start at player */
	else if (who < 0)
	{
		x1 = px;
		y1 = py;
	}

	/* Start at monster */
	else if (who > 0)
	{
		x1 = m_list[who].fx;
		y1 = m_list[who].fy;
	}

	/* Oops */
	else
	{
		x1 = x;
		y1 = y;
	}


	/* Default "destination" */
	y2 = y;
	x2 = x;


	/* Hack -- verify stuff */
	if (flg & (PROJECT_THRU))
	{
		if ((x1 == x2) && (y1 == y2))
		{
			flg &= ~(PROJECT_THRU);
		}
	}

	/* Handle a breath attack */
	if (rad < 0)
	{
		rad = 0 - rad;
		breath = TRUE;
		flg |= PROJECT_HIDE;
	}


	/* Hack -- Assume there will be no blast (max radius 32) */
	for (dist = 0; dist < 32; dist++) gm[dist] = 0;


	/* Hack -- Handle stuff */
	handle_stuff();


	/* Start at the source */
	x = x9 = x1;
	y = y9 = y1;
	dist = 0;

	/* Project until done */
	while (1)
	{
		/* Gather beam grids */
		if (flg & (PROJECT_BEAM))
		{
			gy[grids] = y;
			gx[grids] = x;
			grids++;
		}

		/* XXX XXX Hack -- Display "beam" grids */
		if (!blind && !(flg & (PROJECT_HIDE)) &&
			 dist && (flg & (PROJECT_BEAM)) &&
			 panel_contains(y, x) && player_has_los_bold(y, x))
		{
			/* Hack -- Visual effect -- "explode" the grids */
			print_rel('*', spell_color(typ), y, x);
		}

		/* Never pass through walls */
		if (dist && !cave_floor_bold(y, x)) break;

		/* Check for arrival at "final target" (if desired) */
		if (!(flg & (PROJECT_THRU)) && (x == x2) && (y == y2)) break;

		/* If allowed, and we have moved at all, stop when we hit anybody */
		if ((flg & (PROJECT_STOP)) && dist && (cave_m_idx[y][x] != 0)) break;


		/* Calculate the new location */
		y9 = y;
		x9 = x;
		mmove2(&y9, &x9, y1, x1, y2, x2);

		/* Hack -- Balls explode BEFORE reaching walls or doors */
		if (!cave_floor_bold(y9, x9) && (rad > 0)) break;

		/* Keep track of the distance traveled */
		dist++;

		/* Nothing can travel furthur than the maximal distance */
		if (dist > MAX_RANGE)
		{
			/* Explode at limit */
			dist = MAX_RANGE;
			break;
		}

		/* Only do visual effects (and delay) if requested */
		if (!blind && !(flg & (PROJECT_HIDE)))
		{
			/* Only do visuals if the player can "see" the bolt */
			if (panel_contains(y9, x9) && player_has_los_bold(y9, x9))
			{
				/* Use misc_to_attr/misc_to_char XXX XXX XXX XXX */
				/* Visual effects -- Display, Highlight, Flush, Pause, Erase */
				print_rel(bolt_char(y, x, y9, x9), spell_color(typ), y9, x9);
				move_cursor_relative(y9, x9);
				Term_fresh();
				visual = TRUE;
				Term_xtra(TERM_XTRA_DELAY, msec);
				lite_spot(y9, x9);
				Term_fresh();
			}

			/* Hack -- make sure to delay anyway for consistency */
			else if (visual)
			{
				/* Delay for consistency */
				Term_xtra(TERM_XTRA_DELAY, msec);
			}
		}

		/* Save the new location */
		y = y9;
		x = x9;
	}

	/* Save the "blast epicenter" */
	y2 = y;
	x2 = x;

	/* Start the "explosion" */
	gm[0] = 0;

	/* Hack -- make sure beams get to "explode" */
	gm[1] = grids;

	/* If we found a "target", explode there */
	if (dist <= MAX_RANGE)
	{
		/* Mega-Hack -- remove the final "beam" grid */
		if ((flg & (PROJECT_BEAM)) && (grids > 0)) grids--;

		/*
		 * Create a conical breath attack
		 *
		 *         ***
		 *     ********
		 * D********@**
		 *		 ********
		 *         ***
		 */
		if (breath)
		{
			int by, bx;
			int brad = 0;
			int bdis = 0;
			int cdis;

			/* Not done yet */
			bool done = FALSE;

			flg &= ~(PROJECT_HIDE);

			by = y1;
			bx = x1;

			while (bdis <= dist + rad)
			{
				/* Travel from center outward */
				for (cdis = 0; cdis <= brad; cdis++)
				{
					/* Scan the maximal blast area of radius "cdis" */
					for (y = by - cdis; y <= by + cdis; y++)
					{
						for (x = bx - cdis; x <= bx + cdis; x++)
						{
							/* Ignore "illegal" locations */
							if (!in_bounds(y, x)) continue;

							/* Enforce a circular "ripple" */
							if (distance(y1, x1, y, x) != bdis) continue;

							/* Enforce an arc */
							if (distance(by, bx, y, x) != cdis) continue;

							/* The blast is stopped by walls */
							if (!los(by, bx, y, x)) continue;

							/* Save this grid */
							gy[grids] = y;
							gx[grids] = x;
							grids++;
						}
					}
				}

				/* Encode some more "radius" info */
				gm[bdis + 1] = grids;

				/* Stop moving */
				if ((by == y2) && (bx == x2)) done = TRUE;

				/* Finish */
				if (done)
				{
					bdis++;
					continue;
				}

				/* Ripple outwards */
				mmove2(&by, &bx, y1, x1, y2, x2);

				/* Find the next ripple */
				bdis++;

				/* Increase the size */
				brad = (rad * bdis) / dist;
			}

			/* Store the effect size */
			gm_rad = bdis;
		}

		else
		{
			/* Determine the blast area, work from the inside out */
			for (dist = 0; dist <= rad; dist++)
			{
				/* Scan the maximal blast area of radius "dist" */
				for (y = y2 - dist; y <= y2 + dist; y++)
				{
					for (x = x2 - dist; x <= x2 + dist; x++)
					{
						/* Ignore "illegal" locations */
						if (!in_bounds(y, x)) continue;

						/* Enforce a "circular" explosion */
						if (distance(y2, x2, y, x) != dist) continue;

						/* Ball explosions are stopped by walls */
						if (!los(y2, x2, y, x)) continue;

						/* Save this grid */
						gy[grids] = y;
						gx[grids] = x;
						grids++;
					}
				}

				/* Encode some more "radius" info */
				gm[dist+1] = grids;
			}
		}
	}


	/* Speed -- ignore "non-explosions" */
	if (!grids) return (FALSE);


	/* Display the "blast area" */
	if (!blind && !(flg & (PROJECT_HIDE)))
	{
		/* Then do the "blast", from inside out */
		for (t = 0; t <= gm_rad; t++)
		{
			/* Dump everything with this radius */
			for (i = gm[t]; i < gm[t+1]; i++)
			{
				/* Extract the location */
				y = gy[i];
				x = gx[i];

				/* The player can see it */
				if (panel_contains(y, x) && player_has_los_bold(y, x))
				{
					drawn = TRUE;

					/* Use misc_to_attr/misc_to_char XXX XXX XXX XXX */
					/* Visual effects -- Display */
					print_rel('*', spell_color(typ), y, x);
				}
			}

			/* Hack -- center the cursor */
			move_cursor_relative(y2, x2);

			/* Flush each "radius" seperately */
			Term_fresh();

			/* Delay (efficiently) */
			if (visual || drawn)
			{
				Term_xtra(TERM_XTRA_DELAY, msec);
			}
		}

		/* Flush the erasing */
		if (drawn)
		{
			/* Erase the explosion drawn above */
			for (i = 0; i < grids; i++)
			{
				/* Extract the location */
				y = gy[i];
				x = gx[i];

				/* Hack -- Erase if needed */
				if (panel_contains(y, x) && player_has_los_bold(y, x))
				{
					lite_spot(y, x);
				}
			}

			/* Hack -- center the cursor */
			move_cursor_relative(y2, x2);

			/* Flush the explosion */
			Term_fresh();
		}
	}


	/* Check features */
	if (flg & (PROJECT_GRID))
	{
		/* Start with "dist" of zero */
		dist = 0;

		/* Scan for features */
		for (i = 0; i < grids; i++)
		{
			/* Hack -- Notice new "dist" values */
			if (gm[dist+1] == i) dist++;

			/* Get the grid location */
			y = gy[i];
			x = gx[i];

			/* Find the closest point in the blast */
			if (breath)
			{
				int d = dist_to_line(y, x, y1, x1, y2, x2);

				/* Affect the grid */
				if (project_f(who, d, y, x, dam, typ)) notice = TRUE;
			}
			else
			{
				/* Affect the grid */
				if (project_f(who, dist, y, x, dam, typ)) notice = TRUE;
			}
		}
	}


	/* Check objects */
	if (flg & (PROJECT_ITEM))
	{
		/* Start with "dist" of zero */
		dist = 0;

		/* Scan for objects */
		for (i = 0; i < grids; i++)
		{
			/* Hack -- Notice new "dist" values */
			if (gm[dist+1] == i) dist++;

			/* Get the grid location */
			y = gy[i];
			x = gx[i];


			/* Find the closest point in the blast */
			if (breath)
			{
				int d = dist_to_line(y, x, y1, x1, y2, x2);

				/* Affect the object in the grid */
				if (project_o(who, d, y, x, dam, typ)) notice = TRUE;
			}
			else
			{
				/* Affect the object in the grid */
				if (project_o(who, dist, y, x, dam, typ)) notice = TRUE;
			}
		}
	}


	/* Check monsters */
	if (flg & (PROJECT_KILL))
	{
		/* Mega-Hack */
		project_m_n = 0;
		project_m_x = 0;
		project_m_y = 0;

		/* Start with "dist" of zero */
		dist = 0;

		/* Scan for monsters */
		for (i = 0; i < grids; i++)
		{
			/* Hack -- Notice new "dist" values */
			if (gm[dist+1] == i) dist++;

			/* Get the grid location */
			y = gy[i];
			x = gx[i];

			/* Find the closest point in the blast */
			if (breath)
			{
				int d = dist_to_line(y, x, y1, x1, y2, x2);

				/* Affect the monster in the grid */
				if (project_m(who, d, y, x, dam, typ)) notice = TRUE;
			}
			else
			{
				/* Affect the monster in the grid */
				if (project_m(who, dist, y, x, dam, typ)) notice = TRUE;
			}
		}

		/* Player affected one monster (without "jumping") */
		if ((who < 0) && (project_m_n == 1) && !(flg & (PROJECT_JUMP)))
		{
			/* Location */
			x = project_m_x;
			y = project_m_y;

			/* Track if possible */
			if (cave_m_idx[y][x] > 0)
			{
				monster_type *m_ptr = &m_list[cave_m_idx[y][x]];

				/* Hack -- auto-recall */
				if (m_ptr->ml) monster_race_track(m_ptr->r_idx);

				/* Hack - auto-track */
				if (m_ptr->ml) health_track(cave_m_idx[y][x]);
			}
		}
	}


	/* Check player */
	if (flg & (PROJECT_KILL))
	{
		/* Start with "dist" of zero */
		dist = 0;

		/* Scan for player */
		for (i = 0; i < grids; i++)
		{
			/* Hack -- Notice new "dist" values */
			if (gm[dist+1] == i) dist++;

			/* Get the grid location */
			y = gy[i];
			x = gx[i];

			if ((x != px) || (y != py)) continue;

			/* Find the closest point in the blast */
			if (breath)
			{
				int d = dist_to_line(y, x, y1, x1, y2, x2);

				/* Affect the player */
				if (project_p(who, d, y, x, dam, typ)) notice = TRUE;
			}
			else
			{
				/* Affect the player */
				if (project_p(who, dist, y, x, dam, typ)) notice = TRUE;
			}
		}
	}


	/* Return "something was noticed" */
	return (notice);
}




