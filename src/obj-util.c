/**
 * \file obj-util.c
 * \brief Object utilities
 *
 * Copyright (c) 1997 Ben Harrison, James E. Wilson, Robert A. Koeneke
 *
 * This work is free software; you can redistribute it and/or modify it
 * under the terms of either:
 *
 * a) the GNU General Public License as published by the Free Software
 *    Foundation, version 2, or
 *
 * b) the "Angband licence":
 *    This software may be copied and distributed for educational, research,
 *    and not for profit purposes provided that this copyright and statement
 *    are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"
#include "cave.h"
#include "cmd-core.h"
#include "effects.h"
#include "game-input.h"
#include "game-world.h"
#include "generate.h"
#include "grafmode.h"
#include "init.h"
#include "mon-make.h"
#include "monster.h"
#include "obj-fault.h"
#include "obj-desc.h"
#include "obj-gear.h"
#include "obj-ignore.h"
#include "obj-init.h"
#include "obj-knowledge.h"
#include "obj-make.h"
#include "obj-pile.h"
#include "obj-slays.h"
#include "obj-tval.h"
#include "obj-util.h"
#include "player-history.h"
#include "player-spell.h"
#include "player-util.h"
#include "randname.h"
#include "z-queue.h"

struct object_base *kb_info;
struct object_kind *k_info;
struct artifact *a_info;
struct ego_item *e_info;
struct flavor *flavors;

/**
 * Hold the titles of cards, up to 31 characters each.
 * Longest "Simple" prefix is 6
 * Longest "Creator" suffix is 7
 * Longest "-" infix is 1
 * Longest " XX 1.2.3" version is 9
 */
static char **card_adj;
static int n_card_adj;

/**
 * Hold the titles of pills, 6 to 28 characters each, plus quotes.
 */
static char **pill_adj;
static int n_pill_adj;

static void flavor_assign_fixed(void)
{
	int i;
	struct flavor *f;

	for (f = flavors; f; f = f->next) {
		if (f->sval == SV_UNKNOWN)
			continue;

		for (i = 0; i < z_info->k_max; i++) {
			struct object_kind *k = &k_info[i];
			if (k->tval == f->tval && k->sval == f->sval)
				k->flavor = f;
		}
	}
}


static void flavor_assign_random(byte tval)
{
	int i;
	int flavor_count = 0;
	int choice;
	struct flavor *f;

	/* Count the random flavors for the given tval */
	for (f = flavors; f; f = f->next)
		if (f->tval == tval && f->sval == SV_UNKNOWN)
			flavor_count++;
	for (i = 0; i < z_info->k_max; i++) {
		if (k_info[i].tval != tval || k_info[i].flavor || (tval == TV_LIGHT && kf_has(k_info[i].kind_flags, KF_EASY_KNOW)))
			continue;

		if (!flavor_count) {
			for (f = flavors; f; f = f->next)
				if (f->tval == tval && f->sval == SV_UNKNOWN)
					flavor_count++;
			int need_count = 0;
			for (i = 0; i < z_info->k_max; i++) {
				if (k_info[i].tval != tval || k_info[i].flavor || (tval == TV_LIGHT && kf_has(k_info[i].kind_flags, KF_EASY_KNOW)))
					continue;
				need_count++;
			}
			quit_fmt("Not enough flavors for tval %d (%s), found %d, need %d.", tval, tval_find_name(tval), flavor_count, need_count);
		}

		choice = randint0(flavor_count);

		for (f = flavors; f; f = f->next) {
			if (f->tval != tval || f->sval != SV_UNKNOWN)
				continue;

			if (choice == 0) {
				k_info[i].flavor = f;
				f->sval = k_info[i].sval;
				if (tval == TV_PILL)
					f->text = pill_adj[k_info[i].sval - 1];
				if (tval == TV_CARD)
					f->text = card_adj[k_info[i].sval - 1];
				flavor_count--;
				break;
			}

			choice--;
		}
	}
}

/**
 * Reset svals on flavors, effectively removing any fixed flavors.
 *
 * Mainly useful for randarts so that fixed flavors for standards aren't
 * predictable. The One Ring is kept as fixed, since it lives through randarts.
 */
void flavor_reset_fixed(void)
{
	struct flavor *f;

	for (f = flavors; f; f = f->next)
		f->sval = SV_UNKNOWN;
}

static void clean_strings(char ***array, int *length)
{
	for(int i=0;i<*length;i++)
		string_free((*array)[i]);
	mem_free(*array);
	*array = NULL;
	*length = 0;
}

