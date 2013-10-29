/* File: cmd6.c */

/*
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"

#include "script.h"


/*
 * This file includes code for eating food, drinking potions,
 * reading scrolls, aiming wands, using staffs, zapping rods,
 * and activating artifacts.
 *
 * In all cases, if the player becomes "aware" of the item's use
 * by testing it, mark it as "aware" and reward some experience
 * based on the object's level, always rounding up.  If the player
 * remains "unaware", mark that object "kind" as "tried".
 *
 * This code now correctly handles the unstacking of wands, staffs,
 * and rods.  Note the overly paranoid warning about potential pack
 * overflow, which allows the player to use and drop a stacked item.
 *
 * In all "unstacking" scenarios, the "used" object is "carried" as if
 * the player had just picked it up.  In particular, this means that if
 * the use of an item induces pack overflow, that item will be dropped.
 *
 * For simplicity, these routines induce a full "pack reorganization"
 * which not only combines similar items, but also reorganizes various
 * items to obey the current "sorting" method.  This may require about
 * 400 item comparisons, but only occasionally.
 *
 * There may be a BIG problem with any effect that can cause changes
 * to the inventory.  For example, a "scroll of recharging" can cause
 * a wand/staff to disappear, moving the inventory up.  Luckily, the
 * scrolls all appear BEFORE the staffs/wands, so this is not a problem.
 * But, for example, a "staff of recharging" could cause MAJOR problems.
 * In such a case, it will be best to either (1) "postpone" the effect
 * until the end of the function, or (2) "change" the effect, say, into
 * giving a staff "negative" charges, or "turning a staff into a stick".
 * It seems as though a "rod of recharging" might in fact cause problems.
 * The basic problem is that the act of recharging (and destroying) an
 * item causes the inducer of that action to "move", causing "o_ptr" to
 * no longer point at the correct item, with horrifying results.
 *
 * Note that food/potions/scrolls no longer use bit-flags for effects,
 * but instead use the "sval" (which is also used to sort the objects).
 */



/*
 * This command is used to locate the item to reduce
 * after applying all effects in the inventory. (taken from UnAngband)
 */
int find_item_to_reduce(int item)
{
	int i;

	/* The default case */
	if (inventory[item].ident & (IDENT_BREAKS)) return (item);

	/* Scan inventory to locate */
	for (i = 0; i < INVEN_TOTAL + 1; i++)
	{
		if (inventory[i].ident & (IDENT_BREAKS)) return (i);
	}

	/* Not found - item has been destroyed during use */
	return (-1);
}




/*
 * Eat some food (from the pack or floor)
 */
void do_cmd_eat_food(void)
{
	int item, lev;
	bool ident;

	object_type *o_ptr;
	cptr q, s;

	/* golems cannot eat */
	if (p_ptr->prace == 16)
	{
		msg_print("Golems cannot eat.");
		return;
	}

	/* Restrict choices to food */
	item_tester_tval = TV_FOOD;

	/* Get an item */
	q = "Eat which item? ";
	s = "You have nothing to eat.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

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


	/* Sound */
	sound(MSG_EAT);


	/* Take a turn */
	p_ptr->energy_use = 100;

	/* Identity not known yet */
	ident = FALSE;

	/* Object level */
	lev = k_info[o_ptr->k_idx].level;

	/* Eat the food */
	use_object(o_ptr, &ident);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* We have tried it */
	object_tried(o_ptr);

	/* The player is now aware of the object */
	if (ident && !object_aware_p(o_ptr))
	{
		object_aware(o_ptr);
		gain_exp((lev + (p_ptr->lev / 2)) / p_ptr->lev);
		p_ptr->notice |= PN_SQUELCH;
	}

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP);


	/* Destroy a food in the pack */
	if (item >= 0)
	{
		inven_item_increase(item, -1);
		inven_item_describe(item);
		inven_item_optimize(item);
	}

	/* Destroy a food on the floor */
	else
	{
		floor_item_increase(0 - item, -1);
		floor_item_describe(0 - item);
		floor_item_optimize(0 - item);
	}
}



/*
 * Quaff a potion (from the pack or the floor)
 */
