static int randquest_hero[] = {20, 13, 15, 16,  9, 17, 18, -1};

bool is_randhero()
{
        int i;

        for (i = 0; randquest_hero[i] != -1; i++)
                if (random_quests[dun_level].type == randquest_hero[i]) break;
        if (randquest_hero[i] != -1)
                return (TRUE);
        else
                return (FALSE);
}

void princess_death(s32b m_idx, s32b r_idx)
{
                int r;

                cmsg_print(TERM_YELLOW, "O Great And Noble Hero, you saved me!");
                msg_print("I am heading home now. I cannot reward you as I should, but please take this.");

                /* Look for the princess */
                for (r = m_max - 1; r >= 1; r--)
                {
                        /* Access the monster */
                        monster_type *m_ptr = &m_list[r];
		
                        /* Ignore "dead" monsters */
                        if (!m_ptr->r_idx) continue;

                        /* Is it the princess? */
                        if (m_ptr->r_idx == 969)
                        {
                                int x = m_ptr->fx;
                                int y = m_ptr->fy;
                                int i, j;
                                object_type forge, *q_ptr;
                                obj_theme theme;


                                delete_monster_idx(r);

                                /* Wipe the glass walls and create a stair */
                                for (i = x - 1; i <= x + 1; i++)
                                for (j = y - 1; j <= y + 1; j++)
                                {
                                        if (in_bounds(j, i)) cave_set_feat(j, i, FEAT_FLOOR);
                                }
                                cave_set_feat(y, x, FEAT_MORE);

                                /* Get local object */
                                q_ptr = &forge;

                                /* Wipe the object */
                                object_wipe(q_ptr);

                                /* No themes */
                                theme.treasure = 100;
                                theme.combat = 100;
                                theme.magic = 100;
                                theme.tools = 100;

                                /* Make a great object */
                                make_object(q_ptr, TRUE, TRUE, theme);

                                /* Drop it in the dungeon */
                                drop_near(q_ptr, -1, y + 1, x);

                                random_quests[dun_level].type = 0;
                                random_quests[dun_level].done = TRUE;

                                break;
                        }
                }
}

void hero_death(s32b m_idx, s32b r_idx)
{
        random_quests[dun_level].type = 0;
        random_quests[dun_level].done = TRUE;

        cmsg_print(TERM_YELLOW, "The adventurer steps out of the shadows and picks up his sword:");
        cmsg_print(TERM_YELLOW, "'Ah ! my sword ! my trusty sword ! Thanks.");

        if (!can_create_companion())
        {
                cmsg_print(TERM_YELLOW, "I must go on my own way now, bye.'");
                return;
        }
        cmsg_print(TERM_YELLOW, "If you wish I can help you in your adventures.'");

        if (get_check("Do you want him to join you ?"))
        {
                int x, y, i;

                /* Look for a location */
                for (i = 0; i < 20; ++i)
                {
                        /* Pick a distance */
                        int d = (i / 15) + 1;

                        /* Pick a location */
                        scatter(&y, &x, py, px, d, 0);

                        /* Require "empty" floor grid */
                        if (!cave_empty_bold(y, x)) continue;

                        /* Hack -- no summon on glyph of warding */
                        if (cave[y][x].feat == FEAT_GLYPH) continue;
                        if (cave[y][x].feat == FEAT_MINOR_GLYPH) continue;

                        /* Nor on the between */
                        if (cave[y][x].feat == FEAT_BETWEEN) continue;

                        /* ... nor on the Pattern */
                        if ((cave[y][x].feat >= FEAT_PATTERN_START) &&
                            (cave[y][x].feat <= FEAT_PATTERN_XTRA2))
                                continue;

                        /* Okay */
                        break;
                }

                if (i < 20)
                {
                        int m_idx;

                        m_allow_special[test_monster_name("Adventurer")] = TRUE;
                        m_idx = place_monster_one(y, x, test_monster_name("Adventurer"), 0, FALSE, MSTATUS_COMPANION);
                        m_allow_special[test_monster_name("Adventurer")] = FALSE;
                        if (m_idx)
                        {
                                m_list[m_idx].exp = MONSTER_EXP(1 + (dun_level * 3 / 2));
                                m_list[m_idx].status = MSTATUS_COMPANION;
                                monster_check_experience(m_idx, TRUE);
                        }
                }
                else
                        msg_print("The adventurer suddently seems afraid and flee...");
        }
}