static void insert_string(char *buf, int i, char ***array, int *length)
{
	if (i >= *length) {
		int old = *length;
		*length = i+1;
		*array = mem_realloc(*array, sizeof(*array) * *length);
		for(int j=old; j<*length; j++) {
			(*array)[j] = NULL;
		}
	}
	(*array)[i] = string_make(buf);
}

/**
 * Prepare the "variable" part of the "k_info" array.
 *
 * The "color"/"metal"/"type" of an item is its "flavor".
 * For the most part, flavors are assigned randomly each game.
 *
 * Initialize descriptions for the "colored" objects, including:
 * Devices, Wands, Gadgets, Mushrooms, Pills, Cards.
 *
 * Hack -- make sure everything stays the same for each saved game
 * This is accomplished by the use of a saved "random seed", as in
 * "town_gen()".  Since no other functions are called while the special
 * seed is in effect, so this function is pretty "safe".
 */
void flavor_init(void)
{
	int i;

	/* Hack -- Use the "simple" RNG */
	Rand_quick = true;

	/* Hack -- Induce consistant flavors */
	Rand_value = seed_flavor;

	/* Scrub all flavors and re-parse for new players */
	if (turn == 1) {
		struct flavor *f;

		for (i = 0; i < z_info->k_max; i++) {
			k_info[i].flavor = NULL;
		}
		for (f = flavors; f; f = f->next) {
			f->sval = SV_UNKNOWN;
		}
		cleanup_parser(&flavor_parser);
		run_parser(&flavor_parser);
	}

	if (OPT(player, birth_randarts))
		flavor_reset_fixed();

	clean_strings(&pill_adj, &n_pill_adj);
	clean_strings(&card_adj, &n_card_adj);

	flavor_assign_fixed();

	flavor_assign_random(TV_LIGHT);
	flavor_assign_random(TV_DEVICE);
	flavor_assign_random(TV_WAND);
	flavor_assign_random(TV_GADGET);
	flavor_assign_random(TV_MUSHROOM);

	/* Pills (random titles, always magenta)
	 * The pills will be randomized again by flavor_assign_random.
	 * So this doesn't have to change the base names ("yadar" of "yadarine"), only the suffix.
	 * It also doesn't matter (much) whether all suffixes are used or some used more than once.
	 * so:
	 * For each output name:
	 * 		Copy from a sequential input basename
	 * 		+ a randomly chosen input suffix.
	 */
	char **suff = NULL;
	int n_suff = 0;
	
	/* Pull out suffixes into an array */
	int pills = 0;
	for (struct flavor *f = flavors; f; f = f->next) {
		if (f->tval == TV_PILL && f->sval == SV_UNKNOWN) {
			char *suffix = strchr(f->text, '|');
			assert(suffix);
			suffix++;
			char buf[16];
			strncpy(buf, suffix, sizeof(buf));
			buf[sizeof(buf)-1] = 0;
			insert_string(buf, pills, &suff, &n_suff);
			pills++;
		}
	}

	/* And combine them */
	i = 0;
	if (pill_adj) {
		mem_free(pill_adj);
		pill_adj = NULL;
	}
	n_pill_adj = 0;
	for (struct flavor *f = flavors; f; f = f->next) {
		if (f->tval == TV_PILL && f->sval == SV_UNKNOWN) {
			char base[11];
			strncpy(base, f->text, sizeof(base));
			base[sizeof(base)-1] = 0;
			char *suffix = strchr(base, '|');
			assert(suffix);
			*suffix = 0;
			char buf[64];
			snprintf(buf, sizeof(buf), "%s%s", base, suff[randint0(pills)]);
			buf[sizeof(buf)-1] = 0;
			insert_string(buf, i, &pill_adj, &n_pill_adj);
			i++;
		}
	}
	flavor_assign_random(TV_PILL);

	clean_strings(&suff, &n_suff);

	/* Cards (random titles, always blue) */
	int cards = 0;
	for (struct flavor *f = flavors; f; f = f->next) {
		if (f->tval == TV_CARD && f->sval == SV_UNKNOWN) {
			char *suffix = strchr(f->text, '|');
			assert(suffix);
			suffix++;
			char buf[16];
			strncpy(buf, suffix, sizeof(buf));
			buf[sizeof(buf)-1] = 0;
			insert_string(buf, cards, &suff, &n_suff);
			cards++;
		}
	}

	/* And combine them */
	i = 0;
	for (struct flavor *f = flavors; f; f = f->next) {
		if (f->tval == TV_CARD && f->sval == SV_UNKNOWN) {
			char base[11];
			char ext[11];
			strncpy(base, f->text, sizeof(base));
			base[sizeof(base)-1] = 0;
			char *suffix = strchr(base, '|');
			assert(suffix);
			*suffix = 0;
			strncpy(ext, suff[randint0(cards)], sizeof(ext));
			/* Determine capitalization */
			titlecase(base);
			titlecase(ext);
			/* Determine separator */
			char *sep = "";
			switch(randint0(20)) {
				case 0:
					sep = ".";
					break;
				case 1:
					sep = "/";
					break;
				case 2:
					sep = ":";
					break;
				case 3:
					sep = ".";
					break;
				case 4:
				case 5:
				case 6:
					sep = "-";
					break;
				case 7:
				case 8:
				case 9:
					sep = " ";
					break;
			}
			/* Determine suffix */
			char suff[8];
			strcpy(suff, "");
			int rn1 = randint1(9);
			int rn2 = randint0(randint0(9));
			int rn3 = randint0(randint0(randint0(9)));
			switch(randint0(12)) {
				case 0:
					switch(randint0(7)) {
						default:
							strcpy(suff, " X");
							break;
						case 1:
							strcpy(suff, " Z");
							break;
						case 2:
							strcpy(suff, " ZX");
							break;
						case 3:
							strcpy(suff, " Q");
							break;
						case 4:
							strcpy(suff, " XX");
							break;
						case 5:
							strcpy(suff, " II");
							break;
					}
					if (!one_in_(3))
						break;
					// fall through
				case 5:
					sprintf(suff + strlen(suff), " %d", rn1);
					break;
				case 1:
				case 2:
				case 3:
					sprintf(suff, " %d.%d", rn1, rn2);
					break;
				case 4:
					sprintf(suff, " %d%d", rn1, rn2);
					break;
				case 6:
					sprintf(suff, " %d.%d.%d", rn1, rn2, rn3);
					break;
			}
			char buf[64];
			snprintf(buf, sizeof(buf), "%s%s%s%s", base, sep, ext, suff);
			buf[sizeof(buf)-1] = 0;
			insert_string(buf, i, &card_adj, &n_card_adj);
			i++;
		}
	}
	flavor_assign_random(TV_CARD);

	/* Hack -- Use the "complex" RNG */
	Rand_quick = false;

	/* Analyze every object */
	for (i = 0; i < z_info->k_max; i++) {
		struct object_kind *kind = &k_info[i];

		/* Skip "empty" objects */
		if (!kind->name) continue;

		/* No flavor yields aware */
		if (!kind->flavor) kind->aware = true;
	}
}