void do_cmd_quaff_potion(void)
{
	int item, lev;
	bool ident;
	object_type *o_ptr;
	cptr q, s;


	/* Restrict choices to potions */
	item_tester_tval = TV_POTION;

	/* Get an item */
	q = "Quaff which potion? ";
	s = "You have no potions to quaff.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

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


	/* Sound */
	sound(MSG_QUAFF);


	/* Take a turn */
	p_ptr->energy_use = 100;

	/* Not identified yet */
	ident = FALSE;

	/* Object level */
	lev = k_info[o_ptr->k_idx].level;

	/* Quaff the potion */
	use_object(o_ptr, &ident);

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* The item has been tried */
	object_tried(o_ptr);

	/* An identification was made */
	if (ident && !object_aware_p(o_ptr))
	{
		object_aware(o_ptr);
		gain_exp((lev + (p_ptr->lev / 2)) / p_ptr->lev);
		p_ptr->notice |= PN_SQUELCH;
	}

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP);

	/* Destroy a potion in the pack */
	if (item >= 0)
	{
		inven_item_increase(item, -1);
		inven_item_describe(item);
		inven_item_optimize(item);
	}

	/* Destroy a potion on the floor */
	else
	{
		floor_item_increase(0 - item, -1);
		floor_item_describe(0 - item);
		floor_item_optimize(0 - item);
	}
}

/*
 * The "readable" tester
 */
static bool item_tester_hook_readable(const object_type *o_ptr)
{

	if (o_ptr->tval == TV_SCROLL) return (TRUE);
	if ((o_ptr->tval == TV_SPECIAL) && (o_ptr->sval == SV_TREASURE)) return (TRUE);

	/* Assume not readable */
	return (FALSE);
}


/*
 * Read a scroll (from the pack or floor).
 *
 * Certain scrolls can be "aborted" without losing the scroll.  These
 * include scrolls with no effects but recharge or identify, which are
 * cancelled before use.  XXX Reading them still takes a turn, though.
 *
 * Identify scrolls and *Identify* scrolls also have a chance of not
 * being used up even if not cancelled.
 */
void do_cmd_read_scroll(void)
{
	int item, used_up, lev;
	bool ident;

	object_type *o_ptr;

	cptr q, s;


	/* Check some conditions */
	if ((p_ptr->timed[TMD_BLIND]) && (!p_ptr->timed[TMD_BRAIL]))
	{
		msg_print("You can't see anything.");
		return;
	}
    else if (p_ptr->timed[TMD_BLIND])
	{
		msg_print("You read the scroll with your hands..");
	}
	
	if ((no_lite()) && (!p_ptr->timed[TMD_BRAIL]))
	{
		msg_print("You have no light to read by.");
		return;
	}
	else if (no_lite())
	{
		msg_print("You read the scroll with your hands..");
	}

	if (p_ptr->timed[TMD_CONFUSED])
	{
		msg_print("You are too confused!");
		return;
	}

	/* Restrict choices to scrolls */
	/* item_tester_tval = TV_SCROLL; */
	item_tester_hook = item_tester_hook_readable;

	/* Get an item */
	q = "Read which scroll? ";
	s = "You have no scrolls to read.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];

		/* to make sure scroll of rune of protection gets used up */
        if ((o_ptr->tval == TV_SCROLL) && (o_ptr->sval == SV_SCROLL_RUNE_OF_PROTECTION))
           spellswitch = 3;
	}



	/* Check for amnesia */
	if (rand_int(2) != 0 && p_ptr->timed[TMD_AMNESIA])
	{
		/* Can't remember how */
		msg_print("You can't remember how to read!");

		/* Take (most of) a turn */
		p_ptr->energy_use = 90;
		return;
	}

	/* Not identified yet */
	ident = FALSE;

	/* Object level */
	lev = k_info[o_ptr->k_idx].level;

	/* Read the scroll */
	used_up = use_object(o_ptr, &ident);

	/* Take a turn */
	if ((used_up) || (ident)) p_ptr->energy_use = 100;

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* The item was tried */
	object_tried(o_ptr);

	/* An identification was made */
	if (ident && !object_aware_p(o_ptr))
	{
		object_aware(o_ptr);
		gain_exp((lev + (p_ptr->lev / 2)) / p_ptr->lev);
		p_ptr->notice |= PN_SQUELCH;
	}

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP);

	/* Hack -- allow certain scrolls to be "preserved" */
	if (!used_up) return;

	/* Destroy a scroll in the pack */
	if (item >= 0)
	{
		inven_item_increase(item, -1);
		inven_item_describe(item);
		inven_item_optimize(item);
	}

	/* Destroy a scroll on the floor */
	else
	{
		floor_item_increase(0 - item, -1);
		floor_item_describe(0 - item);
		floor_item_optimize(0 - item);
	}
}




