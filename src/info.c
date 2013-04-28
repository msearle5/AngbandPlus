/* File: info.c */

/*
 * Tables containing object kind descriptions.  Extended object descriptions
 * and information.  Self Knowledge.
 *
 * Copyright (c) 2002 Leon Marrick, Ben Harrison, James E. Wilson,
 * Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research,
 * and not for profit purposes provided that this copyright and statement
 * are included in all such copies.  Other copyrights may also apply.
 */

#include "angband.h"



/*
 * General information about classes of objects.
 *
 * Index is tval.
 */
cptr obj_class_info[101] =
{
	"",	"",	"",
	"Spikes are used to jam doors shut; a well-spiked door is difficult to open.",	"",
	"Components are used to make equipment:  weapons, missiles, and armour.",
	"Parchments are used to make scrolls.",
	"Bottles are used to make potions.",	"",	"",

	"Chests may have some really good treasures hidden inside, but can be perilous to open.  Bashing a locked chest damages whatever is inside, but also any traps that guard it.",	"",	"",	"",	"",
	"",	"Sling ammo.",	"Bow ammo.",	"Crossbow ammo.",	"Missile launchers allow you to inflict damage from a distance without using magic.",

	"Diggers, especially heavy ones, are invaluable for forced entry and escape and can make a lucky miner rich.",	"Hafted weapons rely on blunt force to inflict damage.  Since they spill relatively little blood, priests much prefer to carry one.  Although heavy, they can do a lot of damage.",	"Pole-mounted weapons are often cumbersome and may require two hands to wield, but some offer both a high degree of protection and powerful attacks.  Their base armour class increases with skill.",	"The effectiveness of edged weapons depends on keen edges and sharp points.  They tend to be quite light and are easy to use, but some may not deal enough damage for your liking.",	"",
	"",	"",	"",	"",	"",

	"Footwear protects the feet only, but some rare items of this type have magics to render them fleet, light, or steady.",	"Your hands would benefit from protection too, but many magic users need to keep their fingers unencumbered or magically supple.",	"Many a blow will be struck upon your head, and protection here will certainly be helpful.  Some rare items may protect and enhance your mind.",	"Many a blow will be struck upon your head, and protection here will certainly be helpful.  Some rare items may protect and enhance your mind.",	"Shields can be worn on your arm, or on your back if you need both hands to use your weapon.  So protective can a shield be that it can reduce damage as much or more than body armour, and you can perhaps deflect physical missiles (even shards) or take advantage of opportunities to bash your foe if you have one on your arm.",
	"Experienced adventurers wrap a cloak around their body.  Some rare items of this type may allow you to slip silently around less alert enemies.",	"Some kind of body protection will become a necessity as you face ever more dangerous opponents; rare items of this type may hold many and varied protective magics.",	"Some kind of body protection will become a necessity as you face ever more dangerous opponents; rare items of this type may hold many and varied protective magics.",	"Armour made of dragon scales is rare indeed, and powerful dragon magics allow you to sometimes breathe even as great wyrms do.",	"An adventurer who cannot see is jackal food.  Rare items of this type may have an extended lighting radius or require no fuel.",

	"Amulets slip around your neck, and almost all have magics wondrous or perilous bound inside.",	"",	"",	"",	"",
	"You may wear a ring upon each of your two ring fingers, and benefit or suffer from the magics it contains.",	"",	"",	"",	"",

	"",	"",	"",	"",	"",
	"Staffs are heavy, and take up plenty of space in your backpack, but can hold a lot of sometimes powerful spells that affect large areas.  Staffs are highly durable and recharge easily and well.",	"",	"",	"",	"",

	"",	"",	"",	"",	"",
	"Wands hold a variety of spells, useful both in combat and for exploration.  Bolt spells in wands often beam, and ball spells affect large areas.  Once its charges are exhausted, a wand is useless until recharged.",	"The magics stored in rods never run out, given enough time between uses to recover.  Rods can do a lot of damage, but they affect only small areas and cannot be recharged.  Bolt spells in rods seldom or never beam.",	"",	"",	"",

	"One will often find sheets of parchment with magic spells.  They are easy to read, and are a warrior's best chance at making use of magic.",	"",	"",	"",	"",
	"Healers, alchemists, and sages create potions in vast quantities, and store them in small flasks of clear glass or gleaming crystal.  Once quaffed, a potion is guaranteed to work, but not every strange liquid was mixed for your benefit...",	"",	"",	"",	"",

	"Deep in the murky dungeons strange mushrooms grow, and monsters rout about sealed packets of food that their owners will never need again.  \nSpecial note:  Mushrooms cannot be identified.  If you want to learn about them, you have to eat them...",	"",	"",	"",	"",

	"",	"",	"",	"",	"",

	"A manual of sorcerous magics, bound in fine linen dyed deep red.",	"A shining gospel of piety in flawless white and gold.",	"A runed stone with earthly and natural magics chiseled in brown and rich greens.",	"A black tome of necromantic rituals, with shadows lying deep around it.",	"",
	"",	"",	"",	"The concentrated essence of a kind of magic, element, or quality.  They form near objects, features, and monsters with strong attributes of a particular type.  They are used to make potions, scrolls, rings, and amulets, and to add special qualities to forged weapons and armour.",	"Pouches hold essences.",
	"Small valuables and coins."
};


/*
 * Extract and return extended information about an object, including
 * (sometimes) damage information for magical items, and (if appropriate)
 * ego and artifact lore.
 *
 * Code mostly from object_desc and roff_aux.
 */
void object_info(char *buf, object_type *o_ptr)
{
	object_kind *k_ptr = &k_info[o_ptr->k_idx];

	char *t;
	cptr s;
	cptr v;
	cptr w;
	cptr x;

	/* Assume no flavor string, no ego info, and no base info. */
	cptr modstr = "";
	cptr egoinfo = "";
	char baseinfo[1024];



	/* Artifacts have unique descriptions. */
	if (artifact_p(o_ptr))
	{
		artifact_type *a_ptr = &a_info[o_ptr->artifact_index];

#ifdef DELAY_LOAD_A_TEXT

		int fd;

		/* Build the filename */
		path_build(buf, sizeof(buf), ANGBAND_DIR_DATA, "artifact.raw");

		/* Open the "raw" file */
		fd = fd_open(buf, O_RDONLY);

		/* Use file */
		if (fd >= 0)
		{
			huge pos;

			/* Starting position */
			pos = a_ptr->text;

			/* Additional offsets */
			pos += a_head->head_size;
			pos += a_head->info_size;
			pos += a_head->name_size;

			/* Seek */
			fd_seek(fd, pos);

			/* Read a chunk of data */
			fd_read(fd, buf, sizeof(buf));

			/* Close it */
			fd_close(fd);
		}

#else
		/* If already in memory, simple to access */
		my_strcpy(buf, a_text + a_ptr->text, 2048);

#endif

		/* Return the description, if any. */
		return;
	}

	/* All non-artifact or random artifact objects. */
	else
	{

#ifdef DELAY_LOAD_K_TEXT

		int fd;

		/* Build the filename */
		path_build(buf, sizeof(buf), ANGBAND_DIR_DATA, "object.raw");

		/* Open the "raw" file */
		fd = fd_open(buf, O_RDONLY);

		/* Use file */
		if (fd >= 0)
		{
			huge pos;

			/* Starting position */
			pos = k_ptr->text;

			/* Additional offsets */
			pos += k_head->head_size;
			pos += k_head->info_size;
			pos += k_head->name_size;

			/* Seek */
			fd_seek(fd, pos);

			/* Read a chunk of data */
			fd_read(fd, buf, sizeof(buf));

			/* Close it */
			fd_close(fd);
		}

#else

		/* If already in memory, simple to access */
		strcpy(buf, k_text + k_ptr->text);

#endif

		/* No object description, so return failure. */
		if (!buf[0]) return;


		/* Various object types have different kinds of information. */
		switch (o_ptr->tval)
		{
			/* Dragon Scale Mails */
			case TV_DRAG_ARMOR:
			{
				/* Allow processing of activation information. */
				strcpy(baseinfo, format("%s", buf));
				break;
			}

			/* Amulets */
			case TV_AMULET:
			{
				strcpy(baseinfo, format("An amulet %s", buf));
				break;
			}

			/* Rings */
			case TV_RING:
			{
				strcpy(baseinfo, format("A ring %s", buf));
				break;
			}

			/* Staffs */
			case TV_STAFF:
			{
				strcpy(baseinfo, format("A staff %s", buf));
				break;
			}

			/* Wands */
			case TV_WAND:
			{
				strcpy(baseinfo, format("A wand %s", buf));
				break;
			}

			/* Rods */
			case TV_ROD:
			{
				strcpy(baseinfo, format("A rod %s", buf));
				break;
			}

			/* Scrolls */
			case TV_SCROLL:
			{
				strcpy(baseinfo, format("A parchment scroll %s", buf));
				break;
			}

			/* Potions */
			case TV_POTION:
			{
				strcpy(baseinfo, format("A potion %s", buf));
				break;
			}

			/* All other objects can just display the info text. */
			default:
			{
				/* Store the basic info text. */
				strcpy(baseinfo, format("%s", buf));
			}
		}


		/* Ego-object descriptions are added to any base description. */
		if ((o_ptr->ego_item_index) && (object_known_p(o_ptr)))
		{
			ego_item_type *e_ptr = &e_info[o_ptr->ego_item_index];
			char ebuf[1024];

			/* First, find the information in memory, or get it from
			 * the binary file.
			 */
#ifdef DELAY_LOAD_E_TEXT

			int fd;

			/* Build the filename */
			path_build(ebuf, sizeof(ebuf), ANGBAND_DIR_DATA, "ego_item.raw");

			/* Open the "raw" file */
			fd = fd_open(ebuf, O_RDONLY);

			/* Use file */
			if (fd >= 0)
			{
				huge pos;

				/* Starting position */
				pos = e_ptr->text;

				/* Additional offsets */
				pos += e_head->head_size;
				pos += e_head->info_size;
				pos += e_head->name_size;

				/* Seek */
				fd_seek(fd, pos);

				/* Read a chunk of data */
				fd_read(fd, ebuf, sizeof(ebuf));

				/* Close it */
				fd_close(fd);
			}

#else

			/* If already in memory, simple to access */
			strcpy(ebuf, e_text + e_ptr->text);

#endif

			/* Point to the ego-item information. */
			egoinfo = ebuf;
		}


		/* Point to "buf", and start dumping the result */
		t = buf;


		/*** Assemble the object information. ***/

		/* The object needs an article */
		if (baseinfo[0] == '&')
		{
			/* Skip ampersand and space. */
			s = baseinfo + 2;

			/* Flavor starts with a vowel */
			if (is_a_vowel(modstr[0])) w = "An ";

			/* Flavor starts with a non-vowel */
			else w = "A ";
		}
		else
		{
			w = "";

			/* Start at beginning of base info. */
			s = baseinfo;
		}

		/* Copy the base description, inserting info text. */
		for (; *s; s++)
		{
			/* Insert article */
			if (s != baseinfo)
			{
				for (; *w; w++) *t++ = *w;
			}

			/* Insert numerical info before closing period. */
			if ((*s == '.') && (*(s + 1) == '\0'))
			{
				/* Extra info if object is fully known. */
				if (o_ptr->ident & (IDENT_MENTAL) ||
				    k_ptr->special & (SPECIAL_KNOWN_EFFECT))
				{
					cptr moddata = "";
					bool dummy;

					/* Item is a magical device */
					if ((o_ptr->tval == TV_STAFF) ||
					    (o_ptr->tval == TV_WAND) ||
					    (o_ptr->tval == TV_ROD))
					{
						moddata = do_device(OBJECT_INFO, o_ptr,
							&dummy, &dummy, FALSE);
					}

					/* Item is some food or a mushroom, a potion, or a scroll */
					else if ((o_ptr->tval == TV_FOOD) ||
					         (o_ptr->tval == TV_POTION) ||
					         (o_ptr->tval == TV_SCROLL))
					{
						moddata = do_object(OBJECT_INFO, o_ptr);
					}

					/* If there is any numerical data,  */
					if (strlen(moddata) > 0)
					{
						/* ...insert a space, and */
						*t++ = ' ';

						/* insert the numerical data into the string. */
						for (v = moddata; *v; v++) *t++ = *v;
					}
				}

				/* Otherwise, no extra information. */
			}

			/* Copy over the string. */
			*t++ = *s;
		}

		/* Extra info for ego items. */
		if ((o_ptr->ego_item_index) && (object_known_p(o_ptr)))
		{
			cptr divider = "                                   ---";

			/* Insert a return, a divider, and another return. */
			*t++ = '\n';
			for (x = divider; *x; x++) *t++ = *x;
			*t++ = '\n';

			/* Copy the ego info to the information string. */
			for (x = egoinfo; *x; x++) *t++ = *x;
		}

		/* End the string. */
		*t = '\0';

		/* Return the string. */
		return;
	}
}