/**
 * Set all flavors as aware
 */
void flavor_set_all_aware(void)
{
	int i;

	/* Analyze every object */
	for (i = 0; i < z_info->k_max; i++) {
		struct object_kind *kind = &k_info[i];

		/* Skip empty objects */
		if (!kind->name) continue;

		/* Flavor yields aware */
		if (kind->flavor) kind->aware = true;
	}
}

/**
 * Obtain the flags for an item
 */
void object_flags(const struct object *obj, bitflag flags[OF_SIZE])
{
	of_wipe(flags);
	if (!obj) return;
	of_copy(flags, obj->flags);
}


/**
 * Obtain the flags for an item which are known to the player
 */
void object_flags_known(const struct object *obj, bitflag flags[OF_SIZE])
{
	object_flags(obj, flags);
	of_inter(flags, obj->known->flags);

	if (!obj->kind) {
		return;
	}

	if (object_flavor_is_aware(obj)) {
		of_union(flags, obj->kind->flags);
	}

	if (obj->ego && easy_know(obj)) {
		of_union(flags, obj->ego->flags);
		of_diff(flags, obj->ego->flags_off);
	}
}

/**
 * Obtain the carried flags for an item
 */
void object_carried_flags(const struct object *obj, bitflag flags[OF_SIZE])
{
	of_wipe(flags);
	if (!obj) return;
	of_copy(flags, obj->carried_flags);
}

/**
 * Obtain the carried flags for an item which are known to the player
 */
void object_carried_flags_known(const struct object *obj, bitflag flags[OF_SIZE])
{
	object_carried_flags(obj, flags);
	of_inter(flags, obj->known->carried_flags);

	if (!obj->kind) {
		return;
	}

	if (object_flavor_is_aware(obj)) {
		of_union(flags, obj->kind->carried_flags);
	}

	if (obj->ego && easy_know(obj)) {
		of_union(flags, obj->ego->carried_flags);
		of_diff(flags, obj->ego->carried_flags_off);
	}
}

/**
 * Apply a tester function, skipping all non-objects and gold
 */
bool object_test(item_tester tester, const struct object *obj)
{
	/* Require object */
	if (!obj) return false;

	/* Ignore gold */
	if (tval_is_money(obj)) return false;

	/* Pass without a tester, or tail-call the tester if it exists */
	return !tester || tester(obj);
}