/*
 * Use a staff
 *
 * One charge of one staff disappears.
 *
 * Hack -- staffs of identify can be "cancelled".
 */
void do_cmd_use_staff(void)
{
	int item, chance, lev;

	bool ident;
	bool fluke = FALSE;
	
	object_type *o_ptr;

	bool use_charge;

	cptr q, s;

	/* Restrict choices to staves */
	item_tester_tval = TV_STAFF;

	/* Get an item */
	q = "Use which staff? ";
	s = "You have no staff to use.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR | USE_EQUIP))) return;

	/* Get the item (in the pack) */
	if (item >= 0)
	{
		o_ptr = &inventory[item];

		/* Mark item for reduction */
		o_ptr->ident |= (IDENT_BREAKS);
	}

	/* Get the item (on the floor) */
	else
	{
		o_ptr = &o_list[0 - item];
	}

	/* Notice empty staffs (should come before chance of success) */
	if (o_ptr->charges <= 0)
	{
		/* takes a turn only if you didn't already know it was empty */
		/* (you always know charges if aware) */
		if (!object_aware_p(o_ptr))
		{
			p_ptr->energy_use = 100;
			msg_print("You realize that the staff is out of charges.");
		}
		else
		{
			msg_print("That staff has no charges left.");
		}

		if (flush_failure) flush();		
		o_ptr->ident |= (IDENT_EMPTY);
		p_ptr->notice |= (PN_COMBINE | PN_REORDER);
		p_ptr->window |= (PW_INVEN);

		return;
	}

	/* Take a turn */
	p_ptr->energy_use = 100;

	/* thieves zap slightly faster (usually -not as often as wands & rods) */
	if ((!p_ptr->timed[TMD_CONFUSED]) && (cp_ptr->flags & CF_CLASS_SPEED) && 
		(rand_int(p_ptr->skills[SKILL_DEV]) > 15 + badluck))
	{
		if (p_ptr->lev >= 40) p_ptr->energy_use = 90;
		else if (p_ptr->lev >= 20) p_ptr->energy_use = 95;
	}

	/* Not identified yet */
	ident = FALSE;

	/* Extract the item diffuculty (now separated from item level) */
	lev = k_info[o_ptr->k_idx].extra;

	/* cursed staffs are harder to use */
	if (cursed_p(o_ptr)) lev += 5 + badluck/3;

	/* blessed devices are easier to use */
	if ((o_ptr->blessed > 1) && (lev > 23)) lev -= 12;
	else if ((o_ptr->blessed > 1) && (lev > 12)) lev = 12;
	else if ((o_ptr->blessed) && (lev > 4)) lev -= 4;

	/* Base chance of success */
	chance = p_ptr->skills[SKILL_DEV];

	/* Confusion hurts skill */
	/* (should be able to use staff of curing to cure confusion) */
	if ((p_ptr->timed[TMD_CONFUSED]) && (o_ptr->sval != SV_STAFF_CURING))
    {
       if (goodluck > 16) chance = (chance * 8) / 9;
       else if (goodluck > 9) chance = (chance * 3) / 4;
       else if (goodluck > 2) chance = (chance * 2) / 3;
	   else if (chance >= 60) chance = ((chance * 2) / 3) - (5 + (badluck/2));
       else chance = (chance / 2) - (badluck/3);
    }

	/* High level objects are harder */
	/* no limit now that difficulty is separated from depth */
	/* (very few devices have difficulty > 50) */
	/* chance = chance - ((lev > 50) ? 50 : lev); */
	chance = chance - lev;

	/* Give everyone a (slight) chance (USE_DEVICE==3) */
	if (chance < USE_DEVICE) /* 33% success rate (1 in 3) at best */
	{
		if (chance + 1 < 2) chance = 2;
		else chance += 1;
		if (lev < 9) lev = 9;
		if (rand_int(lev) < chance) fluke = TRUE; /* success */
	}

	/* Roll for usage */
	if ((randint(chance) < USE_DEVICE) && (!fluke))
	{
		if (flush_failure) flush();
		msg_print("You failed to use the staff properly.");
		return;
	}

	/* Sound */
	sound(MSG_USE_STAFF);

	/* Use the staff */
	use_charge = use_object(o_ptr, &ident);


	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);
	
	/* Make sure we have the correct item (from UnAngband) */
	if (item >= 0)
	{
		item = find_item_to_reduce(item);

		/* Item has already been destroyed */
		if (item < 0) return;

		/* Get the object */
		o_ptr = &inventory[item];

		/* Clear marker */
		o_ptr->ident &= ~(IDENT_BREAKS);
	}

	/* Tried the item */
	object_tried(o_ptr);

	/* An identification was made */
	if (ident && !object_aware_p(o_ptr))
	{
		object_aware(o_ptr);
		gain_exp((lev + (p_ptr->lev / 2)) / p_ptr->lev);
		p_ptr->notice |= PN_SQUELCH;
	}

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP);


	/* Hack -- some uses are "free" */
	if (!use_charge) return;

	/* Staff of manafree sometimes uses multiple charges */
    if (p_ptr->manafree > 19)
	{
        if (p_ptr->manafree/10 > o_ptr->charges)
        {
           o_ptr->charges = 0;

           /* If you don't have enough charges it might explode */
           if (randint(75 + badluck) > 55)
           {
		      msg_print("The staff doesn't have enough charges to handle your spell!");
		      msg_print("There is a bright flash of light.");

		      /* Reduce the charges of rods/wands/staves */
		      reduce_charges(o_ptr, 1);

		      /* *Identified* items keep the knowledge about the charges */
		      if (!(o_ptr->ident & IDENT_MENTAL))
		      {
			     /* We no longer "know" the item */
			     o_ptr->ident &= ~(IDENT_KNOWN);
		      }

		      /* Reduce and describe inventory */
		      if (item >= 0)
		      {
			     inven_item_increase(item, -1);
			     inven_item_describe(item);
			     inven_item_optimize(item);
		      }
		      /* Reduce and describe floor item */
		      else
		      {
			      floor_item_increase(0 - item, -1);
			      floor_item_describe(0 - item);
			      floor_item_optimize(0 - item);
		      }
           }
        }
        /* normal usage: 1 charge for every 10 mana cost of the spell cast */
        else o_ptr->charges -= p_ptr->manafree/10;
    }
    else
    {
        /* Use a single charge (every other staff) */
	    o_ptr->charges--;
    }

	/* reset manafree */
    p_ptr->manafree = 0;

	/* Describe charges in the pack */
	if (item >= 0)
	{
		inven_item_charges(item);
	}

	/* Describe charges on the floor */
	else
	{
		floor_item_charges(0 - item);
	}
}