/*
 * Descriptions of pval-dependent qualities.
 */
static cptr pval_desc_text[32] =
{
	"strength",
	"intelligence",
	"wisdom",
	"dexterity",
	"constitution",
	"charisma",
	"XXX6",
	"XXX7",
	"stealth",
	"awareness",
	"infravision",
	"tunneling",
	"speed",
	"invisibility",
	"disarming",
	"device skill",
	"saving throw",
	"mana",
	"light radius",
	"XX19",
	"melee blows",
	"shooting speed",
	"missile weapon power",
	"XX23",
	"XX24",
	"XX25",
	"XX26",
	"XX27",
	"XX28",
	"XX29",
	"XX30",
	"XX31"
};


/*
 * Display most of what is known about any object.  Rewritten to
 * use "roff" and to harmonize with other description code.
 *
 * Fully known objects display all information, known objects display
 * everything except the non pval-dependant and random flags, and others
 * display only basic help text.
 */
void object_details(object_type *o_ptr, bool mental, bool known)
{
	int i, j, k, tmp1, tmp2;
	int y, x;

	int pval[32][2];

	u32b f1, f2, f3;

	int attr_listed = 0;
	int attr_num = 0;



	/* Object is not known -- jump straight to the basic information */
	if (!known) goto basic_info;



	/* Extract the flags */
	object_flags(o_ptr, &f1, &f2, &f3);


	/* Describe activation, if present. */
	if (o_ptr->activate)
	{
		object_kind *k_ptr = &k_info[o_ptr->k_idx];

		if ((mental) || (k_ptr->special |= (SPECIAL_KNOWN_EFFECT)))
		{
			cptr buf = do_activation_aux(OBJECT_INFO, o_ptr);

			roff("Activation:  ", 3, 77);
			roff(format("%s\n\n", buf), 3, 77);
		}
	}

	/* Describe light sources */
	if (get_object_pval(o_ptr, TR_PVAL_LIGHT) > 0)
	{
		int radius = get_object_pval(o_ptr, TR_PVAL_LIGHT);

		if ((o_ptr->tval != TV_LITE) || (f3 & (TR3_NOFUEL)))
		{
			roff(format("It provides light (radius %d) forever.\n",
				radius), 3, 77);
		}
		else
		{
			roff(format("It provides light (radius %d) when fueled.\n",
				radius), 3, 77);
		}
	}

	/* Magical devices can be damaged  XXX XXX */
	if (((o_ptr->tval == TV_STAFF) || (o_ptr->tval == TV_WAND)) &&
		  (o_ptr->ac < k_info[o_ptr->k_idx].ac))
	{
		roff("This object is damaged.\n", 3, 77);
	}


	/* Object has a pval of some kind */
	if (get_object_pval(o_ptr, 0L))
	{
		/* Get the values of a lot of pval-dependant qualities */
		pval[0][0]  = get_object_pval(o_ptr, TR_PVAL_STR);
		pval[1][0]  = get_object_pval(o_ptr, TR_PVAL_INT);
		pval[2][0]  = get_object_pval(o_ptr, TR_PVAL_WIS);
		pval[3][0]  = get_object_pval(o_ptr, TR_PVAL_DEX);
		pval[4][0]  = get_object_pval(o_ptr, TR_PVAL_CON);
		pval[5][0]  = get_object_pval(o_ptr, TR_PVAL_CHR);
		pval[6][0]  = 0;
		pval[7][0]  = 0;

		pval[8][0]  = get_object_pval(o_ptr, TR_PVAL_STEALTH);
		pval[9][0]  = get_object_pval(o_ptr, TR_PVAL_AWARE);
		pval[10][0] = get_object_pval(o_ptr, TR_PVAL_INFRA);
		pval[11][0] = get_object_pval(o_ptr, TR_PVAL_TUNNEL);
		pval[12][0] = get_object_pval(o_ptr, TR_PVAL_SPEED);
		pval[13][0] = get_object_pval(o_ptr, TR_PVAL_INVIS);

		/* Show exact values for new and unfamiliar pvals  XXX XXX */
		pval[14][0] = get_object_pval(o_ptr, TR_PVAL_DISARM) * 10;
		pval[15][0] = get_object_pval(o_ptr, TR_PVAL_DEVICE) *  8;
		pval[16][0] = get_object_pval(o_ptr, TR_PVAL_SAVE)   *  5;
		pval[17][0] = get_object_pval(o_ptr, TR_PVAL_MANA)   * 20;
		pval[18][0] = 0;  /* Do not show light radius here */
		pval[19][0] = 0;

		pval[20][0] = get_object_pval(o_ptr, TR_PVAL_BLOWS);
		pval[21][0] = get_object_pval(o_ptr, TR_PVAL_SHOTS);
		pval[22][0] = get_object_pval(o_ptr, TR_PVAL_MIGHT);
		pval[23][0] = 0;
		pval[24][0] = 0;
		pval[25][0] = 0;
		pval[26][0] = 0;
		pval[27][0] = 0;
		pval[28][0] = 0;
		pval[29][0] = 0;
		pval[30][0] = 0;
		pval[31][0] = 0;

		/* Special case:  all stats */
		if ((pval[0][0] != 0) &&
		    (pval[1][0] == pval[0][0]) && (pval[2][0] == pval[0][0]) &&
		    (pval[3][0] == pval[0][0]) && (pval[4][0] == pval[0][0]) &&
		    (pval[5][0] == pval[0][0]))
		{
			cptr desc = "increases";
			if (pval[0][0] < 0) desc = "decreases";

			roff(format("It %s all your stats by %d.\n", desc,
				ABS(pval[0][0])), 3, 77);

			/* Hack -- stats have been displayed */
			pval[0][0] = pval[1][0] = pval[2][0] = pval[3][0] =
			             pval[4][0] = pval[5][0] = 0;
		}

		/* Save the original indexes */
		for (i = 0; i < 32; i++)
		{
			pval[i][1] = i;
		}

		/* Sort all the pvals by value */
		for (i = 0; i < 32 - 1; i++)
		{
			for (j = 0; j < 32 - 1; j++)
			{
				/* Bubble sort */
				if (pval[j][0] < pval[j + 1][0])
				{
					tmp1 = pval[j][0];
					tmp2 = pval[j][1];
					pval[j][0] = pval[j + 1][0];
					pval[j][1] = pval[j + 1][1];
					pval[j + 1][0] = tmp1;
					pval[j + 1][1] = tmp2;
				}
			}
		}

		/* List all the pvals by value */
		for (k = 0; k < 32;)
		{
			/* Get pval */
			tmp1 = pval[k][0];

			/* Skip pvals of 0 */
			if (!tmp1)
			{
				k++;
				continue;
			}

			/* Figure out how many items use this pval */
			for (j = k + 1, attr_num = 1; j < 32; j++, attr_num++)
			{
				if (pval[j][0] != tmp1) break;
			}


			/* Start the description */
			if (tmp1 > 0) roff("It increases your", 3, 77);
			else          roff("It decreases your", 3, 77);

			/* List all the items with this pval */
			for (j = k, attr_listed = 0; (j < 32) && (attr_listed < attr_num);
			     j++)
			{
				/* Listing another attribute */
				attr_listed++;

				/* Commas separate members of a list of more than two. */
				if ((attr_num > 2) && (attr_listed > 1)) roff(",", 3, 77);

				/* "and" before final member of a list of more than one. */
				if ((attr_num > 1) && (j != k))
				{
					if (attr_num == attr_listed) roff(" and", 3, 77);
				}

				/* List -- use original flag index */
				roff(format(" %s", pval_desc_text[pval[j][1]]), 3, 77);
			}

			/* End this pval's description */
			roff(format(" by %d.\n", ABS(tmp1)), 3, 77);

			/* Advance to next set of pvals */
			k = j;
		}
	}




	/* Require full identification for most info */
	if (mental)
	{
		/* Sustain stats. */
		if ((f1 & (TR1_SUST_STR)) || (f1 & (TR1_SUST_INT)) ||
			 (f1 & (TR1_SUST_WIS)) || (f1 & (TR1_SUST_DEX)) ||
			 (f1 & (TR1_SUST_CON)) || (f1 & (TR1_SUST_CHR)))
		{
			/* Clear number of items to list, and items listed. */
			attr_num = 0;
			attr_listed = 0;

			/* How many attributes need to be listed? */
			if (f1 & (TR1_SUST_STR)) attr_num++;
			if (f1 & (TR1_SUST_INT)) attr_num++;
			if (f1 & (TR1_SUST_WIS)) attr_num++;
			if (f1 & (TR1_SUST_DEX)) attr_num++;
			if (f1 & (TR1_SUST_CON)) attr_num++;
			if (f1 & (TR1_SUST_CHR)) attr_num++;

			/* Special case:  sustain all stats */
			if (attr_num == 6)
			{
				roff("It sustains all your stats", 3, 77);
			}
			else
			{
				roff("It sustains your", 3, 77);

				/* Loop for number of attributes in this group. */
				for (j = 0; j < 6; j++)
				{
					bool list_ok = FALSE;

					if ((j == 0) && (f1 & (TR1_SUST_STR))) list_ok = TRUE;
					if ((j == 1) && (f1 & (TR1_SUST_INT))) list_ok = TRUE;
					if ((j == 2) && (f1 & (TR1_SUST_WIS))) list_ok = TRUE;
					if ((j == 3) && (f1 & (TR1_SUST_DEX))) list_ok = TRUE;
					if ((j == 4) && (f1 & (TR1_SUST_CON))) list_ok = TRUE;
					if ((j == 5) && (f1 & (TR1_SUST_CHR))) list_ok = TRUE;

					if (!list_ok) continue;

					/* Listing another attribute. */
					attr_listed++;

					/* Commas separate members of a list of more than two. */
					if ((attr_num > 2) && (attr_listed > 1)) roff(",", 3, 77);

					/* "and" before final member of a list of more than one. */
					if ((attr_num > 1) && (j != 0))
					{
						if (attr_num == attr_listed) roff(" and", 3, 77);
					}

					/* List the attribute description, in its proper place. */
					if (j == 0) roff(" strength", 3, 77);
					if (j == 1) roff(" intelligence", 3, 77);
					if (j == 2) roff(" wisdom", 3, 77);
					if (j == 3) roff(" dexterity", 3, 77);
					if (j == 4) roff(" constitution", 3, 77);
					if (j == 5) roff(" charisma", 3, 77);
				}
			}

			/* End sentence.  Go to next line. */
			roff(". \n", 3, 77);
		}


		/* Slays. */
		if ((f1 & (TR1_SLAY_ANIMAL)) || (f1 & (TR1_SLAY_EVIL)) ||
			 (f1 & (TR1_SLAY_UNDEAD)) || (f1 & (TR1_SLAY_DEMON)) ||
			 (f1 & (TR1_SLAY_ORC))    || (f1 & (TR1_SLAY_TROLL)) ||
			 (f1 & (TR1_SLAY_GIANT))  || (f1 & (TR1_SLAY_DRAGON)) ||
			 (f1 & (TR1_KILL_DRAGON)))
		{
			/* Clear number of items to list, and items listed. */
			attr_num = 0;
			attr_listed = 0;

			/* How many normal slays need to be listed? */
			if (f1 & (TR1_SLAY_ANIMAL)) attr_num++;
			if (f1 & (TR1_SLAY_EVIL))   attr_num++;
			if (f1 & (TR1_SLAY_UNDEAD)) attr_num++;
			if (f1 & (TR1_SLAY_DEMON))  attr_num++;
			if (f1 & (TR1_SLAY_ORC))    attr_num++;
			if (f1 & (TR1_SLAY_TROLL))  attr_num++;
			if (f1 & (TR1_SLAY_GIANT))  attr_num++;
			if (f1 & (TR1_SLAY_DRAGON)) attr_num++;

			/* Start the sentence */
			if (attr_num) roff("It slays", 3, 77);

			/* Loop for number of attributes in this group. */
			for (j = 0; j < 8; j++)
			{
				bool list_ok = FALSE;

				if ((j == 0) && (f1 & (TR1_SLAY_ANIMAL))) list_ok = TRUE;
				if ((j == 1) && (f1 & (TR1_SLAY_EVIL))) list_ok = TRUE;
				if ((j == 2) && (f1 & (TR1_SLAY_UNDEAD))) list_ok = TRUE;
				if ((j == 3) && (f1 & (TR1_SLAY_DEMON))) list_ok = TRUE;
				if ((j == 4) && (f1 & (TR1_SLAY_ORC))) list_ok = TRUE;
				if ((j == 5) && (f1 & (TR1_SLAY_TROLL))) list_ok = TRUE;
				if ((j == 6) && (f1 & (TR1_SLAY_GIANT))) list_ok = TRUE;
				if ((j == 7) && (f1 & (TR1_SLAY_DRAGON))) list_ok = TRUE;

				if (!list_ok) continue;

				/* Listing another attribute. */
				attr_listed++;

				/* Commas separate members of a list of more than two. */
				if ((attr_num > 2) && (attr_listed > 1)) roff(",", 3, 77);

				/* "and" before final member of a list of more than one. */
				if ((attr_num > 1) && (j != 0))
				{
					if (attr_num == attr_listed) roff(" and", 3, 77);
				}

				/* List the attribute description, in its proper place. */
				if (j == 0) roff(" animals", 3, 77);
				if (j == 1) roff(" evil", 3, 77);
				if (j == 2) roff(" undead", 3, 77);
				if (j == 3) roff(" demons", 3, 77);
				if (j == 4) roff(" orcs", 3, 77);
				if (j == 5) roff(" trolls", 3, 77);
				if (j == 6) roff(" giants", 3, 77);
				if (j == 7) roff(" dragons", 3, 77);
			}

			/* Special cases for the heavy slays */
			if (f1 & (TR1_KILL_DRAGON))
			{
				/* Conjunction */
				if (attr_num) roff(", and ", 3, 77);
				else          roff("It ", 3, 77);

				/* Text */
				roff("is the bane of dragons everywhere", 3, 77);
			}

			/* End sentence.  Go to next line. */
			roff(". \n", 3, 77);
		}


		/* Elemental and poison brands. */
		if ((f1 & (TR1_BRAND_ACID)) || (f1 & (TR1_BRAND_ELEC)) ||
			 (f1 & (TR1_BRAND_FIRE)) || (f1 & (TR1_BRAND_COLD)) ||
			 (f1 & (TR1_BRAND_POIS)) || (f1 & (TR1_BRAND_FLAME)) ||
			 (f1 & (TR1_BRAND_VENOM)))
		{
			/* Clear number of items to list, and items listed. */
			attr_num = 0;
			attr_listed = 0;

			/* How many normal brands need to be listed? */
			if (f1 & (TR1_BRAND_ACID)) attr_num++;
			if (f1 & (TR1_BRAND_ELEC)) attr_num++;
			if (f1 & (TR1_BRAND_FIRE)) attr_num++;
			if (f1 & (TR1_BRAND_COLD)) attr_num++;
			if (f1 & (TR1_BRAND_POIS)) attr_num++;

			/* Start the sentence */
			if (attr_num) roff("It", 3, 77);

			/* Loop for number of attributes in this group. */
			for (j = 0; j < 5; j++)
			{
				bool list_ok = FALSE;

				if ((j == 0) && (f1 & (TR1_BRAND_FIRE))) list_ok = TRUE;
				if ((j == 1) && (f1 & (TR1_BRAND_COLD))) list_ok = TRUE;
				if ((j == 2) && (f1 & (TR1_BRAND_ACID))) list_ok = TRUE;
				if ((j == 3) && (f1 & (TR1_BRAND_ELEC))) list_ok = TRUE;
				if ((j == 4) && (f1 & (TR1_BRAND_POIS))) list_ok = TRUE;

				if (!list_ok) continue;

				/* Listing another attribute. */
				attr_listed++;

				/* Commas separate members of a list of more than two. */
				if ((attr_num > 2) && (attr_listed > 1)) roff(",", 3, 77);

				/* "and" before final member of a list of more than one. */
				if ((attr_num > 1) && (j != 0))
				{
					if (attr_num == attr_listed) roff(" and", 3, 77);
				}

				/* List the attribute description, in its proper place. */
				if (j == 0) roff(" burns", 3, 77);
				if (j == 1) roff(" freezes", 3, 77);
				if (j == 2) roff(" melts", 3, 77);
				if (j == 3) roff(" electrocutes", 3, 77);
				if (j == 4) roff(" poisons", 3, 77);
			}

			/* End the sentence */
			if (attr_num) roff(" your foes", 3, 77);

			/* Special cases for the heavy brands */
			if ((f1 & (TR1_BRAND_FLAME)) || (f1 & (TR1_BRAND_VENOM)))
			{
				/* Conjunction */
				if (attr_num) roff(", and ", 3, 77);
				else          roff("It ", 3, 77);

				/* Text */
				if ((f1 & (TR1_BRAND_FLAME)) && (f1 & (TR1_BRAND_VENOM)))
					roff("incinerates and injects poison into everything it touches", 3, 77);
				else if (f1 & (TR1_BRAND_FLAME))
					roff("incinerates everything it touches", 3, 77);
				else if (f1 & (TR1_BRAND_VENOM))
					roff("injects deadly poisons into everything it touches", 3, 77);
			}

			/* End sentence.  Go to next line. */
			roff(". \n", 3, 77);
		}

		/* Vorpal weapons and missile launchers. */
		if (f1 & (TR1_VORPAL))
		{
			if (o_ptr->tval == TV_BOW)
			{
				roff("The missiles this weapon shoots drive deeply into their targets. \n", 3, 77);
			}
			else if (o_ptr->tval == TV_HAFTED)
			{
				roff("It is a weapon of concussion. \n", 3, 77);
			}
			else if (!is_missile(o_ptr))
			{
				roff("It is a vorpal blade. \n", 3, 77);
			}
			else
			{
				roff("It drives deeply into your foes. \n", 3, 77);
			}
		}

		/* Must be a melee weapon */
		if (is_melee_weapon(o_ptr))
		{
			/* Throwing weapons. */
			if (f1 & (TR1_THROWING))
			{
				if (f1 & (TR1_PERFECT_BALANCE))
				{
					roff("It can be thrown hard and fast. \n", 3, 77);
				}
				roff("It can be thrown effectively. \n", 3, 77);
			}

			/* Two-handed weapons. */
			if (f1 & (TR1_TWO_HANDED_REQ))
			{
				roff("It requires two hands to wield. \n", 3, 77);
			}
			else if (f1 & (TR1_TWO_HANDED_DES))
			{
				roff("Only the strongest can wield this weapon in one hand. \n", 3, 77);
			}
		}


		/* Elemental immunities. */
		if ((f2 & (TR2_IM_ACID)) || (f2 & (TR2_IM_ELEC)) ||
			 (f2 & (TR2_IM_FIRE)) || (f2 & (TR2_IM_COLD)))
		{
			/* Clear number of items to list, and items listed. */
			attr_num = 0;
			attr_listed = 0;

			/* How many attributes need to be listed? */
			if (f2 & (TR2_IM_ACID)) attr_num++;
			if (f2 & (TR2_IM_ELEC)) attr_num++;
			if (f2 & (TR2_IM_FIRE)) attr_num++;
			if (f2 & (TR2_IM_COLD)) attr_num++;

			roff("It provides immunity to", 3, 77);

			/* Loop for number of attributes in this group. */
			for (j = 0; j < 4; j++)
			{
				bool list_ok = FALSE;

				if ((j == 0) && (f2 & (TR2_IM_ACID))) list_ok = TRUE;
				if ((j == 1) && (f2 & (TR2_IM_ELEC))) list_ok = TRUE;
				if ((j == 2) && (f2 & (TR2_IM_FIRE))) list_ok = TRUE;
				if ((j == 3) && (f2 & (TR2_IM_COLD))) list_ok = TRUE;

				if (!list_ok) continue;

				/* Listing another attribute. */
				attr_listed++;

				/* Commas separate members of a list of more than two. */
				if ((attr_num > 2) && (attr_listed > 1)) roff(",", 3, 77);

				/* "and" before final member of a list of more than one. */
				if ((attr_num > 1) && (j != 0))
				{
					if (attr_num == attr_listed) roff(" and", 3, 77);
				}

				/* List the attribute description, in its proper place. */
				if (j == 0) roff(" acid", 3, 77);
				if (j == 1) roff(" electricity", 3, 77);
				if (j == 2) roff(" fire", 3, 77);
				if (j == 3) roff(" frost", 3, 77);
			}

			/* End sentence.  Go to next line. */
			roff(". \n", 3, 77);
		}


		/* Resistances. */
		if ((f2 & (TR2_RES_ACID))  || (f2 & (TR2_RES_ELEC)) ||
			 (f2 & (TR2_RES_FIRE))  || (f2 & (TR2_RES_COLD)) ||
			 (f2 & (TR2_RES_POIS))  || (f2 & (TR2_RES_LITE)) ||
			 (f2 & (TR2_RES_DARK))  || (f2 & (TR2_RES_SOUND)) ||
			 (f2 & (TR2_RES_SHARD)) || (f2 & (TR2_RES_NEXUS)) ||
			 (f2 & (TR2_RES_NETHR)) || (f2 & (TR2_RES_CHAOS)) ||
			 (f2 & (TR2_RES_DISEN)))
		{
			/* Clear number of items to list, and items listed. */
			attr_num = 0;
			attr_listed = 0;

			/* How many attributes need to be listed? */
			if (f2 & (TR2_RES_ACID))  attr_num++;
			if (f2 & (TR2_RES_ELEC))  attr_num++;
			if (f2 & (TR2_RES_FIRE))  attr_num++;
			if (f2 & (TR2_RES_COLD))  attr_num++;
			if (f2 & (TR2_RES_POIS))  attr_num++;
			if (f2 & (TR2_RES_LITE))  attr_num++;
			if (f2 & (TR2_RES_DARK))  attr_num++;
			if (f2 & (TR2_RES_SOUND)) attr_num++;
			if (f2 & (TR2_RES_SHARD)) attr_num++;
			if (f2 & (TR2_RES_NEXUS)) attr_num++;
			if (f2 & (TR2_RES_NETHR)) attr_num++;
			if (f2 & (TR2_RES_CHAOS)) attr_num++;
			if (f2 & (TR2_RES_DISEN)) attr_num++;

			roff("It provides resistance to", 3, 77);

			/* Loop for number of attributes in this group. */
			for (j = 0; j < 13; j++)
			{
				bool list_ok = FALSE;

				if ((j ==  0) && (f2 & (TR2_RES_ACID)))  list_ok = TRUE;
				if ((j ==  1) && (f2 & (TR2_RES_ELEC)))  list_ok = TRUE;
				if ((j ==  2) && (f2 & (TR2_RES_FIRE)))  list_ok = TRUE;
				if ((j ==  3) && (f2 & (TR2_RES_COLD)))  list_ok = TRUE;
				if ((j ==  4) && (f2 & (TR2_RES_POIS)))  list_ok = TRUE;
				if ((j ==  5) && (f2 & (TR2_RES_LITE)))  list_ok = TRUE;
				if ((j ==  6) && (f2 & (TR2_RES_DARK)))  list_ok = TRUE;
				if ((j ==  7) && (f2 & (TR2_RES_SOUND))) list_ok = TRUE;
				if ((j ==  8) && (f2 & (TR2_RES_SHARD))) list_ok = TRUE;
				if ((j ==  9) && (f2 & (TR2_RES_NEXUS))) list_ok = TRUE;
				if ((j == 10) && (f2 & (TR2_RES_NETHR))) list_ok = TRUE;
				if ((j == 11) && (f2 & (TR2_RES_CHAOS))) list_ok = TRUE;
				if ((j == 12) && (f2 & (TR2_RES_DISEN))) list_ok = TRUE;

				if (!list_ok) continue;

				/* Listing another attribute. */
				attr_listed++;

				/* Commas separate members of a list of more than two. */
				if ((attr_num > 2) && (attr_listed > 1)) roff(",", 3, 77);

				/* "and" before final member of a list of more than one. */
				if ((attr_num > 1) && (j != 0))
				{
					if (attr_num == attr_listed) roff(" and", 3, 77);
				}

				/* List the attribute description, in its proper place. */
				if (j ==  0) roff(" acid", 3, 77);
				if (j ==  1) roff(" electricity", 3, 77);
				if (j ==  2) roff(" fire", 3, 77);
				if (j ==  3) roff(" frost", 3, 77);
				if (j ==  4) roff(" poison", 3, 77);
				if (j ==  5) roff(" light", 3, 77);
				if (j ==  6) roff(" darkness", 3, 77);
				if (j ==  7) roff(" sound", 3, 77);
				if (j ==  8) roff(" shards", 3, 77);
				if (j ==  9) roff(" nexus", 3, 77);
				if (j == 10) roff(" nether", 3, 77);
				if (j == 11) roff(" chaos", 3, 77);
				if (j == 12) roff(" disenchantment", 3, 77);
			}

			/* End sentence.  Go to next line. */
			roff(". \n", 3, 77);
		}


		/* Clear a listing variable. */
		attr_num = 0;

		/* Special processing for the three "survival resists" */
		if (f2 & (TR2_RES_FEAR)) attr_num++;
		if (f2 & (TR2_RES_BLIND)) attr_num++;
		if (f2 & (TR2_RES_CONFU)) attr_num++;

		if (f2 & (TR2_RES_FEAR))
		{
			roff("It renders you fearless", 3, 77);
			if (attr_num == 1) roff(". \n", 3, 77);
			else roff(", and", 3, 77);
		}

		if (f2 & (TR2_RES_BLIND))
		{
			if ((attr_num > 1) && (f2 & (TR2_RES_FEAR)))
				roff(" provides resistance to blindness", 3, 77);
			else roff("It provides resistance to blindness", 3, 77);

			if (f2 & (TR2_RES_CONFU)) roff(" and", 3, 77);
			else roff(". \n", 3, 77);
		}

		if (f2 & (TR2_RES_CONFU))
		{
			if ((attr_num > 1) && (!(f2 & (TR2_RES_BLIND))))
				roff(" provides resistance to confusion.\n", 3, 77);
			else if (attr_num > 1) roff(" confusion.\n", 3, 77);
			else roff("It provides resistance to confusion.\n", 3, 77);
		}


		/* Miscellaneous abilities. */
		if ((f3 & (TR3_SLOW_DIGEST)) || (f3 & (TR3_FEATHER)) ||
			 (f3 & (TR3_LITE))        || (f3 & (TR3_REGEN)) ||
			 (f3 & (TR3_TELEPATHY))   || (f3 & (TR3_SEE_INVIS)) ||
			 (f3 & (TR3_FREE_ACT))    || (f3 & (TR3_HOLD_LIFE)) ||
			 (f3 & (TR3_IMPACT))      || (f3 & (TR3_BLESSED)))
		{
			/* Clear number of items to list, and items listed. */
			attr_num = 0;
			attr_listed = 0;

			/* How many attributes need to be listed? */
			if (f3 & (TR3_SLOW_DIGEST)) attr_num++;
			if (f3 & (TR3_FEATHER))     attr_num++;
			if (f3 & (TR3_LITE))        attr_num++;
			if (f3 & (TR3_REGEN))       attr_num++;
			if (f3 & (TR3_TELEPATHY))   attr_num++;
			if (f3 & (TR3_SEE_INVIS))   attr_num++;
			if (f3 & (TR3_FREE_ACT))    attr_num++;
			if (f3 & (TR3_HOLD_LIFE))   attr_num++;
			if (f3 & (TR3_IMPACT))      attr_num++;
			if (f3 & (TR3_BLESSED))     attr_num++;

			roff("It", 3, 77);

			/* Loop for number of attributes in this group. */
			for (j = 0; j < 10; j++)
			{
				bool list_ok = FALSE;

				if ((j == 0) && (f3 & (TR3_SLOW_DIGEST))) list_ok = TRUE;
				if ((j == 1) && (f3 & (TR3_FEATHER)))     list_ok = TRUE;
				if ((j == 2) && (f3 & (TR3_LITE)))        list_ok = TRUE;
				if ((j == 3) && (f3 & (TR3_REGEN)))       list_ok = TRUE;
				if ((j == 4) && (f3 & (TR3_TELEPATHY)))   list_ok = TRUE;
				if ((j == 5) && (f3 & (TR3_SEE_INVIS)))   list_ok = TRUE;
				if ((j == 6) && (f3 & (TR3_FREE_ACT)))    list_ok = TRUE;
				if ((j == 7) && (f3 & (TR3_HOLD_LIFE)))   list_ok = TRUE;
				if ((j == 8) && (f3 & (TR3_IMPACT)))      list_ok = TRUE;
				if ((j == 9) && (f3 & (TR3_BLESSED)))     list_ok = TRUE;

				if (!list_ok) continue;

				/* Listing another attribute. */
				attr_listed++;

				/* Commas separate members of a list of more than two. */
				if ((attr_num > 2) && (attr_listed > 1)) roff(",", 3, 77);

				/* "and" before final member of a list of more than one. */
				if ((attr_num > 1) && (j != 0))
				{
					if (attr_num == attr_listed) roff(" and", 3, 77);
				}

				/* List the attribute description, in its proper place. */
				if (j == 0) roff(" slows your metabolism", 3, 77);
				if (j == 1) roff(" grants feather falling", 3, 77);
				if (j == 2) roff(" provides permanent light", 3, 77);
				if (j == 3) roff(" speeds your regenerative powers", 3, 77);
				if (j == 4) roff(" gives telepathic powers", 3, 77);
				if (j == 5) roff(" allows you to see invisible monsters", 3, 77);
				if (j == 6) roff(" provides immunity to paralysis", 3, 77);
				if (j == 7) roff(" provides resistance to life draining", 3, 77);
				if (j == 8) roff(" is an impact weapon", 3, 77);
				if (j == 9) roff(" has been blessed by the gods", 3, 77);
			}

			/* End sentence.  Go to next line. */
			roff(". \n", 3, 77);
		}

		/* Nastiness. */
		if ((f3 & (TR3_SOULSTEAL)) || (f3 & (TR3_NOMAGIC)) ||
			 (f3 & (TR3_TELEPORT))  || (f3 & (TR3_AGGRAVATE)) ||
			 (f3 & (TR3_DRAIN_EXP)) || (f3 & (TR3_DRAIN_HP)) ||
			 (cursed_p(o_ptr)))
		{
			/* Clear number of items to list, and items listed. */
			attr_num = 0;
			attr_listed = 0;

			/* How many attributes need to be listed? */
			if (f3 & (TR3_SOULSTEAL)) attr_num++;
			if (f3 & (TR3_NOMAGIC))   attr_num++;
			if (f3 & (TR3_TELEPORT))  attr_num++;
			if (f3 & (TR3_AGGRAVATE)) attr_num++;
			if (f3 & (TR3_DRAIN_EXP)) attr_num++;
			if (f3 & (TR3_DRAIN_HP)) attr_num++;

			/* This one will display one of three possible descriptions. */
			if (cursed_p(o_ptr)) attr_num++;

			roff("It", 3, 77);

			/* Loop for number of attributes in this group. */
			for (j = 0; j < 7; j++)
			{
				bool list_ok = FALSE;

				if ((j == 0) && (f3 & (TR3_SOULSTEAL)))   list_ok = TRUE;
				if ((j == 1) && (f3 & (TR3_NOMAGIC)))     list_ok = TRUE;
				if ((j == 2) && (f3 & (TR3_TELEPORT)))    list_ok = TRUE;
				if ((j == 3) && (f3 & (TR3_AGGRAVATE)))   list_ok = TRUE;
				if ((j == 4) && (f3 & (TR3_DRAIN_EXP)))   list_ok = TRUE;
				if ((j == 5) && (f3 & (TR3_DRAIN_HP)))    list_ok = TRUE;
				if ((j == 6) && (cursed_p(o_ptr)))        list_ok = TRUE;

				if (!list_ok) continue;

				/* Listing another attribute. */
				attr_listed++;

				/* Commas separate members of a list of more than two. */
				if ((attr_num > 2) && (attr_listed > 1)) roff(",", 3, 77);

				/* "and" before final member of a list of more than one. */
				if ((attr_num > 1) && (j != 0))
				{
					if (attr_num == attr_listed) roff(" and", 3, 77);
				}

				/* List the attribute description, in its proper place. */
				if (j == 0) roff(" must be fed with blood", 3, 77);
				if (j == 1) roff(" prevents you from casting spells", 3, 77);
				if (j == 2) roff(" induces random teleportation", 3, 77);
				if (j == 3) roff(" aggravates nearby creatures", 3, 77);
				if (j == 4) roff(" drains experience", 3, 77);
				if (j == 5) roff(" drains hitpoints", 3, 77);
				if (j == 6)
				{
					if (f3 & (TR3_PERMA_CURSE))
						roff(" can never be taken off once put on", 3, 77);
					else if (f3 & (TR3_HEAVY_CURSE))
						roff(" is powerfully cursed", 3, 77);
					else
						roff(" is cursed", 3, 77);
				}
			}

			/* End sentence.  Go to next line. */
			roff(". \n", 3, 77);
		}


		/* Ignore various elements. */
		if ((f2 & (TR2_IGNORE_ACID)) || (f2 & (TR2_IGNORE_ELEC)) ||
			(f2 & (TR2_IGNORE_FIRE)) || (f2 & (TR2_IGNORE_COLD)))
		{
			/* Clear number of items to list, and items listed. */
			attr_num = 0;
			attr_listed = 0;

			/* How many attributes need to be listed? */
			if (f2 & (TR2_IGNORE_ACID)) attr_num++;
			if (f2 & (TR2_IGNORE_ELEC)) attr_num++;
			if (f2 & (TR2_IGNORE_FIRE)) attr_num++;
			if (f2 & (TR2_IGNORE_COLD)) attr_num++;

			roff("It cannot be damaged by", 3, 77);

			/* Loop for number of attributes in this group. */
			for (j = 0; j < 4; j++)
			{
				bool list_ok = FALSE;

				if ((j == 0) && (f2 & (TR2_IGNORE_ACID))) list_ok = TRUE;
				if ((j == 1) && (f2 & (TR2_IGNORE_ELEC))) list_ok = TRUE;
				if ((j == 2) && (f2 & (TR2_IGNORE_FIRE))) list_ok = TRUE;
				if ((j == 3) && (f2 & (TR2_IGNORE_COLD))) list_ok = TRUE;

				if (!list_ok) continue;

				/* Listing another attribute. */
				attr_listed++;

				/* Commas separate members of a list of more than two. */
				if ((attr_num > 2) && (attr_listed > 1)) roff(",", 3, 77);

				/* "or" before final member of a list of more than one. */
				if ((attr_num > 1) && (j != 0))
				{
					if (attr_num == attr_listed) roff(" or", 3, 77);
				}

				/* List the attribute description, in its proper place. */
				if (j == 0) roff(" acid", 3, 77);
				if (j == 1) roff(" electricity", 3, 77);
				if (j == 2) roff(" fire", 3, 77);
				if (j == 3) roff(" frost", 3, 77);
			}

			/* End sentence.  Go to next line. */
			roff(". \n", 3, 77);
		}
	}


	/* All objects should display certain basic help text */
	basic_info:


	/* Get cursor location */
	(void)Term_locate(&x, &y);

	/* We do not have enough screen space left  XXX XXX */
	if (y > Term->hgt - 4) return;


	/* Note durability */
	if ((o_ptr->ac > 0) &&
		 ((o_ptr->tval == TV_POTION) ||
	     (o_ptr->tval == TV_BOTTLE) ||
	     (o_ptr->tval == TV_SCROLL) ||
	     (o_ptr->tval == TV_PARCHMENT) ||
	     (o_ptr->tval == TV_SHOT) ||
	     (o_ptr->tval == TV_ARROW) ||
	     (o_ptr->tval == TV_BOLT) ||
	     (o_ptr->tval == TV_STAFF) ||
	     (o_ptr->tval == TV_WAND)))
	{
		roff("This item is unusually durable.\n", 3, 77);
	}


	/* Explain skills and other "gotchas" */
	if (is_melee_weapon(o_ptr))
	{
		roff("\n", 3, 77);
		if (k_info[o_ptr->k_idx].flags1 & (TR1_THROWING))
		{
			roff("This specialized ranged weapon is unsuitable for melee combat.  When thrown, your Throwing skill determines the damage it does.", 3, 77);
		}
		else
		{
			if (o_ptr->tval == TV_SWORD)
			{
				roff("Your Swordsmanship skill affects how well you use this weapon in melee", 3, 77);
			}
			else if (o_ptr->tval == TV_HAFTED)
			{
				roff("Your Clubbing skill affects how well you use this weapon in melee", 3, 77);
			}
			else
			{
				roff("Your Jousting skill affects how well you use this weapon in melee", 3, 77);
			}
			if (o_ptr->flags1 & (TR1_THROWING))
			{
				roff(", but if you throw it, your Throwing skill is what matters", 3, 77);
			}
			roff(".\n", 3, 77);
		}
	}
}