/**
 * Return true if the item is unknown (has yet to be seen by the player).
 */
bool is_unknown(const struct object *obj)
{
	struct grid_data gd = {
		.m_idx = 0,
		.f_idx = 0,
		.first_kind = NULL,
		.trap = NULL,
		.multiple_objects = false,
		.unseen_object = false,
		.unseen_money = false,
		.lighting = LIGHTING_LOS,
		.in_view = false,
		.is_player = false,
		.hallucinate = false,
	};
	map_info(obj->grid, &gd);
	return gd.unseen_object;
}	


/**
 * Looks if "inscrip" is present on the given object.
 */
unsigned check_for_inscrip(const struct object *obj, const char *inscrip)
{
	unsigned i = 0;
	const char *s;

	if (!obj->note) return 0;

	s = quark_str(obj->note);

	/* Needing this implies there are bad instances of obj->note around,
	 * but I haven't been able to track down their origins - NRM */
	if (!s) return 0;

	do {
		s = strstr(s, inscrip);
		if (!s) break;

		i++;
		s++;
	} while (s);

	return i;
}

/**
 * Looks if "inscrip" immediately followed by a decimal integer without a
 * leading sign character is present on the given object.  Returns the number
 * of times such an inscription occurs and, if that value is at least one,
 * sets *ival to the value of the integer that followed the first such
 * inscription.
 */
unsigned check_for_inscrip_with_int(const struct object *obj, const char *inscrip, int* ival)
{
	unsigned i = 0;
	size_t inlen = strlen(inscrip);
	const char *s;

	if (!obj->note) return 0;

	s = quark_str(obj->note);

	/* Needing this implies there are bad instances of obj->note around,
	 * but I haven't been able to track down their origins - NRM */
	if (!s) return 0;

	do {
		s = strstr(s, inscrip);
		if (!s) break;
		if (isdigit(s[inlen])) {
			if (i == 0) {
				long inarg = strtol(s + inlen, 0, 10);

				*ival = (inarg < INT_MAX) ? (int) inarg : INT_MAX;
			}
			i++;
		}
		s++;
	} while (s);

	return i;
}

/*** Object kind lookup functions ***/

/**
 * Return the object kind with the given `tval` and `sval`, or NULL.
 */
struct object_kind *lookup_kind(int tval, int sval)
{
	int k;

	/* Look for it */
	for (k = 0; k < z_info->k_max; k++) {
		struct object_kind *kind = &k_info[k];
		if (kind->tval == tval && kind->sval == sval)
			return kind;
	}

	/* Failure */
	msg("No object: %d:%d (%s)", tval, sval, tval_find_name(tval));
	return NULL;
}

struct object_kind *objkind_byid(int kidx) {
	if (kidx < 0 || kidx >= z_info->k_max)
		return NULL;
	return &k_info[kidx];
}


/*** Textual<->numeric conversion ***/

/**
 * Return the a_idx of the artifact with the given name
 */
struct artifact *lookup_artifact_name(const char *name)
{
	int i;
	int a_idx = -1;

	/* Look for it */
	for (i = 0; i < z_info->a_max; i++) {
		struct artifact *art = &a_info[i];

		/* Test for equality */
		if (art->name && streq(name, art->name))
			return art;
		
		/* Test for close matches */
		if (strlen(name) >= 3 && art->name && my_stristr(art->name, name)
			&& a_idx == -1)
			a_idx = i;
	}

	/* Return our best match */
	return a_idx > 0 ? &a_info[a_idx] : NULL;
}

/**
 * \param name ego type name
 * \param tval object tval
 * \param sval object sval
 * \return eidx of the ego item type
 */
struct ego_item *lookup_ego_item(const char *name, int tval, int sval)
{
	int i;

	/* Look for it */
	for (i = 0; i < z_info->e_max; i++) {
		struct ego_item *ego = &e_info[i];
		struct poss_item *poss_item = ego->poss_items;

		/* Reject nameless and wrong names */
		if (!ego->name) continue;
		if (!streq(name, ego->name)) continue;

		/* Check tval and sval */
		while (poss_item) {
			struct object_kind *kind = lookup_kind(tval, sval);
			if (kind->kidx == poss_item->kidx) {
				return ego;
			}
			poss_item = poss_item->next;
		}
	}

	return NULL;
}