/*
 * Hook to specify wand or rod
 */
static bool item_tester_hook_zap(const object_type *o_ptr)
{
	if (o_ptr->tval == TV_WAND) return TRUE;
	if (o_ptr->tval == TV_ROD) return TRUE;

	return (FALSE);
}


#if nolongerneeded
/*
 * Aim a wand (from the pack or floor).
 *
 * Use a single charge from a single item.
 * Handle "unstacking" in a logical manner.
 *
 * For simplicity, you cannot use a stack of items from the
 * ground.  This would require too much nasty code.
 *
 * There are no wands which can "destroy" themselves, in the inventory
 * or on the ground, so we can ignore this possibility.  Note that this
 * required giving "wand of wonder" the ability to ignore destruction
 * by electric balls.
 *
 * All wands can be "cancelled" at the "Direction?" prompt for free.
 *
 * Note that the basic "bolt" wands do slightly less damage than the
 * basic "bolt" rods, but the basic "ball" wands do the same damage
 * as the basic "ball" rods.
 */
void do_cmd_aim_wand(void)
{
	int item, lev;
	bool ident;
	object_type *o_ptr;
	cptr q, s;


	/* Restrict choices to wands */
	item_tester_tval = TV_WAND;

	/* Get an item */
	q = "Aim which wand? ";
	s = "You have no wand to aim.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

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



	/* Aim the wand */
	if (!use_object(o_ptr, &ident)) return;


	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Mark it as tried */
	object_tried(o_ptr);

	/* Object level */
	lev = k_info[o_ptr->k_idx].level;

	/* Apply identification */
	if (ident && !object_aware_p(o_ptr))
	{
		object_aware(o_ptr);
		gain_exp((lev + (p_ptr->lev / 2)) / p_ptr->lev);
		p_ptr->notice |= PN_SQUELCH;
	}

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP);


	/* Use a single charge */
	o_ptr->charges--;

	/* Describe the charges in the pack */
	if (item >= 0)
	{
		inven_item_charges(item);
	}

	/* Describe the charges on the floor */
	else
	{
		floor_item_charges(0 - item);
	}
}
#endif



