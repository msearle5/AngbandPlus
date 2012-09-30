/* File: effects.c */

/* Purpose: effects of various "objects" */

/*
 * Copyright (c) 1989 James E. Wilson, Robert A. Koeneke
 *
 * This software may be copied and distributed for educational, research, and
 * not for profit purposes provided that this copyright and statement are
 * included in all such copies.
 */

#include "angband.h"

#define REWARD_CHANCE 10

static cptr favor_text[11] = {
  "impossible",
  "impossible",
  "impossible",
  "extremely unlikely",
  "extremely unlikely",
  "extremely unlikely",
  "unlikely",
  "unlikely",
  "likely",
  "certain",
  "certain beyond any doubt"
};


/*
 * Set "p_ptr->parasite" and "p_ptr->parasite_r_idx"
 * notice observable changes
 */
bool set_parasite(int v, int r)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
                if (!p_ptr->parasite)
		{
                        msg_print("You feel something growing in you.");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
                if (p_ptr->parasite)
		{
                        if (magik(80))
                        {
                                char r_name[80];
                                int wx, wy;
                                int attempts = 500;

                                monster_race_desc(r_name, p_ptr->parasite_r_idx, 0);

                                do
                                {
                                        scatter(&wy, &wx, py, px, 10, 0);
                                }
                                while (!(in_bounds(wy,wx) && cave_floor_bold(wy,wx)) && --attempts);

                                if (place_monster_one(wy, wx, p_ptr->parasite_r_idx, 0, FALSE, MSTATUS_ENEMY))
                                {
                                        cmsg_format(TERM_L_BLUE, "Your body convulse and spawn %s.", r_name);
                                        p_ptr->food -= 750;
                                        if (p_ptr->food < 100) p_ptr->food = 100;
                                }
                        }
                        else
                        {
                                cmsg_print(TERM_L_BLUE, "The hideous thing growing in you seems to die.");
                        }
                        notice = TRUE;
		}
	}

	/* Use the value */
        p_ptr->parasite = v;
        p_ptr->parasite_r_idx = r;

	/* Nothing to notice */
	if (!notice)
		return (FALSE);

	/* Disturb */
	if (disturb_state)
		disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->disrupt_shield"
 * notice observable changes
 */