/**
 * Return the numeric sval of the object kind with the given `tval` and
 * name `name`, and an ego item if that is also specified. Will find partial
 * matches ("combat boots" matching "pair of combat boots"), but only if no
 * exact match is found. If neither is found then it will look for a reverse
 * partial match ("hard hat (lamp)" matching "hard hat") - in that case it
 * will try to find an ego item ("hard hat (lamp)" matching "(lamp)").
 * The ego item pointer may be NULL, but if specified the resulting ego item
 * pointer (or NULL) will always be returned through it.
 */
int lookup_sval_ego(int tval, const char *name, const struct ego_item **ego)
{
	int k, e;
	unsigned int r;
	int length = 0;

	/* By default, no ego */
	if (ego)
		*ego = NULL;

	if ((sscanf(name, "%u%n", &r, &length) == 1) && (length == (int)strlen(name)))
		return r;

	/* Look for it */
	for (k = 0; k < z_info->k_max; k++) {
		struct object_kind *kind = &k_info[k];
		char cmp_name[1024];

		if (!kind || !kind->name) continue;

		obj_desc_name_format(cmp_name, sizeof cmp_name, 0, kind->name, 0,
							 false);

		/* Found a match */
		if (kind->tval == tval && !my_stricmp(cmp_name, name)) {
			return kind->sval;
		}
	}

	/* Try for a partial match */
	for (k = 0; k < z_info->k_max; k++) {
		struct object_kind *kind = &k_info[k];
		char cmp_name[1024];

		if (!kind || !kind->name) continue;

		obj_desc_name_format(cmp_name, sizeof cmp_name, 0, kind->name, 0,
							 false);

		/* Found a partial match */
		if (kind->tval == tval && my_stristr(cmp_name, name)) {
			return kind->sval;
		}
	}

	/* Try for a reverse partial match */
	for (k = 0; k < z_info->k_max; k++) {
		struct object_kind *kind = &k_info[k];
		char cmp_name[1024];

		if (!kind || !kind->name) continue;

		obj_desc_name_format(cmp_name, sizeof cmp_name, 0, kind->name, 0,
							 false);

		/* Found a reverse partial match */
		if (kind->tval == tval && my_stristr(name, cmp_name)) {
			if (ego) {
				/* Search for an ego item */
				for (e = 0; e < z_info->e_max; e++) {
					struct ego_item *eitem = &e_info[e];
					if (my_stristr(name, eitem->name)) {
						*ego = eitem;
						break;
					}
				}
			}
			return kind->sval;
		}
	}

	return -1;
}

/**
 * Return the numeric sval of the object kind with the given `tval` and
 * name `name`. Will find partial matches ("combat boots" matching "pair
 * of combat boots"), but only if no exact match is found.
 */
int lookup_sval(int tval, const char *name)
{
	return lookup_sval_ego(tval, name, NULL);
}

void object_short_name(char *buf, size_t max, const char *name)
{
	size_t j, k;
	/* Copy across the name, stripping modifiers & and ~) */
	size_t len = strlen(name);
	for (j = 0, k = 0; j < len && k < max - 1; j++) {
		if (j == 0 && name[0] == '&' && name[1] == ' ')
			j += 2;
		if (name[j] == '~')
			continue;

		buf[k++] = name[j];
	}
	buf[k] = 0;
}

/**
 * Sort comparator for objects using only tval and sval.
 * -1 if o1 should be first
 *  1 if o2 should be first
 *  0 if it doesn't matter
 */
static int compare_types(const struct object *o1, const struct object *o2)
{
	if (o1->tval == o2->tval)
		return CMP(o1->sval, o2->sval);
	else
		return CMP(o1->tval, o2->tval);
}	


/**
 * Sort comparator for objects
 * -1 if o1 should be first
 *  1 if o2 should be first
 *  0 if it doesn't matter
 *
 * The sort order is designed with the "list items" command in mind.
 */
int compare_items(const struct object *o1, const struct object *o2)
{
	/* unknown objects go at the end, order doesn't matter */
	if (is_unknown(o1)) {
		return (is_unknown(o2)) ? 0 : 1;
	} else if (is_unknown(o2)) {
		return -1;
	}

	/* known artifacts will sort first */
	if (object_is_known_artifact(o1) && object_is_known_artifact(o2))
		return compare_types(o1, o2);
	if (object_is_known_artifact(o1)) return -1;
	if (object_is_known_artifact(o2)) return 1;

	/* unknown objects will sort next */
	if (!object_flavor_is_aware(o1) && !object_flavor_is_aware(o2))
		return compare_types(o1, o2);
	if (!object_flavor_is_aware(o1)) return -1;
	if (!object_flavor_is_aware(o2)) return 1;

	/* if only one of them is worthless, the other comes first */
	if (o1->kind->cost == 0 && o2->kind->cost != 0) return 1;
	if (o1->kind->cost != 0 && o2->kind->cost == 0) return -1;

	/* otherwise, just compare tvals and svals */
	/* NOTE: arguably there could be a better order than this */
	return compare_types(o1, o2);
}