/*
 * Zap a Rod
 *
 * Unstack fully charged rods as needed.
 *
 * Hack -- rods of identify can be "cancelled"
 * All rods can be cancelled at the "Direction?" prompt
 */
/* void do_cmd_zap_rod(void) */
void do_cmd_zap_wandorrod(void)
{
	int item;
	bool ident;
	object_type *o_ptr;
	cptr q, s;


	/* Restrict choices to wands or rods */
	/* item_tester_tval = TV_ROD; */
	item_tester_hook = item_tester_hook_zap;

	/* Get an item */
	q = "Zap which wand or rod? ";
	s = "You have no wand or rod to zap.";
	if (!get_item(&item, q, s, (USE_INVEN | USE_FLOOR))) return;

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

	/* Zap the rod */
	if (!use_object(o_ptr, &ident)) return;

	/* Combine / Reorder the pack (later) */
	p_ptr->notice |= (PN_COMBINE | PN_REORDER);

	/* Tried the object */
	object_tried(o_ptr);

	/* Successfully determined the object function */
	if (ident && !object_aware_p(o_ptr))
	{
		/* Object level */
		int lev = k_info[o_ptr->k_idx].level;

		object_aware(o_ptr);
		gain_exp((lev + (p_ptr->lev / 2)) / p_ptr->lev);
		p_ptr->notice |= PN_SQUELCH;
	}

	/* Window stuff */
	p_ptr->window |= (PW_INVEN | PW_EQUIP);
	
	/* For rods, this is done in use-obj.c */
    if (o_ptr->tval == TV_WAND)
	{
		/* Use a single charge */
		o_ptr->charges--;

		/* Describe the charges in the pack */
		if (item >= 0)
		{
			inven_item_charges(item);
		}

		/* Describe the charges on the floor */
		else
		{
			floor_item_charges(0 - item);
		}
	}
}




/*
 * Hook to determine if an object is activatable
 */
static bool item_tester_hook_activate(const object_type *o_ptr)
{
	/* Extract the flags */
	u32b f1, f2, f3, f4;
	object_flags(o_ptr, &f1, &f2, &f3, &f4);

	/* Check activation flag */
	if (f3 & (TR3_ACTIVATE)) return (TRUE);

	/* Assume not */
	return (FALSE);
}


/*
 * Activate a wielded object.  Wielded objects never stack.
 * And even if they did, activatable objects never stack.
 *
 * Note that it always takes a turn to activate an artifact, even if
 * the user hits "escape" at the "direction" prompt.
 */