/*
 * Textual descriptions for positive and negative modifiers
 */
static cptr modifier_blurb[32][2] =
{
	{ "strong",                       "weak"                           },
	{ "intelligent",                  "unintelligent"                  },
	{ "wise",                         "naive"                          },
	{ "dexterous",                     "clumsy"                        },
	{ "resilient",                    "fragile"                        },
	{ "charismatic",                  "uncharismatic"                  },
	{ "XXX6",                         "XXX6"                           },
	{ "XXX7",                         "XXX7"                           },
	{ "stealthy",                     "easy to spot"                   },
	{ "perceptive",                   "unperceptive"                   },
	{ "infrared-aware",               "infrared-blind"                 },
	{ "good at tunneling",            "poor at tunneling"              },
	{ "fast",                         "slow"                           },
	{ "invisible",                    "hard to make invisible"         },
	{ "skilled at disarming",         "unskilled at disarming"         },
	{ "skilled with magical devices", "unskilled with magical devices" },
	{ "resistant to magical effects", "susceptible to magical effects" },
	{ "capable of gathering mana",    "poor at gathering mana"         },
	{ "shining",                      "darkened"                       },
	{ "XX19",                         "XX19"                           },
	{ "rapid in melee",               "slow in melee"                  },
	{ "quick-firing",                 "slow with missile weapons"      },
	{ "deadly in missile combat",     "weak in missile combat"         },
	{ "XX23",                         "XX23"                           },
	{ "XX24",                         "XX24"                           },
	{ "XX25",                         "XX25"                           },
	{ "XX26",                         "XX26"                           },
	{ "XX27",                         "XX27"                           },
	{ "XX28",                         "XX28"                           },
	{ "XX29",                         "XX29"                           },
	{ "XX30",                         "XX30"                           },
	{ "XX31",                         "XX31"                           }
};