bool quest_random_death_hook(char *fmt)
{
        int r_idx;
        s32b m_idx;

        m_idx = get_next_arg(fmt);
        r_idx = m_list[m_idx].r_idx;

        if (!(d_info[dungeon_type].flags1 & DF1_PRINCIPAL)) return (FALSE);
        if ((dun_level < 1) || (dun_level >= MAX_RANDOM_QUEST)) return (FALSE);
        if (!random_quests[dun_level].type) return (FALSE);
        if (p_ptr->inside_quest) return (FALSE);
        if (random_quests[dun_level].r_idx != r_idx) return (FALSE);

        if (!(m_list[m_idx].mflag & MFLAG_QUEST)) return (FALSE);

        /* Killed enough ?*/
        quest[QUEST_RANDOM].data[0]++;
        if (quest[QUEST_RANDOM].data[0] == random_quests[dun_level].type)
        {
                if (is_randhero())
                        hero_death(m_idx, r_idx);
                else
                        princess_death(m_idx, r_idx);
        }

        return (FALSE);
}
bool quest_random_turn_hook(char *fmt)
{
        quest[QUEST_RANDOM].data[0] = 0;
        quest[QUEST_RANDOM].data[1] = 0;
        return (FALSE);
}
bool quest_random_feeling_hook(char *fmt)
{
        if (!(d_info[dungeon_type].flags1 & DF1_PRINCIPAL)) return (FALSE);
        if ((dun_level < 1) || (dun_level >= MAX_RANDOM_QUEST)) return (FALSE);
        if (!random_quests[dun_level].type) return (FALSE);
        if (p_ptr->inside_quest) return (FALSE);
        if (!dun_level) return (FALSE);

        if (is_randhero())
        {
                cmsg_format(TERM_YELLOW, "A strange man wrapped in a dark cloak steps out of the shadows:");
                cmsg_format(TERM_YELLOW, "'Ah please help me ! A group of %s stole my sword! Im nothing without it'", r_info[random_quests[dun_level].r_idx].name + r_name);
        }
        else
                cmsg_format(TERM_YELLOW, "You hear someone shouting: 'Leave me alone, stupid %s'", r_info[random_quests[dun_level].r_idx].name + r_name);
        return (FALSE);
}
bool quest_random_gen_hero_hook(char *fmt)
{
        int i;

        if (!(d_info[dungeon_type].flags1 & DF1_PRINCIPAL)) return (FALSE);
        if ((dun_level < 1) || (dun_level >= MAX_RANDOM_QUEST)) return (FALSE);
        if (!random_quests[dun_level].type) return (FALSE);
        if (p_ptr->inside_quest) return (FALSE);
        if (!is_randhero()) return (FALSE);

        i = random_quests[dun_level].type;

        m_allow_special[random_quests[dun_level].r_idx] = TRUE;
        while (i)
        {
                int m_idx, y = rand_range(1, cur_hgt - 2), x = rand_range(1, cur_wid - 2);

                m_idx = place_monster_one(y, x, random_quests[dun_level].r_idx, 0, FALSE, MSTATUS_ENEMY);
                if (m_idx)
                {
                        monster_type *m_ptr = &m_list[m_idx];
                        m_ptr->mflag |= MFLAG_QUEST;
                        i--;
                }
        }
        m_allow_special[random_quests[dun_level].r_idx] = FALSE;

        return (FALSE);
}
bool quest_random_gen_hook(char *fmt)
{
        s32b x, y, bx0, by0;
        int xstart;
        int ystart;
        int y2, x2, yval, xval;
	int y1, x1, xsize, ysize;
	cave_type *c_ptr;

        by0 = get_next_arg(fmt);
        bx0 = get_next_arg(fmt);

        if (!(d_info[dungeon_type].flags1 & DF1_PRINCIPAL)) return (FALSE);
        if ((dun_level < 1) || (dun_level >= MAX_RANDOM_QUEST)) return (FALSE);
        if (!random_quests[dun_level].type) return (FALSE);
        if (p_ptr->inside_quest) return (FALSE);
        if (quest[QUEST_RANDOM].data[1]) return (FALSE);
        if (is_randhero()) return (FALSE);

	/* Pick a room size */
        xsize = 0;
        ysize = 0;
        init_flags = INIT_GET_SIZE;
        process_dungeon_file_full = TRUE;
        process_dungeon_file(format("qrand%d.map", random_quests[dun_level].type), &ysize, &xsize, cur_hgt, cur_wid, TRUE);
        process_dungeon_file_full = FALSE;

        /* Try to allocate space for room.  If fails, exit */
        if (!room_alloc(xsize+2,ysize+2,FALSE,by0,bx0,&xval,&yval)) return FALSE;

	/* Get corner values */
	y1 = yval - ysize/2;
	x1 = xval - xsize/2;
        y2 = yval + (ysize)/2;
        x2 = xval + (xsize)/2;

	/* Place a full floor under the room */
	for (y = y1 - 1; y <= y2 + 1; y++)
	{
		for (x = x1 - 1; x <= x2 + 1; x++)
		{
			c_ptr = &cave[y][x];
                        c_ptr->feat = floor_type[rand_int(100)];
			c_ptr->info |= (CAVE_ROOM);
                        c_ptr->info |= (CAVE_GLOW);
		}
	}

	build_rectangle(y1 - 1, x1 - 1, y2 + 1, x2 + 1, feat_wall_outer, 0);

	/* Set the correct monster hook */
	set_mon_num_hook();

	/* Prepare allocation table */
	get_mon_num_prep();

        xstart = x1;
        ystart = y1;
        init_flags = INIT_CREATE_DUNGEON;
        process_dungeon_file_full = TRUE;
        process_dungeon_file(format("qrand%d.map", random_quests[dun_level].type), &ystart, &xstart, cur_hgt, cur_wid, TRUE);
        process_dungeon_file_full = FALSE;

        for (x = x1; x < xstart; x++)
        for (y = y1; y < ystart; y++)
        {
                cave[y][x].info |= CAVE_ICKY | CAVE_ROOM;
                if (cave[y][x].feat == FEAT_MARKER)
                {
                        monster_type *m_ptr;
                        int i;

                        m_allow_special[random_quests[dun_level].r_idx] = TRUE;
                        i = place_monster_one(y, x, random_quests[dun_level].r_idx, 0, FALSE, MSTATUS_ENEMY);
                        m_allow_special[random_quests[dun_level].r_idx] = FALSE;
                        if (i)
                        {
                                m_ptr = &m_list[i];
                                m_ptr->mflag |= MFLAG_QUEST;
                        }
                }
        }

        /* Dont try another one for this generation */
        quest[QUEST_RANDOM].data[1] = 1;

	/* Boost level feeling a bit - a la pits */
	rating += 10;

        return (TRUE);
}
bool quest_random_dump_hook(char *fmt)
{
        int i, p = 0;

        for (i = 0; i < MAX_RANDOM_QUEST; i++)
        {
                if (random_quests[i].done) p++;
        }

        if (p > 1) fprintf(hook_file, "\n You saved %d princesses.", p);
        else if (p == 1) fprintf(hook_file, "\n You saved one princess.");
        else fprintf(hook_file, "\n You saved no princesses.");

        return (FALSE);
}
bool quest_random_init_hook(int q_idx)
{
        add_hook(HOOK_MONSTER_DEATH, quest_random_death_hook, "rand_death");
        add_hook(HOOK_NEW_LEVEL, quest_random_turn_hook, "rand_new_lvl");
        add_hook(HOOK_LEVEL_REGEN, quest_random_turn_hook, "rand_regen_lvl");
        add_hook(HOOK_LEVEL_END_GEN, quest_random_gen_hero_hook, "rand_gen_hero");
        add_hook(HOOK_BUILD_ROOM1, quest_random_gen_hook, "rand_gen");
        add_hook(HOOK_FEELING, quest_random_feeling_hook, "rand_feel");
        add_hook(HOOK_CHAR_DUMP, quest_random_dump_hook, "rand_dump");
        return (FALSE);
}