/**
 * Determine if an object has charges
 */
bool obj_has_charges(const struct object *obj)
{
	if (!tval_can_have_charges(obj)) return false;

	if (obj->pval <= 0) return false;

	return true;
}

/**
 * Determine if an object is zappable
 */
bool obj_can_zap(const struct object *obj)
{
	/* Any rods not charging? */
	if (tval_can_have_timeout(obj) && (!tval_is_light(obj)) && number_charging(obj) < obj->number)
		return true;

	return false;
}

/**
 * Determine if an object is activatable from inventory.
 * Most activatable items aren't, but printers (and any future unequippable activatable tools) must be.
 * Lights are less clunky if they can be activated in the pack.
 */
bool obj_is_pack_activatable(const struct object *obj)
{
	if ((tval_is_light(obj)) && (kf_has(obj->kind->kind_flags, KF_MIMIC_KNOW)))
		return true;

	if (obj_is_activatable(obj)) {
		/* Object has an activation */
		if (object_is_equipped(player->body, obj)) {
			/* If so, and you are wearing it, it's activatable */
			return true;
		} else {
			/* If not, it might still be activatable if it's the right tval */
			if (tval_is_printer(obj)) return true;
		}
	}
	return false;
}

/**
 * Determine if an object is activatable
 */
bool obj_is_activatable(const struct object *obj)
{
	return (object_effect(obj) && (!of_has(obj->flags, OF_NO_ACTIVATION))) ? true : false;
}

/**
 * Determine if an object can be activated now
 */
bool obj_can_activate(const struct object *obj)
{
	/* Candle type light sources can always be activated - it's equivalent to equipping and unequipping it.
	 * And not equivalent to running the effect (which happens on timeout).
	 */
	if ((tval_is_light(obj)) && (kf_has(obj->kind->kind_flags, KF_MIMIC_KNOW)))
		return true;
	if (obj_is_activatable(obj)) {
		/* Check the recharge */
		if (!obj->timeout) return true;
	}

	return false;
}

/**
 * Check if an object can be used to refuel other objects.
 */
bool obj_can_refill(const struct object *obj)
{
	const struct object *light = equipped_item_by_slot_name(player, "light");

	/* Need fuel? */
	if (of_has(obj->flags, OF_NO_FUEL)) return false;

	/* A lamp can be refueled from a battery */
	if (light && of_has(light->flags, OF_TAKES_FUEL)) {
		if (tval_is_fuel(obj))
			return true;
	}

	return false;
}

/* Can only take off non-sticky (or for the special case of lamps, uncharged) items */
bool obj_can_takeoff(const struct object *obj)
{
	if (of_has(obj->flags, OF_NO_EQUIP))
		return false;
	if (!obj_has_flag(obj, OF_STICKY))
		return true;
	if (tval_is_light(obj) && (obj->timeout == 0))
		return true;
	return false;
}

/* Equivalent, but implants can be removed (for use by the store) */
bool obj_cyber_can_takeoff(const struct object *obj)
{
	if (!obj_has_flag(obj, OF_STICKY))
		return true;
	if (tval_is_light(obj) && (obj->timeout == 0))
		return true;
	return false;
}

/* Can only put on wieldable items */
bool obj_can_wear(const struct object *obj)
{
	return ((wield_slot(obj) >= 0) && (!(of_has(obj->flags, OF_NO_EQUIP))));
}

/* Can only fire an item with the right tval */
bool obj_can_fire(const struct object *obj)
{
	return obj->tval == player->state.ammo_tval;
}

/**
 * Determine if an object is designed for throwing
 */
bool obj_is_throwing(const struct object *obj)
{
	return of_has(obj->flags, OF_THROWING);
}

/**
 * Determine if an object is a known artifact
 */
bool obj_is_known_artifact(const struct object *obj)
{
	if (!obj->artifact) return false;
	if (!obj->known) return false;
	if (!obj->known->artifact) return false;
	return true;
}

/* Can has inscrip pls */
bool obj_has_inscrip(const struct object *obj)
{
	return (obj->note ? true : false);
}

bool obj_has_flag(const struct object *obj, int flag)
{
	struct fault_data *c = obj->faults;

	/* Check the object's own flags */
	if (of_has(obj->flags, flag)) {
		return true;
	}

	/* Check any fault object flags */
	if (c) {
		int i;
		for (i = 1; i < z_info->fault_max; i++) {
			if (c[i].power && of_has(faults[i].obj->flags, flag)) {
				return true;
			}
		}
	}
	return false;
}

