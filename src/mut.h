/* New Mutation System, carefully crafted to be backwards compatible with the old ... 
   You are free to add new mutations (though extending MUT_FLAG_SIZE requires save file
   work), and place activatable mutations where ever you please.
*/
#define MUT_SPIT_ACID      0   /* Start of old mutations */
#define MUT_BR_FIRE        1
#define MUT_HYPN_GAZE      2
#define MUT_TELEKINESIS    3
#define MUT_TELEPORT       4
#define MUT_MIND_BLAST     5
#define MUT_RADIATION      6
#define MUT_VAMPIRISM      7
#define MUT_SMELL_METAL    8
#define MUT_SMELL_MONSTERS 9
#define MUT_BLINK          10
#define MUT_EAT_ROCK       11
#define MUT_SWAP_POS       12
#define MUT_SHRIEK         13
#define MUT_ILLUMINE       14
#define MUT_DET_CURSE      15
#define MUT_BERSERK        16
#define MUT_POLYMORPH      17
#define MUT_MIDAS_TOUCH    18
#define MUT_GROW_MOLD      19
#define MUT_RESIST         20
#define MUT_EARTHQUAKE     21
#define MUT_EAT_MAGIC      22
#define MUT_WEIGH_MAGIC    23
#define MUT_STERILITY      24
#define MUT_PANIC_HIT      25
#define MUT_DAZZLE         26
#define MUT_LASER_EYE      27
#define MUT_RECALL         28
#define MUT_BANISH         29
#define MUT_COLD_TOUCH     30
#define MUT_LAUNCHER       31

#define MUT_BERS_RAGE      32
#define MUT_COWARDICE      33
#define MUT_TELEPORT_RND   34
#define MUT_ALCOHOL        35
#define MUT_HALLUCINATION  36
#define MUT_FLATULENT      37
#define MUT_SCORPION_TAIL  38
#define MUT_HORNS          39
#define MUT_BEAK           40
#define MUT_ATTRACT_DEMON  41
#define MUT_PROD_MANA      42
#define MUT_SPEED_FLUX     43
#define MUT_BANISH_ALL_RND 44
#define MUT_EAT_LIGHT      45
#define MUT_TRUNK          46
#define MUT_ATTRACT_ANIMAL 47
#define MUT_TENTACLES      48
#define MUT_RAW_CHAOS      49
#define MUT_NORMALITY      50
#define MUT_WRAITH         51
#define MUT_POLY_WOUND     52
#define MUT_WASTING        53
#define MUT_ATTRACT_DRAGON 54
#define MUT_WEIRD_MIND     55
#define MUT_NAUSEA         56
#define MUT_CHAOS_GIFT     57
#define MUT_SHADOW_WALK    58
#define MUT_WARNING        59
#define MUT_INVULN         60
#define MUT_SP_TO_HP       61
#define MUT_HP_TO_SP       62
#define MUT_FUMBLING       63

#define MUT_HYPER_STR      64
#define MUT_PUNY           65
#define MUT_HYPER_INT      66
#define MUT_MORONIC        67
#define MUT_RESILIENT      68
#define MUT_XTRA_FAT       69
#define MUT_ALBINO         70
#define MUT_FLESH_ROT      71
#define MUT_SILLY_VOICE    72
#define MUT_BLANK_FACE     73
#define MUT_ILL_NORM       74
#define MUT_XTRA_EYES      75
#define MUT_MAGIC_RES      76
#define MUT_XTRA_NOISE     77
#define MUT_INFRAVISION    78
#define MUT_XTRA_LEGS      79
#define MUT_SHORT_LEG      80
#define MUT_ELEC_AURA      81
#define MUT_FIRE_AURA      82
#define MUT_WARTS          83
#define MUT_SCALES         84
#define MUT_STEEL_SKIN     85
#define MUT_WINGS          86
#define MUT_FEARLESS       87
#define MUT_REGEN          88
#define MUT_ESP            89
#define MUT_LIMBER         90
#define MUT_ARTHRITIS      91
#define MUT_BAD_LUCK       92
#define MUT_VULN_ELEM      93
#define MUT_MOTION         94
#define MUT_GOOD_LUCK      95   /* End of the old mutations */

#define MAX_MUTATIONS      96   /* see also MUT_FLAG_SIZE in defines.h */

typedef enum {
	MUT_RATING_AWFUL = -2,
	MUT_RATING_BAD   = -1,
	MUT_RATING_AVERAGE = 0,
	MUT_RATING_GOOD = 1,
	MUT_RATING_GREAT = 2
} mutation_rating;

typedef enum {
	MUT_TYPE_ACTIVATION = 0x01,	/* Mutation activates as a racial power (e.g. Harden to Elements) */
	MUT_TYPE_EFFECT = 0x02,     /* Mutation has a passive effect (e.g. Flatulence)*/
	MUT_TYPE_BONUS = 0x04       /* Mutation confers a passive bonus (e.g. +4 Str) */
} mutation_type;