/*
 * Hack -- move the cursor to either column 0 or 40 so that certain
 * character attributes can appear in two columns.
 */
static byte sk_get_col(void)
{
	int y, x;

	/* Obtain the cursor */
	(void)Term_locate(&x, &y);

	/* Determine which column we should enter text in */
	if (x >= 40)
	{
		/* Go to next line, first column */
		y++;
		x = 0;
	}
	else if (x == 0)
	{
		/* Don't move */
	}
	else
	{
		/* Go to this line, second column */
		x = 40;
	}

	/* Move the cursor */
	move_cursor(y, x);

	/* Return the correct starting position */
	return ((byte)x);
}


/*
 * Hack -- acquire self knowledge.  Idea originally from Nethack.
 *
 * List various information about the player.
 *
 * See also "object_details()".
 */
void self_knowledge(bool full)
{
	int i, j, k;
	int weapons = 0;

	int attr_num = 0;
	int attr_listed = 0;

	u32b f[4];

	char buf[80];

	cptr title;
	bool quotes = FALSE;

	int old_rows = screen_rows;

	u32b flag;

	int pval_equip[32];
	int pval_intrinsic[32];

	int pval_pos_equip = 0;
	int pval_neg_equip = 0;

	int pval_pos_intrinsic = 0;
	int pval_neg_intrinsic = 0;



	object_type *o_ptr;


	/* Get intrinsic pval adjustments */
	for (i = 0; i < 32; i++)
	{
		/* Get flag */
		flag = 1L << i;

		/* Save the pval */
		pval_intrinsic[i] = player_flags_pval(flag, TRUE);

		/* Count positive and negative changes */
		if (pval_intrinsic[i] > 0) pval_pos_intrinsic++;
		if (pval_intrinsic[i] < 0) pval_neg_intrinsic++;
	}

	/* Set all equipment pval adjustments to zero */
	for (i = 0; i < 32; i++) pval_equip[i] = 0;

	/* Get equipment pval adjustments */
	for (i = INVEN_WIELD; i < INVEN_SUBTOTAL; i++)
	{
		o_ptr = &inventory[i];

		/* Skip non-objects */
		if (!o_ptr->k_idx) continue;

		/* Get the pval-dependant attributes */
		for (j = 0; j < 32; j++)
		{
			/* Get flag */
			flag = 1L << j;

			/* Save the pval */
			pval_equip[j] += get_object_pval(o_ptr, flag);
		}
	}

	/* Count positive and negative changes from equipment */
	for (i = 0; i < 32; i++)
	{
		if (pval_equip[i] > 0) pval_pos_equip++;
		if (pval_equip[i] < 0) pval_neg_equip++;
	}



	/* Save screen */
	screen_save();

	/* Clear everything */
	clear_from(0);

	/* Set to 50 screen rows */
	Term_rows(TRUE);

	/* Move cursor to top-left corner of screen */
	move_cursor(0, 0);

	/* Get title */
	title = get_title(80, FALSE);

	/* Note that title is meant to be in quotes, skip past marker */
	if (title[0] == '#')
	{
		quotes = TRUE;
		title++;
	}

	/* Build a name and title string */
	sprintf(buf, "%s, %s%s%c",
			  (!op_ptr->full_name ? "Anonymous" : op_ptr->full_name),
			  (quotes ? "\"" : "the "), title,
			  (quotes ? '\"' : '\0'));

	/* Display character name and title */
	c_roff_centered(TERM_VIOLET, buf, 0, 0);

	/* Underline it */
	c_roff_centered(TERM_WHITE,
		"----------------------------------\n\n", 0, 0);

	/* Set cursor to start of line 3 */
	move_cursor(3, 0);

	/* Print header */
	c_roff_centered(TERM_L_BLUE, "Character Attributes\n", 0, 0);

	/* Print shapechange */
	if (p_ptr->schange)
	{
		cptr p = "You have shapechanged into ";

		switch (p_ptr->schange)
		{
			case SHAPE_GOAT:    roff(format("%sa goat.\n",    p), 0, 0);
				break;
			case SHAPE_BEAR:    roff(format("%sa bear.\n",    p), 0, 0);
				break;
			case SHAPE_MOUSE:   roff(format("%sa mouse.\n",   p), 0, 0);
				break;
			case SHAPE_HOUND:   roff(format("%sa hound.\n",   p), 0, 0);
				break;
			case SHAPE_CHEETAH: roff(format("%sa cheetah.\n", p), 0, 0);
				break;
			case SHAPE_LION:    roff(format("%sa lion.\n",    p), 0, 0);
				break;
			case SHAPE_DRAGON:  roff(format("%sa dragon.\n",  p), 0, 0);
				break;
			case SHAPE_ENT:     roff(format("%san ent.\n",    p), 0, 0);
				break;
			case SHAPE_TROLL:   roff(format("%sa troll.\n",   p), 0, 0);
				break;
			case SHAPE_BAT:     roff(format("%sa bat.\n",     p), 0, 0);
				break;
			default:  roff(format("%san unknown creature.\n", p), 0, 0);
				break;
		}
	}


	/*** Display permanent or semi-permanent character attributes ***/
	if (p_ptr->slow_digest)
	{
		roff("Your appetite is small.", sk_get_col(), 0);
	}
	if (p_ptr->ffall)
	{
		roff("You land gently.", sk_get_col(), 0);
	}
	if (p_ptr->lite)
	{
		roff("You are glowing with light.", sk_get_col(), 0);
	}
	if (p_ptr->regenerate)
	{
		roff("You regenerate quickly.", sk_get_col(), 0);
	}
	if (p_ptr->telepathy)
	{
		roff("You sense the minds of your enemies.", sk_get_col(), 0);
	}
	if ((p_ptr->see_inv) || (p_ptr->detect_inv))
	{
		roff("You can see invisible creatures.", sk_get_col(), 0);
	}
	if (p_ptr->esp_evil)
	{
		roff("You are aware of evil creatures.", sk_get_col(), 0);
	}
	if (p_ptr->free_act)
	{
		roff("You have free action.", sk_get_col(), 0);
	}
	if (p_ptr->hold_life)
	{
		roff("You have a firm hold on your lifeforce.", sk_get_col(), 0);
	}
	if (p_ptr->invisible)
	{
		roff("You are partially invisible.", sk_get_col(), 0);
	}
	if (p_ptr->wraithform)
	{
		roff("You can pass through granite walls.", sk_get_col(), 0);
	}
	if (p_ptr->resist_mana_drain)
	{
		roff("You resist mana draining attacks.", sk_get_col(), 0);
	}
	if (p_ptr->aggravate)
	{
		roff("You aggravate monsters.", sk_get_col(), 0);
	}
	if (p_ptr->teleport)
	{
		roff("Your position is very uncertain.", sk_get_col(), 0);
	}
	if (p_ptr->drain_light)
	{
		roff("You are draining away light.", sk_get_col(), 0);
	}
	if (p_ptr->nomagic)
	{
		roff("You are prevented from casting spells.", sk_get_col(), 0);
	}
	if (p_ptr->drain_exp)
	{
		roff("Your equipment drains experience.", sk_get_col(), 0);
	}
	if (p_ptr->black_breath)
	{
		c_roff_centered(TERM_RED, "\nYour lifeforce is being sucked away by the black breath.", 0, 0);
	}



	/*** Display intrinsic and equipment modifiers ***/

	/* Print header */
	roff("\n\n", 0, 0);
	c_roff_centered(TERM_L_BLUE, "Intrinsic and Equipment Modifiers\n", 0, 0);

	/* Display positive intrinsic modifiers */
	if (pval_pos_intrinsic)
	{
		roff("You are intrinsically", 0, 0);

		for (i = 0, j = 0; i < 32; i++)
		{
			/* All done */
			if (j == pval_pos_intrinsic) break;

			/* List another modifier */
			if (pval_intrinsic[i] > 0)
			{
				/* Display this modifier */
				roff(format(" %s", modifier_blurb[i][0]), 0, 0);

				/* Another modifier is being listed */
				j++;

				/* Commas between members of a list of more than two */
				if ((pval_pos_intrinsic > 2) && (j < pval_pos_intrinsic))
					roff(",", 0, 0);

				/* "and" before last member of a list of more than one */
				if (j == pval_pos_intrinsic - 1) roff(" and", 0, 0);
			}
		}
	}

	/* Display negative intrinsic modifiers */
	if (pval_neg_intrinsic)
	{
		/* Conjunction text */
		if (pval_pos_intrinsic) roff(", but also intrinsically", 0, 0);
		else roff("You are intrinsically", 0, 0);

		for (i = 0, j = 0; i < 32; i++)
		{
			/* All done */
			if (j == pval_neg_intrinsic) break;

			/* List another modifier */
			if (pval_intrinsic[i] < 0)
			{
				/* Display this modifier */
				roff(format(" %s", modifier_blurb[i][1]), 0, 0);

				/* Another modifier is being listed */
				j++;

				/* Commas between members of a list of more than two */
				if ((pval_neg_intrinsic > 2) && (j < pval_neg_intrinsic))
					roff(",", 0, 0);

				/* "and" before last member of a list of more than one */
				if (j == pval_neg_intrinsic - 1) roff(" and", 0, 0);
			}
		}
	}

	/* End this sentence */
	if ((pval_pos_intrinsic) || (pval_neg_intrinsic)) roff(".\n", 0, 0);


	/* Display equipment modifiers:  first positive, then negative */
	for (k = 0; k < 2; k++)
	{
		/* Get number of modifiers to list in this round */
		int num = ((k == 0) ? pval_pos_equip : pval_neg_equip);

		/* No modifiers -- skip */
		if (!num) continue;

		/* Introduction and conjunction text */
		if ((k == 0) && (pval_pos_equip))
		{
			roff("Your equipment increases your", 0, 0);
		}
		if ((k == 1) && (pval_neg_equip))
		{
			if (pval_pos_equip) roff(", but also decreases your", 0, 0);
			else roff("Your equipment decreases your", 0, 0);
		}

		for (i = 0, j = 0; i < 32; i++)
		{
			/* List another modifier */
			if (((k == 0) && (pval_equip[i] > 0)) ||
				 ((k == 1) && (pval_equip[i] < 0)))
			{
				/* Display this modifier */
				roff(format(" %s", pval_desc_text[i]), 0, 0);

				/* Another modifier is being listed */
				j++;

				/* Commas between members of a list of more than two */
				if ((num > 2) && (j < num))
					roff(",", 0, 0);

				/* "and" before last member of a list of more than one */
				if (j == num - 1) roff(" and", 0, 0);
			}
		}

		/* End this sentence */
		if ((k == 0) && (pval_pos_equip))
		{
			if (!pval_neg_equip) roff(".\n", 0, 0);
		}
		if ((k == 1) && (pval_neg_equip)) roff(".\n", 0, 0);
	}


	/*** Display sustains, resists, and immunities ***/

	/* Print header */
	roff("\n", 0, 0);
	c_roff_centered(TERM_L_BLUE, "\nSustains, Resists, and Immunities\n", 0, 0);

	if ((p_ptr->sustain_str) || (p_ptr->sustain_int) ||
		 (p_ptr->sustain_wis) || (p_ptr->sustain_dex) ||
		 (p_ptr->sustain_con) || (p_ptr->sustain_chr))
	{
		/* Clear number of items to list, and items listed. */
		attr_num = 0;
		attr_listed = 0;

		/* How many attributes need to be listed? */
		if (p_ptr->sustain_str) attr_num++;
		if (p_ptr->sustain_int) attr_num++;
		if (p_ptr->sustain_wis) attr_num++;
		if (p_ptr->sustain_dex) attr_num++;
		if (p_ptr->sustain_con) attr_num++;
		if (p_ptr->sustain_chr) attr_num++;

		/* Start */
		roff("Your", 10, 70);

		/* Loop for number of attributes in this group. */
		for (j = 0; j < A_MAX; j++)
		{
			bool list_ok = FALSE;

			if ((j == 0) && (p_ptr->sustain_str)) list_ok = TRUE;
			if ((j == 1) && (p_ptr->sustain_int)) list_ok = TRUE;
			if ((j == 2) && (p_ptr->sustain_wis)) list_ok = TRUE;
			if ((j == 3) && (p_ptr->sustain_dex)) list_ok = TRUE;
			if ((j == 4) && (p_ptr->sustain_con)) list_ok = TRUE;
			if ((j == 5) && (p_ptr->sustain_chr)) list_ok = TRUE;

			if (!list_ok) continue;

			/* Listing another attribute. */
			attr_listed++;

			/* Commas separate members of a list of more than two. */
			if ((attr_num > 2) && (attr_listed > 1)) roff(",", 10, 70);

			/* "and" before final member of a list of more than one. */
			if ((attr_num > 1) && (j != 0))
			{
				if (attr_num == attr_listed) roff(" and", 10, 70);
			}

			/* List the attribute description, in its proper place. */
			if (j == 0) roff(" strength", 10, 70);
			if (j == 1) roff(" intelligence", 10, 70);
			if (j == 2) roff(" wisdom", 10, 70);
			if (j == 3) roff(" dexterity", 10, 70);
			if (j == 4) roff(" constitution", 10, 70);
			if (j == 5) roff(" charisma", 10, 70);
		}

		/* End this sentence */
		roff(format(" %s sustained.", ((attr_num > 1) ? "are" : "is")), 10, 70);
		roff("\n", 0, 0);
	}


	/* Handle the basic resists, poison, and the survival immunities */
	if (p_ptr->immune_acid)
	{
		roff("You are immune to acid.", sk_get_col(), 0);
	}
	else if ((p_ptr->resist_acid) && (p_ptr->oppose_acid))
	{
		roff("You resist acid exceptionally well.", sk_get_col(), 0);
	}
	else if (p_ptr->resist_acid)
	{
		roff("You resist acid.", sk_get_col(), 0);
	}
	else if (p_ptr->oppose_acid)
	{
		roff("You temporarily resist acid.", sk_get_col(), 0);
	}

	if (p_ptr->immune_elec)
	{
		roff("You are immune to electricity.", sk_get_col(), 0);
	}
	else if ((p_ptr->resist_elec) && (p_ptr->oppose_elec))
	{
		roff("You resist electricity exceptionally well.", sk_get_col(), 0);
	}
	else if (p_ptr->resist_elec)
	{
		roff("You resist electricity.", sk_get_col(), 0);
	}
	else if (p_ptr->oppose_elec)
	{
		roff("You temporarily resist electricity.", sk_get_col(), 0);
	}

	if (p_ptr->immune_fire)
	{
		roff("You are immune to fire.", sk_get_col(), 0);
	}
	else if ((p_ptr->resist_fire) && (p_ptr->oppose_fire))
	{
		roff("You resist fire exceptionally well.", sk_get_col(), 0);
	}
	else if (p_ptr->resist_fire)
	{
		roff("You resist fire.", sk_get_col(), 0);
	}
	else if (p_ptr->oppose_fire)
	{
		roff("You temporarily resist fire.", sk_get_col(), 0);
	}

	if (p_ptr->immune_cold)
	{
		roff("You are immune to cold.", sk_get_col(), 0);
	}
	else if ((p_ptr->resist_cold) && (p_ptr->oppose_cold))
	{
		roff("You resist cold exceptionally well.", sk_get_col(), 0);
	}
	else if (p_ptr->resist_cold)
	{
		roff("You resist cold.", sk_get_col(), 0);
	}
	else if (p_ptr->oppose_cold)
	{
		roff("You temporarily resist cold.", sk_get_col(), 0);
	}

	if ((p_ptr->resist_pois) && (p_ptr->oppose_pois))
	{
		roff("You resist poison exceptionally well.", sk_get_col(), 0);
	}
	else if (p_ptr->resist_pois)
	{
		roff("You resist poison.", sk_get_col(), 0);
	}
	else if (p_ptr->oppose_pois)
	{
		roff("You temporarily resist poison.", sk_get_col(), 0);
	}

	if (p_ptr->resist_fear)
	{
		roff("You resist fear.", sk_get_col(), 0);
	}
	else
	{
		roff("You can be frightened.", sk_get_col(), 0);
	}

	if (p_ptr->resist_confu)
	{
		roff("You resist confusion.", sk_get_col(), 0);
	}

	else
	{
		roff("You can be confused.", sk_get_col(), 0);
	}

	if (p_ptr->resist_blind)
	{
		roff("You resist blindness.", sk_get_col(), 0);
	}
	else
	{
		roff("You can be blinded.", sk_get_col(), 0);
	}


	/* Handle the higher resists */
	roff("\n", 0, 0);

	/* Resistances. */
	if ((p_ptr->oppose_ethereal) ||
		 (p_ptr->resist_lite)  || (p_ptr->resist_dark) ||
		 (p_ptr->resist_sound) || (p_ptr->resist_shard) ||
		 (p_ptr->resist_nexus) || (p_ptr->resist_nethr) ||
		 (p_ptr->resist_chaos) || (p_ptr->resist_disen))
	{
		/* Clear number of items to list, and items listed. */
		attr_num = 0;
		attr_listed = 0;

		/* How many attributes need to be listed? */
		if (p_ptr->resist_lite || p_ptr->oppose_ethereal)  attr_num++;
		if (p_ptr->resist_dark || p_ptr->oppose_ethereal)  attr_num++;
		if (p_ptr->resist_sound) attr_num++;
		if (p_ptr->resist_shard) attr_num++;
		if (p_ptr->resist_nexus) attr_num++;
		if (p_ptr->resist_nethr) attr_num++;
		if (p_ptr->resist_chaos) attr_num++;
		if (p_ptr->resist_disen) attr_num++;

		roff("You are resistant to", 10, 70);

		/* Loop for number of attributes in this group. */
		for (j = 0; j < 8; j++)
		{
			bool list_ok = FALSE;

			if ((j ==  0) && (p_ptr->resist_lite || p_ptr->oppose_ethereal))
				list_ok = TRUE;
			if ((j ==  1) && (p_ptr->resist_dark || p_ptr->oppose_ethereal))
				list_ok = TRUE;
			if ((j ==  2) && (p_ptr->resist_sound)) list_ok = TRUE;
			if ((j ==  3) && (p_ptr->resist_shard)) list_ok = TRUE;
			if ((j ==  4) && (p_ptr->resist_nexus)) list_ok = TRUE;
			if ((j ==  5) && (p_ptr->resist_nethr)) list_ok = TRUE;
			if ((j ==  6) && (p_ptr->resist_chaos)) list_ok = TRUE;
			if ((j ==  7) && (p_ptr->resist_disen)) list_ok = TRUE;

			if (!list_ok) continue;

			/* Listing another attribute. */
			attr_listed++;

			/* Commas separate members of a list of more than two. */
			if ((attr_num > 2) && (attr_listed > 1)) roff(",", 10, 70);

			/* "and" before final member of a list of more than one. */
			if ((attr_num > 1) && (j != 0))
			{
				if (attr_num == attr_listed) roff(" and", 10, 70);
			}

			/* List the attribute description, in its proper place. */
			if (j ==  0) roff(" light", 10, 70);
			if (j ==  1) roff(" darkness", 10, 70);
			if (j ==  2) roff(" sound", 10, 70);
			if (j ==  3) roff(" shards", 10, 70);
			if (j ==  4) roff(" nexus", 10, 70);
			if (j ==  5) roff(" nether", 10, 70);
			if (j ==  6) roff(" chaos", 10, 70);
			if (j ==  7) roff(" disenchantment", 10, 70);
		}

		/* End sentence.  Go to next line. */
		roff(".\n", 10, 70);
	}



	/*** Display temporary conditions ***/

	/* Print header */
	c_roff_centered(TERM_L_BLUE, "\nTemporary Conditions\n", 0, 0);

	if (p_ptr->blind)
	{
		roff("You cannot see.", sk_get_col(), 0);
	}
	if (p_ptr->confused)
	{
		roff("You are confused.", sk_get_col(), 0);
	}
	if (p_ptr->afraid)
	{
		roff("You are terrified.", sk_get_col(), 0);
	}
	if (p_ptr->image)
	{
		roff("You are hallucinating.", sk_get_col(), 0);
	}
	if (p_ptr->poisoned)
	{
		roff("You are poisoned.", sk_get_col(), 0);
	}
	if (p_ptr->poisoned)
	{
		roff("You suffer from a wasting disease.", sk_get_col(), 0);
	}
	if (p_ptr->cut)
	{
		if (p_ptr->cut >= WOUND_MORTAL)
			c_roff(TERM_RED, "You have a mortal wound.", sk_get_col(), 0);
		else roff("You are bleeding.", sk_get_col(), 0);
	}
	if (p_ptr->stun)
	{
		roff(format("You are%s stunned.",
			p_ptr->stun >= HVY_STUN ? " heavily" : ""), sk_get_col(), 0);
	}

	/* Luck */
	if (TRUE)
	{
		if (p_ptr->luck == 100) c_roff(TERM_L_GREEN,
			"You are not unlucky.", sk_get_col(), 0);
		else
		{
			if (p_ptr->luck <= 30) c_roff(TERM_RED,
				"You are dangerously unlucky.", sk_get_col(), 0);
			else if (p_ptr->luck <= 80) c_roff(TERM_ORANGE,
				"You are unlucky.", sk_get_col(), 0);
			else c_roff(TERM_YELLOW,
				"You are slightly unlucky.", sk_get_col(), 0);
		}
	}

	if (p_ptr->protevil)
	{
		roff("You are protected from evil.", sk_get_col(), 0);
	}
	if (p_ptr->hero)
	{
		roff("You feel heroic.", sk_get_col(), 0);
	}
	else if (p_ptr->bold)
	{
		roff("You feel unusually brave.", sk_get_col(), 0);
	}
	if (p_ptr->berserk)
	{
		roff("You are in a battle rage.", sk_get_col(), 0);
	}
	if (p_ptr->necro_rage)
	{
		roff("You are in a blood-rage.", sk_get_col(), 0);
	}
	if (p_ptr->shield)
	{
		roff("You are shielded.", sk_get_col(), 0);
	}
	if (p_ptr->holy)
	{
		roff("You are surrounded by a holy aura.", sk_get_col(), 0);
	}
	else if (p_ptr->blessed)
	{
		roff("You feel righteous.", sk_get_col(), 0);
	}
	if (p_ptr->tim_stealth)
	{
		roff("You are especially well-hidden.", sk_get_col(), 0);
	}
	if (p_ptr->regen_hp)
	{
		roff("You have enhanced stealth.", sk_get_col(), 0);
	}
	if (p_ptr->regen_hp)
	{
		roff("You heal unusually quickly.", sk_get_col(), 0);
	}
	if (p_ptr->regen_mana)
	{
		roff("You recover mana unusually quickly.", sk_get_col(), 0);
	}
	if (p_ptr->vitality)
	{
		roff("You recover quickly from ailments.", sk_get_col(), 0);
	}
	if (p_ptr->mania)
	{
		roff("You suffer manic-depressive fits.", sk_get_col(), 0);
	}
	if (p_ptr->res_dam)
	{
		roff("You resist all forms of damage.", sk_get_col(), 0);
	}
	if (p_ptr->forbid_summoning)
	{
		roff("Few monsters will be summoned near you.", sk_get_col(), 0);
	}

	if (p_ptr->wiz_prot)
	{
		roff("You are protected from magic.", sk_get_col(), 0);
	}
	if (p_ptr->word_recall)
	{
		roff("You will soon be recalled.", sk_get_col(), 0);
	}
	if (p_ptr->dancing_feet)
	{
		roff(format("You are blinking around %s.",
			(p_ptr->dancing_feet_safe ? "safely" : "uncontrollably")),
			sk_get_col(), 0);
	}
	if (p_ptr->phasing_foes)
	{
		roff("Your foes are being blinked around.", sk_get_col(), 0);
	}
	if (p_ptr->mental_barrier)
	{
		roff("Your mind is powerful in mental combat.", sk_get_col(), 0);
	}

	if (p_ptr->aura_cold)
	{
		roff("Your cold magic is unusually strong.", sk_get_col(), 0);
	}
	if (p_ptr->aura_fire)
	{
		roff("Your fire magic is unusually strong.", sk_get_col(), 0);
	}
	if (p_ptr->pois_power)
	{
		roff("Your poison magic is unusually strong.", sk_get_col(), 0);
	}
	if (p_ptr->chaos_power)
	{
		roff("Your chaos spells are unusually strong.", sk_get_col(), 0);
	}
	if (p_ptr->nexus_field)
	{
		roff("You are surrounded by a nexus field.", sk_get_col(), 0);
	}



	/*** Display combat modifiers ***/

	/* Print header */
	roff("\n\n", 0, 0);
	c_roff_centered(TERM_L_BLUE, "Combat Information\n", 0, 0);

	/* Print temporary combat modifiers */
	if (p_ptr->special_attack || p_ptr->acid_attack ||
	    p_ptr->elec_attack    || p_ptr->fire_attack ||
	    p_ptr->cold_attack    || p_ptr->pois_attack)
	{
		if (p_ptr->special_attack & (ATTACK_CONFUSE))
			roff("You will attempt to confuse the next monster you strike.  ", 0, 0);
		if (p_ptr->special_attack & (ATTACK_BLKBRTH))
			roff("You will attempt to inflict the Black Breath on the next monster you strike.  ", 0, 0);
		if (p_ptr->special_attack & (ATTACK_FLEE))
			roff("You will flee after striking the next monster.  ", 0, 0);
		if (p_ptr->special_attack & (ATTACK_VORPAL))
			roff("Your blows are temporarily vorpal.  ", 0, 0);
		if (p_ptr->special_attack & (ATTACK_HOLY))
			roff("Your blows strike with holy fury.  ", 0, 0);

		if (p_ptr->special_attack & (ATTACK_PIERCING))
				roff("Your next shot may pierce through several foes.  ", 0, 0);
		if (p_ptr->special_attack & (ATTACK_DEADLY))
				roff("Your next shot will be very deadly.  ", 0, 0);
		if (p_ptr->special_attack & (ATTACK_IMPACT))
				roff("Your next shot will knock enemies back.  ", 0, 0);
		if (p_ptr->special_attack & (ATTACK_ACCURATE))
				roff("Your next shot will be very accurate.  ", 0, 0);
		if (p_ptr->special_attack & (ATTACK_SHOT_FIRE))
				roff("Your next shot will burn with fire.  ", 0, 0);
		if (p_ptr->special_attack & (ATTACK_SHOT_COLD))
				roff("Your next shot will freeze with cold.  ", 0, 0);
		if (p_ptr->special_attack & (ATTACK_BARD))
				roff("Your next shot will strike dragons down.  ", 0, 0);

		if (p_ptr->acid_attack)
			roff("Your blows are infused with acid.  ", 0, 0);
		if (p_ptr->elec_attack)
			roff("Your blows are infused with electricity.  ", 0, 0);
		if (p_ptr->fire_attack)
			roff("Your blows are infused with fire.  ", 0, 0);
		if (p_ptr->cold_attack)
			roff("Your blows are infused with frost.  ", 0, 0);
		if (p_ptr->pois_attack)
			roff("Your blows are infused with poison.  ", 0, 0);
	}


	/* Check both possible melee weapons */
	for (k = 0; k < 2; k++)
	{
		/* Access the current weapon */
		o_ptr = &inventory[((k == 0) ? INVEN_WIELD : INVEN_ARM)];

		/* Require a melee weapon */
		if (is_melee_weapon(o_ptr))
		{
			/* Not listing anything yet */
			bool flag1 = FALSE;
			bool flag2 = FALSE;

			/* We see another weapon */
			weapons++;

			/* Get object flags */
			object_flags(o_ptr, &f[1], &f[2], &f[3]);

			/* Start us off */
			roff(format("\n\n     Your %s melee weapon ",
				((weapons == 1) ? "primary" : "secondary")), 0, 0);

			/* Check for slays */
			if (f[1] & (TR1_SLAY_ANIMAL | TR1_SLAY_EVIL |
			            TR1_SLAY_UNDEAD | TR1_SLAY_DEMON | TR1_SLAY_ORC | TR1_SLAY_TROLL | TR1_SLAY_GIANT | TR1_SLAY_DRAGON |
			            TR1_KILL_DRAGON))
			{
				/* Clear number of items to list, and items listed. */
				attr_num = 0;
				attr_listed = 0;

				/* How many attributes need to be listed? */
				if (f[1] & (TR1_SLAY_ANIMAL)) attr_num++;
				if (f[1] & (TR1_SLAY_EVIL))   attr_num++;
				if (f[1] & (TR1_SLAY_UNDEAD)) attr_num++;
				if (f[1] & (TR1_SLAY_DEMON))  attr_num++;
				if (f[1] & (TR1_SLAY_ORC))    attr_num++;
				if (f[1] & (TR1_SLAY_TROLL))  attr_num++;
				if (f[1] & (TR1_SLAY_GIANT))  attr_num++;
				if (f[1] & (TR1_SLAY_DRAGON)) attr_num++;

				roff("slays", 0, 0);

				/* Loop for number of attributes in this group. */
				for (j = 0; j < 8; j++)
				{
					bool list_ok = FALSE;

					if ((j ==  0) && (f[1] & (TR1_SLAY_ANIMAL))) list_ok = TRUE;
					if ((j ==  1) && (f[1] & (TR1_SLAY_EVIL)))   list_ok = TRUE;
					if ((j ==  2) && (f[1] & (TR1_SLAY_ORC)))    list_ok = TRUE;
					if ((j ==  3) && (f[1] & (TR1_SLAY_TROLL)))  list_ok = TRUE;
					if ((j ==  4) && (f[1] & (TR1_SLAY_GIANT)))  list_ok = TRUE;
					if ((j ==  5) && (f[1] & (TR1_SLAY_DEMON)))  list_ok = TRUE;
					if ((j ==  6) && (f[1] & (TR1_SLAY_DRAGON))) list_ok = TRUE;
					if ((j ==  7) && (f[1] & (TR1_SLAY_UNDEAD))) list_ok = TRUE;

					if (!list_ok) continue;

					/* Listing another attribute. */
					attr_listed++;

					/* Commas separate members of a list of more than two. */
					if ((attr_num > 2) && (attr_listed > 1)) roff(",", 0, 0);

					/* "and" before final member of a list of more than one. */
					if ((attr_num > 1) && (j != 0))
					{
						if (attr_num == attr_listed) roff(" and", 0, 0);
					}

					/* List the attribute description, in its proper place. */
					if (j ==  0) roff(" animals", 0, 0);
					if (j ==  1) roff(" evil", 0, 0);
					if (j ==  2) roff(" orcs", 0, 0);
					if (j ==  3) roff(" trolls", 0, 0);
					if (j ==  4) roff(" giants", 0, 0);
					if (j ==  5) roff(" demons", 0, 0);
					if (j ==  6) roff(" dragons", 0, 0);
					if (j ==  7) roff(" the undead", 0, 0);
				}

				/* Special cases for the heavy slays */
				if (f[1] & (TR1_KILL_DRAGON))
				{
					/* Conjunction */
					if (attr_num) roff(", and ", 3, 77);
					else          roff("It ", 3, 77);

					/* Text */
					roff("is the bane of dragons everywhere", 3, 77);
					flag2 = TRUE;
				}

				flag1 = TRUE;
			}

			/* Check for brands */
			if (f[1] & (TR1_BRAND_ACID | TR1_BRAND_ELEC |
			            TR1_BRAND_FIRE | TR1_BRAND_COLD | TR1_BRAND_POIS |
			            TR1_BRAND_FLAME | TR1_BRAND_VENOM))
			{
				/* Clear number of items to list, and items listed. */
				attr_num = 0;
				attr_listed = 0;

				/* How many attributes need to be listed? */
				if (f[1] & (TR1_BRAND_ACID)) attr_num++;
				if (f[1] & (TR1_BRAND_ELEC)) attr_num++;
				if (f[1] & (TR1_BRAND_FIRE)) attr_num++;
				if (f[1] & (TR1_BRAND_COLD)) attr_num++;
				if (f[1] & (TR1_BRAND_POIS)) attr_num++;

				/* Write some conjunction text, if necessary */
				if      (flag2) roff(".  It ", 0, 0);
				else if (flag1) roff(", and ", 0, 0);

				/* Start off this section */
				if (attr_num) roff("is branded with ", 0, 0);

				/* Loop for number of attributes in this group. */
				for (j = 0; j < 5; j++)
				{
					bool list_ok = FALSE;

					if ((j ==  0) && (f[1] & (TR1_BRAND_ACID))) list_ok = TRUE;
					if ((j ==  1) && (f[1] & (TR1_BRAND_ELEC))) list_ok = TRUE;
					if ((j ==  2) && (f[1] & (TR1_BRAND_FIRE))) list_ok = TRUE;
					if ((j ==  3) && (f[1] & (TR1_BRAND_COLD))) list_ok = TRUE;
					if ((j ==  4) && (f[1] & (TR1_BRAND_POIS))) list_ok = TRUE;

					if (!list_ok) continue;

					/* Listing another attribute. */
					attr_listed++;

					/* Commas separate members of a list of more than two. */
					if ((attr_num > 2) && (attr_listed > 1)) roff(",", 0, 0);

					/* "and" before final member of a list of more than one. */
					if ((attr_num > 1) && (j != 0))
					{
						if (attr_num == attr_listed) roff(" and", 0, 0);
					}

					/* List the attribute description, in its proper place. */
					if (j ==  0) roff(" acid", 0, 0);
					if (j ==  1) roff(" electricity", 0, 0);
					if (j ==  2) roff(" fire", 0, 0);
					if (j ==  3) roff(" frost", 0, 0);
					if (j ==  4) roff(" poison", 0, 0);
				}

				/* Special cases for the heavy brands */
				if ((f[1] & (TR1_BRAND_FLAME)) || (f[1] & (TR1_BRAND_VENOM)))
				{
					/* Conjunction */
					if (attr_num) roff(", and ", 3, 77);

					/* Text */
					if ((f[1] & (TR1_BRAND_FLAME)) && (f[1] & (TR1_BRAND_VENOM)))
						roff("incinerates and injects poison into everything it touches", 3, 77);
					else if (f[1] & (TR1_BRAND_FLAME))
						roff("incinerates everything it touches", 3, 77);
					else if (f[1] & (TR1_BRAND_VENOM))
						roff("injects deadly poisons into everything it touches", 3, 77);
				}

				flag1 = TRUE;
			}

			/* Finish this sentence */
			if (flag1)      roff(".  ", 0, 0);
			else if (!full) roff("has no known brands or slays.  ", 0, 0);
			else            roff("has no brands or slays.  ", 0, 0);


			/* Vorpal/concussion weapon */
			if (f[1] & (TR1_VORPAL))
			{
				if (o_ptr->tval == TV_SWORD) roff("It is a vorpal blade.  ", 0, 0);
				else if (o_ptr->tval != TV_HAFTED) roff("It has a vorpal blade.  ", 0, 0);
				else roff("It is a weapon of concussion.  ", 0, 0);
			}

			/* Blessed weapon (and we care) */
			if ((f[3] & (TR3_BLESSED)) && (p_ptr->realm == PRIEST))
			{
				if (o_ptr->tval != TV_HAFTED)
				{
					if (f[1] & (TR1_VORPAL)) roff("It is blessed.  ", 0, 0);
					else roff("It is a blessed blade.  ", 0, 0);
				}
			}

			/* Impact weapon */
			if (f[3] & (TR3_IMPACT))
			{
				roff("It is an impact weapon", 0, 0);

				if (!(f[1] & (TR1_PERFECT_BALANCE)) ||
				     (f[1] & (TR1_THROWING)))
				{
					roff(".  ", 0, 0);
				}
			}

			/* Throwing weapon */
			if (f[1] & (TR1_PERFECT_BALANCE))
			{
				if (f[3] & (TR3_IMPACT)) roff(", and can also ", 0, 0);
				else roff("It can ", 0, 0);

				roff("be thrown hard and fast.  ", 0, 0);
			}
			else if (f[1] & (TR1_THROWING))
			{
				if (f[3] & (TR3_IMPACT)) roff(", and can also ", 0, 0);
				else roff("It can ", 0, 0);

				roff("be thrown effectively.  ", 0, 0);
			}

			/* Hungry */
			if (f[3] & (TR3_SOULSTEAL))
			{
				roff("It hungers, and you must feed it blood.  ", 0, 0);
			}

			/* Need both hands */
			if (needs_two_hands(f[1], o_ptr->weight))
			{
				roff("You need both hands to wield it.  ", 0, 0);
			}
		}
	}

	/* If we don't have a melee weapon, we're fighting barehanded */
	if (!weapons)
	{
		if (p_ptr->barehand == S_KARATE)
		{
			roff("\n\nYou are using karate.", 0, 0);
		}
		else
		{
			roff("\n\nYou are wrestling.", 0, 0);
		}
	}

	/* Access the current weapon */
	o_ptr = &inventory[INVEN_BOW];


	/* Require a missile weapon */
	if (o_ptr->tval == TV_BOW)
	{
		/* Not listing anything yet */
		flag = FALSE;

		/* Get object flags */
		object_flags(o_ptr, &f[1], &f[2], &f[3]);

		/* Start us off */
		roff(format("\n\n     Your missile weapon has a damage multiplier of %d.  ", p_ptr->ammo_mult), 0, 0);

		/* Check for slays */
		if (f[1] & (TR1_SLAY_ANIMAL | TR1_SLAY_EVIL |
						TR1_SLAY_UNDEAD | TR1_SLAY_DEMON | TR1_SLAY_ORC |
						TR1_SLAY_TROLL | TR1_SLAY_GIANT | TR1_SLAY_DRAGON))
		{
			/* Clear number of items to list, and items listed. */
			attr_num = 0;
			attr_listed = 0;

			/* How many attributes need to be listed? */
			if (f[1] & (TR1_SLAY_ANIMAL)) attr_num++;
			if (f[1] & (TR1_SLAY_EVIL))   attr_num++;
			if (f[1] & (TR1_SLAY_UNDEAD)) attr_num++;
			if (f[1] & (TR1_SLAY_DEMON))  attr_num++;
			if (f[1] & (TR1_SLAY_ORC))    attr_num++;
			if (f[1] & (TR1_SLAY_TROLL))  attr_num++;
			if (f[1] & (TR1_SLAY_GIANT))  attr_num++;
			if (f[1] & (TR1_SLAY_DRAGON)) attr_num++;

			roff("It is especially deadly against", 0, 0);

			/* Loop for number of attributes in this group. */
			for (j = 0; j < 8; j++)
			{
				bool list_ok = FALSE;

				if ((j ==  0) && (f[1] & (TR1_SLAY_ANIMAL))) list_ok = TRUE;
				if ((j ==  1) && (f[1] & (TR1_SLAY_EVIL)))   list_ok = TRUE;
				if ((j ==  2) && (f[1] & (TR1_SLAY_ORC)))    list_ok = TRUE;
				if ((j ==  3) && (f[1] & (TR1_SLAY_TROLL)))  list_ok = TRUE;
				if ((j ==  4) && (f[1] & (TR1_SLAY_GIANT)))  list_ok = TRUE;
				if ((j ==  5) && (f[1] & (TR1_SLAY_DEMON)))  list_ok = TRUE;
				if ((j ==  6) && (f[1] & (TR1_SLAY_DRAGON))) list_ok = TRUE;
				if ((j ==  7) && (f[1] & (TR1_SLAY_UNDEAD))) list_ok = TRUE;

				if (!list_ok) continue;

				/* Listing another attribute. */
				attr_listed++;

				/* Commas separate members of a list of more than two. */
				if ((attr_num > 2) && (attr_listed > 1)) roff(",", 0, 0);

				/* "and" before final member of a list of more than one. */
				if ((attr_num > 1) && (j != 0))
				{
					if (attr_num == attr_listed) roff(" and", 0, 0);
				}

				/* List the attribute description, in its proper place. */
				if (j ==  0) roff(" animals", 0, 0);
				if (j ==  1) roff(" evil", 0, 0);
				if (j ==  2) roff(" orcs", 0, 0);
				if (j ==  3) roff(" trolls", 0, 0);
				if (j ==  4) roff(" giants", 0, 0);
				if (j ==  5) roff(" demons", 0, 0);
				if (j ==  6) roff(" dragons", 0, 0);
				if (j ==  7) roff(" the undead", 0, 0);
			}

			flag = TRUE;
		}

		/* Check for brands */
		if (f[1] & (TR1_BRAND_ACID | TR1_BRAND_ELEC |
						TR1_BRAND_FIRE | TR1_BRAND_COLD | TR1_BRAND_POIS))
		{
			/* Clear number of items to list, and items listed. */
			attr_num = 0;
			attr_listed = 0;

			/* How many attributes need to be listed? */
			if (f[1] & (TR1_BRAND_ACID)) attr_num++;
			if (f[1] & (TR1_BRAND_ELEC)) attr_num++;
			if (f[1] & (TR1_BRAND_FIRE)) attr_num++;
			if (f[1] & (TR1_BRAND_COLD)) attr_num++;
			if (f[1] & (TR1_BRAND_POIS)) attr_num++;

			/* Write some conjunction text, if necessary */
			if (flag) roff(", and imparts ", 0, 0);
			else      roff("It imparts ", 0, 0);

			/* Special case -- one attribute being listed */
			if (attr_num == 1) roff("the ", 0, 0);

			/* Loop for number of attributes in this group. */
			for (j = 0; j < 5; j++)
			{
				bool list_ok = FALSE;

				if ((j ==  0) && (f[1] & (TR1_BRAND_ACID))) list_ok = TRUE;
				if ((j ==  1) && (f[1] & (TR1_BRAND_ELEC))) list_ok = TRUE;
				if ((j ==  2) && (f[1] & (TR1_BRAND_FIRE))) list_ok = TRUE;
				if ((j ==  3) && (f[1] & (TR1_BRAND_COLD))) list_ok = TRUE;
				if ((j ==  4) && (f[1] & (TR1_BRAND_POIS))) list_ok = TRUE;

				if (!list_ok) continue;

				/* Listing another attribute. */
				attr_listed++;

				/* Commas separate members of a list of more than two. */
				if ((attr_num > 2) && (attr_listed > 1)) roff(",", 0, 0);

				/* "and" before final member of a list of more than one. */
				if ((attr_num > 1) && (j != 0))
				{
					if (attr_num == attr_listed) roff(" and", 0, 0);
				}

				/* List the attribute description, in its proper place. */
				if (j ==  0) roff(" acid", 0, 0);
				if (j ==  1) roff(" electric", 0, 0);
				if (j ==  2) roff(" fire", 0, 0);
				if (j ==  3) roff(" frost", 0, 0);
				if (j ==  4) roff(" poison", 0, 0);
			}

			/* End this sentence */
			if (attr_num == 1) roff("brand",  0, 0);
			else               roff("brands", 0, 0);

			roff(" to every missile it fires", 0, 0);

			flag = TRUE;
		}

		/* Weapon of penetration */
		if (f[1] & (TR1_VORPAL))
		{
			if (flag)
			{
				roff(", and ", 0, 0);
			}
			else
			{
				roff("It ", 0, 0);
				flag = TRUE;
			}

			if (p_ptr->ammo_tval == TV_SHOT)
				roff("fires missiles of concussion", 0, 0);
			else
				roff("fires vorpal missiles", 0, 0);
		}

		/* Impact weapon */
		if (f[3] & (TR3_IMPACT))
		{
			if (flag)
			{
				roff(", and ", 0, 0);
			}
			else
			{
				roff("It ", 0, 0);
				flag = TRUE;
			}

			roff("knocks your foes back with every shot", 0, 0);
		}

		/* End the sentence */
		if (flag) roff(".", 0, 0);
	}


	/* Pause */
	prt("[Press any key to continue]", 49, 0);
	(void)inkey();

	/* Set to 25 screen rows, if we were not showing 50 before */
	if (old_rows != 50) Term_rows(FALSE);

	/* Load screen */
	screen_load();
}