bool obj_is_useable(const struct object *obj)
{
	if (tval_is_useable(obj))
		return true;

	if (object_effect(obj))
		return true;

	if (tval_is_ammo(obj))
		return obj->tval == player->state.ammo_tval;

	return false;
}

const struct object_material *obj_material(const struct object *obj)
{
	return material + obj->kind->material;
}

bool obj_is_metal(const struct object *obj)
{
	return obj_material(obj)->metal;
}

/*** Generic utility functions ***/

/**
 * Return an object's effect.
 */
struct effect *object_effect(const struct object *obj)
{
	if (obj->activation)
		return obj->activation->effect;
	else if (obj->effect)
		return obj->effect;
	else
		return NULL;
}

/**
 * Does the given object need to be aimed?
 */ 
bool obj_needs_aim(struct object *obj)
{
	struct effect *effect = object_effect(obj);

	/* If the effect needs aiming, or if the object type needs
	   aiming, this object needs aiming. */
	return effect_aim(effect) || tval_is_ammo(obj) ||
			tval_is_wand(obj) ||
			(tval_is_rod(obj) && !object_flavor_is_aware(obj));
}

/**
 * Can the object fail if used?
 */
bool obj_can_fail(const struct object *o)
{
	if (tval_can_have_failure(o))
		return true;

	return wield_slot(o) == -1 ? false : true;
}


/**
 * Returns the number of times in 1000 that @ will FAIL
 * - thanks to Ed Graham for the formula
 */
int get_use_device_chance(const struct object *obj)
{
	int lev, fail, numerator, denominator;

	int skill = player->state.skills[SKILL_DEVICE];

	int skill_min = 10;
	int skill_max = 141;
	int diff_min  = 1;
	int diff_max  = 100;

	/* Depends on what kind of object in it */
	if (tval_is_printer(obj) || tval_is_battery(obj)) {
		/* Printers are limited by chunk supplies and fail after using chunks,
		 * so don't need any further limiting. Batteries are supposed to be
		 * obvious... and all batteries that will get here (that is, reusable
		 * ones like the atomic cell) are limited by the cooldown.
		 */
		return 0;
	}

	/* Extract the item level, which is the difficulty rating */
	if (obj->artifact)
		lev = obj->artifact->level;
	else
		lev = obj->kind->level;

	/* TODO: maybe use something a little less convoluted? */
	numerator   = (skill - lev) - (skill_max - diff_min);
	denominator = (lev - skill) - (diff_max - skill_min);

	/* Make sure that we don't divide by zero */
	if (denominator == 0) denominator = numerator > 0 ? 1 : -1;

	fail = (100 * numerator) / denominator;

	/* Ensure failure rate is between 1% and 75% */
	if (fail > 750) fail = 750;
	if (fail < 10) fail = 10;

	return fail;
}


/**
 * Distribute charges of rods, devices, or wands.
 *
 * \param source is the source item
 * \param dest is the target item, must be of the same type as source
 * \param amt is the number of items that are transfered
 */
void distribute_charges(struct object *source, struct object *dest, int amt)
{
	int charge_time = randcalc(source->time, 0, AVERAGE), max_time;

	/*
	 * Hack -- If rods, devices, or wands are dropped, the total maximum
	 * timeout or charges need to be allocated between the two stacks.
	 * If all the items are being dropped, it makes for a neater message
	 * to leave the original stack's pval alone. -LM-
	 */
	if (tval_can_have_charges(source)) {
		dest->pval = source->pval * amt / source->number;

		if (amt < source->number)
			source->pval -= dest->pval;
	}

	/*
	 * Hack -- Rods also need to have their timeouts distributed.
	 *
	 * The dropped stack will accept all time remaining to charge up to
	 * its maximum.
	 */
	if (tval_can_have_timeout(source) && (!tval_is_light(source))) {
		max_time = charge_time * amt;

		if (source->timeout > max_time)
			dest->timeout = max_time;
		else
			dest->timeout = source->timeout;

		if (amt < source->number)
			source->timeout -= dest->timeout;
	}
}


/**
 * Number of items (usually rods) charging
 */
int number_charging(const struct object *obj)
{
	int charge_time, num_charging;

	charge_time = randcalc(obj->time, 0, AVERAGE);

	/* Item has no timeout */
	if (charge_time <= 0) return 0;

	/* No items are charging */
	if (obj->timeout <= 0) return 0;

	/* Calculate number charging based on timeout */
	num_charging = (obj->timeout + charge_time - 1) / charge_time;

	/* Number charging cannot exceed stack size */
	if (num_charging > obj->number) num_charging = obj->number;

	return num_charging;
}