bool set_disrupt_shield(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
                if (!p_ptr->disrupt_shield)
		{
                        msg_print("You feel invulnerable.");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
                if (p_ptr->disrupt_shield)
		{
                        msg_print("You are more vulnerable.");
			notice = TRUE;
		}
	}

	/* Use the value */
        p_ptr->disrupt_shield = v;

	/* Nothing to notice */
	if (!notice)
		return (FALSE);

	/* Disturb */
	if (disturb_state)
		disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->prob_travel"
 * notice observable changes
 */
bool set_prob_travel(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
                if (!p_ptr->prob_travel)
		{
                        msg_print("You feel instable.");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
                if (p_ptr->prob_travel)
		{
                        msg_print("You are more stable.");
			notice = TRUE;
		}
	}

	/* Use the value */
        p_ptr->prob_travel = v;

	/* Nothing to notice */
	if (!notice)
		return (FALSE);

	/* Disturb */
	if (disturb_state)
		disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->tim_invis", and "p_ptr->tim_inv_pow",
 * notice observable changes
 */
bool set_invis(int v, int p)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
                if (!p_ptr->tim_invisible)
		{
			msg_print("You feel your body fade away.");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
                if (p_ptr->tim_invisible)
		{
			msg_print("You are no longer invisible.");
			notice = TRUE;
			p = 0;
		}
	}

	/* Use the value */
        p_ptr->tim_invisible = v;
	p_ptr->tim_inv_pow = p;

	/* Nothing to notice */
	if (!notice)
		return (FALSE);

	/* Disturb */
	if (disturb_state)
		disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Result */
	return (TRUE);
}

/*
 * Set "no_breeds"
 */
bool set_no_breeders(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
                if (!no_breeds)
		{
                        msg_print("You feel an anti-sexual aura.");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
                if (no_breeds)
		{
                        msg_print("You no longer feel an anti-sexual aura.");
			notice = TRUE;
		}
	}

	/* Use the value */
        no_breeds = v;

	/* Nothing to notice */
	if (!notice)
		return (FALSE);

	/* Disturb */
	if (disturb_state)
		disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->tim_deadly"
 */
bool set_tim_deadly(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
                if (!p_ptr->tim_deadly)
		{
                        msg_print("You feel very accurate.");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
                if (p_ptr->tim_deadly)
		{
                        msg_print("You are suddenly less accurate.");
			notice = TRUE;
		}
	}

	/* Use the value */
        p_ptr->tim_deadly = v;

	/* Nothing to notice */
	if (!notice)
		return (FALSE);

	/* Disturb */
	if (disturb_state)
		disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->tim_ffall"
 */
bool set_tim_ffall(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
                if (!p_ptr->tim_ffall)
		{
                        msg_print("You feel very light.");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
                if (p_ptr->tim_ffall)
		{
                        msg_print("You are suddenly heavier.");
			notice = TRUE;
		}
	}

	/* Use the value */
        p_ptr->tim_ffall = v;

	/* Nothing to notice */
	if (!notice)
		return (FALSE);

	/* Disturb */
	if (disturb_state)
		disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->meditation"
 */
bool set_meditation(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
                if (!p_ptr->meditation)
		{
                        msg_print("You start meditating on yourself...");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
                if (p_ptr->meditation)
		{
                        msg_print("You stop your self meditation.");
			notice = TRUE;
		}
	}

	/* Use the value */
        p_ptr->meditation = v;

	/* Nothing to notice */
	if (!notice)
		return (FALSE);

	/* Disturb */
	if (disturb_state)
		disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);
        p_ptr->update |= (PU_MANA);

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->tim_reflect"
 */
bool set_tim_reflect(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
                if (!p_ptr->tim_reflect)
		{
                        msg_print("You start reflecting the world around you.");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
                if (p_ptr->tim_reflect)
		{
                        msg_print("You stop reflecting.");
			notice = TRUE;
		}
	}

	/* Use the value */
        p_ptr->tim_reflect = v;

	/* Nothing to notice */
	if (!notice)
		return (FALSE);

	/* Disturb */
	if (disturb_state)
		disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->tim_res_time"
 */
bool set_tim_res_time(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
                if (!p_ptr->tim_res_time)
		{
                        msg_print("You are now protected against the space-time distortions.");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
                if (p_ptr->tim_res_time)
		{
                        msg_print("You are no longer protected against the space-time distortions.");
			notice = TRUE;
		}
	}

	/* Use the value */
        p_ptr->tim_res_time = v;

	/* Nothing to notice */
	if (!notice)
		return (FALSE);

	/* Disturb */
	if (disturb_state)
		disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->tim_fire_aura"
 */
bool set_tim_fire_aura(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
                if (!p_ptr->tim_fire_aura)
		{
                        msg_print("You are enveloped in flames.");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
                if (p_ptr->tim_fire_aura)
		{
                        msg_print("You are no longer enveloped in flames.");
			notice = TRUE;
		}
	}

	/* Use the value */
        p_ptr->tim_fire_aura = v;

	/* Nothing to notice */
	if (!notice)
		return (FALSE);

	/* Disturb */
	if (disturb_state)
		disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->strike"
 */
bool set_strike(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
                if (!p_ptr->strike)
		{
                        msg_print("You feel very accurate.");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
                if (p_ptr->strike)
		{
                        msg_print("You are no longer very accurate.");
			notice = TRUE;
		}
	}

	/* Use the value */
        p_ptr->strike = v;

	/* Nothing to notice */
	if (!notice)
		return (FALSE);

	/* Disturb */
	if (disturb_state)
		disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->oppose_ld"
 */
bool set_oppose_ld(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
                if (!p_ptr->oppose_ld)
		{
                        msg_print("You feel protected against the light's fluctuation.");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
                if (p_ptr->oppose_ld)
		{
                        msg_print("You are no longer protected against the light's fluctuation.");
			notice = TRUE;
		}
	}

	/* Use the value */
        p_ptr->oppose_ld = v;

	/* Nothing to notice */
	if (!notice)
		return (FALSE);

	/* Disturb */
	if (disturb_state)
		disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->oppose_cc"
 */
bool set_oppose_cc(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
                if (!p_ptr->oppose_cc)
		{
                        msg_print("You feel protected against raw chaos.");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
                if (p_ptr->oppose_cc)
		{
                        msg_print("You are no longer protected against chaos.");
			notice = TRUE;
		}
	}

	/* Use the value */
        p_ptr->oppose_cc = v;

	/* Nothing to notice */
	if (!notice)
		return (FALSE);

	/* Disturb */
	if (disturb_state)
		disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->oppose_ss"
 */
bool set_oppose_ss(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
                if (!p_ptr->oppose_ss)
		{
                        msg_print("You feel protected against the ravages of sound and shards.");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
                if (p_ptr->oppose_ss)
		{
                        msg_print("You are no longer protected against the ravages of sound and shards.");
			notice = TRUE;
		}
	}

	/* Use the value */
        p_ptr->oppose_ss = v;

	/* Nothing to notice */
	if (!notice)
		return (FALSE);

	/* Disturb */
	if (disturb_state)
		disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->oppose_nex"
 */
bool set_oppose_nex(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
                if (!p_ptr->oppose_nex)
		{
                        msg_print("You feel protected against the strange forces of nexus.");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
                if (p_ptr->oppose_nex)
		{
                        msg_print("You are no longer protected against the strange forces of nexus.");
			notice = TRUE;
		}
	}

	/* Use the value */
        p_ptr->oppose_nex = v;

	/* Nothing to notice */
	if (!notice)
		return (FALSE);

	/* Disturb */
	if (disturb_state)
		disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->tim_mimic", and "p_ptr->mimic_form",
 * notice observable changes
 */
bool set_mimic(int v, int p)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
                if (!p_ptr->tim_mimic)
		{
                        msg_print("You feel your body change.");
                        p_ptr->mimic_form=p;
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
                if (p_ptr->tim_mimic)
		{
                        msg_print("You are no longer transformed.");
                        p_ptr->mimic_form=0;
			notice = TRUE;
			p = 0;
		}
	}

	/* Use the value */
        p_ptr->tim_mimic = v;

	/* Nothing to notice */
	if (!notice)
		return (FALSE);

	/* Disturb */
	if (disturb_state)
		disturb(0, 0);

        /* Redraw title */
        p_ptr->redraw |= (PR_TITLE);

	/* Recalculate bonuses */
        p_ptr->update |= (PU_BONUS | PU_SANITY);

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->blind", notice observable changes
 *
 * Note the use of "PU_UN_LITE" and "PU_UN_VIEW", which is needed to
 * memorize any terrain features which suddenly become "visible".
 * Note that blindness is currently the only thing which can affect
 * "player_can_see_bold()".
 */
bool set_blind(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->blind)
		{
			msg_print("You are blind!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->blind)
		{
			msg_print("You can see again.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->blind = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Fully update the visuals */
	p_ptr->update |= (PU_UN_VIEW | PU_UN_LITE | PU_VIEW | PU_LITE | PU_MONSTERS);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Redraw the "blind" */
	p_ptr->redraw |= (PR_BLIND);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->tim_lite", notice observable changes
 *
 * Note the use of "PU_LITE" and "PU_VIEW", which is needed to
 * memorize any terrain features which suddenly become "visible".
 * Note that blindness is currently the only thing which can affect
 * "player_can_see_bold()".
 */
bool set_lite(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
                if (!p_ptr->tim_lite)
		{
                        msg_print("You suddenly seem brighter!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
                if (p_ptr->tim_lite)
		{
                        msg_print("You are no longer bright.");
			notice = TRUE;
		}
	}

	/* Use the value */
        p_ptr->tim_lite = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Fully update the visuals */
        p_ptr->update |= (PU_VIEW | PU_LITE | PU_MONSTERS);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->confused", notice observable changes
 */
bool set_confused(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->confused)
		{
			msg_print("You are confused!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->confused)
		{
			msg_print("You feel less confused now.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->confused = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Redraw the "confused" */
	p_ptr->redraw |= (PR_CONFUSED);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->poisoned", notice observable changes
 */
bool set_poisoned(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->poisoned)
		{
			msg_print("You are poisoned!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->poisoned)
		{
			msg_print("You are no longer poisoned.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->poisoned = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Redraw the "poisoned" */
	p_ptr->redraw |= (PR_POISONED);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->afraid", notice observable changes
 */
bool set_afraid(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->afraid)
		{
			msg_print("You are terrified!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->afraid)
		{
			msg_print("You feel bolder now.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->afraid = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Redraw the "afraid" */
	p_ptr->redraw |= (PR_AFRAID);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->paralyzed", notice observable changes
 */
bool set_paralyzed(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->paralyzed)
		{
			msg_print("You are paralyzed!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->paralyzed)
		{
			msg_print("You can move again.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->paralyzed = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Redraw the state */
	p_ptr->redraw |= (PR_STATE);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->image", notice observable changes
 *
 * Note that we must redraw the map when hallucination changes.
 */
bool set_image(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->image)
		{
            msg_print("Oh, wow! Everything looks so cosmic now!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->image)
		{
			msg_print("You can see clearly again.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->image = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Update monsters */
	p_ptr->update |= (PU_MONSTERS);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD | PW_M_LIST);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->lightspeed", notice observable changes
 */
bool set_light_speed(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
                if (!p_ptr->lightspeed)
		{
                        msg_print("You feel as if time has stopped!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
                if (p_ptr->lightspeed)
		{
                        msg_print("You feel time returning to its normal rate.");
			notice = TRUE;
		}
	}

	/* Use the value */
        p_ptr->lightspeed = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}

bool set_fast(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->fast)
		{
			msg_print("You feel yourself moving faster!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->fast)
		{
			msg_print("You feel yourself slow down.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->fast = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->slow", notice observable changes
 */
bool set_slow(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->slow)
		{
			msg_print("You feel yourself moving slower!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->slow)
		{
			msg_print("You feel yourself speed up.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->slow = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->shield", notice observable changes
 */
bool set_shield(int v, int p, s16b o)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->shield)
		{
                        msg_print("Your skin turns to stone.");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->shield)
		{
                        msg_print("Your skin returns to normal.");
			notice = TRUE;
                        p = 0;
		}
	}

	/* Use the value */
	p_ptr->shield = v;
        p_ptr->shield_power = p;
        p_ptr->shield_opt = o;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}



/*
 * Set "p_ptr->blessed", notice observable changes
 */
bool set_blessed(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->blessed)
		{
			msg_print("You feel righteous!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->blessed)
		{
			msg_print("The prayer has expired.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->blessed = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->hero", notice observable changes
 */
bool set_hero(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->hero)
		{
			msg_print("You feel like a hero!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->hero)
		{
			msg_print("The heroism wears off.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->hero = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Recalculate hitpoints */
	p_ptr->update |= (PU_HP);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->holy", notice observable changes
 */
bool set_holy(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
                if (!p_ptr->holy)
		{
                        msg_print("You feel a holy aura around you!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
                if (p_ptr->holy)
		{
                        msg_print("The holy aura vanishes.");
			notice = TRUE;
		}
	}

	/* Use the value */
        p_ptr->holy = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Recalculate hitpoints */
        p_ptr->update |= (PU_LITE);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->walk_water", notice observable changes
 */
bool set_walk_water(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
                if (!p_ptr->walk_water)
		{
                        msg_print("You feel strangely insubmersible!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
                if (p_ptr->walk_water)
		{
                        msg_print("You are no longer insubmersible.");
			notice = TRUE;
		}
	}

	/* Use the value */
        p_ptr->walk_water = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->shero", notice observable changes
 */
bool set_shero(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->shero)
		{
			msg_print("You feel like a killing machine!");
			notice = TRUE;

                        /* Redraw map */
                        p_ptr->redraw |= (PR_MAP);

                        /* Update monsters */
                        p_ptr->update |= (PU_MONSTERS);

                        /* Window stuff */
                        p_ptr->window |= (PW_OVERHEAD);
                }
	}

	/* Shut */
	else
	{
		if (p_ptr->shero)
		{
			msg_print("You feel less Berserk.");
			notice = TRUE;

                        /* Redraw map */
                        p_ptr->redraw |= (PR_MAP);

                        /* Update monsters */
                        p_ptr->update |= (PU_MONSTERS);

                        /* Window stuff */
                        p_ptr->window |= (PW_OVERHEAD);
		}
	}

	/* Use the value */
	p_ptr->shero = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Recalculate hitpoints */
	p_ptr->update |= (PU_HP);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->protevil", notice observable changes
 */
bool set_protevil(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->protevil)
		{
			msg_print("You feel safe from evil!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->protevil)
		{
			msg_print("You no longer feel safe from evil.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->protevil = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->protgood", notice observable changes
 */
bool set_protgood(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->protgood)
		{
			msg_print("You feel safe from good!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->protgood)
		{
			msg_print("You no longer feel safe from good.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->protgood = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->protundead", notice observable changes
 */
bool set_protundead(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
                if (!p_ptr->protundead)
		{
                        msg_print("You feel safe from undead!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
                if (p_ptr->protundead)
		{
                        msg_print("You no longer feel safe from undead.");
			notice = TRUE;
		}
	}

	/* Use the value */
        p_ptr->protundead = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->set_shadow", notice observable changes
 */
bool set_shadow(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
        if (!p_ptr->tim_wraith)
		{

            msg_print("You leave the physical world and turn into a wraith-being!");
			notice = TRUE;

            {
                /* Redraw map */
                p_ptr->redraw |= (PR_MAP);

                /* Update monsters */
                p_ptr->update |= (PU_MONSTERS);

                /* Window stuff */
                p_ptr->window |= (PW_OVERHEAD);
            }
		}
	}

	/* Shut */
	else
	{
        if (p_ptr->tim_wraith)
		{
            msg_print("You feel opaque.");
			notice = TRUE;
            {
                /* Redraw map */
                p_ptr->redraw |= (PR_MAP);

                /* Update monsters */
                p_ptr->update |= (PU_MONSTERS);

                /* Window stuff */
                p_ptr->window |= (PW_OVERHEAD);
            }
		}
	}

	/* Use the value */
    p_ptr->tim_wraith = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);




	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);

}




/*
 * Set "p_ptr->invuln", notice observable changes
 */
bool set_invuln(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->invuln)
		{

            msg_print("Invulnerability!");
			notice = TRUE;

            {
                /* Redraw map */
                p_ptr->redraw |= (PR_MAP);

                /* Update monsters */
                p_ptr->update |= (PU_MONSTERS);

                /* Window stuff */
                p_ptr->window |= (PW_OVERHEAD);
            }
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->invuln)
		{
            msg_print("The invulnerability wears off.");
			notice = TRUE;
            {
                /* Redraw map */
                p_ptr->redraw |= (PR_MAP);

                /* Update monsters */
                p_ptr->update |= (PU_MONSTERS);

                /* Window stuff */
                p_ptr->window |= (PW_OVERHEAD);
            }
		}
	}

	/* Use the value */
	p_ptr->invuln = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);




	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);

}



/*
 * Set "p_ptr->tim_esp", notice observable changes
 */
bool set_tim_esp(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
        if (!p_ptr->tim_esp)
		{
            msg_print("You feel your consciousness expand!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
        if (p_ptr->tim_esp)
		{
            msg_print("Your consciousness contracts again.");
			notice = TRUE;
		}
	}

	/* Use the value */
    p_ptr->tim_esp = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Update the monsters */
	p_ptr->update |= (PU_MONSTERS);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->tim_invis", notice observable changes
 */
bool set_tim_invis(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->tim_invis)
		{
			msg_print("Your eyes feel very sensitive!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->tim_invis)
		{
			msg_print("Your eyes feel less sensitive.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->tim_invis = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Update the monsters */
	p_ptr->update |= (PU_MONSTERS);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->tim_infra", notice observable changes
 */
bool set_tim_infra(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->tim_infra)
		{
			msg_print("Your eyes begin to tingle!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->tim_infra)
		{
			msg_print("Your eyes stop tingling.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->tim_infra = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Update the monsters */
	p_ptr->update |= (PU_MONSTERS);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->tim_mental_barrier", notice observable changes
 */
bool set_mental_barrier(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
                if (!p_ptr->tim_mental_barrier)
		{
                        msg_print("Your mind grows stronger!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
                if (p_ptr->tim_mental_barrier)
		{
                        msg_print("Your mind is no longer especially strong.");
			notice = TRUE;
		}
	}

	/* Use the value */
        p_ptr->tim_mental_barrier = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}

/*
 * Set "p_ptr->oppose_acid", notice observable changes
 */
bool set_oppose_acid(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->oppose_acid)
		{
			msg_print("You feel resistant to acid!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->oppose_acid)
		{
			msg_print("You feel less resistant to acid.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->oppose_acid = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->oppose_elec", notice observable changes
 */
bool set_oppose_elec(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->oppose_elec)
		{
			msg_print("You feel resistant to electricity!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->oppose_elec)
		{
			msg_print("You feel less resistant to electricity.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->oppose_elec = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->oppose_fire", notice observable changes
 */
bool set_oppose_fire(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->oppose_fire)
		{
			msg_print("You feel resistant to fire!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->oppose_fire)
		{
			msg_print("You feel less resistant to fire.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->oppose_fire = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->oppose_cold", notice observable changes
 */
bool set_oppose_cold(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->oppose_cold)
		{
			msg_print("You feel resistant to cold!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->oppose_cold)
		{
			msg_print("You feel less resistant to cold.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->oppose_cold = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->oppose_pois", notice observable changes
 */
bool set_oppose_pois(int v)
{
	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

	/* Open */
	if (v)
	{
		if (!p_ptr->oppose_pois)
		{
			msg_print("You feel resistant to poison!");
			notice = TRUE;
		}
	}

	/* Shut */
	else
	{
		if (p_ptr->oppose_pois)
		{
			msg_print("You feel less resistant to poison.");
			notice = TRUE;
		}
	}

	/* Use the value */
	p_ptr->oppose_pois = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->stun", notice observable changes
 *
 * Note the special code to only notice "range" changes.
 */
bool set_stun(int v)
{
	int old_aux, new_aux;
	bool notice = FALSE;


	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

    if (p_ptr->prace == RACE_ENT)
        v = 0;

	/* Knocked out */
	if (p_ptr->stun > 100)
	{
		old_aux = 3;
	}

	/* Heavy stun */
	else if (p_ptr->stun > 50)
	{
		old_aux = 2;
	}

	/* Stun */
	else if (p_ptr->stun > 0)
	{
		old_aux = 1;
	}

	/* None */
	else
	{
		old_aux = 0;
	}

	/* Knocked out */
	if (v > 100)
	{
		new_aux = 3;
	}

	/* Heavy stun */
	else if (v > 50)
	{
		new_aux = 2;
	}

	/* Stun */
	else if (v > 0)
	{
		new_aux = 1;
	}

	/* None */
	else
	{
		new_aux = 0;
	}

	/* Increase cut */
	if (new_aux > old_aux)
	{
		/* Describe the state */
		switch (new_aux)
		{
			/* Stun */
			case 1:
			msg_print("You have been stunned.");
			break;

			/* Heavy stun */
			case 2:
			msg_print("You have been heavily stunned.");
			break;

			/* Knocked out */
			case 3:
			msg_print("You have been knocked out.");
			break;
		}

        if (randint(1000)<v || randint(16)==1)
        {

            msg_print("A vicious blow hits your head.");
            if(randint(3)==1)
            {
                if (!p_ptr->sustain_int) { (void) do_dec_stat(A_INT, STAT_DEC_NORMAL); }
                if (!p_ptr->sustain_wis) { (void) do_dec_stat(A_WIS, STAT_DEC_NORMAL); }
            }
            else if (randint(2)==1)
            {
                if (!p_ptr->sustain_int) { (void) do_dec_stat(A_INT, STAT_DEC_NORMAL); }
            }
            else
            {
                if (!p_ptr->sustain_wis) { (void) do_dec_stat(A_WIS, STAT_DEC_NORMAL); }
            }
        }

		/* Notice */
		notice = TRUE;
	}

	/* Decrease cut */
	else if (new_aux < old_aux)
	{
		/* Describe the state */
		switch (new_aux)
		{
			/* None */
			case 0:
			msg_print("You are no longer stunned.");
			if (disturb_state) disturb(0, 0);
			break;
		}

		/* Notice */
		notice = TRUE;
	}

	/* Use the value */
	p_ptr->stun = v;

	/* No change */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Redraw the "stun" */
	p_ptr->redraw |= (PR_STUN);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->cut", notice observable changes
 *
 * Note the special code to only notice "range" changes.
 */
bool set_cut(int v)
{
	int old_aux, new_aux;

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 10000) ? 10000 : (v < 0) ? 0 : v;

        if ((p_ptr->pracem == RMOD_SPECTRE) || (p_ptr->pracem == RMOD_SKELETON))
                v = 0;

        /* Mortal wound */
	if (p_ptr->cut > 1000)
	{
		old_aux = 7;
	}

	/* Deep gash */
	else if (p_ptr->cut > 200)
	{
		old_aux = 6;
	}

	/* Severe cut */
	else if (p_ptr->cut > 100)
	{
		old_aux = 5;
	}

	/* Nasty cut */
	else if (p_ptr->cut > 50)
	{
		old_aux = 4;
	}

	/* Bad cut */
	else if (p_ptr->cut > 25)
	{
		old_aux = 3;
	}

	/* Light cut */
	else if (p_ptr->cut > 10)
	{
		old_aux = 2;
	}

	/* Graze */
	else if (p_ptr->cut > 0)
	{
		old_aux = 1;
	}

	/* None */
	else
	{
		old_aux = 0;
	}

	/* Mortal wound */
	if (v > 1000)
	{
		new_aux = 7;
	}

	/* Deep gash */
	else if (v > 200)
	{
		new_aux = 6;
	}

	/* Severe cut */
	else if (v > 100)
	{
		new_aux = 5;
	}

	/* Nasty cut */
	else if (v > 50)
	{
		new_aux = 4;
	}

	/* Bad cut */
	else if (v > 25)
	{
		new_aux = 3;
	}

	/* Light cut */
	else if (v > 10)
	{
		new_aux = 2;
	}

	/* Graze */
	else if (v > 0)
	{
		new_aux = 1;
	}

	/* None */
	else
	{
		new_aux = 0;
	}

	/* Increase cut */
	if (new_aux > old_aux)
	{
		/* Describe the state */
		switch (new_aux)
		{
			/* Graze */
			case 1:
			msg_print("You have been given a graze.");
			break;

			/* Light cut */
			case 2:
			msg_print("You have been given a light cut.");
			break;

			/* Bad cut */
			case 3:
			msg_print("You have been given a bad cut.");
			break;

			/* Nasty cut */
			case 4:
			msg_print("You have been given a nasty cut.");
			break;

			/* Severe cut */
			case 5:
			msg_print("You have been given a severe cut.");
			break;

			/* Deep gash */
			case 6:
			msg_print("You have been given a deep gash.");
			break;

			/* Mortal wound */
			case 7:
			msg_print("You have been given a mortal wound.");
			break;
		}

		/* Notice */
		notice = TRUE;

    if (randint(1000)<v || randint(16)==1)
        { 
            if(!p_ptr->sustain_chr)
            {
            msg_print("You have been horribly scarred.");

            do_dec_stat(A_CHR, STAT_DEC_NORMAL);
            }
        }

	}

	/* Decrease cut */
	else if (new_aux < old_aux)
	{
		/* Describe the state */
		switch (new_aux)
		{
			/* None */
			case 0:
			msg_print("You are no longer bleeding.");
			if (disturb_state) disturb(0, 0);
			break;
		}

		/* Notice */
		notice = TRUE;
	}

	/* Use the value */
	p_ptr->cut = v;

	/* No change */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Redraw the "cut" */
	p_ptr->redraw |= (PR_CUT);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}


/*
 * Set "p_ptr->food", notice observable changes
 *
 * The "p_ptr->food" variable can get as large as 20000, allowing the
 * addition of the most "filling" item, Elvish Waybread, which adds
 * 7500 food units, without overflowing the 32767 maximum limit.
 *
 * Perhaps we should disturb the player with various messages,
 * especially messages about hunger status changes.  XXX XXX XXX
 *
 * Digestion of food is handled in "dungeon.c", in which, normally,
 * the player digests about 20 food units per 100 game turns, more
 * when "fast", more when "regenerating", less with "slow digestion",
 * but when the player is "gorged", he digests 100 food units per 10
 * game turns, or a full 1000 food units per 100 game turns.
 *
 * Note that the player's speed is reduced by 10 units while gorged,
 * so if the player eats a single food ration (5000 food units) when
 * full (15000 food units), he will be gorged for (5000/100)*10 = 500
 * game turns, or 500/(100/5) = 25 player turns (if nothing else is
 * affecting the player speed).
 */
bool set_food(int v)
{
	int old_aux, new_aux;

	bool notice = FALSE;

	/* Hack -- Force good values */
	v = (v > 20000) ? 20000 : (v < 0) ? 0 : v;

	/* Fainting / Starving */
	if (p_ptr->food < PY_FOOD_FAINT)
	{
		old_aux = 0;
	}

	/* Weak */
	else if (p_ptr->food < PY_FOOD_WEAK)
	{
		old_aux = 1;
	}

	/* Hungry */
	else if (p_ptr->food < PY_FOOD_ALERT)
	{
		old_aux = 2;
	}

	/* Normal */
	else if (p_ptr->food < PY_FOOD_FULL)
	{
		old_aux = 3;
	}

	/* Full */
	else if (p_ptr->food < PY_FOOD_MAX)
	{
		old_aux = 4;
	}

	/* Gorged */
	else
	{
		old_aux = 5;
	}

	/* Fainting / Starving */
	if (v < PY_FOOD_FAINT)
	{
		new_aux = 0;
	}

	/* Weak */
	else if (v < PY_FOOD_WEAK)
	{
		new_aux = 1;
	}

	/* Hungry */
	else if (v < PY_FOOD_ALERT)
	{
		new_aux = 2;
	}

	/* Normal */
	else if (v < PY_FOOD_FULL)
	{
		new_aux = 3;
	}

	/* Full */
	else if (v < PY_FOOD_MAX)
	{
		new_aux = 4;
	}

	/* Gorged */
	else
	{
		new_aux = 5;
	}

	/* Food increase */
	if (new_aux > old_aux)
	{
		/* Describe the state */
		switch (new_aux)
		{
			/* Weak */
			case 1:
			msg_print("You are still weak.");
			break;

			/* Hungry */
			case 2:
			msg_print("You are still hungry.");
			break;

			/* Normal */
			case 3:
			msg_print("You are no longer hungry.");
			break;

			/* Full */
			case 4:
			msg_print("You are full!");
			break;

			/* Bloated */
			case 5:
			msg_print("You have gorged yourself!");
			break;
		}

		/* Change */
		notice = TRUE;
	}

	/* Food decrease */
	else if (new_aux < old_aux)
	{
		/* Describe the state */
		switch (new_aux)
		{
			/* Fainting / Starving */
			case 0:
			msg_print("You are getting faint from hunger!");
			break;

			/* Weak */
			case 1:
			msg_print("You are getting weak from hunger!");
			break;

			/* Hungry */
			case 2:
			msg_print("You are getting hungry.");
			break;

			/* Normal */
			case 3:
			msg_print("You are no longer full.");
			break;

			/* Full */
			case 4:
			msg_print("You are no longer gorged.");
			break;
		}

		/* Change */
		notice = TRUE;
	}

	/* Use the value */
	p_ptr->food = v;

	/* Nothing to notice */
	if (!notice) return (FALSE);

	/* Disturb */
	if (disturb_state) disturb(0, 0);

	/* Recalculate bonuses */
	p_ptr->update |= (PU_BONUS);

	/* Redraw hunger */
	p_ptr->redraw |= (PR_HUNGER);

	/* Handle stuff */
	handle_stuff();

	/* Result */
	return (TRUE);
}


/*
 * Advance experience levels and print experience
 */
void check_experience(void)
{
	int		i;
	bool level_reward = FALSE;
        bool level_corruption = FALSE;


	/* Note current level */
	i = p_ptr->lev;

	/* Hack -- lower limit */
	if (p_ptr->exp < 0) p_ptr->exp = 0;

	/* Hack -- lower limit */
	if (p_ptr->max_exp < 0) p_ptr->max_exp = 0;

	/* Hack -- upper limit */
	if (p_ptr->exp > PY_MAX_EXP) p_ptr->exp = PY_MAX_EXP;

	/* Hack -- upper limit */
	if (p_ptr->max_exp > PY_MAX_EXP) p_ptr->max_exp = PY_MAX_EXP;

	/* Hack -- maintain "max" experience */
	if (p_ptr->exp > p_ptr->max_exp) p_ptr->max_exp = p_ptr->exp;

	/* Redraw experience */
	p_ptr->redraw |= (PR_EXP);

	/* Handle stuff */
	handle_stuff();


	/* Lose levels while possible */
	while ((p_ptr->lev > 1) &&
	       (p_ptr->exp < (player_exp[p_ptr->lev-2] * p_ptr->expfact / 100L)))
	{
		/* Lose a level */
		p_ptr->lev--;
		lite_spot(py, px);

		/* Update some stuff */
                p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS | PU_SANITY);

		/* Redraw some stuff */
                p_ptr->redraw |= (PR_LEV | PR_TITLE | PR_EXP);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER);

		/* Handle stuff */
		handle_stuff();
	}


	/* Gain levels while possible */
	while ((p_ptr->lev < PY_MAX_LEVEL) &&
	       (p_ptr->exp >= (player_exp[p_ptr->lev-1] * p_ptr->expfact / 100L)))
	{
		/* Gain a level */
		p_ptr->lev++;
		lite_spot(py, px);

		/* Save the highest level */
		if (p_ptr->lev > p_ptr->max_plv)
		{
			p_ptr->max_plv = p_ptr->lev;
			if (p_ptr->pclass == CLASS_CHAOS_WARRIOR)
			{
				level_reward = TRUE;
			}
			if (p_ptr->muta2 & MUT2_CHAOS_GIFT)
			{
				level_reward = TRUE;
			}
                        if ((p_ptr->pracem == RMOD_MUTANT) &&
                            (randint(3) == 1))
                        {
                                level_corruption = TRUE;
                        }
                }

		/* Sound */
		sound(SOUND_LEVEL);

		/* Message */
                cmsg_format(TERM_L_GREEN, "Welcome to level %d.", p_ptr->lev);

                /* If auto-note taking enabled, write a note to the file. 
                 * Only write this note when the level is gained for the first
                 * time.
                 */
                if (take_notes && auto_notes)
                {
                  char note[80];

		  /* Write note */
		  sprintf(note, "Reached level %d", p_ptr->lev);

		  add_note(note, 'L');

                }

		/* Update some stuff */
                p_ptr->update |= (PU_BONUS | PU_HP | PU_MANA | PU_SPELLS | PU_SANITY);

		/* Redraw some stuff */
                p_ptr->redraw |= (PR_LEV | PR_TITLE | PR_EXP);

		/* Window stuff */
		p_ptr->window |= (PW_PLAYER | PW_SPELL);

		/* Handle stuff */
		handle_stuff();

		if(level_reward)
		{
			gain_level_reward(0);
			level_reward = FALSE;
		}

                if (level_corruption)
		{
			msg_print("You feel different...");
                        corrupt_corrupted();
                        level_corruption = FALSE;
		}
	}

        /* Hook it ! */
        process_hooks(HOOK_PLAYER_LEVEL, 0);
}
/*
 * Advance experience levels and print experience
 */
void check_experience_obj(object_type *o_ptr)
{
	int		i;

	/* Note current level */
        i = o_ptr->elevel;

	/* Hack -- lower limit */
        if (o_ptr->exp < 0) o_ptr->exp = 0;

	/* Hack -- upper limit */
        if (o_ptr->exp > PY_MAX_EXP) o_ptr->exp = PY_MAX_EXP;

	/* Gain levels while possible */
        while ((o_ptr->elevel < PY_MAX_LEVEL) &&
               (o_ptr->exp >= (player_exp[o_ptr->elevel - 1] * 5 / 2)))
	{
                char buf[100];

                /* Add a level */
                o_ptr->elevel++;

                /* Get object name */
                object_desc(buf, o_ptr, 1, 0);
                cmsg_format(TERM_L_BLUE, "%s gains a level!", buf);

                /* What does it gains ? */
                object_gain_level(o_ptr);
	}
}


/*
 * Gain experience (share it to objecst if needed)
 */
void gain_exp(s32b amount)
{
        int i, num = 1;

        /* Count the gaining xp objects */
        for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
        {
                object_type *o_ptr = &inventory[i];
                u32b f1, f2, f3, f4, f5, esp;

                object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

                if (!o_ptr->k_idx) continue;

                if (f4 & TR4_ART_EXP) num++;
        }

        /* Now give the xp */
        for (i = INVEN_WIELD; i < INVEN_TOTAL; i++)
        {
                object_type *o_ptr = &inventory[i];
                u32b f1, f2, f3, f4, f5, esp;

                object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

                if (!o_ptr->k_idx) continue;

                if (f4 & TR4_ART_EXP)
                {
                        o_ptr->exp += 2 * amount / (num * 3);

                        /* Hack -- upper limit */
                        if (o_ptr->exp > PY_MAX_EXP) o_ptr->exp = PY_MAX_EXP;
                }
        }

	  if ((p_ptr->max_exp > 0) && (p_ptr->pracem == RMOD_MUTANT))
	  {
		if ((randint(p_ptr->max_exp) < amount) || (randint(12000000) < amount))
		{
			msg_print("You feel different...");
                        corrupt_corrupted();
		};
		/* 12,000,000 is equal to double Morgoth's raw XP value (60,000 * his Dlev (100))*/
	  };

        /* Gain some experience */
        p_ptr->exp += amount / num;

        /* Slowly recover from experience drainage */
        if (p_ptr->exp < p_ptr->max_exp)
        {
                /* Gain max experience (20%) (was 10%) */
                p_ptr->max_exp += amount / 5;
        }

	/* Check Experience */
	check_experience();
}


/*
 * Lose experience
 */
void lose_exp(s32b amount)
{
	/* Never drop below zero experience */
	if (amount > p_ptr->exp) amount = p_ptr->exp;

	/* Lose some experience */
	p_ptr->exp -= amount;

	/* Check Experience */
	check_experience();
}




/*
 * Hack -- Return the "automatic coin type" of a monster race
 * Used to allocate proper treasure when "Creeping coins" die
 *
 * XXX XXX XXX Note the use of actual "monster names"
 */
int get_coin_type(monster_race *r_ptr)
{
	cptr name = (r_name + r_ptr->name);

	/* Analyze "coin" monsters */
	if (r_ptr->d_char == '$')
	{
		/* Look for textual clues */
		if (strstr(name, " copper ")) return (2);
		if (strstr(name, " silver ")) return (5);
		if (strstr(name, " gold ")) return (10);
		if (strstr(name, " mithril ")) return (16);
		if (strstr(name, " adamantite ")) return (17);

		/* Look for textual clues */
		if (strstr(name, "Copper ")) return (2);
		if (strstr(name, "Silver ")) return (5);
		if (strstr(name, "Gold ")) return (10);
		if (strstr(name, "Mithril ")) return (16);
		if (strstr(name, "Adamantite ")) return (17);
	}

	/* Assume nothing */
	return (0);
}

/*
 * This routine handles the production of corpses/skeletons/heads/skulls
 * when a monster is killed.
 */
void place_corpse(monster_type *m_ptr)
{
   monster_race *r_ptr = race_inf(m_ptr);

   object_type *i_ptr;
   object_type object_type_body;

   int x = m_ptr->fx;
   int y = m_ptr->fy;
#if 0 /* If I can find some time to implement the decapitation ... */
   object_type *w_ptr = &inventory[INVEN_WIELD];

   int i = w_ptr->weight + ((p_ptr->to_h + w_ptr->to_h) * 5) + (p_ptr->lev * 3);

	/* Handle decapitations. This is not allowed with hafted weapons. */
   bool crit = (randint(5000) <= i);

   bool decapitate = ((rand_int(m_ptr->maxhp) <= -(m_ptr->hp)) &&
                       (w_ptr->tval != TV_HAFTED) && crit);
#endif

  	/* Get local object */
	i_ptr = &object_type_body;

   /* It has a physical form */
   if (r_ptr->flags9 & RF9_DROP_CORPSE)
   {
		/* Wipe the object */
                object_prep(i_ptr, lookup_kind(TV_CORPSE, SV_CORPSE_CORPSE));

     	/* Unique corpses are unique */
     	if (r_ptr->flags1 & RF1_UNIQUE)
     	{
        	object_aware(i_ptr);
                i_ptr->name1 = 201;
     	}

     	/* Calculate length of time before decay */
        i_ptr->pval = r_ptr->weight + rand_int(r_ptr->weight);

		/* Set weight */
        i_ptr->weight = (r_ptr->weight + rand_int(r_ptr->weight) / 10) + 1;

     	/* Remember what we are */
        i_ptr->pval2 = m_ptr->r_idx;

        /* Some hp */
        i_ptr->pval3 = ((maxroll(r_ptr->hdice, r_ptr->hside) + p_ptr->mhp) / 2);
        i_ptr->pval3 -= randint(i_ptr->pval3) / 3;

		/* Drop it in the dungeon */
		drop_near(i_ptr, -1, y, x);
   }

   /* The creature is an animated skeleton. */
   if (!(r_ptr->flags9 & RF9_DROP_CORPSE) && (r_ptr->flags9 & RF9_DROP_SKELETON))
   {
		/* Wipe the object */
                object_prep(i_ptr, lookup_kind(TV_CORPSE, SV_CORPSE_SKELETON));

     	/* Unique corpses are unique */
     	if (r_ptr->flags1 & RF1_UNIQUE)
     	{
			object_aware(i_ptr);
                i_ptr->name1 = 201;
     	}

      i_ptr->pval = 0;

		/* Set weight */
        i_ptr->weight = (r_ptr->weight / 4 + rand_int(r_ptr->weight) / 40) + 1;

     	/* Remember what we are */
        i_ptr->pval2 = m_ptr->r_idx;

		/* Drop it in the dungeon */
		drop_near(i_ptr, -1, y, x);
   }

#if 0 /* If I can find some time to implement the decapitation ... */
   /*
    * Decapitated it if it has a head, or if it *is* a head.
    * This is rather messy.
    */
   if ((!(r_ptr->flags9 & RF9_HAS_NO_HEAD)) && (decapitate ||
         (!(r_ptr->flags9 & RF9_DROP_CORPSE) &&
       !(r_ptr->flags9 & RF9_DROP_SKELETON))))
   {
		/* Wipe the object */
                object_prep(i_ptr, lookup_kind(TV_CORPSE, SV_CORPSE_HEAD));

     	/* Unique heads are unique */
      if (r_ptr->flags1 & RF1_UNIQUE)
      {
         object_aware(i_ptr);
                i_ptr->name1 = 201;
		}

      /* Calculate length of time before decay */
        i_ptr->pval = r_ptr->weight / 30 + rand_int(r_ptr->weight) / 30;

		/* Set weight */
           i_ptr->weight = (r_ptr->weight / 30 + rand_int(r_ptr->weight) / 300) + 1;

      /* Remember what we are */
        i_ptr->pval2 = m_ptr->r_idx;

		/* Drop it in the dungeon */
		drop_near(i_ptr, -1, y, x);
   }

   /* It has a skull, but no head */
   if (!(r_ptr->flags9 & RF9_HAS_NO_HEAD) && (!(r_ptr->flags9 & RF9_HAS_NO_SKULL)) &&
       (decapitate || (!(r_ptr->flags9 & RF9_DROP_CORPSE) &&
       !(r_ptr->flags9 & RF9_DROP_SKELETON))))
   {
		/* Wipe the object */
                object_prep(i_ptr, lookup_kind(TV_CORPSE, SV_CORPSE_SKULL));

     	/* Unique heads are unique */
      if (r_ptr->flags1 & RF1_UNIQUE)
		{
			object_aware(i_ptr);
                        i_ptr->name1 = 201;
		}

		i_ptr->pval = 0;

		/* Set weight */
                i_ptr->weight = (r_ptr->weight / 60 + rand_int(r_ptr->weight) / 600) + 1;

		/* Remember what we are */
                i_ptr->pval2 = m_ptr->r_idx;

		/* Drop it in the dungeon */
		drop_near(i_ptr, -1, y, x);
	}
#endif
}


/*
 * Handle the "death" of a monster.
 *
 * Disperse treasures centered at the monster location based on the
 * various flags contained in the monster flags fields.
 *
 * Check for "Quest" completion when a quest monster is killed.
 *
 * Note that only the player can induce "monster_death()" on Uniques.
 * Thus (for now) all Quest monsters should be Uniques.
 *
 * Note that monsters can now carry objects, and when a monster dies,
 * it drops all of its objects, which may disappear in crowded rooms.
 */
void monster_death(int m_idx)
{
        int i, y, x, ny, nx;

	int dump_item = 0;
	int dump_gold = 0;

	s16b this_o_idx, next_o_idx = 0;

	monster_type *m_ptr = &m_list[m_idx];

        monster_race *r_ptr = race_inf(m_ptr);

	bool visible = (m_ptr->ml || (r_ptr->flags1 & (RF1_UNIQUE)));


	bool cloned = FALSE;
	bool create_stairs = FALSE;
	int force_coin = get_coin_type(r_ptr);

	object_type forge;
	object_type *q_ptr;

	/* Get the location */
	y = m_ptr->fy;
	x = m_ptr->fx;

        /* Process the appropriate hooks */
        process_hooks(HOOK_MONSTER_DEATH, m_idx);

        /* If companion dies, take note */
        if (m_ptr->status == MSTATUS_COMPANION) p_ptr->companion_killed++;

        /* Handle reviving if undead */
        if((p_ptr->class_extra3 & CLASS_UNDEAD) && p_ptr->class_extra4)
        {
                p_ptr->class_extra4--;

                if(!p_ptr->class_extra4)
                {
                        msg_print("Your death has been avenged -- you return to life.");
                        p_ptr->class_extra3 &= ~CLASS_UNDEAD;

                        /* Display the hitpoints */
                        p_ptr->update |= (PU_HP);
                        p_ptr->redraw |= (PR_HP);

                        /* Window stuff */
                        p_ptr->window |= (PW_PLAYER);
                }
                else
                {
                        msg_format("You still have to kill %d monster%s.", p_ptr->class_extra4, (p_ptr->class_extra4 == 1)?"":"s");
                }
        }

	/* Handle the possibility of player vanquishing arena combatant -KMW- */
	if (p_ptr->inside_arena)
	{
		p_ptr->exit_bldg = TRUE;
		msg_print("Victorious! You're on your way to becoming Champion.");
		p_ptr->arena_number++;
	}

	if (m_ptr->smart &(SM_CLONED))
		cloned = TRUE;

        /* If the doppleganger die, the variable must be set accordingly */
        if (r_ptr->flags9 & RF9_DOPPLEGANGER)
                doppleganger = 0;

	/* Drop objects being carried */
	for (this_o_idx = m_ptr->hold_o_idx; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type *o_ptr;
		
		/* Acquire object */
		o_ptr = &o_list[this_o_idx];

		/* Acquire next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Paranoia */
		o_ptr->held_m_idx = 0;

		/* Get local object */
		q_ptr = &forge;

		/* Copy the object */
		object_copy(q_ptr, o_ptr);

		/* Delete the object */
		delete_object_idx(this_o_idx);

                if(q_ptr->tval == TV_GOLD) dump_gold++;
                else dump_item++;

		/* Drop it */
		drop_near(q_ptr, -1, y, x);
	}

	/* Forget objects */
	m_ptr->hold_o_idx = 0;

	/* Average dungeon and monster levels */
        object_level = (dun_level + m_ptr->level) / 2;

	/* Mega^2-hack -- destroying the Stormbringer gives it us! */
	if (strstr((r_name + r_ptr->name),"Stormbringer"))
	{
		/* Get local object */
		q_ptr = &forge;

		/* Prepare to make the Stormbringer */
		object_prep(q_ptr, lookup_kind(TV_SWORD, SV_BLADE_OF_CHAOS));

		/* Mega-Hack -- Name the sword  */

		q_ptr->art_name = quark_add("'Stormbringer'");
		q_ptr->to_h = 16;
		q_ptr->to_d = 16;
		q_ptr->ds = 6;
		q_ptr->dd = 6;
		q_ptr->pval = 2;

		q_ptr->art_flags1 |= ( TR1_VAMPIRIC | TR1_STR | TR1_CON | TR1_BLOWS );
		q_ptr->art_flags2 |= ( TR2_FREE_ACT | TR2_HOLD_LIFE |
		                       TR2_RES_NEXUS | TR2_RES_CHAOS | TR2_RES_NETHER |
		                       TR2_RES_CONF ); /* No longer resist_disen */
		q_ptr->art_flags3 |= ( TR3_IGNORE_ACID | TR3_IGNORE_ELEC |
		                       TR3_IGNORE_FIRE | TR3_IGNORE_COLD);
		/* Just to be sure */

		q_ptr->art_flags3 |= TR3_NO_TELE; /* How's that for a downside? */

		/* For game balance... */
		q_ptr->art_flags3 |= (TR3_CURSED | TR3_HEAVY_CURSE);
		q_ptr->ident |= IDENT_CURSED;

		if (randint(2)==1)
			q_ptr->art_flags3 |= (TR3_DRAIN_EXP);
		else
			q_ptr->art_flags3 |= (TR3_AGGRAVATE);

		/* Drop it in the dungeon */
		drop_near(q_ptr, -1, y, x);
	}

	/*
	 * Mega^3-hack: killing a 'Warrior of the Dawn' is likely to
	 * spawn another in the fallen one's place!
	 */
	else if (strstr((r_name + r_ptr->name),"the Dawn"))
	{
		if (!(randint(20)==13))
		{
			int wy = py, wx = px;
			int attempts = 100;

			do
			{
				scatter(&wy, &wx, py, px, 20, 0);
			}
			while (!(in_bounds(wy,wx) && cave_floor_bold(wy,wx)) && --attempts);

			if (attempts > 0)
			{
                                if (is_friend(m_ptr) > 0)
				{
					if (summon_specific_friendly(wy, wx, 100, SUMMON_DAWN, FALSE))
					{
						if (player_can_see_bold(wy, wx))
							msg_print ("A new warrior steps forth!");
					}
				}
				else
				{
					if (summon_specific(wy, wx, 100, SUMMON_DAWN))
					{
						if (player_can_see_bold(wy, wx))
							msg_print ("A new warrior steps forth!");
					}
				}
			}
		}
	}

	/* One more ultra-hack: An Unmaker goes out with a big bang! */
	else if (strstr((r_name + r_ptr->name),"Unmaker"))
	{
		int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;
		(void)project(m_idx, 6, y, x, 100, GF_CHAOS, flg);
	}

        /* Raal's Tomes of Destruction drop a Raal's Tome of Destruction */
        else if ((strstr((r_name + r_ptr->name),"Raal's Tome of Destruction")) && (rand_int(100) < 20))
	{
		/* Get local object */
		q_ptr = &forge;

                /* Prepare to make a Raal's Tome of Destruction */
                object_prep(q_ptr, lookup_kind(TV_MAGIC_BOOK, 8));

		/* Drop it in the dungeon */
                drop_near(q_ptr, -1, y, x);
	}

	/* Pink horrors are replaced with 2 Blue horrors */
	else if (strstr((r_name + r_ptr->name),"ink horror"))
	{
                for (i = 0; i < 2; i++)
		{
			int wy = py, wx = px;
			int attempts = 100;

			do
			{
				scatter(&wy, &wx, py, px, 3, 0);
			}
			while (!(in_bounds(wy,wx) && cave_floor_bold(wy,wx)) && --attempts);

			if (attempts > 0)
			{
                                if (summon_specific(wy, wx, 100, SUMMON_BLUE_HORROR))
				{
					if (player_can_see_bold(wy, wx))
						msg_print ("A blue horror appears!");
				}
                        }                
                }
	}

	/* Mega-Hack -- drop "winner" treasures */
	else if (r_ptr->flags1 & (RF1_DROP_CHOSEN))
	{
                if (strstr((r_name + r_ptr->name),"Morgoth, Lord of Darkness"))
		{
			/* Get local object */
			q_ptr = &forge;

			/* Mega-Hack -- Prepare to make "Grond" */
			object_prep(q_ptr, lookup_kind(TV_HAFTED, SV_GROND));

			/* Mega-Hack -- Mark this item as "Grond" */
			q_ptr->name1 = ART_GROND;

			/* Mega-Hack -- Actually create "Grond" */
			apply_magic(q_ptr, -1, TRUE, TRUE, TRUE);

			/* Drop it in the dungeon */
			drop_near(q_ptr, -1, y, x);

			/* Get local object */
			q_ptr = &forge;

			/* Mega-Hack -- Prepare to make "Morgoth" */
			object_prep(q_ptr, lookup_kind(TV_CROWN, SV_MORGOTH));

			/* Mega-Hack -- Mark this item as "Morgoth" */
			q_ptr->name1 = ART_MORGOTH;

			/* Mega-Hack -- Actually create "Morgoth" */
			apply_magic(q_ptr, -1, TRUE, TRUE, TRUE);

			/* Drop it in the dungeon */
			drop_near(q_ptr, -1, y, x);
		}
                else if (strstr((r_name + r_ptr->name),"Smeagol"))
                {
			/* Get local object */
			q_ptr = &forge;

                        object_wipe(q_ptr);

                        /* Mega-Hack -- Prepare to make a ring of invisibility */
                        object_prep(q_ptr, lookup_kind(TV_RING, SV_RING_INVIS));
                        q_ptr->number = 1;

                        apply_magic(q_ptr, -1, TRUE, TRUE, FALSE);

			/* Drop it in the dungeon */
			drop_near(q_ptr, -1, y, x);
                }
                else if (r_ptr->flags7 & RF7_NAZGUL)
                {
			/* Get local object */
			q_ptr = &forge;

                        object_wipe(q_ptr);

                        /* Mega-Hack -- Prepare to make a Ring of Power */
                        object_prep(q_ptr, lookup_kind(TV_RING, SV_RING_SPECIAL));
                        q_ptr->number = 1;

                        apply_magic(q_ptr, -1, TRUE, TRUE, FALSE);

                        /* Create a random artifact */
                        create_artifact(q_ptr, TRUE, FALSE);

                        /* Save the inscription */
                        q_ptr->art_name = quark_add(format("(%s)", r_name + r_ptr->name));

			/* Drop it in the dungeon */
			drop_near(q_ptr, -1, y, x);
                }
                else
		{
			byte a_idx = 0;
			int chance = 0;
			int I_kind = 0;

                        if (strstr((r_name + r_ptr->name),"T'ron , the rebel DragonRider"))
			{
                                a_idx = ART_TRON;
                                chance = 75;
			}
                        else if (strstr((r_name + r_ptr->name),"Mardra, rider of the Gold Loranth"))
			{
                                a_idx = ART_MARDA;
                                chance = 50;
			}
                        else if (strstr((r_name + r_ptr->name),"Saruman of Many Colours"))
			{
				a_idx = ART_ELENDIL;
                                chance = 30;
			}
                        else if (strstr((r_name + r_ptr->name),"Hagen, son of Alberich"))
			{
				a_idx = ART_NIMLOTH;
				chance = 66;
                        }
                        else if (strstr((r_name + r_ptr->name),"Muar, the Balrog"))
			{
                                a_idx = ART_CALRIS;
                                chance = 60;
			}
                        else if (strstr((r_name + r_ptr->name),"Gothmog, the High Captain of Balrogs"))
			{
                                a_idx = ART_GOTHMOG;
                                chance = 50;
			}
                        else if (strstr((r_name + r_ptr->name),"Eol the Dark Elf"))
			{
                                a_idx = ART_ANGUIREL;
                                chance = 50;
			}

			if ((a_idx > 0) && ((randint(99)<chance) || (wizard)))
			{
				if (a_info[a_idx].cur_num == 0)
				{
					artifact_type *a_ptr = &a_info[a_idx];

					/* Get local object */
					q_ptr = &forge;

					/* Wipe the object */
					object_wipe(q_ptr);

					/* Acquire the "kind" index */
					I_kind = lookup_kind(a_ptr->tval, a_ptr->sval);

					/* Create the artifact */
					object_prep(q_ptr, I_kind);

					/* Save the name */
					q_ptr->name1 = a_idx;

					/* Extract the fields */
					q_ptr->pval = a_ptr->pval;
					q_ptr->ac = a_ptr->ac;
					q_ptr->dd = a_ptr->dd;
					q_ptr->ds = a_ptr->ds;
					q_ptr->to_a = a_ptr->to_a;
					q_ptr->to_h = a_ptr->to_h;
					q_ptr->to_d = a_ptr->to_d;
					q_ptr->weight = a_ptr->weight;

					/* Hack -- acquire "cursed" flag */
					if (a_ptr->flags3 & (TR3_CURSED)) q_ptr->ident |= (IDENT_CURSED);

					random_artifact_resistance(q_ptr);

					a_info[a_idx].cur_num = 1;

					/* Drop the artifact from heaven */
					drop_near(q_ptr, -1, y, x);
				}
			}
		}
	}

        /* Hack - the Dragonriders give some firestone */
        else if (r_ptr->flags3 & RF3_DRAGONRIDER)
	{
		/* Get local object */
		q_ptr = &forge;

                /* Prepare to make some Firestone */
                object_prep(q_ptr, lookup_kind(TV_FIRESTONE, SV_FIRESTONE));
                q_ptr->number = (byte)rand_range(10,20);

		/* Drop it in the dungeon */
                drop_near(q_ptr, -1, y, x);
	}

        /* Hack - the protected monsters must be advanged */
        else if (r_ptr->flags9 & RF9_WYRM_PROTECT)
	{
                int xx = x,yy = y;
                int attempts = 100;

                cmsg_print(TERM_VIOLET, "This monster was under the protection of a great wyrm of power!");

                do
                {
                        scatter(&yy, &xx, y, x, 6, 0);
                }
                while (!(in_bounds(yy, xx) && cave_floor_bold(yy, xx)) && --attempts);

                place_monster_aux(yy, xx, test_monster_name("Great Wyrm of Power"), FALSE, FALSE, m_ptr->status);
	}

	/* Let monsters explode! */
	for (i = 0; i < 4; i++)
	{
                if (m_ptr->blow[i].method == RBM_EXPLODE)
		{
			int flg = PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;
			int typ = GF_MISSILE;
                        int d_dice = m_ptr->blow[i].d_dice;
                        int d_side = m_ptr->blow[i].d_side;
			int damage = damroll(d_dice, d_side);

                        switch (m_ptr->blow[i].effect)
			{
				case RBE_HURT:      typ = GF_MISSILE; break;
				case RBE_POISON:    typ = GF_POIS; break;
				case RBE_UN_BONUS:  typ = GF_DISENCHANT; break;
				case RBE_UN_POWER:  typ = GF_MISSILE; break; /* ToDo: Apply the correct effects */
				case RBE_EAT_GOLD:  typ = GF_MISSILE; break;
				case RBE_EAT_ITEM:  typ = GF_MISSILE; break;
				case RBE_EAT_FOOD:  typ = GF_MISSILE; break;
				case RBE_EAT_LITE:  typ = GF_MISSILE; break;
				case RBE_ACID:      typ = GF_ACID; break;
				case RBE_ELEC:      typ = GF_ELEC; break;
				case RBE_FIRE:      typ = GF_FIRE; break;
				case RBE_COLD:      typ = GF_COLD; break;
				case RBE_BLIND:     typ = GF_MISSILE; break;
                                case RBE_HALLU:     typ = GF_CONFUSION; break;
				case RBE_CONFUSE:   typ = GF_CONFUSION; break;
				case RBE_TERRIFY:   typ = GF_MISSILE; break;
				case RBE_PARALYZE:  typ = GF_MISSILE; break;
				case RBE_LOSE_STR:  typ = GF_MISSILE; break;
				case RBE_LOSE_DEX:  typ = GF_MISSILE; break;
				case RBE_LOSE_CON:  typ = GF_MISSILE; break;
				case RBE_LOSE_INT:  typ = GF_MISSILE; break;
				case RBE_LOSE_WIS:  typ = GF_MISSILE; break;
				case RBE_LOSE_CHR:  typ = GF_MISSILE; break;
				case RBE_LOSE_ALL:  typ = GF_MISSILE; break;
                                case RBE_PARASITE:  typ = GF_MISSILE; break;
				case RBE_SHATTER:   typ = GF_ROCKET; break;
				case RBE_EXP_10:    typ = GF_MISSILE; break;
				case RBE_EXP_20:    typ = GF_MISSILE; break;
				case RBE_EXP_40:    typ = GF_MISSILE; break;
				case RBE_EXP_80:    typ = GF_MISSILE; break;
				case RBE_DISEASE:   typ = GF_POIS; break;
				case RBE_TIME:      typ = GF_TIME; break;
                                case RBE_SANITY:    typ = GF_MISSILE; break;
			}

			project(m_idx, 3, y, x, damage, typ, flg);
			break;
		}
	}

        if((!force_coin)&&(randint(100)<50)) place_corpse(m_ptr);

	/* Take note of any dropped treasure */
	if (visible && (dump_item || dump_gold))
	{
		/* Take notes on treasure */
		lore_treasure(m_idx, dump_item, dump_gold);
	}

        /* Current quest */
	i = p_ptr->inside_quest;

	/* Create a magical staircase */
        if (create_stairs && (dun_level < d_info[dungeon_type].maxdepth))
	{
                int i, j;

                for(i = -1; i <= 1; i++)
                for(j = -1; j <= 1; j++)
                        if(!(f_info[cave[y + j][x + i].feat].flags1 & FF1_PERMANENT)) cave_set_feat(y + j, x + i, d_info[dungeon_type].floor1);

		/* Stagger around */
		while (!cave_valid_bold(y, x))
		{
			int d = 1;

			/* Pick a location */
			scatter(&ny, &nx, y, x, d, 0);

			/* Stagger */
			y = ny; x = nx;
		}

		/* Destroy any objects */
		delete_object(y, x);

		/* Explain the staircase */
		msg_print("A magical staircase appears...");

		/* Create stairs down */
		cave_set_feat(y, x, FEAT_MORE);

		/* Remember to update everything */
		p_ptr->update |= (PU_VIEW | PU_LITE | PU_FLOW | PU_MONSTERS);
	}
}




/*
 * Decreases monsters hit points, handling monster death.
 *
 * We return TRUE if the monster has been killed (and deleted).
 *
 * We announce monster death (using an optional "death message"
 * if given, and a otherwise a generic killed/destroyed message).
 *
 * Only "physical attacks" can induce the "You have slain" message.
 * Missile and Spell attacks will induce the "dies" message, or
 * various "specialized" messages.  Note that "You have destroyed"
 * and "is destroyed" are synonyms for "You have slain" and "dies".
 *
 * Hack -- unseen monsters yield "You have killed it." message.
 *
 * Added fear (DGK) and check whether to print fear messages -CWS
 *
 * Genericized name, sex, and capitilization -BEN-
 *
 * As always, the "ghost" processing is a total hack.
 *
 * Hack -- we "delay" fear messages by passing around a "fear" flag.
 *
 * XXX XXX XXX Consider decreasing monster experience over time, say,
 * by using "(m_exp * m_lev * (m_lev)) / (p_lev * (m_lev + n_killed))"
 * instead of simply "(m_exp * m_lev) / (p_lev)", to make the first
 * monster worth more than subsequent monsters.  This would also need
 * to induce changes in the monster recall code.
 */
bool mon_take_hit(int m_idx, int dam, bool *fear, cptr note)
{
	char tmp[1024];

	monster_type    *m_ptr = &m_list[m_idx];
        monster_race    *r_ptr = race_inf(m_ptr);
	s32b            div, new_exp, new_exp_frac;


	/* Redraw (later) if needed */
	if (health_who == m_idx) p_ptr->redraw |= (PR_HEALTH);

        /* Some mosnters are immune to death */
        if (r_ptr->flags7 & RF7_NO_DEATH) return FALSE;

	/* Wake it up */
	m_ptr->csleep = 0;

	/* Hurt it */
	m_ptr->hp -= dam;

	/* It is dead now */
	if (m_ptr->hp < 0)
	{
		char m_name[80];

                /* Lets face it, you cannot get rid of a possessor that easily */
                if (m_ptr->possessor)
                {
                        ai_deincarnate(m_idx);

                        return FALSE;
                }

		/* Extract monster name */
		monster_desc(m_name, m_ptr, 0);

                if ((r_ptr->flags7 & RF7_DG_CURSE) && (randint(2) == 1))
		{
                        int curses = 2 + randint(5);

                        cmsg_format(TERM_VIOLET, "%^s puts a terrible morgothian curse on you!", m_name);
                        curse_equipment_dg(100, 50);

			do
			{
                                activate_dg_curse();
			}
			while (--curses);
		}

		if (speak_unique && (r_ptr->flags2 & (RF2_CAN_SPEAK)))
		{
			char line_got[80];
                        int reward = 0;

			/* Dump a message */

			get_rnd_line("mondeath.txt", line_got);
			msg_format("%^s says: %s", m_name, line_got);

			if (randint(REWARD_CHANCE)==1)
			{
				msg_format("There was a price on %s's head.", m_name);
				get_rnd_line("crime.txt", line_got);
				msg_format("%^s was wanted for %s", m_name, line_got);
                                reward = 250 * (randint (10) + m_ptr->level - 5);

				if (reward > 32000) reward = 32000;/* Force 'good' values */
				else if (reward < 250) reward = 250;

				msg_format("You collect a reward of %d gold pieces.", reward);
				p_ptr -> au += reward;
				p_ptr->redraw |= (PR_GOLD);
			}
		}


		/* Make a sound */
		sound(SOUND_KILL);

		/* Death by Missile/Spell attack */
		if (note)
		{
                        cmsg_format(TERM_L_RED, "%^s%s", m_name, note);
		}

		/* Death by physical attack -- invisible monster */
		else if (!m_ptr->ml)
		{
                        cmsg_format(TERM_L_RED, "You have killed %s.", m_name);
		}

		/* Death by Physical attack -- non-living monster */
		else if ((r_ptr->flags3 & (RF3_DEMON)) ||
		         (r_ptr->flags3 & (RF3_UNDEAD)) ||
		         (r_ptr->flags2 & (RF2_STUPID)) ||
			 (r_ptr->flags3 & (RF3_NONLIVING)) ||
		         (strchr("Evg", r_ptr->d_char)))
		{
                        cmsg_format(TERM_L_RED, "You have destroyed %s.", m_name);
		}

		/* Death by Physical attack -- living monster */
		else
		{
                        cmsg_format(TERM_L_RED, "You have slain %s.", m_name);
		}

		/* Maximum player level */
		div = p_ptr->max_plv;

		/* Give some experience for the kill */
                new_exp = ((long)r_ptr->mexp * m_ptr->level) / div;

		/* Handle fractional experience */
                new_exp_frac = ((((long)r_ptr->mexp * m_ptr->level) % div)
		                * 0x10000L / div) + p_ptr->exp_frac;

		/* Keep track of experience */
		if (new_exp_frac >= 0x10000L)
		{
			new_exp++;
			p_ptr->exp_frac = new_exp_frac - 0x10000L;
		}
		else
		{
			p_ptr->exp_frac = new_exp_frac;
		}

		/* Gain experience */
		gain_exp(new_exp);

                if(!note)
                {
                        object_type *o_ptr;
                        object_kind *k_ptr;
                        u32b f1, f2, f3, f4, f5, esp;

                        /* Access the weapon */
                        o_ptr = &inventory[INVEN_WIELD];
                        k_ptr = &k_info[o_ptr->k_idx];
                        object_flags(o_ptr, &f1, &f2, &f3, &f4, &f5, &esp);

                        /* Can the weapon gain levels ? */
                        if((o_ptr->k_idx) && (f4 & TR4_LEVELS))
                        {
                                /* Give some experience for the kill */
                                new_exp = ((long)r_ptr->mexp * m_ptr->level) / (div * 2);

                                /* Gain experience */
                                o_ptr->exp += new_exp;
                                check_experience_obj(o_ptr);
                        }
                }

		/* When the player kills a Unique, it stays dead */
                if (r_ptr->flags1 & (RF1_UNIQUE))
                {
                        if ((r_ptr->flags7 & RF7_NAZGUL) && r_info[test_monster_name("Sauron, the Sorcerer")].max_num)
                        {
                                msg_print("Somehow you feel he is not totaly destroyed...");
                                r_ptr->max_num = 1;
                        }
                        else if ((m_ptr->r_idx == test_monster_name("Sauron, the Sorcerer")) && (quest[QUEST_ONE].status < QUEST_STATUS_FINISHED))
                        {
                                msg_print("Sauron will not be permanently defeated until the One Ring is eiter destroyed or used...");
                                r_ptr->max_num = 1;
                        }
                        else
                        {
                                r_ptr->max_num = 0;
                        }
                }

		/* Generate treasure */
		monster_death(m_idx);

		/* XXX XXX Mega-Hack -- allow another ghost later
		 * Remove the slain bone file */
                if (m_ptr->r_idx == max_r_idx - 1)
		{
			r_ptr->max_num = 1;

			/* Delete the bones file */
                        sprintf(tmp, "%s%sbone%03d.%03d", ANGBAND_DIR_BONE, PATH_SEP, dungeon_type, dun_level);
			
			fd_kill(tmp);
		}

                /* If the player kills a Unique, and the notes options are on, write a note */
                if ((r_ptr->flags1 & RF1_UNIQUE) && take_notes && auto_notes)
                {
			char note[80];

                        /* Get true name even if blinded/hallucinating */
                        cptr monst = (r_name + r_ptr->name);
 
			/* Write note */
			sprintf(note, "Killed %s", monst);
 
			add_note(note, 'U');
                }

		/* Recall even invisible uniques or winners */
		if (m_ptr->ml || (r_ptr->flags1 & (RF1_UNIQUE)))
		{
			/* Count kills this life */
			if (r_ptr->r_pkills < MAX_SHORT) r_ptr->r_pkills++;

			/* Count kills in all lives */
			if (r_ptr->r_tkills < MAX_SHORT) r_ptr->r_tkills++;

			/* Hack -- Auto-recall */
                        monster_race_track(m_ptr->r_idx, m_ptr->ego);
		}

		/* Delete the monster */
		delete_monster_idx(m_idx);

		/* Not afraid */
		(*fear) = FALSE;

		/* Monster is dead */
		return (TRUE);
	}


#ifdef ALLOW_FEAR

	/* Mega-Hack -- Pain cancels fear */
	if (m_ptr->monfear && (dam > 0))
	{
		int tmp = randint(dam);

		/* Cure a little fear */
		if (tmp < m_ptr->monfear)
		{
			/* Reduce fear */
			m_ptr->monfear -= tmp;
		}

		/* Cure all the fear */
		else
		{
			/* Cure fear */
			m_ptr->monfear = 0;

			/* No more fear */
			(*fear) = FALSE;
		}
	}

	/* Sometimes a monster gets scared by damage */
	if (!m_ptr->monfear && !(r_ptr->flags3 & (RF3_NO_FEAR)))
	{
		int percentage;

		/* Percentage of fully healthy */
		percentage = (100L * m_ptr->hp) / m_ptr->maxhp;

		/*
		 * Run (sometimes) if at 10% or less of max hit points,
		 * or (usually) when hit for half its current hit points
		 */
		if (((percentage <= 10) && (rand_int(10) < percentage)) ||
		    ((dam >= m_ptr->hp) && (rand_int(100) < 80)))
		{
			/* Hack -- note fear */
			(*fear) = TRUE;

			/* XXX XXX XXX Hack -- Add some timed fear */
			m_ptr->monfear = (randint(10) +
			                  (((dam >= m_ptr->hp) && (percentage > 7)) ?
			                   20 : ((11 - percentage) * 5)));
		}
	}

#endif


	/* Not dead yet */
	return (FALSE);
}



/*
 * Calculates current boundaries
 * Called below and from "do_cmd_locate()".
 */
void panel_bounds(void)
{
	panel_row_max = panel_row_min + SCREEN_HGT - 1;
	panel_row_prt = panel_row_min - 1;
	panel_col_max = panel_col_min + SCREEN_WID - 1;
	panel_col_prt = panel_col_min - 13;
}



/*
 * Given an row (y) and col (x), this routine detects when a move
 * off the screen has occurred and figures new borders. -RAK-
 *
 * "Update" forces a "full update" to take place.
 *
 * The map is reprinted if necessary, and "TRUE" is returned.
 */
void verify_panel(void)
{
	int y = py;
	int x = px;

        int prow = panel_row_min;
        int pcol = panel_col_min;

	/* Center on player */
        if (center_player)
	{
		/* Center vertically */
                prow = y - (PANEL_HGT);
                if (prow < 0) prow = 0;
                else if (prow > max_panel_rows * (SCREEN_HGT / 2)) prow = max_panel_rows * (SCREEN_HGT / 2);

		/* Center horizontally */
                pcol = x - (PANEL_WID);
                if (pcol < 0) pcol = 0;
                else if (pcol > max_panel_cols * (SCREEN_WID / 2)) pcol = max_panel_cols * (SCREEN_WID / 2);
	}
	else
	{
                /* Scroll screen when 2 grids from top/bottom edge */
                if (y < panel_row_min + 2)
                {
                        prow = ((y - 2) / PANEL_HGT) * PANEL_HGT;
                        if (prow > max_panel_rows * PANEL_HGT) prow = max_panel_rows * PANEL_HGT;
                        else if (prow < 0) prow = 0;
                }
                else if (y > panel_row_max - 2)
                {
                        prow = (y / PANEL_HGT) * PANEL_HGT;
                        if (prow > max_panel_rows * PANEL_HGT) prow = max_panel_rows * PANEL_HGT;
                        else if (prow < 0) prow = 0;
                }

                /* Scroll screen when 4 grids from left/right edge */
                if (x < panel_col_min + 4)
                {
                        pcol = ((x - 4) / PANEL_WID) * PANEL_WID;
                        if (pcol > max_panel_cols * PANEL_WID) pcol = max_panel_cols * PANEL_WID;
                        else if (pcol < 0) pcol = 0;
                }
                else if (x > panel_col_max - 4)
                {
                        pcol = (x / PANEL_WID) * PANEL_WID;
                        if (pcol > max_panel_cols * PANEL_WID) pcol = max_panel_cols * PANEL_WID;
                        else if (pcol < 0) pcol = 0;
                }
        }

	/* Check for "no change" */
        if ((prow == panel_row_min) && (pcol == panel_col_min)) return;

	/* Hack -- optional disturb on "panel change" */
	if (disturb_panel && !center_player) disturb(0, 0);

	/* Save the new panel info */
        panel_row_min = prow;
        panel_col_min = pcol;

	/* Recalculate the boundaries */
        panel_bounds();

	/* Hack - merchants detect items */
	if (p_ptr->pclass == CLASS_MERCHANT)
	{
                hack_no_detect_message = TRUE;
		detect_objects_normal();
		detect_objects_gold();
                hack_no_detect_message = FALSE;
	}

	/* Update stuff */
	p_ptr->update |= (PU_MONSTERS);

	/* Redraw map */
	p_ptr->redraw |= (PR_MAP);

	/* Window stuff */
	p_ptr->window |= (PW_OVERHEAD);
}



/*
 * Monster health description
 */
cptr look_mon_desc(int m_idx)
{
	monster_type *m_ptr = &m_list[m_idx];
        monster_race *r_ptr = race_inf(m_ptr);

	bool          living = TRUE;
	int           perc;


	/* Determine if the monster is "living" (vs "undead") */
	if (r_ptr->flags3 & (RF3_UNDEAD)) living = FALSE;
	if (r_ptr->flags3 & (RF3_DEMON)) living = FALSE;
	if (r_ptr->flags3 & (RF3_NONLIVING)) living = FALSE;
	if (strchr("Egv", r_ptr->d_char)) living = FALSE;


	/* Healthy monsters */
	if (m_ptr->hp >= m_ptr->maxhp)
	{
		/* No damage */
		return (living ? "unhurt" : "undamaged");
	}


	/* Calculate a health "percentage" */
	perc = 100L * m_ptr->hp / m_ptr->maxhp;

	if (perc >= 60)
	{
		return (living ? "somewhat wounded" : "somewhat damaged");
	}

	if (perc >= 25)
	{
		return (living ? "wounded" : "damaged");
	}

	if (perc >= 10)
	{
		return (living ? "badly wounded" : "badly damaged");
	}

	return (living ? "almost dead" : "almost destroyed");
}



/*
 * Angband sorting algorithm -- quick sort in place
 *
 * Note that the details of the data we are sorting is hidden,
 * and we rely on the "ang_sort_comp()" and "ang_sort_swap()"
 * function hooks to interact with the data, which is given as
 * two pointers, and which may have any user-defined form.
 */
void ang_sort_aux(vptr u, vptr v, int p, int q)
{
	int z, a, b;

	/* Done sort */
	if (p >= q) return;

	/* Pivot */
	z = p;

	/* Begin */
	a = p;
	b = q;

	/* Partition */
	while (TRUE)
	{
		/* Slide i2 */
		while (!(*ang_sort_comp)(u, v, b, z)) b--;

		/* Slide i1 */
		while (!(*ang_sort_comp)(u, v, z, a)) a++;

		/* Done partition */
		if (a >= b) break;

		/* Swap */
		(*ang_sort_swap)(u, v, a, b);

		/* Advance */
		a++, b--;
	}

	/* Recurse left side */
	ang_sort_aux(u, v, p, b);

	/* Recurse right side */
	ang_sort_aux(u, v, b+1, q);
}


/*
 * Angband sorting algorithm -- quick sort in place
 *
 * Note that the details of the data we are sorting is hidden,
 * and we rely on the "ang_sort_comp()" and "ang_sort_swap()"
 * function hooks to interact with the data, which is given as
 * two pointers, and which may have any user-defined form.
 */
void ang_sort(vptr u, vptr v, int n)
{
	/* Sort the array */
	ang_sort_aux(u, v, 0, n-1);
}



/*** Targetting Code ***/


/*
 * Determine is a monster makes a reasonable target
 *
 * The concept of "targetting" was stolen from "Morgul" (?)
 *
 * The player can target any location, or any "target-able" monster.
 *
 * Currently, a monster is "target_able" if it is visible, and if
 * the player can hit it with a projection, and the player is not
 * hallucinating.  This allows use of "use closest target" macros.
 *
 * Future versions may restrict the ability to target "trappers"
 * and "mimics", but the semantics is a little bit weird.
 */
bool target_able(int m_idx)
{
	monster_type *m_ptr = &m_list[m_idx];

	/* Monster must be alive */
	if (!m_ptr->r_idx) return (FALSE);

	/* Monster must be visible */
	if (!m_ptr->ml) return (FALSE);

	/* Monster must be projectable */
	if (!projectable(py, px, m_ptr->fy, m_ptr->fx)) return (FALSE);

	/* Hack -- no targeting hallucinations */
	if (p_ptr->image) return (FALSE);

        /* Dont target pets */
        if (is_friend(m_ptr) > 0) return (FALSE);

	/* XXX XXX XXX Hack -- Never target trappers */
	/* if (CLEAR_ATTR && (CLEAR_CHAR)) return (FALSE); */

	/* Assume okay */
	return (TRUE);
}




/*
 * Update (if necessary) and verify (if possible) the target.
 *
 * We return TRUE if the target is "okay" and FALSE otherwise.
 */
bool target_okay(void)
{
	/* Accept stationary targets */
	if (target_who < 0) return (TRUE);

	/* Check moving targets */
	if (target_who > 0)
	{
		/* Accept reasonable targets */
		if (target_able(target_who))
		{
			monster_type *m_ptr = &m_list[target_who];

			/* Acquire monster location */
			target_row = m_ptr->fy;
			target_col = m_ptr->fx;

			/* Good target */
			return (TRUE);
		}
	}

	/* Assume no target */
	return (FALSE);
}



/*
 * Sorting hook -- comp function -- by "distance to player"
 *
 * We use "u" and "v" to point to arrays of "x" and "y" positions,
 * and sort the arrays by double-distance to the player.
 */
static bool ang_sort_comp_distance(vptr u, vptr v, int a, int b)
{
	byte *x = (byte*)(u);
	byte *y = (byte*)(v);

	int da, db, kx, ky;

	/* Absolute distance components */
	kx = x[a]; kx -= px; kx = ABS(kx);
	ky = y[a]; ky -= py; ky = ABS(ky);

	/* Approximate Double Distance to the first point */
	da = ((kx > ky) ? (kx + kx + ky) : (ky + ky + kx));

	/* Absolute distance components */
	kx = x[b]; kx -= px; kx = ABS(kx);
	ky = y[b]; ky -= py; ky = ABS(ky);

	/* Approximate Double Distance to the first point */
	db = ((kx > ky) ? (kx + kx + ky) : (ky + ky + kx));

	/* Compare the distances */
	return (da <= db);
}


/*
 * Sorting hook -- swap function -- by "distance to player"
 *
 * We use "u" and "v" to point to arrays of "x" and "y" positions,
 * and sort the arrays by distance to the player.
 */
static void ang_sort_swap_distance(vptr u, vptr v, int a, int b)
{
	byte *x = (byte*)(u);
	byte *y = (byte*)(v);

	byte temp;

	/* Swap "x" */
	temp = x[a];
	x[a] = x[b];
	x[b] = temp;

	/* Swap "y" */
	temp = y[a];
	y[a] = y[b];
	y[b] = temp;
}



/*
 * Hack -- help "select" a location (see below)
 */
static s16b target_pick(int y1, int x1, int dy, int dx)
{
	int i, v;

	int x2, y2, x3, y3, x4, y4;

	int b_i = -1, b_v = 9999;


	/* Scan the locations */
	for (i = 0; i < temp_n; i++)
	{
		/* Point 2 */
		x2 = temp_x[i];
		y2 = temp_y[i];

		/* Directed distance */
		x3 = (x2 - x1);
		y3 = (y2 - y1);

		/* Verify quadrant */
		if (dx && (x3 * dx <= 0)) continue;
		if (dy && (y3 * dy <= 0)) continue;

		/* Absolute distance */
		x4 = ABS(x3);
		y4 = ABS(y3);

		/* Verify quadrant */
		if (dy && !dx && (x4 > y4)) continue;
		if (dx && !dy && (y4 > x4)) continue;

		/* Approximate Double Distance */
		v = ((x4 > y4) ? (x4 + x4 + y4) : (y4 + y4 + x4));

		/* XXX XXX XXX Penalize location */

		/* Track best */
		if ((b_i >= 0) && (v >= b_v)) continue;

		/* Track best */
		b_i = i; b_v = v;
	}

	/* Result */
	return (b_i);
}


/*
 * Hack -- determine if a given location is "interesting"
 */
static bool target_set_accept(int y, int x)
{
	cave_type *c_ptr;

	s16b this_o_idx, next_o_idx = 0;


	/* Player grid is always interesting */
	if ((y == py) && (x == px)) return (TRUE);


	/* Handle hallucination */
	if (p_ptr->image) return (FALSE);


	/* Examine the grid */
	c_ptr = &cave[y][x];

	/* Visible monsters */
	if (c_ptr->m_idx)
	{
		monster_type *m_ptr = &m_list[c_ptr->m_idx];

		/* Visible monsters */
		if (m_ptr->ml) return (TRUE);
	}

	/* Scan all objects in the grid */
	for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
	{
		object_type *o_ptr;
		
		/* Acquire object */
		o_ptr = &o_list[this_o_idx];

		/* Acquire next object */
		next_o_idx = o_ptr->next_o_idx;

		/* Memorized object */
		if (o_ptr->marked) return (TRUE);
	}

	/* Interesting memorized features */
	if (c_ptr->info & (CAVE_MARK))
	{
                if(f_info[c_ptr->feat].flags1 & FF1_NOTICE) return TRUE;
	}

        /* Known traps */
        if (c_ptr->info & (CAVE_TRDT))
        {
  		if (c_ptr->t_idx) return TRUE;
  	}

	/* Nope */
	return (FALSE);
}


/*
 * Prepare the "temp" array for "target_set"
 *
 * Return the number of target_able monsters in the set.
 */
static void target_set_prepare(int mode)
{
	int y, x;

	/* Reset "temp" array */
	temp_n = 0;

	/* Scan the current panel */
	for (y = panel_row_min; y <= panel_row_max; y++)
	{
		for (x = panel_col_min; x <= panel_col_max; x++)
		{
			cave_type *c_ptr = &cave[y][x];

			/* Require line of sight, unless "look" is "expanded" */
			if (!expand_look && !player_has_los_bold(y, x)) continue;

			/* Require "interesting" contents */
			if (!target_set_accept(y, x)) continue;

			/* Require target_able monsters for "TARGET_KILL" */
			if ((mode & (TARGET_KILL)) && !target_able(c_ptr->m_idx)) continue;

			/* Save the location */
			temp_x[temp_n] = x;
			temp_y[temp_n] = y;
			temp_n++;
		}
	}

	/* Set the sort hooks */
	ang_sort_comp = ang_sort_comp_distance;
	ang_sort_swap = ang_sort_swap_distance;

	/* Sort the positions */
	ang_sort(temp_x, temp_y, temp_n);
}


/*
 * Examine a grid, return a keypress.
 *
 * The "mode" argument contains the "TARGET_LOOK" bit flag, which
 * indicates that the "space" key should scan through the contents
 * of the grid, instead of simply returning immediately.  This lets
 * the "look" command get complete information, without making the
 * "target" command annoying.
 *
 * The "info" argument contains the "commands" which should be shown
 * inside the "[xxx]" text.  This string must never be empty, or grids
 * containing monsters will be displayed with an extra comma.
 *
 * Note that if a monster is in the grid, we update both the monster
 * recall info and the health bar info to track that monster.
 *
 * Eventually, we may allow multiple objects per grid, or objects
 * and terrain features in the same grid. XXX XXX XXX
 *
 * This function must handle blindness/hallucination.
 */
static int target_set_aux(int y, int x, int mode, cptr info)
{
	cave_type *c_ptr = &cave[y][x];

	s16b this_o_idx, next_o_idx = 0;

        cptr s1, s2, s3;

	bool boring;

	int feat;

	int query;

	char out_val[160];


	/* Repeat forever */
	while (1)
	{
		/* Paranoia */
		query = ' ';

		/* Assume boring */
		boring = TRUE;

		/* Default */
		s1 = "You see ";
		s2 = "";
		s3 = "";

		/* Hack -- under the player */
		if ((y == py) && (x == px))
		{
			/* Description */
			s1 = "You are ";
			
			/* Preposition */
			s2 = "on ";
		}


		/* Hack -- hallucination */
		if (p_ptr->image)
		{
			cptr name = "something strange";

			/* Display a message */
			sprintf(out_val, "%s%s%s%s [%s]", s1, s2, s3, name, info);
			prt(out_val, 0, 0);
			move_cursor_relative(y, x);
			query = inkey();

			/* Stop on everything but "return" */
			if ((query != '\r') && (query != '\n')) break;
			
			/* Repeat forever */
			continue;
		}


		/* Actual monsters */
		if (c_ptr->m_idx)
		{
			monster_type *m_ptr = &m_list[c_ptr->m_idx];
                        monster_race *r_ptr = race_inf(m_ptr);

			/* Visible */
			if (m_ptr->ml)
			{
				bool recall = FALSE;

				char m_name[80];

				/* Not boring */
				boring = FALSE;

				/* Get the monster name ("a kobold") */
				monster_desc(m_name, m_ptr, 0x08);

				/* Hack -- track this monster race */
                                monster_race_track(m_ptr->r_idx, m_ptr->ego);

				/* Hack -- health bar for this monster */
				health_track(c_ptr->m_idx);

				/* Hack -- handle stuff */
				handle_stuff();

				/* Interact */
				while (1)
				{
					/* Recall */
					if (recall)
					{
						/* Save */
                                                character_icky = TRUE;
						Term_save();

						/* Recall on screen */
                                                screen_roff(m_ptr->r_idx, m_ptr->ego, 0);

						/* Hack -- Complete the prompt (again) */
						Term_addstr(-1, TERM_WHITE, format("  [r,%s]", info));
					
						/* Command */
						query = inkey();

						/* Restore */
						Term_load();
                                                character_icky = FALSE;
					}

					/* Normal */
					else
					{
                                                cptr mstat;

                                                switch (m_ptr->status)
                                                {
                                                        case MSTATUS_NEUTRAL:
                                                        case MSTATUS_NEUTRAL_M:
                                                        case MSTATUS_NEUTRAL_P:
                                                                mstat = " (neutral) ";
                                                                break;
                                                        case MSTATUS_PET:
                                                                mstat = " (pet) ";
                                                                break;
                                                        case MSTATUS_FRIEND:
                                                                mstat = " (coaligned) ";
                                                                break;
                                                        case MSTATUS_COMPANION:
                                                                mstat = " (companion) ";
                                                                break;
                                                        default:
                                                                mstat = " ";
                                                                break;
                                                }

						/* Describe, and prompt for recall */
                                                sprintf(out_val, "%s%s%s%s (level %d, %s)%s%s[r,%s]",
                                                    s1, s2, s3, m_name, m_ptr->level, look_mon_desc(c_ptr->m_idx),
						    (m_ptr->smart & SM_CLONED ? " (clone)": ""),
                                                    (mstat), info);

						prt(out_val, 0, 0);

						/* Place cursor */
						move_cursor_relative(y, x);
					
						/* Command */
						query = inkey();
					}

					/* Normal commands */
					if (query != 'r') break;

					/* Toggle recall */
					recall = !recall;
				}

				/* Always stop at "normal" keys */
				if ((query != '\r') && (query != '\n') && (query != ' ')) break;

				/* Sometimes stop at "space" key */
				if ((query == ' ') && !(mode & (TARGET_LOOK))) break;

				/* Change the intro */
				s1 = "It is ";

				/* Hack -- take account of gender */
				if (r_ptr->flags1 & (RF1_FEMALE)) s1 = "She is ";
				else if (r_ptr->flags1 & (RF1_MALE)) s1 = "He is ";

				/* Use a preposition */
				s2 = "carrying ";

				/* Scan all objects being carried */
				for (this_o_idx = m_ptr->hold_o_idx; this_o_idx; this_o_idx = next_o_idx)
				{
					char o_name[80];

					object_type *o_ptr;
				
					/* Acquire object */
					o_ptr = &o_list[this_o_idx];

					/* Acquire next object */
					next_o_idx = o_ptr->next_o_idx;

					/* Obtain an object description */
					object_desc(o_name, o_ptr, TRUE, 3);

					/* Describe the object */
					sprintf(out_val, "%s%s%s%s [%s]", s1, s2, s3, o_name, info);
					prt(out_val, 0, 0);
					move_cursor_relative(y, x);
					query = inkey();

					/* Always stop at "normal" keys */
					if ((query != '\r') && (query != '\n') && (query != ' ')) break;

					/* Sometimes stop at "space" key */
					if ((query == ' ') && !(mode & (TARGET_LOOK))) break;

					/* Change the intro */
					s2 = "also carrying ";
				}

				/* Double break */
				if (this_o_idx) break;

				/* Use a preposition */
				s2 = "on ";
			}
		}



		/* Scan all objects in the grid */
		for (this_o_idx = c_ptr->o_idx; this_o_idx; this_o_idx = next_o_idx)
		{
			object_type *o_ptr;
		
			/* Acquire object */
			o_ptr = &o_list[this_o_idx];

			/* Acquire next object */
			next_o_idx = o_ptr->next_o_idx;

			/* Describe it */
			if (o_ptr->marked)
			{
				char o_name[80];

				/* Not boring */
				boring = FALSE;

				/* Obtain an object description */
				object_desc(o_name, o_ptr, TRUE, 3);

				/* Describe the object */
				sprintf(out_val, "%s%s%s%s [%s]", s1, s2, s3, o_name, info);
				prt(out_val, 0, 0);
				move_cursor_relative(y, x);
				query = inkey();

				/* Always stop at "normal" keys */
				if ((query != '\r') && (query != '\n') && (query != ' ')) break;

				/* Sometimes stop at "space" key */
				if ((query == ' ') && !(mode & (TARGET_LOOK))) break;

				/* Change the intro */
				s1 = "It is ";

				/* Plurals */
				if (o_ptr->number != 1) s1 = "They are ";

				/* Preposition */
				s2 = "on ";
			}
		}

		/* Double break */
		if (this_o_idx) break;

		/* Actual traps */
		if ((c_ptr->t_idx) && (c_ptr->info & (CAVE_TRDT)))
		{
                        cptr name = "a trap", s4;

			/* Name trap */
                        if (t_info[c_ptr->t_idx].ident)
                                s4 = format("(%s)", t_name + t_info[c_ptr->t_idx].name);
                        else
                                s4 = "an unknown trap";

			/* Display a message */
			sprintf(out_val, "%s%s%s%s [%s]", s1, s2, s3, name, s4);
			prt(out_val, 0, 0);
			move_cursor_relative(y, x);
			query = inkey();

			/* Stop on everything but "return" */
			if ((query != '\r') && (query != '\n')) break;

			/* Repeat forever */
			continue;
                }

		/* Feature (apply "mimic") */
                if (c_ptr->mimic)
                        feat = c_ptr->mimic;
                else
                        feat = f_info[c_ptr->feat].mimic;

		/* Require knowledge about grid, or ability to see grid */
		if (!(c_ptr->info & (CAVE_MARK)) && !player_can_see_bold(y,x))
		{
			/* Forget feature */
			feat = FEAT_NONE;
		}

		/* Terrain feature if needed */
		if (boring || (feat >= FEAT_GLYPH))
		{
			cptr name;

			/* Hack -- special handling for building doors */
                        if (feat == FEAT_SHOP)
			{
                                name = st_name + st_info[c_ptr->special].name;
			}
			else
			{
                                name = f_name + f_info[feat].name;
			}

			/* Hack -- handle unknown grids */
			if (feat == FEAT_NONE) name = "unknown grid";

  			/* Pick a prefix */
                        if (*s2 &&
                            (((feat >= FEAT_MINOR_GLYPH) &&
                              (feat <= FEAT_PATTERN_XTRA2)) ||
                             (feat == FEAT_DIRT) ||
                             (feat == FEAT_GRASS) ||
                             (feat == FEAT_FLOWER))) s2 = "on ";
                        else if (*s2 && (feat == FEAT_SMALL_TREES)) s2 = "by ";
  			else if (*s2 && (feat >= FEAT_DOOR_HEAD)) s2 = "in ";
  
  			/* Pick proper indefinite article */
  			s3 = (is_a_vowel(name[0])) ? "an " : "a ";

                        /* Hack -- special introduction for store & building doors */
                        if (feat == FEAT_SHOP)
			{
				s3 = "the entrance to the ";
			}

                        if ((feat == FEAT_MORE) && c_ptr->special)
                        {
                                s3 = "";
                                name = d_text + d_info[c_ptr->special].text;
                        }

                        if (p_ptr->wild_mode && (feat == FEAT_TOWN))
                        {
                                s3 = "";
                                name = format("%s(%s)", wf_name + wf_info[wild_map[y][x].feat].name, wf_text + wf_info[wild_map[y][x].feat].text);
                        }
			
			if ((feat==FEAT_FOUNTAIN) && (c_ptr->info & CAVE_IDNT))
			{
				object_kind *k_ptr;
				int tv, sv;
				
				if (c_ptr->special <= SV_POTION_LAST)
				{
					tv = TV_POTION;
					sv = c_ptr->special;
				}
				else
				{
					tv = TV_POTION2;
					sv = c_ptr->special - SV_POTION_LAST;
				}
				
				k_ptr = &k_info[lookup_kind(tv, sv)];
                                info = k_name + k_ptr->name;
			}
			
			/* Display a message */
                        if (!wizard)
                                sprintf(out_val, "%s%s%s%s [%s]", s1, s2, s3, name, info);
                        else
                                sprintf(out_val, "%s%s%s%s [%s] (%d:%d:%d)", s1, s2, s3, name, info, c_ptr->feat, c_ptr->mimic, c_ptr->special);
			prt(out_val, 0, 0);
			move_cursor_relative(y, x);
			query = inkey();
			
			/* Always stop at "normal" keys */
			if ((query != '\r') && (query != '\n') && (query != ' ')) break;
		}

		/* Stop on everything but "return" */
		if ((query != '\r') && (query != '\n')) break;
	}

	/* Keep going */
	return (query);
}




/*
 * Handle "target" and "look".
 *
 * Note that this code can be called from "get_aim_dir()".
 *
 * All locations must be on the current panel.  Consider the use of
 * "panel_bounds()" to allow "off-panel" targets, perhaps by using
 * some form of "scrolling" the map around the cursor.  XXX XXX XXX
 * That is, consider the possibility of "auto-scrolling" the screen
 * while the cursor moves around.  This may require changes in the
 * "update_mon()" code to allow "visibility" even if off panel, and
 * may require dynamic recalculation of the "temp" grid set.
 *
 * Hack -- targetting/observing an "outer border grid" may induce
 * problems, so this is not currently allowed.
 *
 * The player can use the direction keys to move among "interesting"
 * grids in a heuristic manner, or the "space", "+", and "-" keys to
 * move through the "interesting" grids in a sequential manner, or
 * can enter "location" mode, and use the direction keys to move one
 * grid at a time in any direction.  The "t" (set target) command will
 * only target a monster (as opposed to a location) if the monster is
 * target_able and the "interesting" mode is being used.
 *
 * The current grid is described using the "look" method above, and
 * a new command may be entered at any time, but note that if the
 * "TARGET_LOOK" bit flag is set (or if we are in "location" mode,
 * where "space" has no obvious meaning) then "space" will scan
 * through the description of the current grid until done, instead
 * of immediately jumping to the next "interesting" grid.  This
 * allows the "target" command to retain its old semantics.
 *
 * The "*", "+", and "-" keys may always be used to jump immediately
 * to the next (or previous) interesting grid, in the proper mode.
 *
 * The "return" key may always be used to scan through a complete
 * grid description (forever).
 *
 * This command will cancel any old target, even if used from
 * inside the "look" command.
 */
bool target_set(int mode)
{
	int		i, d, m;
	int		y = py;
	int		x = px;

	bool	done = FALSE;

	bool	flag = TRUE;

	char	query;

	char	info[80];
	
	cave_type		*c_ptr;


	/* Cancel target */
	target_who = 0;


	/* Cancel tracking */
	/* health_track(0); */


	/* Prepare the "temp" array */
	target_set_prepare(mode);

	/* Start near the player */
	m = 0;

	/* Interact */
	while (!done)
	{
		/* Interesting grids */
		if (flag && temp_n)
		{
			y = temp_y[m];
			x = temp_x[m];

			/* Access */
			c_ptr = &cave[y][x];

			/* Allow target */
			if (target_able(c_ptr->m_idx))
			{
				strcpy(info, "q,t,p,o,+,-,<dir>");
			}

			/* Dis-allow target */
			else
			{
				strcpy(info, "q,p,o,+,-,<dir>");
			}

			/* Describe and Prompt */
			query = target_set_aux(y, x, mode, info);

			/* Cancel tracking */
			/* health_track(0); */

			/* Assume no "direction" */
			d = 0;

			/* Analyze */
			switch (query)
			{
				case ESCAPE:
				case 'q':
				{
					done = TRUE;
					break;
				}

				case 't':
				case '.':
				case '5':
				case '0':
				{
					if (target_able(c_ptr->m_idx))
					{
						health_track(c_ptr->m_idx);
						target_who = c_ptr->m_idx;
						target_row = y;
						target_col = x;
						done = TRUE;
					}
					else
					{
						bell();
					}
					break;
				}

				case ' ':
				case '*':
				case '+':
				{
					if (++m == temp_n)
					{
						m = 0;
						if (!expand_list) done = TRUE;
					}
					break;
				}

				case '-':
				{
					if (m-- == 0)
					{
						m = temp_n - 1;
						if (!expand_list) done = TRUE;
					}
					break;
				}

				case 'p':
				{
					y = py;
					x = px;
				}

				case 'o':
				{
					flag = !flag;
					break;
				}

				case 'm':
				{
					break;
				}

				default:
				{
					/* Extract the action (if any) */
					d = get_keymap_dir(query);

					if (!d) bell();
					break;
				}
			}

			/* Hack -- move around */
			if (d)
			{
				/* Find a new monster */
				i = target_pick(temp_y[m], temp_x[m], ddy[d], ddx[d]);

				/* Use that grid */
				if (i >= 0) m = i;
			}
		}

		/* Arbitrary grids */
		else
		{
			/* Access */
			c_ptr = &cave[y][x];

			/* Default prompt */
			strcpy(info, "q,t,p,m,+,-,<dir>");

			/* Describe and Prompt (enable "TARGET_LOOK") */
			query = target_set_aux(y, x, mode | TARGET_LOOK, info);

			/* Cancel tracking */
			/* health_track(0); */

			/* Assume no direction */
			d = 0;

			/* Analyze the keypress */
			switch (query)
			{
				case ESCAPE:
				case 'q':
				{
					done = TRUE;
					break;
				}

				case 't':
				case '.':
				case '5':
				case '0':
				{
					target_who = -1;
					target_row = y;
					target_col = x;
					done = TRUE;
					break;
				}

				case ' ':
				case '*':
				case '+':
				case '-':
				{
					break;
				}

				case 'p':
				{
					y = py;
					x = px;
				}

				case 'o':
				{
					break;
				}

				case 'm':
				{
					flag = !flag;
					break;
				}

				default:
				{
					/* Extract the action (if any) */
					d = get_keymap_dir(query);

					if (!d) bell();
					break;
				}
			}

			/* Handle "direction" */
			if (d)
			{
				x += ddx[d];
				y += ddy[d];

                                if (!wizard)
                                {
                                        /* Hack -- Verify x */
                                        if ((x>=cur_wid-1) || (x>panel_col_max)) x--;
                                        else if ((x <= 0) || (x<panel_col_min)) x++;

                                        /* Hack -- Verify y */
                                        if ((y>=cur_hgt-1) || (y>panel_row_max)) y--;
                                        else if ((y <= 0) || (y<panel_row_min)) y++;
                                }
                                else
                                {
                                        /* Hack -- Verify x */
                                        if ((x > cur_wid-1) || (x > panel_col_max)) x--;
                                        else if ((x < 0) || (x < panel_col_min)) x++;

                                        /* Hack -- Verify y */
                                        if ((y > cur_hgt-1) || (y > panel_row_max)) y--;
                                        else if ((y < 0) || (y < panel_row_min)) y++;
                                }
        		}
		}
	}

	/* Forget */
	temp_n = 0;

	/* Clear the top line */
	prt("", 0, 0);

	/* Failure to set target */
	if (!target_who) return (FALSE);

	/* Success */
	return (TRUE);
}



/*
 * Get an "aiming direction" from the user.
 *
 * The "dir" is loaded with 1,2,3,4,6,7,8,9 for "actual direction", and
 * "0" for "current target", and "-1" for "entry aborted".
 *
 * Note that "Force Target", if set, will pre-empt user interaction,
 * if there is a usable target already set.
 *
 * Note that confusion over-rides any (explicit?) user choice.
 */
bool get_aim_dir(int *dp)
{
	int		dir;

	char	command;

	cptr	p;

#ifdef ALLOW_REPEAT /* TNB */

	if (repeat_pull(dp))
	{
		/* Confusion? */

		/* Verify */
		if (!(*dp == 5 && !target_okay()))
		{
			return (TRUE);
		}
	}

#endif /* ALLOW_REPEAT -- TNB */

	/* Initialize */
	(*dp) = 0;

	/* Global direction */
	dir = command_dir;

	/* Hack -- auto-target if requested */
	if (use_old_target && target_okay()) dir = 5;

	/* Ask until satisfied */
	while (!dir)
	{
		/* Choose a prompt */
		if (!target_okay())
		{
			p = "Direction ('*' to choose a target, Escape to cancel)? ";
		}
		else
		{
			p = "Direction ('5' for target, '*' to re-target, Escape to cancel)? ";
		}

		/* Get a command (or Cancel) */
		if (!get_com(p, &command)) break;

		/* Convert various keys to "standard" keys */
		switch (command)
		{
			/* Use current target */
			case 'T':
			case 't':
			case '.':
			case '5':
			case '0':
			{
				dir = 5;
				break;
			}

			/* Set new target */
			case '*':
			{
				if (target_set(TARGET_KILL)) dir = 5;
				break;
			}

			default:
			{
				/* Extract the action (if any) */
				dir = get_keymap_dir(command);

				break;
			}
		}

		/* Verify requested targets */
		if ((dir == 5) && !target_okay()) dir = 0;

		/* Error */
		if (!dir) bell();
	}

	/* No direction */
	if (!dir) return (FALSE);

	/* Save the direction */
	command_dir = dir;

	/* Check for confusion */
	if (p_ptr->confused)
	{
		/* XXX XXX XXX */
		/* Random direction */
		dir = ddd[rand_int(8)];
	}

	/* Notice confusion */
	if (command_dir != dir)
	{
		/* Warn the user */
		msg_print("You are confused.");
	}

	/* Save direction */
	(*dp) = dir;

#ifdef ALLOW_REPEAT /* TNB */

    repeat_push(dir);

#endif /* ALLOW_REPEAT -- TNB */

	/* A "valid" direction was entered */
	return (TRUE);
}



/*
 * Request a "movement" direction (1,2,3,4,6,7,8,9) from the user,
 * and place it into "command_dir", unless we already have one.
 *
 * This function should be used for all "repeatable" commands, such as
 * run, walk, open, close, bash, disarm, spike, tunnel, etc, as well
 * as all commands which must reference a grid adjacent to the player,
 * and which may not reference the grid under the player.  Note that,
 * for example, it is no longer possible to "disarm" or "open" chests
 * in the same grid as the player.
 *
 * Direction "5" is illegal and will (cleanly) abort the command.
 *
 * This function tracks and uses the "global direction", and uses
 * that as the "desired direction", to which "confusion" is applied.
 */
bool get_rep_dir(int *dp)
{
	int dir;

#ifdef ALLOW_REPEAT /* TNB */

	if (repeat_pull(dp))
	{
		return (TRUE);
	}

#endif /* ALLOW_REPEAT -- TNB */

	/* Initialize */
	(*dp) = 0;

	/* Global direction */
	dir = command_dir;

	/* Get a direction */
	while (!dir)
	{
		char ch;

		/* Get a command (or Cancel) */
		if (!get_com("Direction (Escape to cancel)? ", &ch)) break;

		/* Look up the direction */
		dir = get_keymap_dir(ch);

		/* Oops */
		if (!dir) bell();
	}

	/* Prevent weirdness */
	if (dir == 5) dir = 0;

	/* Aborted */
	if (!dir) return (FALSE);

	/* Save desired direction */
	command_dir = dir;

	/* Apply "confusion" */
	if (p_ptr->confused)
	{
		/* Standard confusion */
		if (rand_int(100) < 75)
		{
			/* Random direction */
			dir = ddd[rand_int(8)];
		}
	}
	
	/* Notice confusion */
	if (command_dir != dir)
	{
		/* Warn the user */
		msg_print("You are confused.");
	}

	/* Save direction */
	(*dp) = dir;

#ifdef ALLOW_REPEAT /* TNB */

	repeat_push(dir);

#endif /* ALLOW_REPEAT -- TNB */

	/* Success */
	return (TRUE);
}


int get_chaos_patron(void)
{
	return (((p_ptr->age)+(p_ptr->sc))%MAX_PATRON);
}


void gain_level_reward(int chosen_reward)
{
	object_type *q_ptr;
	object_type forge;
	char        wrath_reason[32] = "";
	int         nasty_chance = 6;
	int         dummy = 0, dummy2 = 0;
	int         type, effect;


	if (!chosen_reward)
	{
		if (multi_rew) return;
		else multi_rew = TRUE;
	}


	if (p_ptr->lev == 13) nasty_chance = 2;
	else if (!(p_ptr->lev % 13)) nasty_chance = 3;
	else if (!(p_ptr->lev % 14)) nasty_chance = 12;

	if (randint(nasty_chance) == 1)
		type = randint(20); /* Allow the 'nasty' effects */
	else
		type = randint(15) + 5; /* Or disallow them */

	if (type < 1) type = 1;
	if (type > 20) type = 20;
	type--;


	sprintf(wrath_reason, "the Wrath of %s",
		chaos_patrons[p_ptr->chaos_patron]);

	effect = chaos_rewards[p_ptr->chaos_patron][type];

	if ((randint(6) == 1) && !chosen_reward)
	{
                msg_format("%^s rewards you with a corruption!",
			chaos_patrons[p_ptr->chaos_patron]);
                (void)gain_random_corruption(0);
		return;
	}

	switch (chosen_reward?chosen_reward:effect)
	{
		case REW_POLY_SLF:
			msg_format("The voice of %s booms out:",
				chaos_patrons[p_ptr->chaos_patron]);
			msg_print("'Thou needst a new form, mortal!'");
			do_poly_self();
			break;
		case REW_GAIN_EXP:
			msg_format("The voice of %s booms out:",
				chaos_patrons[p_ptr->chaos_patron]);
			msg_print("'Well done, mortal! Lead on!'");
			if (p_ptr->exp < PY_MAX_EXP)
			{
				s32b ee = (p_ptr->exp / 2) + 10;
				if (ee > 100000L) ee = 100000L;
				msg_print("You feel more experienced.");
				gain_exp(ee);
			}
			break;
		case REW_LOSE_EXP:
			msg_format("The voice of %s booms out:",
				chaos_patrons[p_ptr->chaos_patron]);
			msg_print("'Thou didst not deserve that, slave.'");
			lose_exp(p_ptr->exp / 6);
			break;
		case REW_GOOD_OBJ:
			msg_format("The voice of %s whispers:",
				chaos_patrons[p_ptr->chaos_patron]);
			msg_print("'Use my gift wisely.'");
			acquirement(py, px, 1, FALSE, FALSE);
			break;
		case REW_GREA_OBJ:
			msg_format("The voice of %s booms out:",
				chaos_patrons[p_ptr->chaos_patron]);
			msg_print("'Use my gift wisely.'");
			acquirement(py, px, 1, TRUE, FALSE);
			break;
		case REW_CHAOS_WP:
			msg_format("The voice of %s booms out:",
				chaos_patrons[p_ptr->chaos_patron]);
			msg_print("'Thy deed hath earned thee a worthy blade.'");
			/* Get local object */
			q_ptr = &forge;
			dummy = TV_SWORD;
			switch(randint(p_ptr->lev))
			{
			case 0: case 1:
				dummy2 = SV_DAGGER;
				break;
			case 2: case 3:
				dummy2 = SV_MAIN_GAUCHE;
				break;
			case 4:
			case 5: case 6:
				dummy2 = SV_RAPIER;
				break;
			case 7: case 8:
				dummy2 = SV_SMALL_SWORD;
				break;
			case 9: case 10:
				dummy2 = SV_BASILLARD;
				break;
			case 11: case 12: case 13:
				dummy2 = SV_SHORT_SWORD;
				break;
			case 14: case 15:
				dummy2 = SV_SABRE;
				break;
			case 16: case 17:
				dummy2 = SV_CUTLASS;
				break;
			case 18:
			case 19:
				dummy2 = SV_KHOPESH;
				break;
			case 20:
				dummy2 = SV_TULWAR;
				break;
			case 21:
				dummy2 = SV_BROAD_SWORD;
				break;
			case 22: case 23:
				dummy2 = SV_LONG_SWORD;
				break;
			case 24: case 25:
				dummy2 = SV_SCIMITAR;
				break;
			case 26:
			case 27:
				dummy2 = SV_KATANA;
				break;
			case 28: case 29:
				dummy2 = SV_BASTARD_SWORD;
				break;
			case 30:
				dummy2 = SV_GREAT_SCIMITAR;
				break;
			case 31:
				dummy2 = SV_CLAYMORE;
				break;
			case 32:
				dummy2 = SV_ESPADON;
				break;
			case 33:
				dummy2 = SV_TWO_HANDED_SWORD;
				break;
			case 34:
				dummy2 = SV_FLAMBERGE;
				break;
			case 35:
			case 36:
				dummy2 = SV_EXECUTIONERS_SWORD;
				break;
			case 37:
				dummy2 = SV_ZWEIHANDER;
				break;
			default:
				dummy2 = SV_BLADE_OF_CHAOS;
			}

			object_prep(q_ptr, lookup_kind(dummy, dummy2));
			q_ptr->to_h = 3 + (randint(dun_level)) % 10;
			q_ptr->to_d = 3 + (randint(dun_level)) % 10;
			random_resistance(q_ptr, FALSE, (randint(34) + 4));
                        q_ptr->name2 = EGO_CHAOTIC;
                        /* Apply the ego */
                        apply_magic(q_ptr, dun_level, FALSE, FALSE, FALSE);
			/* Drop it in the dungeon */
			drop_near(q_ptr, -1, py, px);
			break;
		case REW_GOOD_OBS:
			msg_format("The voice of %s booms out:",
				chaos_patrons[p_ptr->chaos_patron]);
			msg_print("'Thy deed hath earned thee a worthy reward.'");
			acquirement(py, px, randint(2) + 1, FALSE, FALSE);
			break;
		case REW_GREA_OBS:
			msg_format("The voice of %s booms out:",
				chaos_patrons[p_ptr->chaos_patron]);
			msg_print("'Behold, mortal, how generously I reward thy loyalty.'");
			acquirement(py, px, randint(2) + 1, TRUE, FALSE);
			break;
		case REW_TY_CURSE:
			msg_format("The voice of %s thunders:",
				chaos_patrons[p_ptr->chaos_patron]);
			msg_print("'Thou art growing arrogant, mortal.'");
			activate_ty_curse();
			break;
		case REW_SUMMON_M:
			msg_format("The voice of %s booms out:",
				chaos_patrons[p_ptr->chaos_patron]);
			msg_print("'My pets, destroy the arrogant mortal!'");
			for (dummy = 0; dummy < randint(5) + 1; dummy++)
			{
				(void)summon_specific(py, px, dun_level, 0);
			}
			break;
		case REW_H_SUMMON:
			msg_format("The voice of %s booms out:",
				chaos_patrons[p_ptr->chaos_patron]);
			msg_print("'Thou needst worthier opponents!'");
			activate_hi_summon();
			break;
		case REW_DO_HAVOC:
			msg_format("The voice of %s booms out:",
				chaos_patrons[p_ptr->chaos_patron]);
			msg_print("'Death and destruction! This pleaseth me!'");
			call_chaos();
			break;
		case REW_GAIN_ABL:				  
			msg_format("The voice of %s rings out:",
				chaos_patrons[p_ptr->chaos_patron]);
			msg_print("'Stay, mortal, and let me mold thee.'");
			if ((randint(3) == 1) && !(chaos_stats[p_ptr->chaos_patron] < 0))
				do_inc_stat(chaos_stats[p_ptr->chaos_patron]);
			else
				do_inc_stat(randint(6) - 1);
			break;
		case REW_LOSE_ABL:
			msg_format("The voice of %s booms out:",
				chaos_patrons[p_ptr->chaos_patron]);
			msg_print("'I grow tired of thee, mortal.'");
			if ((randint(3) == 1) && !(chaos_stats[p_ptr->chaos_patron] < 0))
				do_dec_stat(chaos_stats[p_ptr->chaos_patron], STAT_DEC_NORMAL);
			else
				(void)do_dec_stat(randint(6) - 1, STAT_DEC_NORMAL);
			break;
		case REW_RUIN_ABL:
			msg_format("The voice of %s thunders:",
				chaos_patrons[p_ptr->chaos_patron]);
			msg_print("'Thou needst a lesson in humility, mortal!'");
			msg_print("You feel less powerful!");
			for (dummy = 0; dummy < 6; dummy++)
			{
				(void)dec_stat(dummy, 10 + randint(15), TRUE);
			}
			break;
		case REW_POLY_WND:
			msg_format("You feel the power of %s touch you.",
				chaos_patrons[p_ptr->chaos_patron]);
			do_poly_wounds();
			break;
		case REW_AUGM_ABL:
			msg_format("The voice of %s booms out:",
				chaos_patrons[p_ptr->chaos_patron]);
			msg_print("'Receive this modest gift from me!'");
			for (dummy = 0; dummy < 6; dummy++)
			{
				(void) do_inc_stat(dummy);
			}
			break;
		case REW_HURT_LOT:
			msg_format("The voice of %s booms out:",
				chaos_patrons[p_ptr->chaos_patron]);
			msg_print("'Suffer, pathetic fool!'");
			fire_ball(GF_DISINTEGRATE, 0, (p_ptr->lev * 4), 4);
			take_hit(p_ptr->lev * 4, wrath_reason);
			break;
	   case REW_HEAL_FUL:
			msg_format("The voice of %s booms out:",
				chaos_patrons[p_ptr->chaos_patron]);
			msg_print("'Rise, my servant!'");
			restore_level();
			(void)set_poisoned(0);
			(void)set_blind(0);
			(void)set_confused(0);
			(void)set_image(0);
			(void)set_stun(0);
			(void)set_cut(0);
			hp_player(5000);
			for (dummy = 0; dummy < 6; dummy++)
			{
				(void) do_res_stat(dummy);
			}
			break;
		case REW_CURSE_WP:
			msg_format("The voice of %s booms out:",
				chaos_patrons[p_ptr->chaos_patron]);
			msg_print("'Thou reliest too much on thy weapon.'");
			(void)curse_weapon();
			break;
		case REW_CURSE_AR:
			msg_format("The voice of %s booms out:",
				chaos_patrons[p_ptr->chaos_patron]);
			msg_print("'Thou reliest too much on thine equipment.'");
			(void)curse_armor();
			break;
		case REW_PISS_OFF:
			msg_format("The voice of %s whispers:",
				chaos_patrons[p_ptr->chaos_patron]);
			msg_print("'Now thou shalt pay for annoying me.'");
			switch(randint(4))
			{
					case 1:
						activate_ty_curse();
						break;
					case 2:
						activate_hi_summon();
						break;
					case 3:
						if (randint(2) == 1) (void)curse_weapon();
						else (void)curse_armor();
						break;
					default:
					for (dummy = 0; dummy < 6; dummy++)
					{
						(void) dec_stat(dummy, 10 + randint(15), TRUE);
					}
			}
			break;
		case REW_WRATH:
			msg_format("The voice of %s thunders:",
				chaos_patrons[p_ptr->chaos_patron]);
			msg_print("'Die, mortal!'");
			take_hit(p_ptr->lev * 4, wrath_reason);
			for (dummy = 0; dummy < 6; dummy++)
			{
				(void) dec_stat(dummy, 10 + randint(15), FALSE);
			}
			activate_hi_summon();
			activate_ty_curse();
			if (randint(2)==1) (void)curse_weapon();
			if (randint(2)==1) (void)curse_armor();
			break;
		case REW_DESTRUCT:
			/* Prevent destruction of quest levels and town */
			if (!is_quest(dun_level) && dun_level)
			{
				msg_format("The voice of %s booms out:",
					chaos_patrons[p_ptr->chaos_patron]);
				msg_print("'Death and destruction! This pleaseth me!'");
				destroy_area(py, px, 25, TRUE);
			}
			break;
		case REW_GENOCIDE:
			msg_format("The voice of %s booms out:",
				chaos_patrons[p_ptr->chaos_patron]);
			msg_print("'Let me relieve thee of thine oppressors!'");
			(void) genocide(FALSE);
			break;
		case REW_MASS_GEN:
			msg_format("The voice of %s booms out:",
				chaos_patrons[p_ptr->chaos_patron]);
			msg_print("'Let me relieve thee of thine oppressors!'");
			(void) mass_genocide(FALSE);
			break;
		case REW_DISPEL_C:
			msg_format("You can feel the power of %s assault your enemies!",
				chaos_patrons[p_ptr->chaos_patron]);
			(void) dispel_monsters(p_ptr->lev * 4);
			break;
		case REW_IGNORE:
			msg_format("%s ignores you.",
				chaos_patrons[p_ptr->chaos_patron]);
			break;
		case REW_SER_DEMO:
			msg_format("%s rewards you with a demonic servant!",chaos_patrons[p_ptr->chaos_patron]);
			if (!(summon_specific_friendly(py, px, dun_level, SUMMON_DEMON, FALSE)))
			msg_print("Nobody ever turns up...");
			break;
		case REW_SER_MONS:
			msg_format("%s rewards you with a servant!",chaos_patrons[p_ptr->chaos_patron]);
			if (!(summon_specific_friendly(py, px, dun_level, SUMMON_NO_UNIQUES, FALSE)))
			msg_print("Nobody ever turns up...");
			break;
		case REW_SER_UNDE:
			msg_format("%s rewards you with an undead servant!",chaos_patrons[p_ptr->chaos_patron]);
			if (!(summon_specific_friendly(py, px, dun_level, SUMMON_UNDEAD, FALSE)))
			msg_print("Nobody ever turns up...");
			break;
		default:
			msg_format("The voice of %s stammers:",
				chaos_patrons[p_ptr->chaos_patron]);
			msg_format("'Uh... uh... the answer's %d/%d, what's the question?'", type,
				effect );
	}
}


/*
 * old -- from PsiAngband.
 */
bool tgt_pt(int *x,int *y)
{
	char ch = 0;
	int d,cu,cv;
	bool success = FALSE;

	*x = px;
	*y = py;

	cu = Term->scr->cu;
	cv = Term->scr->cv;
	Term->scr->cu = 0;
	Term->scr->cv = 1;
	msg_print("Select a point and press space.");

	while ((ch != 27) && (ch != ' '))
	{
		move_cursor_relative(*y,*x);
		ch = inkey();
		switch (ch)
		{
		case 27:
			break;
		case ' ':
			success = TRUE; break;
		default:
			/* Look up the direction */
			d = get_keymap_dir(ch);

			if (!d) break;

			*x += ddx[d];
			*y += ddy[d];

			/* Hack -- Verify x */
			if ((*x>=cur_wid-1) || (*x>=panel_col_min + SCREEN_WID)) (*x)--;
			else if ((*x<=0) || (*x<=panel_col_min)) (*x)++;

			/* Hack -- Verify y */
			if ((*y>=cur_hgt-1) || (*y>=panel_row_min + SCREEN_HGT)) (*y)--;
			else if ((*y<=0) || (*y<=panel_row_min)) (*y)++;

			break;
		}
	}

	Term->scr->cu = cu;
	Term->scr->cv = cv;
	Term_fresh();
	return success;
}


bool gain_random_corruption(int choose_mut)
{
	int attempts_left = 20;
	cptr muta_desc = "";
	bool muta_chosen = FALSE;
	int muta_which = 0;
	u32b * muta_class = 0;
	
	if (choose_mut) attempts_left = 1;

        p_ptr->update |= PU_BONUS;

	while (attempts_left--)
	{
		switch(choose_mut?choose_mut:randint(194))
		{
		case 1: case 2: case 3: case 4:
			muta_class = &(p_ptr->muta1);
                        muta_which = MUT1_SPIT_ACID;
			break;
		case 5: case 6: case 7:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_BR_FIRE;
			break;
		case 8: case 9:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_HYPN_GAZE;
			break;
		case 10: case 11:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_TELEKINES;
			break;
		case 12: case 13: case 14:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_VTELEPORT;
			break;
		case 15: case 16:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_MIND_BLST;
			break;
		case 17: case 18:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_RADIATION;
			break;
		case 19: case 20:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_VAMPIRISM;
			break;
		case 21: case 22: case 23:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_SMELL_MET;
			break;
		case 24: case 25: case 26: case 27:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_SMELL_MON;
			break;
		case 28: case 29: case 30:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_BLINK;
			break;
		case 31: case 32:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_EAT_ROCK;
			break;
		case 33: case 34:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_SWAP_POS;
			break;
		case 35: case 36: case 37:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_SHRIEK;
			break;
		case 38: case 39: case 40:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_ILLUMINE;
			break;
		case 41: case 42:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_DET_CURSE;
			break;
		case 43: case 44: case 45:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_BERSERK;
			break;
		case 46:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_POLYMORPH;
			break;
		case 47: case 48:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_MIDAS_TCH;
			break;
		case 49:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_GROW_MOLD;
			break;
		case 50: case 51: case 52:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_RESIST;
			break;
		case 53: case 54: case 55:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_EARTHQUAKE;
			break;
		case 56:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_EAT_MAGIC;
			break;
		case 57: case 58:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_WEIGH_MAG;
			break;
		case 59:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_STERILITY;
			break;
		case 60: case 61:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_PANIC_HIT;
			break;
		case 62: case 63: case 64:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_DAZZLE;
			break;
		case 65: case 66: case 67:
			muta_class = &(p_ptr->muta1);
                        muta_which = MUT1_DARKRAY;
			break;
		case 68: case 69:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_RECALL;
			break;
		case 70:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_BANISH;
			break;
		case 71: case 72:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_COLD_TOUCH;
			break;
		case 73: case 74:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_LAUNCHER;
			break;
		case 75:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_BERS_RAGE;
			muta_desc = "You become subject to fits of berserk rage!";
			break;
		case 76:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_COWARDICE;
			muta_desc = "You become an incredible coward!";
			break;
		case 77:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_RTELEPORT;
			muta_desc = "Your position seems very uncertain...";
			break;
		case 78:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_ALCOHOL;
			muta_desc = "Your body starts producing alcohol!";
			break;
		case 79:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_HALLU;
			muta_desc = "You are afflicted by a hallucinatory insanity!";
			break;
		case 80:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_FLATULENT;
			muta_desc = "You become subject to uncontrollable flatulence.";
			break;
		case 81: case 82:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_SCOR_TAIL;
			muta_desc = "You grow a scorpion tail!";
			break;
		case 83: case 84:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_HORNS;
                        muta_desc = "Horns pop forth from your forehead!";
			break;
		case 85: case 86:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_BEAK;
			muta_desc = "Your mouth turns into a sharp, powerful beak!";
			break;
		case 87: case 88:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_ATT_DEMON;
			muta_desc = "You start attracting demons.";
			break;
		case 89:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_PROD_MANA;
			muta_desc = "You start producing magical energy uncontrollably.";
			break;
		case 90: case 91:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_SPEED_FLUX;
			muta_desc = "You become manic-depressive.";
			break;
		case 92: case 93:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_BANISH_ALL;
			muta_desc = "You feel a terrifying power lurking behind you.";
			break;
		case 94:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_EAT_LIGHT;
			muta_desc = "You feel a strange kinship with Ungoliant.";
			break;
		case 95: case 96:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_TRUNK;
			muta_desc = "Your nose grows into an elephant-like trunk.";
			break;
		case 97:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_ATT_ANIMAL;
			muta_desc = "You start attracting animals.";
			break;
		case 98:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_TENTACLES;
			muta_desc = "Evil-looking tentacles sprout from your sides.";
			break;
		case 99:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_RAW_CHAOS;
			muta_desc = "You feel the universe is less stable around you.";
			break;
		case 100: case 101: case 102:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_NORMALITY;
			muta_desc = "You feel strangely normal.";
			break;
		case 103:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_WRAITH;
			muta_desc = "You start to fade in and out of the physical world.";
			break;
		case 104:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_POLY_WOUND;
			muta_desc = "You feel forces of chaos entering your old scars.";
			break;
		case 105:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_WASTING;
			muta_desc = "You suddenly contract a horrible wasting disease.";
			break;
		case 106:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_ATT_DRAGON;
			muta_desc = "You start attracting dragons.";
			break;
		case 107: case 108:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_WEIRD_MIND;
			muta_desc = "Your thoughts suddenly take off in strange directions.";
			break;
		case 109:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_NAUSEA;
			muta_desc = "Your stomach starts to roil nauseously.";
			break;
		case 110: case 111:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_CHAOS_GIFT;
			muta_desc = "You attract the notice of a chaos deity!";
			break;
		case 112:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_WALK_SHAD;
			muta_desc = "You feel like reality is as thin as paper.";
			break;
		case 113: case 114:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_WARNING;
			muta_desc = "You suddenly feel paranoid.";
			break;
		case 115:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_INVULN;
			muta_desc = "You are blessed with fits of invulnerability.";
			break;
		case 116: case 117:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_SP_TO_HP;
			muta_desc = "You are subject to fits of magical healing.";
			break;
		case 118:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_HP_TO_SP;
			muta_desc = "You are subject to fits of painful clarity.";
			break;
		case 119:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_DISARM;
			muta_desc = "Your feet grow to four times their former size.";
			break;
		case 120: case 121: case 122:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_HYPER_STR;
			muta_desc = "You turn into a superhuman he-man!";
			break;
		case 123: case 124: case 125:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_PUNY;
			muta_desc = "Your muscles wither away...";
			break;
		case 126: case 127: case 128:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_HYPER_INT;
			muta_desc = "Your brain evolves into a living computer!";
			break;
		case 129: case 130: case 131:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_MORONIC;
			muta_desc = "Your brain withers away...";
			break;
		case 132: case 133:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_RESILIENT;
			muta_desc = "You become extraordinarily resilient.";
			break;
		case 134: case 135:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_XTRA_FAT;
			muta_desc = "You become sickeningly fat!";
			break;
		case 136: case 137:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_ALBINO;
			muta_desc = "You turn into an albino! You feel frail...";
			break;
		case 138: case 139: case 140:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_FLESH_ROT;
			muta_desc = "Your flesh is afflicted by a rotting disease!";
			break;
		case 141: case 142:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_SILLY_VOI;
			muta_desc = "Your voice turns into a ridiculous squeak!";
			break;
		case 143: case 144:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_BLANK_FAC;
			muta_desc = "Your face becomes completely featureless!";
			break;
		case 145:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_ILL_NORM;
			muta_desc = "You start projecting a reassuring image.";
			break;
		case 146: case 147: case 148:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_XTRA_EYES;
			muta_desc = "You grow an extra pair of eyes!";
			break;
		case 149: case 150:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_MAGIC_RES;
			muta_desc = "You become resistant to magic.";
			break;
		case 151: case 152: case 153:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_XTRA_NOIS;
			muta_desc = "You start making strange noise!";
			break;
		case 154: case 155: case 156:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_INFRAVIS;
			muta_desc = "Your infravision is improved.";
			break;
		case 157: case 158:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_XTRA_LEGS;
			muta_desc = "You grow an extra pair of legs!";
			break;
		case 159: case 160:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_SHORT_LEG;
			muta_desc = "Your legs turn into short stubs!";
			break;
		case 161: case 162:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_ELEC_TOUC;
			muta_desc = "Electricity starts running through you!";
			break;
		case 163: case 164:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_FIRE_BODY;
			muta_desc = "Your body is enveloped in flames!";
			break;
		case 165: case 166: case 167:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_WART_SKIN;
			muta_desc = "Disgusting warts appear everywhere on you!";
			break;
		case 168: case 169: case 170:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_SCALES;
			muta_desc = "Your skin turns into black scales!";
			break;
		case 171: case 172:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_IRON_SKIN;
			muta_desc = "Your skin turns to steel!";
			break;
		case 173: case 174:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_WINGS;
			muta_desc = "You grow a pair of wings.";
			break;
		case 175: case 176: case 177:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_FEARLESS;
			muta_desc = "You become completely fearless.";
			break;
		case 178: case 179:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_REGEN;
			muta_desc = "You start regenerating.";
			break;
		case 180: case 181:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_ESP;
			muta_desc = "You develop a telepathic ability!";
			break;
		case 182: case 183: case 184:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_LIMBER;
			muta_desc = "Your muscles become limber.";
			break;
		case 185: case 186: case 187:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_ARTHRITIS;
			muta_desc = "Your joints suddenly hurt.";
			break;
		case 188:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_RES_TIME;
			muta_desc = "You feel immortal.";
			break;
		case 189:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_VULN_ELEM;
			muta_desc = "You feel strangely exposed.";
			break;
		case 190: case 191: case 192:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_MOTION;
			muta_desc = "You move with new assurance.";
			break;
		case 193: case 194:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_SUS_STATS;
			muta_desc = "You feel like you can recover from anything.";
			break;
		default:
			muta_class = 0;
			muta_which = 0;
		}

		if (muta_class && muta_which)
		{
			if (!(*(muta_class) & muta_which))
			{
				muta_chosen = TRUE;
			}
		}
		if (muta_chosen == TRUE) break;
	}

	if (!muta_chosen)
	{
		msg_print("You feel normal.");
		return FALSE;
	}
	else
	{
                if (p_ptr->pracem == RMOD_VAMPIRE &&
		  !(p_ptr->muta1 & MUT1_HYPN_GAZE) &&
		   (randint(10)<7))
		{
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_HYPN_GAZE;
		}

                if (p_ptr->pclass == CLASS_SYMBIANT &&
                  !(p_ptr->muta1 & MUT1_GROW_MOLD) &&
		   (randint(10)<7))
		{
			muta_class = &(p_ptr->muta1);
                        muta_which = MUT1_GROW_MOLD;
		}

                msg_print("You get corrupted by the dark powers of Morgoth!");
                if (muta_class != &(p_ptr->muta1)) msg_print(muta_desc);
		*(muta_class) |= muta_which;
		p_ptr->update |= PU_BONUS;
		handle_stuff();

		if (muta_class == &(p_ptr->muta3))
		{
			if (muta_which == MUT3_PUNY)
			{
				if (p_ptr->muta3 & MUT3_HYPER_STR)
				{
					msg_print("You no longer feel super-strong!");
					p_ptr->muta3 &= ~(MUT3_HYPER_STR);
				}
			}
			else if (muta_which == MUT3_HYPER_STR)
			{
				if (p_ptr->muta3 & MUT3_PUNY)
				{
					msg_print("You no longer feel puny!");
					p_ptr->muta3 &= ~(MUT3_PUNY);
				}
			}
			else if (muta_which == MUT3_MORONIC)
			{
				if (p_ptr->muta3 & MUT3_HYPER_INT)
				{
					msg_print("Your brain is no longer a living computer.");
					p_ptr->muta3 &= ~(MUT3_HYPER_INT);
				}
			}
			else if (muta_which == MUT3_HYPER_INT)
			{
				if (p_ptr->muta3 & MUT3_MORONIC)
				{
					msg_print("You are no longer moronic.");
					p_ptr->muta3 &= ~(MUT3_MORONIC);
				}
			}
			else if (muta_which == MUT3_IRON_SKIN)
			{
				if (p_ptr->muta3 & MUT3_SCALES)
				{
					msg_print("You lose your scales.");
					p_ptr->muta3 &= ~(MUT3_SCALES);
				}
				if (p_ptr->muta3 & MUT3_FLESH_ROT)
				{
					msg_print("Your flesh rots no longer.");
					p_ptr->muta3 &= ~(MUT3_FLESH_ROT);
				}
				if (p_ptr->muta3 & MUT3_WART_SKIN)
				{
					msg_print("You lose your warts.");
					p_ptr->muta3 &= ~(MUT3_WART_SKIN);
				}
			}
			else if (muta_which == MUT3_WART_SKIN || muta_which == MUT3_SCALES
				|| muta_which == MUT3_FLESH_ROT)
			{
				if (p_ptr->muta3 & MUT3_IRON_SKIN)
				{
					msg_print("Your skin is no longer made of steel.");
					p_ptr->muta3 &= ~(MUT3_IRON_SKIN);
				}
			}
			else if (muta_which == MUT3_FEARLESS)
			{
				if (p_ptr->muta2 & MUT2_COWARDICE)
				{
					msg_print("You are no longer cowardly.");
					p_ptr->muta2 &= ~(MUT2_COWARDICE);
				}
			}
			else if (muta_which == MUT3_FLESH_ROT)
			{
				if (p_ptr->muta3 & MUT3_REGEN)
				{
					msg_print("You stop regenerating.");
					p_ptr->muta3 &= ~(MUT3_REGEN);
				}
			}
			else if (muta_which == MUT3_REGEN)
			{
				if (p_ptr->muta3 & MUT3_FLESH_ROT)
				{
					msg_print("Your flesh stops rotting.");
					p_ptr->muta3 &= ~(MUT3_FLESH_ROT);
				}
			}
			else if (muta_which == MUT3_LIMBER)
			{
				if (p_ptr->muta3 & MUT3_ARTHRITIS)
				{
					msg_print("Your joints stop hurting.");
					p_ptr->muta3 &= ~(MUT3_ARTHRITIS);
				}
			}
			else if (muta_which == MUT3_ARTHRITIS)
			{
				if (p_ptr->muta3 & MUT3_LIMBER)
				{
					msg_print("You no longer feel limber.");
					p_ptr->muta3 &= ~(MUT3_LIMBER);
				}
			}
		}
		else if (muta_class == &(p_ptr->muta2))
		{
			if (muta_which == MUT2_COWARDICE)
			{
				if (p_ptr->muta3 & MUT3_FEARLESS)
				{
					msg_print("You no longer feel fearless.");
					p_ptr->muta3 &= ~(MUT3_FEARLESS);
				}
			}
			if (muta_which == MUT2_BEAK)
			{
				if (p_ptr->muta2 & MUT2_TRUNK)
				{
					msg_print("Your nose is no longer elephantine.");
					p_ptr->muta2 &= ~(MUT2_TRUNK);
				}
			}
			if (muta_which == MUT2_TRUNK)
			{
				if (p_ptr->muta2 & MUT2_BEAK)
				{
					msg_print("You no longer have a hard beak.");
					p_ptr->muta2 &= ~(MUT2_BEAK);
				}
			}
		}
		p_ptr->update |= PU_BONUS;
		handle_stuff();
		return TRUE;
	}
}

bool lose_corruption(int choose_mut)
{
	int attempts_left = 20;
	cptr muta_desc = "";
	bool muta_chosen = FALSE;
	int muta_which = 0;
	u32b * muta_class = 0;
	
	if (choose_mut) attempts_left = 1;

        p_ptr->update |= PU_BONUS;

	while (attempts_left--)
	{
		switch(choose_mut?choose_mut:randint(194))
		{
		case 1: case 2: case 3: case 4:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_SPIT_ACID;
			break;
		case 5: case 6: case 7:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_BR_FIRE;
			break;
		case 8: case 9:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_HYPN_GAZE;
			break;
		case 10: case 11:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_TELEKINES;
			break;
		case 12: case 13: case 14:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_VTELEPORT;
			break;
		case 15: case 16:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_MIND_BLST;
			break;
		case 17: case 18:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_RADIATION;
			break;
		case 19: case 20:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_VAMPIRISM;
                        break;
		case 21: case 22: case 23:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_SMELL_MET;
			break;
		case 24: case 25: case 26: case 27:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_SMELL_MON;
			break;
		case 28: case 29: case 30:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_BLINK;
			break;
		case 31: case 32:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_EAT_ROCK;
			break;
		case 33: case 34:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_SWAP_POS;
			break;
		case 35: case 36: case 37:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_SHRIEK;
			break;
		case 38: case 39: case 40:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_ILLUMINE;
			break;
		case 41: case 42:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_DET_CURSE;
			break;
		case 43: case 44: case 45:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_BERSERK;
			break;
		case 46:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_POLYMORPH;
			break;
		case 47: case 48:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_MIDAS_TCH;
			break;
		case 49:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_GROW_MOLD;
			break;
		case 50: case 51: case 52:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_RESIST;
			break;
		case 53: case 54: case 55:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_EARTHQUAKE;
			break;
		case 56:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_EAT_MAGIC;
			break;
		case 57: case 58:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_WEIGH_MAG;
			break;
		case 59:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_STERILITY;
			break;
		case 60: case 61:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_PANIC_HIT;
			break;
		case 62: case 63: case 64:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_DAZZLE;
			break;
		case 65: case 66: case 67:
			muta_class = &(p_ptr->muta1);
                        muta_which = MUT1_DARKRAY;
			break;
		case 68: case 69:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_RECALL;
			break;
		case 70:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_BANISH;
			break;
		case 71: case 72:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_COLD_TOUCH;
			break;
		case 73: case 74:
			muta_class = &(p_ptr->muta1);
			muta_which = MUT1_LAUNCHER;
			break;
		case 75:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_BERS_RAGE;
			muta_desc = "You are no longer subject to fits of berserk rage!";
			break;
		case 76:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_COWARDICE;
			muta_desc = "You are no longer an incredible coward!";
			break;
		case 77:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_RTELEPORT;
			muta_desc = "Your position seems more certain.";
			break;
		case 78:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_ALCOHOL;
                        muta_desc = "Your body stops producing alcohol!";
			break;
		case 79:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_HALLU;
			muta_desc = "You are no longer afflicted by a hallucinatory insanity!";
			break;
		case 80:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_FLATULENT;
			muta_desc = "You are no longer subject to uncontrollable flatulence.";
			break;
		case 81: case 82:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_SCOR_TAIL;
			muta_desc = "You lose your scorpion tail!";
			break;
		case 83: case 84:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_HORNS;
			muta_desc = "Your horns vanish from your forehead!";
			break;
		case 85: case 86:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_BEAK;
			muta_desc = "Your mouth reverts to normal!";
			break;
		case 87: case 88:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_ATT_DEMON;
			muta_desc = "You stop attracting demons.";
			break;
		case 89:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_PROD_MANA;
			muta_desc = "You stop producing magical energy uncontrollably.";
			break;
		case 90: case 91:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_SPEED_FLUX;
			muta_desc = "You are no longer manic-depressive.";
			break;
		case 92: case 93:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_BANISH_ALL;
			muta_desc = "You no longer feel a terrifying power lurking behind you.";
			break;
		case 94:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_EAT_LIGHT;
			muta_desc = "You feel the world's a brighter place.";
			break;
		case 95: case 96:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_TRUNK;
			muta_desc = "Your nose returns to a normal length.";
			break;
		case 97:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_ATT_ANIMAL;
			muta_desc = "You stop attracting animals.";
			break;
		case 98:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_TENTACLES;
			muta_desc = "Your tentacles vanish from your sides.";
			break;
		case 99:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_RAW_CHAOS;
			muta_desc = "You feel the universe is more stable around you.";
			break;
		case 100: case 101: case 102:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_NORMALITY;
			muta_desc = "You feel normally strange.";
			break;
		case 103:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_WRAITH;
			muta_desc = "You are firmly in the physical world.";
			break;
		case 104:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_POLY_WOUND;
			muta_desc = "You feel forces of chaos departing your old scars.";
			break;
		case 105:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_WASTING;
			muta_desc = "You are cured of the horrible wasting disease!";
			break;
		case 106:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_ATT_DRAGON;
			muta_desc = "You stop attracting dragons.";
			break;
		case 107: case 108:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_WEIRD_MIND;
			muta_desc = "Your thoughts return to boring paths.";
			break;
		case 109:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_NAUSEA;
			muta_desc = "Your stomach stops roiling.";
			break;
		case 110: case 111:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_CHAOS_GIFT;
			muta_desc = "You lose the attention of the chaos deities.";
			break;
		case 112:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_WALK_SHAD;
			muta_desc = "You feel like you're trapped in reality.";
			break;
		case 113: case 114:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_WARNING;
			muta_desc = "You no longer feel paranoid.";
			break;
		case 115:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_INVULN;
			muta_desc = "You are no longer blessed with fits of invulnerability.";
			break;
		case 116: case 117:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_SP_TO_HP;
			muta_desc = "You are no longer subject to fits of magical healing.";
			break;
		case 118:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_HP_TO_SP;
			muta_desc = "You are no longer subject to fits of painful clarity.";
			break;
		case 119:
			muta_class = &(p_ptr->muta2);
			muta_which = MUT2_DISARM;
			muta_desc = "Your feet shrink to their former size.";
			break;
		case 120: case 121: case 122:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_HYPER_STR;
			muta_desc = "Your muscles revert to normal.";
			break;
		case 123: case 124: case 125:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_PUNY;
			muta_desc = "Your muscles revert to normal.";
			break;
		case 126: case 127: case 128:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_HYPER_INT;
			muta_desc = "Your brain reverts to normal.";
			break;
		case 129: case 130: case 131:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_MORONIC;
			muta_desc = "Your brain reverts to normal";
			break;
		case 132: case 133:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_RESILIENT;
			muta_desc = "You become ordinarily resilient again.";
			break;
		case 134: case 135:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_XTRA_FAT;
			muta_desc = "You benefit from a miracle diet!";
			break;
		case 136: case 137:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_ALBINO;
			muta_desc = "You are no longer an albino!";
			break;
		case 138: case 139: case 140:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_FLESH_ROT;
			muta_desc = "Your flesh is no longer afflicted by a rotting disease!";
			break;
		case 141: case 142:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_SILLY_VOI;
			muta_desc = "Your voice returns to normal.";
			break;
		case 143: case 144:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_BLANK_FAC;
			muta_desc = "Your facial features return.";
			break;
		case 145:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_ILL_NORM;
			muta_desc = "You stop projecting a reassuring image.";
			break;
		case 146: case 147: case 148:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_XTRA_EYES;
			muta_desc = "Your extra eyes vanish!";
			break;
		case 149: case 150:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_MAGIC_RES;
			muta_desc = "You become susceptible to magic again.";
			break;
		case 151: case 152: case 153:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_XTRA_NOIS;
			muta_desc = "You stop making strange noise!";
			break;
		case 154: case 155: case 156:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_INFRAVIS;
			muta_desc = "Your infravision is degraded.";
			break;
		case 157: case 158:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_XTRA_LEGS;
			muta_desc = "Your extra legs disappear!";
			break;
		case 159: case 160:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_SHORT_LEG;
			muta_desc = "Your legs lengthen to normal.";
			break;
		case 161: case 162:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_ELEC_TOUC;
			muta_desc = "Electricity stops running through you.";
			break;
		case 163: case 164:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_FIRE_BODY;
			muta_desc = "Your body is no longer enveloped in flames.";
			break;
		case 165: case 166: case 167:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_WART_SKIN;
			muta_desc = "Your warts disappear!";
			break;
		case 168: case 169: case 170:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_SCALES;
			muta_desc = "Your scales vanish!";
			break;
		case 171: case 172:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_IRON_SKIN;
			muta_desc = "Your skin reverts to flesh!";
			break;
		case 173: case 174:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_WINGS;
			muta_desc = "Your wings fall off.";
			break;
		case 175: case 176: case 177:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_FEARLESS;
			muta_desc = "You begin to feel fear again.";
			break;
		case 178: case 179:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_REGEN;
			muta_desc = "You stop regenerating.";
			break;
		case 180: case 181:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_ESP;
			muta_desc = "You lose your telepathic ability!";
			break;
		case 182: case 183: case 184:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_LIMBER;
			muta_desc = "Your muscles stiffen.";
			break;
		case 185: case 186: case 187:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_ARTHRITIS;
			muta_desc = "Your joints stop hurting.";
			break;
		case 188:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_RES_TIME;
			muta_desc = "You feel all too mortal.";
			break;
		case 189:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_VULN_ELEM;
			muta_desc = "You feel less exposed.";
			break;
		case 190: case 191: case 192:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_MOTION;
			muta_desc = "You move with less assurance.";
			break;
		case 193: case 194:
			muta_class = &(p_ptr->muta3);
			muta_which = MUT3_SUS_STATS;
			muta_desc = "You no longer feel like you can recover from anything.";
			break;
		default:
			muta_class = 0;
			muta_which = 0;
		}

		if (muta_class && muta_which)
		{
			if (*(muta_class) & muta_which)
			{
				muta_chosen = TRUE;
			}
		}
		if (muta_chosen == TRUE) break;
	}

	if (!muta_chosen)
	{
		msg_print("You feel oddly normal.");
		return FALSE;
	}
	else
	{
		msg_print(muta_desc);
                if (muta_class != &(p_ptr->muta1)) msg_print(muta_desc);
		*(muta_class) &= ~(muta_which);
		
                p_ptr->update |= PU_BONUS;
		handle_stuff();
		return TRUE;
	}
}


bool get_hack_dir(int *dp)
{
	int		dir;
	cptr    p;
	char command;
	
	
	/* Initialize */
	(*dp) = 0;
	
	/* Global direction */
	dir = 0;
	
	/* (No auto-targetting */
	
	/* Ask until satisfied */
	while (!dir)
	{
		/* Choose a prompt */
		if (!target_okay())
		{
			p = "Direction ('*' to choose a target, Escape to cancel)? ";
		}
		else
		{
			p = "Direction ('5' for target, '*' to re-target, Escape to cancel)? ";
		}
		
		/* Get a command (or Cancel) */
		if (!get_com(p, &command)) break;
		
		/* Convert various keys to "standard" keys */
		switch (command)
		{
			/* Use current target */
			case 'T':
			case 't':
			case '.':
			case '5':
			case '0':
			{
				dir = 5;
				break;
			}
			
			/* Set new target */
			case '*':
			{
				if (target_set(TARGET_KILL)) dir = 5;
				break;
			}
			
			default:
			{
				/* Look up the direction */
				dir = get_keymap_dir(command);
				
				break;
			}
		}
		
		/* Verify requested targets */
		if ((dir == 5) && !target_okay()) dir = 0;
		
		/* Error */
		if (!dir) bell();
	}
	
	/* No direction */
	if (!dir) return (FALSE);
	
	/* Save the direction */
	command_dir = dir;
	
	/* Check for confusion */
	if (p_ptr->confused)
	{
		/* XXX XXX XXX */
		/* Random direction */
		dir = ddd[rand_int(8)];
	}
	
	/* Notice confusion */
	if (command_dir != dir)
	{
		/* Warn the user */
		msg_print("You are confused.");
	}
	
	/* Save direction */
	(*dp) = dir;
	
	/* A "valid" direction was entered */
	return (TRUE);
}


void dump_corruptions(FILE * OutFile)
{
	
	if (!OutFile) return;
	
	if (p_ptr->muta1)
	{
		if (p_ptr->muta1 & MUT1_SPIT_ACID)
		{
			fprintf(OutFile, " You can spit acid (dam lvl).\n");
		}
		if (p_ptr->muta1 & MUT1_BR_FIRE)
		{
			fprintf(OutFile, " You can breathe fire (dam lvl * 2).\n");
		}
		if (p_ptr->muta1 & MUT1_HYPN_GAZE)
		{
			fprintf(OutFile, " Your gaze is hypnotic.\n");
		}
		if (p_ptr->muta1 & MUT1_TELEKINES)
		{
			fprintf(OutFile, " You are telekinetic.\n");
		}
		if (p_ptr->muta1 & MUT1_VTELEPORT)
		{
			fprintf(OutFile, " You can teleport at will.\n");
		}
		if (p_ptr->muta1 & MUT1_MIND_BLST)
		{
			fprintf(OutFile, " You can Mind Blast your enemies.\n");
		}
		if (p_ptr->muta1 & MUT1_RADIATION)
		{
			fprintf(OutFile, " You can emit hard radiation at will.\n");
		}
		if (p_ptr->muta1 & MUT1_VAMPIRISM)
		{
			fprintf(OutFile, " You can drain life from a foe like a vampire.\n");
		}
		if (p_ptr->muta1 & MUT1_SMELL_MET)
		{
			fprintf(OutFile, " You can smell nearby precious metal.\n");
		}
		if (p_ptr->muta1 & MUT1_SMELL_MON)
		{
			fprintf(OutFile, " You can smell nearby monsters.\n");
		}
		if (p_ptr->muta1 & MUT1_BLINK)
		{
			fprintf(OutFile, " You can teleport yourself short distances.\n");
		}
		if (p_ptr->muta1 & MUT1_EAT_ROCK)
		{
			fprintf(OutFile, " You can consume solid rock.\n");
		}
		if (p_ptr->muta1 & MUT1_SWAP_POS)
		{
			fprintf(OutFile, " You can switch locations with another being.\n");
		}
		if (p_ptr->muta1 & MUT1_SHRIEK)
		{
			fprintf(OutFile, " You can emit a horrible shriek.\n");
		}
		if (p_ptr->muta1 & MUT1_ILLUMINE)
		{
			fprintf(OutFile, " You can emit bright light.\n");
		}
		if (p_ptr->muta1 & MUT1_DET_CURSE)
		{
			fprintf(OutFile, " You can feel the danger of evil magic.\n");
		}
		if (p_ptr->muta1 & MUT1_BERSERK)
		{
			fprintf(OutFile, " You can drive yourself into a berserk frenzy.\n");
		}
		if (p_ptr->muta1 & MUT1_POLYMORPH)
		{
			fprintf(OutFile, " You can polymorph yourself at will.\n");
		}
		if (p_ptr->muta1 & MUT1_MIDAS_TCH)
		{
			fprintf(OutFile, " You can turn ordinary items to gold.\n");
		}
		if (p_ptr->muta1 & MUT1_GROW_MOLD)
		{
			fprintf(OutFile, " You can cause mold to grow near you.\n");
		}
		if (p_ptr->muta1 & MUT1_RESIST)
		{
			fprintf(OutFile, " You can harden yourself to the ravages of the elements.\n");
		}
		if (p_ptr->muta1 & MUT1_EARTHQUAKE)
		{
			fprintf(OutFile, " You can bring down the dungeon around your ears.\n");
		}
		if (p_ptr->muta1 & MUT1_EAT_MAGIC)
		{
			fprintf(OutFile, " You can consume magic energy for your own use.\n");
		}
		if (p_ptr->muta1 & MUT1_WEIGH_MAG)
		{
			fprintf(OutFile, " You can feel the strength of the magics affecting you.\n");
		}
		if (p_ptr->muta1 & MUT1_STERILITY)
		{
			fprintf(OutFile, " You can cause mass impotence.\n");
		}
		if (p_ptr->muta1 & MUT1_PANIC_HIT)
		{
			fprintf(OutFile, " You can run for your life after hitting something.\n");
		}
		if (p_ptr->muta1 & MUT1_DAZZLE)
		{
			fprintf(OutFile, " You can emit confusing, blinding radiation.\n");
		}
                if (p_ptr->muta1 & MUT1_DARKRAY)
		{
                        fprintf(OutFile, " You can create a spear of darkness.\n");
		}
		if (p_ptr->muta1 & MUT1_RECALL)
		{
			fprintf(OutFile, " You can travel between town and the depths.\n");
		}
		if (p_ptr->muta1 & MUT1_BANISH)
		{
			fprintf(OutFile, " You can send evil creatures directly to Hell.\n");
		}
		if (p_ptr->muta1 & MUT1_COLD_TOUCH)
		{
			fprintf(OutFile, " You can freeze things with a touch.\n");
		}
		if (p_ptr->muta1 & MUT1_LAUNCHER)
		{
			fprintf(OutFile, " You can hurl objects with great force.\n");
		}
	}

	if (p_ptr->muta2)
	{
		if (p_ptr->muta2 & MUT2_BERS_RAGE)
		{
			fprintf(OutFile, " You are subject to berserker fits.\n");
		}
		if (p_ptr->muta2 & MUT2_COWARDICE)
		{
			fprintf(OutFile, " You are subject to cowardice.\n");
		}
		if (p_ptr->muta2 & MUT2_RTELEPORT)
		{
			fprintf(OutFile, " You are teleporting randomly.\n");
		}
		if (p_ptr->muta2 & MUT2_ALCOHOL)
		{
			fprintf(OutFile, " Your body produces alcohol.\n");
		}
		if (p_ptr->muta2 & MUT2_HALLU)
		{
			fprintf(OutFile, " You have a hallucinatory insanity.\n");
		}
		if (p_ptr->muta2 & MUT2_FLATULENT)
		{
			fprintf(OutFile, " You are subject to uncontrollable flatulence.\n");
		}
		if (p_ptr->muta2 & MUT2_PROD_MANA)
		{
			fprintf(OutFile, " You are producing magical energy uncontrollably.\n");
		}
		if (p_ptr->muta2 & MUT2_ATT_DEMON)
		{
			fprintf(OutFile, " You attract demons.\n");
		}
		if (p_ptr->muta2 & MUT2_SCOR_TAIL)
		{
			fprintf(OutFile, " You have a scorpion tail (poison, 3d7).\n");
		}
		if (p_ptr->muta2 & MUT2_HORNS)
		{
			fprintf(OutFile, " You have horns (dam. 2d6).\n");
		}
		if (p_ptr->muta2 & MUT2_BEAK)
		{
			fprintf(OutFile, " You have a beak (dam. 2d4).\n");
		}
		if (p_ptr->muta2 & MUT2_SPEED_FLUX)
		{
			fprintf(OutFile, " You move faster or slower randomly.\n");
		}
		if (p_ptr->muta2 & MUT2_BANISH_ALL)
		{
			fprintf(OutFile, " You sometimes cause nearby creatures to vanish.\n");
		}
		if (p_ptr->muta2 & MUT2_EAT_LIGHT)
		{
			fprintf(OutFile, " You sometimes feed off of the light around you.\n");
		}
		if (p_ptr->muta2 & MUT2_TRUNK)
		{
			fprintf(OutFile, " You have an elephantine trunk (dam 1d4).\n");
		}
		if (p_ptr->muta2 & MUT2_ATT_ANIMAL)
		{
			fprintf(OutFile, " You attract animals.\n");
		}
		if (p_ptr->muta2 & MUT2_TENTACLES)
		{
			fprintf(OutFile, " You have evil looking tentacles (dam 2d5).\n");
		}
		if (p_ptr->muta2 & MUT2_RAW_CHAOS)
		{
			fprintf(OutFile, " You occasionally are surrounded with raw chaos.\n");
		}
		if (p_ptr->muta2 & MUT2_NORMALITY)
		{
                        fprintf(OutFile, " You may be corrupted, but you're recovering.\n");
		}
		if (p_ptr->muta2 & MUT2_WRAITH)
		{
			fprintf(OutFile, " You fade in and out of physical reality.\n");
		}
		if (p_ptr->muta2 & MUT2_POLY_WOUND)
		{
			fprintf(OutFile, " Your health is subject to chaotic forces.\n");
		}
		if (p_ptr->muta2 & MUT2_WASTING)
		{
			fprintf(OutFile, " You have a horrible wasting disease.\n");
		}
		if (p_ptr->muta2 & MUT2_ATT_DRAGON)
		{
			fprintf(OutFile, " You attract dragons.\n");
		}
		if (p_ptr->muta2 & MUT2_WEIRD_MIND)
		{
			fprintf(OutFile, " Your mind randomly expands and contracts.\n");
		}
		if (p_ptr->muta2 & MUT2_NAUSEA)
		{
			fprintf(OutFile, " You have a seriously upset stomach.\n");
		}
		if (p_ptr->muta2 & MUT2_CHAOS_GIFT)
		{
			fprintf(OutFile, " Chaos deities give you gifts.\n");
		}
		if (p_ptr->muta2 & MUT2_WALK_SHAD)
		{
			fprintf(OutFile, " You occasionally stumble into other shadows.\n");
		}
		if (p_ptr->muta2 & MUT2_WARNING)
		{
			fprintf(OutFile, " You receive warnings about your foes.\n");
		}
		if (p_ptr->muta2 & MUT2_INVULN)
		{
			fprintf(OutFile, " You occasionally feel invincible.\n");
		}
		if (p_ptr->muta2 & MUT2_SP_TO_HP)
		{
			fprintf(OutFile, " Your blood sometimes rushes to your muscles.\n");
		}
		if (p_ptr->muta2 & MUT2_HP_TO_SP)
		{
			fprintf(OutFile, " Your blood sometimes rushes to your head.\n");
		}
		if (p_ptr->muta2 & MUT2_DISARM)
		{
			fprintf(OutFile, " You occasionally stumble and drop things.\n");
		}
	}

	if (p_ptr->muta3)
	{
		if (p_ptr->muta3 & MUT3_HYPER_STR)
		{
			fprintf(OutFile, " You are superhumanly strong (+4 STR).\n");
		}
		if (p_ptr->muta3 & MUT3_PUNY)
		{
			fprintf(OutFile, " You are puny (-4 STR).\n");
		}
		if (p_ptr->muta3 & MUT3_HYPER_INT)
		{
			fprintf(OutFile, " Your brain is a living computer (+4 INT/WIS).\n");
		}
		if (p_ptr->muta3 & MUT3_MORONIC)
		{
			fprintf(OutFile, " You are moronic (-4 INT/WIS).\n");
		}
		if (p_ptr->muta3 & MUT3_RESILIENT)
		{
			fprintf(OutFile, " You are very resilient (+4 CON).\n");
		}
		if (p_ptr->muta3 & MUT3_XTRA_FAT)
		{
			fprintf(OutFile, " You are extremely fat (+2 CON, -2 speed).\n");
		}
		if (p_ptr->muta3 & MUT3_ALBINO)
		{
			fprintf(OutFile, " You are albino (-4 CON).\n");
		}
		if (p_ptr->muta3 & MUT3_FLESH_ROT)
		{
			fprintf(OutFile, " Your flesh is rotting (-2 CON, -1 CHR).\n");
		}
		if (p_ptr->muta3 & MUT3_SILLY_VOI)
		{
			fprintf(OutFile, " Your voice is a silly squeak (-4 CHR).\n");
		}
		if (p_ptr->muta3 & MUT3_BLANK_FAC)
		{
			fprintf(OutFile, " Your face is featureless (-1 CHR).\n");
		}
		if (p_ptr->muta3 & MUT3_ILL_NORM)
		{
			fprintf(OutFile, " Your appearance is masked with illusion.\n");
		}
		if (p_ptr->muta3 & MUT3_XTRA_EYES)
		{
			fprintf(OutFile, " You have an extra pair of eyes (+15 search).\n");
		}
		if (p_ptr->muta3 & MUT3_MAGIC_RES)
		{
			fprintf(OutFile, " You are resistant to magic.\n");
		}
		if (p_ptr->muta3 & MUT3_XTRA_NOIS)
		{
			fprintf(OutFile, " You make a lot of strange noise (-3 stealth).\n");
		}
		if (p_ptr->muta3 & MUT3_INFRAVIS)
		{
			fprintf(OutFile, " You have remarkable infravision (+3).\n");
		}
		if (p_ptr->muta3 & MUT3_XTRA_LEGS)
		{
			fprintf(OutFile, " You have an extra pair of legs (+3 speed).\n");
		}
		if (p_ptr->muta3 & MUT3_SHORT_LEG)
		{
			fprintf(OutFile, " Your legs are short stubs (-3 speed).\n");
		}
		if (p_ptr->muta3 & MUT3_ELEC_TOUC)
		{
			fprintf(OutFile, " Electricity is running through your veins.\n");
		}
		if (p_ptr->muta3 & MUT3_FIRE_BODY)
		{
			fprintf(OutFile, " Your body is enveloped in flames.\n");
		}
		if (p_ptr->muta3 & MUT3_WART_SKIN)
		{
			fprintf(OutFile, " Your skin is covered with warts (-2 CHR, +5 AC).\n");
		}
		if (p_ptr->muta3 & MUT3_SCALES)
		{
			fprintf(OutFile, " Your skin has turned into scales (-1 CHR, +10 AC).\n");
		}
		if (p_ptr->muta3 & MUT3_IRON_SKIN)
		{
			fprintf(OutFile, " Your skin is made of steel (-1 DEX, +25 AC).\n");
		}
		if (p_ptr->muta3 & MUT3_WINGS)
		{
			fprintf(OutFile, " You have wings.\n");
		}
		if (p_ptr->muta3 & MUT3_FEARLESS)
		{
			fprintf(OutFile, " You are completely fearless.\n");
		}
		if (p_ptr->muta3 & MUT3_REGEN)
		{
			fprintf(OutFile, " You are regenerating.\n");
		}
		if (p_ptr->muta3 & MUT3_ESP)
		{
			fprintf(OutFile, " You are telepathic.\n");
		}
		if (p_ptr->muta3 & MUT3_LIMBER)
		{
			fprintf(OutFile, " Your body is very limber (+3 DEX).\n");
		}
		if (p_ptr->muta3 & MUT3_ARTHRITIS)
		{
			fprintf(OutFile, " Your joints ache constantly (-3 DEX).\n");
		}
		if (p_ptr->muta3 & MUT3_RES_TIME)
		{
			fprintf(OutFile, " You are protected from the ravages of time.\n");
		}
		if (p_ptr->muta3 & MUT3_VULN_ELEM)
		{
			fprintf(OutFile, " You are susceptible to damage from the elements.\n");
		}
		if (p_ptr->muta3 & MUT3_MOTION)
		{
			fprintf(OutFile, " Your movements are precise and forceful (+1 STL).\n");
		}
		if (p_ptr->muta3 & MUT3_SUS_STATS)
		{
			fprintf(OutFile, " Your body resists serious damage.\n");
		}
	}
}

/*
 * Set "p_ptr->grace", notice observable changes
 */
void set_grace(s32b v) {
  p_ptr->grace = v;
  p_ptr->update |= PU_BONUS;
  handle_stuff();
}

/*
 * Show religious info.
 */

bool show_god_info(bool ext) {
  int badness = interpret_grace();
  int pgod = p_ptr->pgod - 1;
  int tmp;

  deity *d_ptr;

  if (pgod < 0) {
    msg_print("You don't worship anyone.");
    msg_print(NULL);
    return FALSE;

  } else {
    d_ptr = &deity_info[pgod];

    msg_print(NULL);

    character_icky = TRUE;
    Term_save();

    roff(format("You worship %s, the %s deity of %s. ", d_ptr->name,
                deity_rarity[d_ptr->rarity], d_ptr->god_of));
    roff(format("%s is %s, and you are %s by him/her. ", d_ptr->name,
               deity_niceness[d_ptr->grace_deduction],
               deity_standing[badness]));
    if (noscore)
       roff(format("Your current grace is %d, and god's favor is %d. ",
       p_ptr->grace, p_ptr->god_favor));
    roff("\n");

    if (ext || noscore) {
      int fav = badness - interpret_favor();

      roff(format("It is %s that your prayers will be answered.\n", favor_text[fav]));
    }

    tmp = inkey();

    Term_load();
    character_icky = FALSE;
  }

  return TRUE;
}

/*
 * Return a number denoting your current standing with your god,
 * ranging from 0 (really bad) to 10 (really good).
 * Note that 0-5 mean ``cursed'', 6-7 mean ``neutral'',
 * and 8-10 mean ``blessed''.
 */

int interpret_grace(void) {
  if (p_ptr->grace <      -60000) return 0;
  else if (p_ptr->grace < -50000) return 1;
  else if (p_ptr->grace < -40000) return 2;
  else if (p_ptr->grace < -30000) return 3;
  else if (p_ptr->grace < -20000) return 4;
  else if (p_ptr->grace < -10000) return 5;
  else if (p_ptr->grace <      0) return 6;

  else if (p_ptr->grace >  20000) return 10;
  else if (p_ptr->grace >  10000) return 9;
  else if (p_ptr->grace >   5000) return 8;
  else if (p_ptr->grace >      0) return 7;

  /* Should never happen! */
  return -1;
}


/*
 * Same as interpret_grace, but for p_ptr->god_favor.
 */

int interpret_favor(void) {
  if (p_ptr->god_favor <      -60000) return 0;
  else if (p_ptr->god_favor < -50000) return 1;
  else if (p_ptr->god_favor < -40000) return 2;
  else if (p_ptr->god_favor < -30000) return 3;
  else if (p_ptr->god_favor < -20000) return 4;
  else if (p_ptr->god_favor < -10000) return 5;
  else if (p_ptr->god_favor <      0) return 6;

  else if (p_ptr->god_favor >  20000) return 10;
  else if (p_ptr->god_favor >  10000) return 9;
  else if (p_ptr->god_favor >   5000) return 8;
  else if (p_ptr->god_favor >      0) return 7;

  /* Should never happen! */
  return -1;
}

/*
 * Here come the side effect functions. They are classified according
 * to how beneficial they are, to avoid a giant nested switch/case
 * statement.
 */

/* Great side effect. */

void great_side_effect(void) {
        int dir, tmp, a, b, power = ((deity_info[p_ptr->pgod].race1 == p_ptr->prace) || (deity_info[p_ptr->pgod].race2 == p_ptr->prace));

        int per_hp = p_ptr->chp * 100 / p_ptr->mhp;
        int per_sp = (p_ptr->msp) ? p_ptr->csp * 100 / p_ptr->msp : 100;

        tmp = randint(100);

        if((per_hp < 40) || (per_sp < 20))
        {
                if(per_hp > per_sp)
                {
                        p_ptr->csp = p_ptr->msp;
                        p_ptr->csp_frac = 0;
                        msg_print("Your feel your head clear.");
                        p_ptr->redraw |= (PR_MANA);
                        p_ptr->window |= (PW_PLAYER);
                        p_ptr->window |= (PW_SPELL);
                }
                else
                {
                        p_ptr->chp = p_ptr->mhp;
                        p_ptr->chp_frac = 0;
                        msg_print("You feel your body healed.");
                        p_ptr->redraw |= (PR_HP);
                        p_ptr->window |= (PW_PLAYER);
                }
        }
        else if(magik(50))
        {
        switch(p_ptr->pgod)
        {
                case GOD_YAVANNA:
                        msg_print("Yavanna summons some Ents to help you!");
                        for(a = 0; a < 5 + (5 * power); a++)
                        {
                                int y = py, x = px;

                                scatter(&y, &x, py, px, 5, 0);
                                place_monster_one(y, x, test_monster_name("Ent"), 0, FALSE, MSTATUS_FRIEND);
                        }
                        break;
                case GOD_ULMO:
                        msg_print("The power of the Ulmo is flooding the area around you!");
                        for(a = px - 7 - (3 * power); a < px + 7 + (3 * power); a++)
                                for(b = py - 7 - (3 * power); b < py + 7 + (3 * power); b++)
                                        if(distance(py, px, b, a) <= 7 + (3 * power))
                                        {
                                                if(f_info[cave[b][a].feat].flags1 & FF1_FLOOR)
                                                {
                                                        cave_set_feat(b, a, FEAT_DEEP_WATER);
                                                }
                                        }
                        break;
                case GOD_AULE:
                        msg_print("Aule, the Smith, reworks the level!");
                        alter_reality();
                        break;
                case GOD_MELKOR:
                        msg_print("The power of Melkor focuses into a Darkness Storm!");
                        if (get_aim_dir(&dir))
                               fire_ball(GF_DARK, dir, 200 + (200 * power), 3 + (2 * power));
                        break;
                case GOD_TILION:
                        msg_print("You are enlighted by the power of Tilion!");
                        wiz_lite_extra();
                        if(power)
                        {
                                detect_all();
                        }
                        break;
                case GOD_ARIEN:
                        msg_print("You are enlighted by the power of Arien!");
                        wiz_lite_extra();
                        if(power)
                        {
                                detect_all();
                        }
                        break;
                case GOD_TULKAS:
                        msg_print("The power of Tulkas floods through your body!");
                        set_invuln(16 + (8 * power));
                        break;
                case GOD_MANWE:
                        msg_print("Suddently a great wind coming from the west is blowing up everything!");
                        mass_genocide(FALSE);
                        break;
                case GOD_VARDA:
                        msg_print("You are enlighted by the power of Varda!");
                        wiz_lite_extra();
                        if(power)
                        {
                                detect_all();
                        }
                        break;
                case GOD_ERU:
                        msg_print("The power of Eru Iluvatar morphs you into a Vala!");
                        set_mimic(10 + (10 * power), MIMIC_VALAR);
                        break;
                case GOD_RNG:
                        msg_print("The power of The RNG randomizes you");
                        if(rand_int(2) == 0)
                                do_inc_stat(A_STR);
                        else
                                do_dec_stat(A_STR, STAT_DEC_NORMAL);
                        if(rand_int(2) == 0)
                                do_inc_stat(A_INT);
                        else
                                do_dec_stat(A_INT, STAT_DEC_NORMAL);
                        if(rand_int(2) == 0)
                                do_inc_stat(A_WIS);
                        else
                                do_dec_stat(A_WIS, STAT_DEC_NORMAL);
                        if(rand_int(2) == 0)
                                do_inc_stat(A_DEX);
                        else
                                do_dec_stat(A_DEX, STAT_DEC_NORMAL);
                        if(rand_int(2) == 0)
                                do_inc_stat(A_CON);
                        else
                                do_dec_stat(A_CON, STAT_DEC_NORMAL);
                        if(rand_int(2) == 0)
                                do_inc_stat(A_CHR);
                        else
                                do_dec_stat(A_CHR, STAT_DEC_NORMAL);
                        break;
        }
        }
        else
        {
  if (tmp <= 10) {
    if (summon_specific_friendly(py, px, dun_level+rand_spread(10, 5), damroll(4,6), FALSE)) {
      msg_print("Something materializes out of thin air.");
    } else {
      msg_print("You feel a strange tingling, but the feeling passes.");
    }
  } else if (tmp <= 20) {
    msg_print("You feel something land at your feet.");
    acquirement(py, px, 1, 1, 1);
  } else if (tmp <= 30) {
    enchant_spell(0, 0, randint(3) + 2, 0);
  } else if (tmp <= 40) {
    enchant_spell(randint(3), randint(3), 0, 0);
  } else if (tmp <= 50) {
    fire_explosion(py, px, GF_MAKE_GLYPH, 9, 0);
  } else if (tmp <= 60) {
    recharge(100);
  } else if (tmp <= 70) {
    msg_print("You feel like someone has taken a burden off your back.");
    remove_all_curse();
  } else if (tmp <= 80) {
    identify_fully();
  } else if (tmp <= 90) {
    msg_print("The power of your deity restores you!");
    hp_player(5000);
    (void)set_poisoned(0);
    (void)set_blind(0);
    (void)set_confused(0);
    (void)set_image(0);
    (void)set_stun(0);
    (void)set_cut(0);
    (void)do_res_stat(A_STR);
    (void)do_res_stat(A_CON);
    (void)do_res_stat(A_DEX);
    (void)do_res_stat(A_WIS);
    (void)do_res_stat(A_INT);
    (void)do_res_stat(A_CHR);
  } else if (tmp <= 100) {
    msg_print("You feel a great surge of holiness!");
    do_inc_stat(A_STR);
    do_inc_stat(A_INT);
    do_inc_stat(A_WIS);
    do_inc_stat(A_DEX);
    do_inc_stat(A_CON);
    do_inc_stat(A_CHR);
  }
  }
}

/* Nasty side effect. */

void nasty_side_effect(void) {
  int tmp;

  tmp = randint(100);
  if (tmp < 10) {
    set_poisoned(p_ptr->poisoned + 10 + randint(10));
  } else if (tmp < 20) {
    set_confused(p_ptr->confused + 10 + randint(10));
  } else if (tmp < 30) {
    set_blind(p_ptr->blind + 10 + randint(10));
  } else if (tmp < 40) {
    set_slow(p_ptr->slow + 10 + randint(10));
  } else if (tmp < 50) {
    set_cut(p_ptr->cut + 10 + randint(100));
  } else if (tmp < 60) {
    set_stun(p_ptr->stun + randint(110));
  } else if (tmp < 70) {
    msg_print("You hear a loud shriek!");
    aggravate_monsters(1);
  } else if (tmp < 80) {
    set_image(p_ptr->image + 20 + randint(20));
  } else if (tmp < 90) {
    msg_print("You feel very sick.");
    set_slow(p_ptr->slow + 10 + randint(10));
    set_confused(p_ptr->confused + 10 + randint(10));
    set_poisoned(p_ptr->poisoned + 10 + randint(10));
  } else if (tmp < 100) {
    if (summon_specific(py, px, dun_level+randint(7), damroll(4,6))) {
      msg_print("Something materializes out of thin air.");
    } else {
      msg_print("You feel a strange tingling, but the feeling passes.");
    }
  }
}

/* Deadly side effect. */

void deadly_side_effect(bool god) {
  int tmp;
  bool (*boom)(int, int, int, int, int);

  /* Lisp-like hack to save typing. */
  if (god) {
    boom = fire_godly_wrath;
  } else {
    boom = fire_explosion;
  }

  tmp = randint(100);

  if (tmp <= 10) {
    if (summon_specific(py, px, dun_level+20, damroll(4,6))) {
      msg_print("Something materializes out of thin air.");
    } else {
      msg_print("You feel a strange tingling, but the feeling passes.");
    }
  } else if (tmp <= 20) {
    activate_ty_curse();
  } else if (tmp <= 30) {
    msg_print("The world twists!");
    destroy_area(py, px, 9, 0);
  } else if (tmp <= 40) {
    msg_print("Your nerves and muscles feel weak and lifeless.");
    (void)dec_stat(A_STR, 5, TRUE);
    (void)dec_stat(A_INT, 5, TRUE);
    (void)dec_stat(A_WIS, 5, TRUE);
    (void)dec_stat(A_DEX, 5, TRUE);
    (void)dec_stat(A_CON, 5, TRUE);
    (void)dec_stat(A_CHR, 5, TRUE);
  } else if (tmp <= 50) {
    msg_print("You feel somehow inadequate...");
    p_ptr->exp -= (p_ptr->exp / 4);
    p_ptr->max_exp -= (p_ptr->exp / 4);
    check_experience();
  } else if (tmp <= 60) {
    msg_print("Your whole life flashes before your eyes.");
    boom(py, px, GF_TIME, 5, 100);
  } else if (tmp <= 70) {
    msg_print("Everything seems grayer somehow...");
    boom(py, px, GF_DISENCHANT, 5, 100);
  } else if (tmp <= 80) {
    msg_print("There is a loud cackle...");
    boom(py, px, GF_MAKE_TRAP, 9, 0);
  } else if (tmp <= 90) {
    msg_print("Something is trying to destroy your brain!");
    take_sanity_hit(damroll(10,10), "blasted by an angry deity");
  } else if (tmp <= 100) {
    godly_wrath_blast();
  }
}

/*
 * Fire a godly blast from the sky.
 * Note that only attacks which are not resisted are used.
 * (Gods are omnipotent, aren't they?)
 */

void godly_wrath_blast(void) {
  int tmp;
  int type = 0;
  bool ok = FALSE;
  int flg = PROJECT_STOP | PROJECT_GRID | PROJECT_ITEM | PROJECT_KILL;

  while (1) {
    tmp = randint(10);

    switch (tmp) {
    case 1:
      if (!p_ptr->immune_acid) {
        type = GF_ACID;
        ok = TRUE;
        msg_print("You are blasted by acid from the sky!");
      }
      break;

    case 2:
      if (!p_ptr->immune_elec) {
        type = GF_ELEC;
        ok = TRUE;
        msg_print("You are blasted by a giant ball lightning from the sky!");
      }
      break;

    case 3:
      if (!(p_ptr->resist_pois || p_ptr->oppose_pois)) {
        type = GF_POIS;
        ok = TRUE;
        msg_print("A poisonous cloud descends from the sky!");
      }
      break;

    case 4:
      if (!p_ptr->resist_neth) {
        type = GF_NETHER;
        ok = TRUE;
        msg_print("A force of death surrounds you!");
      }
      break;

    case 5:
      if (!(p_ptr->resist_sound || p_ptr->resist_conf)) {
        type = GF_WATER;
        ok = TRUE;
        msg_print("A flood of water falls from the sky!");
      }
      break;

    case 6:
      if (!p_ptr->resist_chaos) {
        type = GF_CHAOS;
        ok = TRUE;
        msg_print("You are blasted by a gale of chaos!");
      }
      break;

    case 7:
      if (!p_ptr->resist_shard) {
        type = GF_SHARDS;
        ok = TRUE;
        msg_print("A multitude of shards descends on you from the sky!");
      }
      break;

    case 8:
      if (!p_ptr->resist_disen) {
        type = GF_DISENCHANT;
        ok = TRUE;
        msg_print("You feel your magical aura wane!");
      }
      break;

    case 9:
      type = GF_TIME;
      ok = TRUE;
      msg_print("A dizzying array of blinking dots falls from the sky!");
      break;

    case 10:
      type = GF_METEOR;
      ok = TRUE;
      msg_print("A giant meteor falls on you from the sky!");
      break;
    }

    if (ok) break;
  }

  (void)project(-99, 1, py, px, damroll(4, ((p_ptr->lev / 3) > 5)?(p_ptr->lev / 3):5), type, flg);
}

bool test_object_wish(char *name, object_type *o_ptr, object_type *forge, char *what)
{
        int i, j;
        char buf[200];

        /* try all objects, this *IS* a very ugly and slow method :( */
        for (i = 0; i < max_k_idx; i++)
        {
                object_kind *k_ptr = &k_info[i];

                o_ptr = forge;

                if (!k_ptr->name) continue;
                if (k_ptr->flags3 & TR3_NORM_ART) continue;
                if (k_ptr->flags3 & TR3_INSTA_ART) continue;

                object_prep(o_ptr, i);
                o_ptr->name1 = 0;
                o_ptr->name2 = 0;
                apply_magic(o_ptr, dun_level, FALSE, FALSE, FALSE);
                object_aware(o_ptr);
                object_known(o_ptr);
                object_desc(buf, o_ptr, FALSE, 0);
                strlower(buf);

                if (strstr(name, buf))
                {
                        /* You can't wish for a wish ! */
                        if ((o_ptr->tval == TV_STAFF) && (o_ptr->sval == SV_STAFF_WISHING))
                        {
                                msg_format("You cannot %s for a wish !", what);
                                return FALSE;
                        }

                        /* try all ego */
                        for (j = max_e_idx - 1; j >= 0; j--)
                        {
                                ego_item_type *e_ptr = &e_info[j];
                                bool ok = FALSE;

                                if (j && !e_ptr->name) continue;

                                /* Must have the correct fields */
                                if (j)
                                {
                                        int z;

                                        for (z = 0; z < 6; z++)
                                        {
                                                if (e_ptr->tval[z] == k_ptr->tval)
                                                {
                                                        if ((e_ptr->min_sval[z] <= k_ptr->sval) && (e_ptr->max_sval[z] >= k_ptr->sval)) ok = TRUE;
                                                }
                                                if (ok) break;
                                        }
                                        if (!ok)
                                        {
                                                continue;
                                        }
                                }

                                object_prep(o_ptr, i);
                                o_ptr->name1 = 0;
                                o_ptr->name2 = j;
                                apply_magic(o_ptr, dun_level, FALSE, FALSE, FALSE);
                                object_aware(o_ptr);
                                object_known(o_ptr);
                                object_desc(buf, o_ptr, FALSE, 0);
                                strlower(buf);

                                if(!stricmp(buf, name))
                                {
                                        /* Don't search any more */
                                        return TRUE;
                                }
                        }
                }
        }
        return FALSE;
}

void clean_wish_name(char *buf, char *name)
{
        char *p;
        int i, j;

        /* Lowercase the wish */
        strlower(buf);

        /* Nuke uneccesary spaces */
        p = buf;
        while (*p == ' ') p++;
        i = 0;
        j = 0;
        while (p[i])
        {
                if ((p[i] == ' ') && (p[i + 1] == ' '))
                {
                        i++;
                        continue;
                }
                name[j++] = p[i++];
        }
        name[j++] = '\0';
        if (j)
        {
                j--;
                while (j && (name[j] == ' '))
                {
                        name[j] = '\0';
                        j--;
                }
        }
}

/*
 * Allow the player to make a wish
 */
void make_wish(void)
{
        char buf[200], name[200], *mname;
        int i, j, mstatus = MSTATUS_ENEMY;
        object_type forge, *o_ptr = &forge;

        /* Make an empty string */
        buf[0] = 0;

        /* Ask for the wish */
        if(!get_string("Wish for what? ", buf, 80)) return;

        clean_wish_name(buf, name);

        /* You can't wish for a wish ! */
        if (strstr(name, "wish"))
        {
                msg_print("You can't wish for a wish !");
                return;
        }

        if (test_object_wish(name, o_ptr, &forge, "wish"))
        {
                msg_print("Your wish become truth!");

                /* Give it to the player */
                drop_near(o_ptr, -1, py, px);

                return;
        }

        /* try monsters */
        if (prefix(name, "enemy ")) { mstatus = MSTATUS_ENEMY; mname = name + 6; }
        else if (prefix(name, "neutral ")) { mstatus = MSTATUS_NEUTRAL; mname = name + 8; }
        else if (prefix(name, "friendly ")) { mstatus = MSTATUS_FRIEND; mname = name + 9; }
        else if (prefix(name, "pet ")) { mstatus = MSTATUS_PET; mname = name + 4; }
        else if (prefix(name, "companion "))
        {
                if (can_create_companion()) mstatus = MSTATUS_COMPANION;
                else mstatus = MSTATUS_PET;
                mname = name + 10;
        }
        else mname = name;
        for (i = 1; i < max_r_idx - 1; i++)
        {
                monster_race *r_ptr = &r_info[i];

                if (!r_ptr->name) continue;

                if (r_ptr->flags9 & RF9_SPECIAL_GENE) continue;
                if (r_ptr->flags9 & RF9_NEVER_GENE) continue;
                if (r_ptr->flags1 & RF1_UNIQUE) continue;

                sprintf(buf, "%s", r_ptr->name + r_name);
                strlower(buf);

                if (strstr(mname, buf))
                {
                        /* try all ego */
                        for (j = max_re_idx - 1; j >= 0; j--)
                        {
                                monster_ego *re_ptr = &re_info[j];

                                if (j && !re_ptr->name) continue;

                                if (!mego_ok(i, j)) continue;

                                if (j)
                                {
                                        if (re_ptr->before) sprintf(buf, "%s %s", re_name + re_ptr->name, r_ptr->name + r_name);
                                        else sprintf(buf, "%s %s", r_ptr->name + r_name, re_name + re_ptr->name);
                                }
                                else
                                {
                                        sprintf(buf, "%s", r_ptr->name + r_name);
                                }
                                strlower(buf);

                                if(!stricmp(mname, buf))
                                {
                                        int wy = py, wx = px;
                                        int attempts = 100;

                                        do
                                        {
                                                scatter(&wy, &wx, py, px, 5, 0);
                                        }
                                        while (!(in_bounds(wy, wx) && cave_floor_bold(wy, wx)) && --attempts);

                                        /* Create the monster */
                                        if (place_monster_one(wy, wx, i, j, FALSE, mstatus))
                                                msg_print("Your wish become truth!");

                                        /* Don't search any more */
                                        return;
                                }
                        }
                }
        }
}


/* Corrupted have a 1/3 chance of losing a mutation each time this is called, 
   assuming they have any in the first place */

void corrupt_corrupted (void)
{
	if (randint(3) == 1)
        {
                if (!(lose_corruption (0)))
                        gain_random_corruption(0);
        }
        else
                gain_random_corruption(0);
        return;
}