typedef bool (*mut_pred)(int mut_idx);
extern bool mut_berserker_pred(int mut_idx);
extern bool mut_good_pred(int mut_idx);
extern bool mut_unlocked_pred(int mut_idx);

extern void mut_calc_bonuses(void);
extern int  mut_gain_random_aux(mut_pred pred);
extern int  mut_count(mut_pred pred);
extern void mut_do_cmd_knowledge(void);
extern void mut_dump_file(FILE* file);
extern bool mut_gain(int mut_idx);
extern bool mut_gain_random(mut_pred pred);
extern int  mut_get_powers(spell_info* spells, int max);
extern void mut_lock(int mut_idx);
extern bool mut_locked(int mut_idx);
extern bool mut_lose(int mut_idx);
extern void mut_lose_all(void);
extern bool mut_lose_random(mut_pred pred);
extern void mut_name(int i, char* buf);
extern bool mut_present(int mut_idx);
extern void mut_process(void);
extern int  mut_rating(int mut_idx);
extern int  mut_regenerate_mod(void);
extern int  mut_type(int mut_idx);
extern void mut_unlock(int mut_idx);

/* Non-spell mutations ... */
extern void albino_mut(int cmd, variant *res);
extern void alcohol_mut(int cmd, variant *res);
extern void arthritis_mut(int cmd, variant *res);
extern void attract_animal_mut(int cmd, variant *res);
extern void attract_demon_mut(int cmd, variant *res);
extern void attract_dragon_mut(int cmd, variant *res);
extern void bad_luck_mut(int cmd, variant *res);
extern void beak_mut(int cmd, variant *res);
extern void berserk_rage_mut(int cmd, variant *res);
extern void blank_face_mut(int cmd, variant *res);
extern void chaos_deity_mut(int cmd, variant *res);
extern void cowardice_mut(int cmd, variant *res);
extern void eat_light_mut(int cmd, variant *res);
extern void einstein_mut(int cmd, variant *res);
extern void elec_aura_mut(int cmd, variant *res);
extern void extra_eyes_mut(int cmd, variant *res);
extern void extra_legs_mut(int cmd, variant *res);
extern void extra_noise_mut(int cmd, variant *res);
extern void fat_mut(int cmd, variant *res);
extern void fearless_mut(int cmd, variant *res);
extern void fire_aura_mut(int cmd, variant *res);
extern void flatulence_mut(int cmd, variant *res);
extern void fumbling_mut(int cmd, variant *res);
extern void good_luck_mut(int cmd, variant *res);
extern void hallucination_mut(int cmd, variant *res);
extern void he_man_mut(int cmd, variant *res);
extern void horns_mut(int cmd, variant *res);
extern void illusion_normal_mut(int cmd, variant *res);
extern void infravision_mut(int cmd, variant *res);
extern void invulnerability_mut(int cmd, variant *res);
extern void limber_mut(int cmd, variant *res);
extern void magic_resistance_mut(int cmd, variant *res);
extern void moron_mut(int cmd, variant *res);
extern void motion_mut(int cmd, variant *res);
extern void nausea_mut(int cmd, variant *res);
extern void normality_mut(int cmd, variant *res);
extern void polymorph_wounds_mut(int cmd, variant *res);
extern void produce_mana_mut(int cmd, variant *res);
extern void puny_mut(int cmd, variant *res);
extern void random_banish_mut(int cmd, variant *res);
extern void random_teleport_mut(int cmd, variant *res);
extern void raw_chaos_mut(int cmd, variant *res);
extern void regeneration_mut(int cmd, variant *res);
extern void resilient_mut(int cmd, variant *res);
extern void rotting_flesh_mut(int cmd, variant *res);
extern void scales_mut(int cmd, variant *res);
extern void scorpion_tail_mut(int cmd, variant *res);
extern void shadow_walk_mut(int cmd, variant *res);
extern void short_legs_mut(int cmd, variant *res);
extern void silly_voice_mut(int cmd, variant *res);
extern void speed_flux_mut(int cmd, variant *res);
extern void steel_skin_mut(int cmd, variant *res);
extern void telepathy_mut(int cmd, variant *res);
extern void tentacles_mut(int cmd, variant *res);
extern void trunk_mut(int cmd, variant *res);
extern void vulnerability_mut(int cmd, variant *res);
extern void warning_mut(int cmd, variant *res);
extern void warts_mut(int cmd, variant *res);
extern void wasting_mut(int cmd, variant *res);
extern void weird_mind_mut(int cmd, variant *res);
extern void wings_mut(int cmd, variant *res);
extern void wraith_mut(int cmd, variant *res);