/**
 * Allow a stack of charging objects to charge by one unit per charging object
 * Return true if something recharged
 */
bool recharge_timeout(struct object *obj)
{
	int charging_before, charging_after;

	/* Find the number of charging items */
	charging_before = charging_after = number_charging(obj);

	/* Nothing to charge */	
	if (charging_before == 0)
		return false;

	/* Decrease the timeout */
	if ((!tval_is_light(obj)) || (obj->timeout < randcalc(obj->kind->pval, 0, AVERAGE))) {
		if (obj->timeout > 0) {
			obj->timeout -= MIN(charging_before, obj->timeout);
			charging_after = number_charging(obj);
			if (obj->timeout <= 0) {
				obj->timeout = 0;
				if (tval_is_light(obj))
					light_timeout(obj);
				/* The object may no longer exist */
			}
		}
	}

	/* Return true if at least 1 item obtained a charge */
	if (charging_after < charging_before)
		return true;
	else
		return false;
}

/**
 * Verify the choice of an item.
 *
 * The item can be negative to mean "item on floor".
 */
bool verify_object(const char *prompt, const struct object *obj)
{
	char o_name[80];

	char out_val[160];

	/* Describe */
	object_desc(o_name, sizeof(o_name), obj, ODESC_PREFIX | ODESC_FULL);

	/* Prompt */
	strnfmt(out_val, sizeof(out_val), "%s %s? ", prompt, o_name);

	/* Query */
	return (get_check(out_val));
}


typedef enum {
	MSG_TAG_NONE,
	MSG_TAG_NAME,
	MSG_TAG_KIND,
	MSG_TAG_FLAVOR,
	MSG_TAG_VERB,
	MSG_TAG_VERB_IS
} msg_tag_t;

static msg_tag_t msg_tag_lookup(const char *tag)
{
	if (strncmp(tag, "name", 4) == 0) {
		return MSG_TAG_NAME;
	} else if (strncmp(tag, "kind", 4) == 0) {
		return MSG_TAG_KIND;
	} else if (strncmp(tag, "flavor", 6) == 0) {
		return MSG_TAG_FLAVOR;
	} else if (strncmp(tag, "s", 1) == 0) {
		return MSG_TAG_VERB;
	} else if (strncmp(tag, "is", 2) == 0) {
		return MSG_TAG_VERB_IS;
	} else {
		return MSG_TAG_NONE;
	}
}

/**
 * Print a message from a string, customised to include details about an object
 */
void print_custom_message(struct object *obj, const char *string, int msg_type)
{
	char buf[1024] = "\0";
	const char *next;
	const char *s;
	const char *tag;
	size_t end = 0;

	/* Not always a string */
	if (!string) return;

	next = strchr(string, '{');
	while (next) {
		/* Copy the text leading up to this { */
		strnfcat(buf, 1024, &end, "%.*s", next - string, string); 

		s = next + 1;
		while (*s && isalpha((unsigned char) *s)) s++;

		/* Valid tag */
		if (*s == '}') {
			/* Start the tag after the { */
			tag = next + 1;
			string = s + 1;

			switch(msg_tag_lookup(tag)) {
			case MSG_TAG_NAME:
				if (obj) {
					end += object_desc(buf, 1024, obj,
									   ODESC_PREFIX | ODESC_BASE);
				} else {
					strnfcat(buf, 1024, &end, "hands");
				}
				break;
			case MSG_TAG_FLAVOR:
				if (obj && (obj->kind->flavor) && (obj->kind->flavor->text) && (!object_flavor_is_aware(obj))) {
					obj_desc_name_format(&buf[end], sizeof(buf) - end, 0, obj->kind->flavor->text, NULL, false);
					end += strlen(&buf[end]);
					break;
				}
				/* FALL THROUGH */
			case MSG_TAG_KIND:
				if (obj) {
					object_kind_name(&buf[end], 1024 - end, obj->kind, true);
					end += strlen(&buf[end]);
				} else {
					strnfcat(buf, 1024, &end, "hands");
				}
				break;
			case MSG_TAG_VERB:
				if (obj && obj->number == 1) {
					strnfcat(buf, 1024, &end, "s");
				}
				break;
			case MSG_TAG_VERB_IS:
				if ((!obj) || (obj->number > 1)) {
					strnfcat(buf, 1024, &end, "are");
				} else {
					strnfcat(buf, 1024, &end, "is");
				}
			default:
				break;
			}
		} else
			/* An invalid tag, skip it */
			string = next + 1;

		next = strchr(string, '{');
	}
	strnfcat(buf, 1024, &end, string);

	msgt(msg_type, "%s", buf);
}