void do_cmd_activate(void)
{
	int item, lev, chance;
	bool ident;
	bool forget = FALSE;
	bool fluke = FALSE;
	object_type *o_ptr;

	cptr q, s;


	/* Prepare the hook */
	item_tester_hook = item_tester_hook_activate;

	/* Get an item */
	/* (allow thrown item artifacts to activate) */
	q = "Activate which item? ";
	s = "You have nothing to activate.";
	if (!get_item(&item, q, s, (USE_EQUIP | USE_QUIVER))) return;

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

	/* Check the recharge (moved here from activate_object()) */
	if (o_ptr->timeout)
	{
		msg_print("It whines, glows and fades...");
		return;
	}

	/* Take a turn */
	p_ptr->energy_use = 100;


	/* Extract the item diffuculty (now separated from item level) */
	lev = k_info[o_ptr->k_idx].extra;
	
	/* cursed items are harder to use (but not cursed artifacts) */
	/* (currently these items are never cursed but that will likely change) */
	if (cursed_p(o_ptr)) lev += 5 + badluck/3;

	/* blessed items are easier to activate */
	if ((o_ptr->blessed > 1) && (lev > 23)) lev -= 12;
	else if ((o_ptr->blessed > 1) && (lev > 12)) lev = 12;
	else if ((o_ptr->blessed) && (lev > 4)) lev -= 4;

	/* Hack -- use artifact level instead (bless/curse doesn't effect artifact activation) */
	if (artifact_p(o_ptr)) lev = a_info[o_ptr->name1].level;

	/* Base chance of success */
	chance = p_ptr->skills[SKILL_DEV];

	/* Confusion hurts skill */
	if (p_ptr->timed[TMD_CONFUSED])
    {
       if (goodluck > 16) chance = (chance * 8) / 9;
       else if (goodluck > 9) chance = (chance * 3) / 4;
       else if (goodluck > 2) chance = (chance * 2) / 3;
	   else if (chance >= 60) chance = ((chance * 2) / 3) - (5 + (badluck/2));
       else chance = (chance / 2) - (badluck/3);
    }

	/* High level objects are harder */
	/* no limit now that difficulty is separated from depth */
	/* (very few devices have difficulty > 50) */
	if (artifact_p(o_ptr)) chance = chance - ((lev > 50) ? 50 : lev);
	else chance = chance - lev;


	/* Check for amnesia */
	if (rand_int(2) != 0 && p_ptr->timed[TMD_AMNESIA])
	{
		if (chance < 60)
		{
			if (flush_failure) flush();
			msg_print("You can't remember how to activate it.");
			return;
		}
		else
		{
			chance = chance / 3;
			forget = TRUE;
		}
	}

	/* Give everyone a (slight) chance (USE_DEVICE==3) */
	if (chance < USE_DEVICE) /* 33% success rate (1 in 3) at best */
	{
		if (chance + 1 < 2) chance = 2;
		else chance += 1;
		if (lev < 9) lev = 9;
		if (rand_int(lev) < chance) fluke = TRUE; /* success */
	}

	/* Roll for usage */
	if ((randint(chance) < USE_DEVICE) && (!fluke))
	{
		if (flush_failure) flush();
		if (forget) msg_print("You can't remember how to activate it.");
		else msg_print("You failed to activate it properly.");
		return;
	}
	
	/* Activate the object */
	if (use_object(o_ptr, &ident))
	{
	   u32b f1, f2, f3, f4;
       /* Learn from the activation */
       if (!object_known_p(o_ptr))
	   {
			char o_name[80];

			object_aware(o_ptr);
			object_known(o_ptr);

			gain_exp((lev - 5 + (p_ptr->lev / 2)) / p_ptr->lev);
		  
			/* notice the discovery */
			object_desc(o_name, sizeof(o_name), o_ptr, FALSE, 3);		  
			msg_format("You have %s.", o_name);
	   }

	   /* Extract the flags */
	   object_flags(o_ptr, &f1, &f2, &f3, &f4);

       /* exp drain is much more likely when */
       /* activating an object with the DRAIN_EXP flag */
       /* (two chances to kick in: here and in project() function) */
       if ((f2 & (TR2_DRAIN_EXP)) && (p_ptr->exp_drain)) rxp_drain(25);
    }
}
