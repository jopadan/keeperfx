/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file lvl_script_commands_old.c
 * @par  This file is the old way of working 
 * DON'T ADD NEW LOGIC HERE
 * see lvl_script_commands.c on how new commands should be added
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 * @author   KeeperFX Team
 */
/******************************************************************************/
#include "pre_inc.h"
#include "lvl_script_commands_old.h"
#include "bflib_math.h"
#include "config_strings.h"
#include "config_magic.h"
#include "config_terrain.h"
#include "console_cmd.h"
#include "player_instances.h"
#include "player_data.h"
#include "lvl_filesdk1.h"
#include "game_merge.h"
#include "game_legacy.h"
#include "keeperfx.hpp"
#include "bflib_sndlib.h"
#include "lvl_script_value.h"
#include "lvl_script_lib.h"
#include "lvl_script_conditions.h"
#include "lvl_script_commands.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/

#define CONDITION_ALWAYS (CONDITIONS_COUNT)

void command_add_value(unsigned long var_index, unsigned long plr_range_id, long val2, long val3, long val4)
{
    ALLOCATE_SCRIPT_VALUE(var_index, plr_range_id);

    value->longs[0] = val2;
    value->longs[1] = val3;
    value->longs[2] = val4;

    if ((get_script_current_condition() == CONDITION_ALWAYS) && (next_command_reusable == 0))
    {
        script_process_value(var_index, plr_range_id, val2, val3, val4, value);
        return;
    }
}

static void command_create_party(const char *prtname)
{
    if (get_script_current_condition() != CONDITION_ALWAYS)
    {
        SCRPTWRNLOG("Party '%s' defined inside conditional statement",prtname);
    }
    create_party(prtname);
}

static void command_tutorial_flash_button(long btn_id, long duration)
{
    command_add_value(Cmd_TUTORIAL_FLASH_BUTTON, ALL_PLAYERS, btn_id, duration, 0);
}

static void command_add_party_to_level(long plr_range_id, const char *prtname, const char *locname, long ncopies)
{
    TbMapLocation location;
    if (ncopies < 1)
    {
        SCRPTERRLOG("Invalid NUMBER parameter");
        return;
    }
    if (game.script.party_triggers_num >= PARTY_TRIGGERS_COUNT)
    {
        SCRPTERRLOG("Too many ADD_CREATURE commands in script");
        return;
    }
    // Verify player
    long plr_id = get_players_range_single(plr_range_id);
    if (plr_id < 0) {
        SCRPTERRLOG("Given owning player is not supported in this command");
        return;
    }
    // Recognize place where party is created
    if (!get_map_location_id(locname, &location))
        return;
    // Recognize party name
    long prty_id = get_party_index_of_name(prtname);
    if (prty_id < 0)
    {
        SCRPTERRLOG("Party of requested name, '%s', is not defined",prtname);
        return;
    }
    if ((get_script_current_condition() == CONDITION_ALWAYS) && (next_command_reusable == 0))
    {
        struct Party* party = &game.script.creature_partys[prty_id];
        script_process_new_party(party, plr_id, location, ncopies);
    } else
    {
        struct PartyTrigger* pr_trig = &game.script.party_triggers[game.script.party_triggers_num % PARTY_TRIGGERS_COUNT];
        pr_trig->flags = TrgF_CREATE_PARTY;
        pr_trig->flags |= next_command_reusable?TrgF_REUSABLE:0;
        pr_trig->plyr_idx = plr_id;
        pr_trig->creatr_id = prty_id;
        pr_trig->location = location;
        pr_trig->ncopies = ncopies;
        pr_trig->condit_idx = get_script_current_condition();
        game.script.party_triggers_num++;
    }
}

static void command_add_creature_to_level(long plr_range_id, const char *crtr_name, const char *locname, long ncopies, CrtrExpLevel exp_level, long carried_gold, const char *spawn_type)
{
    TbMapLocation location;
    if ((exp_level < 1) || (exp_level > CREATURE_MAX_LEVEL))
    {
        SCRPTERRLOG("Invalid CREATURE LEVEL parameter");
        return;
    }
    if ((ncopies <= 0) || (ncopies >= CREATURES_COUNT))
    {
        SCRPTERRLOG("Invalid number of creatures to add");
        return;
    }
    if (ncopies > game.conf.rules.game.creatures_count)
    {
        SCRPTWRNLOG("Trying to add %ld creatures which is over map limit %u", ncopies, game.conf.rules.game.creatures_count);
    }
    if (game.script.party_triggers_num >= PARTY_TRIGGERS_COUNT)
    {
        SCRPTERRLOG("Too many ADD_CREATURE commands in script");
        return;
    }
    long crtr_id = get_rid(creature_desc, crtr_name);
    if (crtr_id == -1)
    {
        SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
        return;
    }
    long spawn_type_id;
    if ((strcmp(spawn_type, "") == 0))
    {
        spawn_type_id = SpwnT_Default;
    }
    else
    {
        spawn_type_id = get_rid(spawn_type_desc, spawn_type);
    }
    if (spawn_type_id == -1)
    {
        SCRPTERRLOG("Unknown spawn type, '%s'", spawn_type);
        return;
    }
    // Verify player
    long plr_id = get_players_range_single(plr_range_id);
    if (plr_id < 0) {
        SCRPTERRLOG("Given owning player is not supported in this command");
        return;
    }
    // Recognize place where party is created
    if (!get_map_location_id(locname, &location))
        return;
    if (get_script_current_condition() == CONDITION_ALWAYS)
    {
        script_process_new_creatures(plr_id, crtr_id, location, ncopies, carried_gold, exp_level-1, spawn_type_id);
    } else
    {
        struct PartyTrigger* pr_trig = &game.script.party_triggers[game.script.party_triggers_num % PARTY_TRIGGERS_COUNT];
        pr_trig->flags = TrgF_CREATE_CREATURE;
        pr_trig->flags |= next_command_reusable?TrgF_REUSABLE:0;

        pr_trig->plyr_idx = plr_id;
        pr_trig->creatr_id = crtr_id;
        pr_trig->exp_level = exp_level-1;
        pr_trig->carried_gold = carried_gold;
        pr_trig->location = location;
        pr_trig->ncopies = ncopies;
        pr_trig->spawn_type = spawn_type_id;
        pr_trig->condit_idx = get_script_current_condition();
        game.script.party_triggers_num++;
    }
}


static void command_display_information(long msg_num, const char *where, long x, long y)
{
    TbMapLocation location;
    if ((msg_num < 0) || (msg_num >= STRINGS_MAX))
    {
      SCRPTERRLOG("Invalid TEXT number");
      return;
    }
    if (!get_map_location_id(where, &location))
      return;
    command_add_value(Cmd_DISPLAY_INFORMATION, ALL_PLAYERS, msg_num, location, get_subtile_number(x,y));
}

static void command_dead_creatures_return_to_pool(long val)
{
    command_add_value(Cmd_DEAD_CREATURES_RETURN_TO_POOL, ALL_PLAYERS, val, 0, 0);
}

static void command_bonus_level_time(long game_turns, long real)
{
    if (game_turns < 0)
    {
        SCRPTERRLOG("Bonus time must be nonnegative");
        return;
    }
    command_add_value(Cmd_BONUS_LEVEL_TIME, ALL_PLAYERS, game_turns, real, 0);
}

static void command_set_start_money(long plr_range_id, long gold_val)
{
    int plr_start;
    int plr_end;
    if (get_players_range(plr_range_id, &plr_start, &plr_end) < 0)
    {
        SCRPTERRLOG("Given owning player range %d is not supported in this command", (int)plr_range_id);
        return;
  }
  if (get_script_current_condition() != CONDITION_ALWAYS)
  {
    SCRPTWRNLOG("Start money set inside conditional block; condition ignored");
  }
  for (int i = plr_start; i < plr_end; i++)
  {
      if (gold_val > SENSIBLE_GOLD)
      {
          gold_val = SENSIBLE_GOLD;
          SCRPTWRNLOG("Gold added to player %d reduced to %d", (int)plr_range_id, SENSIBLE_GOLD);
      }
      player_add_offmap_gold(i, gold_val);
  }
}

static void command_room_available(long plr_range_id, const char *roomname, unsigned long can_resrch, unsigned long can_build)
{
    long room_id = get_rid(room_desc, roomname);
    if (room_id == -1)
    {
      SCRPTERRLOG("Unknown room name, '%s'", roomname);
      return;
    }
    command_add_value(Cmd_ROOM_AVAILABLE, plr_range_id, room_id, can_resrch, can_build);
}

static void command_creature_available(long plr_range_id, const char *crtr_name, unsigned long can_be_avail, unsigned long force_avail)
{
    long crtr_id = get_rid(creature_desc, crtr_name);
    if (crtr_id == -1)
    {
      SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
      return;
    }
    command_add_value(Cmd_CREATURE_AVAILABLE, plr_range_id, crtr_id, can_be_avail, force_avail);
}

static void command_magic_available(long plr_range_id, const char *magname, unsigned long can_resrch, unsigned long can_use)
{
    long mag_id = get_rid(power_desc, magname);
    if (mag_id == -1)
    {
      SCRPTERRLOG("Unknown magic, '%s'", magname);
      return;
    }
    command_add_value(Cmd_MAGIC_AVAILABLE, plr_range_id, mag_id, can_resrch, can_use);
}

static void command_trap_available(long plr_range_id, const char *trapname, unsigned long can_build, unsigned long amount)
{
    long trap_id = get_rid(trap_desc, trapname);
    if (trap_id == -1)
    {
      SCRPTERRLOG("Unknown trap, '%s'", trapname);
      return;
    }
    command_add_value(Cmd_TRAP_AVAILABLE, plr_range_id, trap_id, can_build, amount);
}

/**
 * Updates amount of RESEARCH points needed for the item to be researched.
 * Will not reorder the RESEARCH items.
 */
static void command_research(long plr_range_id, const char *trg_type, const char *trg_name, unsigned long val)
{
    long item_type = get_rid(research_desc, trg_type);
    long item_id = get_research_id(item_type, trg_name, __func__);
    if (item_id < 0)
      return;
    command_add_value(Cmd_RESEARCH, plr_range_id, item_type, item_id, val);
}

/**
 * Updates amount of RESEARCH points needed for the item to be researched.
 * Reorders the RESEARCH items - needs all items to be re-added.
 */
static void command_research_order(long plr_range_id, const char *trg_type, const char *trg_name, unsigned long val)
{
    int plr_start;
    int plr_end;
    if (get_players_range(plr_range_id, &plr_start, &plr_end) < 0) {
        SCRPTERRLOG("Given owning player range %d is not supported in this command",(int)plr_range_id);
        return;
    }
    for (long i = plr_start; i < plr_end; i++)
    {
        struct Dungeon* dungeon = get_dungeon(i);
        if (dungeon_invalid(dungeon))
            continue;
        if (dungeon->research_num >= DUNGEON_RESEARCH_COUNT)
        {
          SCRPTERRLOG("Too many RESEARCH ITEMS, for player %ld", i);
          return;
        }
    }
    long item_type = get_rid(research_desc, trg_type);
    long item_id = get_research_id(item_type, trg_name, __func__);
    if (item_id < 0)
      return;
    command_add_value(Cmd_RESEARCH_ORDER, plr_range_id, item_type, item_id, val);
}

static void command_if_action_point(long apt_num, long plr_range_id)
{
    if (game.script.conditions_num >= CONDITIONS_COUNT)
    {
        SCRPTERRLOG("Too many (over %d) conditions in script", CONDITIONS_COUNT);
        return;
    }
    // Check the Action Point
    long apt_idx = action_point_number_to_index(apt_num);
    if (!action_point_exists_idx(apt_idx))
    {
        SCRPTERRLOG("Non-existing Action Point, no %ld", apt_num);
        return;
    }
    command_add_condition(plr_range_id, 0, SVar_ACTION_POINT_TRIGGERED, apt_idx, 0);
}

static void command_if_slab_owner(MapSlabCoord slb_x, MapSlabCoord slb_y, long plr_range_id)
{
    if (game.script.conditions_num >= CONDITIONS_COUNT)
    {
        SCRPTERRLOG("Too many (over %d) conditions in script", CONDITIONS_COUNT);
        return;
    }
    command_add_condition(slb_x, 1, SVar_SLAB_OWNER, slb_y, plr_range_id);
}

static void command_if_slab_type(MapSlabCoord slb_x, MapSlabCoord slb_y, long slab_type)
{
    if (game.script.conditions_num >= CONDITIONS_COUNT)
    {
        SCRPTERRLOG("Too many (over %d) conditions in script", CONDITIONS_COUNT);
        return;
    }
    command_add_condition(slb_x, 1, SVar_SLAB_TYPE, slb_y, slab_type);
}

static void command_set_timer(long plr_range_id, const char *timrname)
{
    long timr_id = get_rid(timer_desc, timrname);
    if (timr_id == -1)
    {
        SCRPTERRLOG("Unknown timer, '%s'", timrname);
        return;
    }
    command_add_value(Cmd_SET_TIMER, plr_range_id, timr_id, 0, 0);
}

static void command_win_game(void)
{
    if (get_script_current_condition() == CONDITION_ALWAYS)
    {
        SCRPTERRLOG("Command WIN GAME found with no condition");
    }
    if (game.script.win_conditions_num >= WIN_CONDITIONS_COUNT)
    {
        SCRPTERRLOG("Too many WIN GAME conditions in script");
        return;
    }
    game.script.win_conditions[game.script.win_conditions_num] = get_script_current_condition();
    game.script.win_conditions_num++;
}

static void command_lose_game(void)
{
  if (get_script_current_condition() == CONDITION_ALWAYS)
  {
    SCRPTERRLOG("Command LOSE GAME found with no condition");
  }
  if (game.script.lose_conditions_num >= WIN_CONDITIONS_COUNT)
  {
    SCRPTERRLOG("Too many LOSE GAME conditions in script");
    return;
  }
  game.script.lose_conditions[game.script.lose_conditions_num] = get_script_current_condition();
  game.script.lose_conditions_num++;
}

static void command_set_flag(long plr_range_id, const char *flgname, long val)
{
    long flg_id;
    long flag_type;
    if (!parse_set_varib(flgname, &flg_id, &flag_type))
    {
        SCRPTERRLOG("Unknown flag, '%s'", flgname);
        return;
    }
    command_add_value(Cmd_SET_FLAG, plr_range_id, flg_id, val, flag_type);
}

static void command_add_to_flag(long plr_range_id, const char *flgname, long val)
{
    long flg_id;
    long flag_type;

    if (!parse_set_varib(flgname, &flg_id, &flag_type))
    {
        SCRPTERRLOG("Unknown flag, '%s'", flgname);
        return;
    }
    command_add_value(Cmd_ADD_TO_FLAG, plr_range_id, flg_id, val, flag_type);
}

static void command_max_creatures(long plr_range_id, long val)
{
    command_add_value(Cmd_MAX_CREATURES, plr_range_id, val, 0, 0);
}

static void command_door_available(long plr_range_id, const char *doorname, unsigned long a3, unsigned long a4)
{
    long door_id = get_rid(door_desc, doorname);
    if (door_id == -1)
    {
        SCRPTERRLOG("Unknown door, '%s'", doorname);
        return;
  }
  command_add_value(Cmd_DOOR_AVAILABLE, plr_range_id, door_id, a3, a4);
}

static void command_add_tunneller_to_level(long plr_range_id, const char *locname, const char *objectv, long target, CrtrExpLevel exp_level, unsigned long carried_gold)
{
    TbMapLocation location;
    TbMapLocation heading;
    if ((exp_level < 1) || (exp_level > CREATURE_MAX_LEVEL))
    {
        SCRPTERRLOG("Invalid CREATURE LEVEL parameter");
        return;
    }
    if (game.script.tunneller_triggers_num >= TUNNELLER_TRIGGERS_COUNT)
    {
        SCRPTERRLOG("Too many ADD_TUNNELLER commands in script");
        return;
    }
    // Verify player
    long plr_id = get_players_range_single(plr_range_id);
    if (plr_id < 0) {
        SCRPTERRLOG("Given owning player is not supported in this command");
        return;
    }
    // Recognize place where party is created
    if (!get_map_location_id(locname, &location))
        return;
    // Recognize place where party is going
    if (!get_map_heading_id(objectv, target, &heading))
        return;
    if (get_script_current_condition() == CONDITION_ALWAYS)
    {
        script_process_new_tunneler(plr_id, location, heading, exp_level-1, carried_gold);
    } else
    {
        struct TunnellerTrigger* tn_trig = &game.script.tunneller_triggers[game.script.tunneller_triggers_num % TUNNELLER_TRIGGERS_COUNT];
        set_flag_value(tn_trig->flags, TrgF_REUSABLE, next_command_reusable);
        clear_flag(tn_trig->flags, TrgF_DISABLED);
        tn_trig->plyr_idx = plr_id;
        tn_trig->location = location;
        tn_trig->heading = heading;
        tn_trig->carried_gold = carried_gold;
        tn_trig->exp_level = exp_level-1;
        tn_trig->carried_gold = carried_gold;
        tn_trig->party_id = 0;
        tn_trig->condit_idx = get_script_current_condition();
        game.script.tunneller_triggers_num++;
    }
}

static void command_add_tunneller_party_to_level(long plr_range_id, const char *prtname, const char *locname, const char *objectv, long target, CrtrExpLevel exp_level, unsigned long carried_gold)
{
    TbMapLocation location;
    TbMapLocation heading;
    if ((exp_level < 1) || (exp_level > CREATURE_MAX_LEVEL))
    {
        SCRPTERRLOG("Invalid CREATURE LEVEL parameter");
        return;
    }
    if (game.script.tunneller_triggers_num >= TUNNELLER_TRIGGERS_COUNT)
    {
        SCRPTERRLOG("Too many ADD_TUNNELLER commands in script");
        return;
    }
    // Verify player
    long plr_id = get_players_range_single(plr_range_id);
    if (plr_id < 0) {
        SCRPTERRLOG("Given owning player is not supported in this command");
        return;
    }
    // Recognize place where party is created
    if (!get_map_location_id(locname, &location))
        return;
    // Recognize place where party is going
    if (!get_map_heading_id(objectv, target, &heading))
        return;
    // Recognize party name
    long prty_id = get_party_index_of_name(prtname);
    if (prty_id < 0)
    {
        SCRPTERRLOG("Party of requested name, '%s', is not defined", prtname);
        return;
    }
    struct Party* party = &game.script.creature_partys[prty_id];
    if (party->members_num >= GROUP_MEMBERS_COUNT-1)
    {
        SCRPTERRLOG("Party too big for ADD_TUNNELLER (Max %d members)", GROUP_MEMBERS_COUNT-1);
        return;
    }
    // Either add the party or add item to conditional triggers list
    if (get_script_current_condition() == CONDITION_ALWAYS)
    {
        script_process_new_tunneller_party(plr_id, prty_id, location, heading, exp_level-1, carried_gold);
    } else
    {
        struct TunnellerTrigger* tn_trig = &game.script.tunneller_triggers[game.script.tunneller_triggers_num % TUNNELLER_TRIGGERS_COUNT];
        set_flag_value(tn_trig->flags, TrgF_REUSABLE, next_command_reusable);
        clear_flag(tn_trig->flags, TrgF_DISABLED);
        tn_trig->plyr_idx = plr_id;
        tn_trig->location = location;
        tn_trig->heading = heading;
        tn_trig->carried_gold = carried_gold;
        tn_trig->exp_level = exp_level-1;
        tn_trig->carried_gold = carried_gold;
        tn_trig->party_id = prty_id+1;
        tn_trig->condit_idx = get_script_current_condition();
        game.script.tunneller_triggers_num++;
    }
}

static void command_add_creature_to_pool(const char *crtr_name, long amount)
{
    long crtr_id = get_rid(creature_desc, crtr_name);
    if (crtr_id == -1)
    {
        SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
        return;
    }
    if ((amount <= -CREATURES_COUNT) || (amount >= CREATURES_COUNT))
    {
        SCRPTERRLOG("Invalid number of '%s' creatures for pool, %ld", crtr_name, amount);
        return;
    }
    command_add_value(Cmd_ADD_CREATURE_TO_POOL, ALL_PLAYERS, crtr_id, amount, 0);
}

static void command_set_hate(long trgt_plr_range_id, long enmy_plr_range_id, long hate_val)
{
    // Verify enemy player
    long enmy_plr_id = get_players_range_single(enmy_plr_range_id);
    if (enmy_plr_id < 0) {
        SCRPTERRLOG("Given enemy player is not supported in this command");
        return;
    }
    command_add_value(Cmd_SET_HATE, trgt_plr_range_id, enmy_plr_id, hate_val, 0);
}

static void command_set_creature_health(const char *crtr_name, long val)
{
    long crtr_id = get_rid(creature_desc, crtr_name);
    if (crtr_id == -1)
    {
        SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
        return;
  }
  if ((val < 0) || (val > USHRT_MAX))
  {
    SCRPTERRLOG("Invalid '%s' health value, %ld", crtr_name, val);
    return;
  }
  command_add_value(Cmd_SET_CREATURE_HEALTH, ALL_PLAYERS, crtr_id, val, 0);
}

static void command_set_creature_strength(const char *crtr_name, long val)
{
    long crtr_id = get_rid(creature_desc, crtr_name);
    if (crtr_id == -1)
    {
        SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
        return;
    }
    if ((val < 0) || (val > USHRT_MAX))
    {
        SCRPTERRLOG("Invalid '%s' strength value, %ld", crtr_name, val);
        return;
    }
    command_add_value(Cmd_SET_CREATURE_STRENGTH, ALL_PLAYERS, crtr_id, val, 0);
}

static void command_set_creature_armour(const char *crtr_name, long val)
{
    long crtr_id = get_rid(creature_desc, crtr_name);
    if (crtr_id == -1)
    {
        SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
        return;
  }
  if ((val < 0) || (val > UCHAR_MAX))
  {
    SCRPTERRLOG("Invalid '%s' armour value, %ld", crtr_name, val);
    return;
  }
  command_add_value(Cmd_SET_CREATURE_ARMOUR, ALL_PLAYERS, crtr_id, val, 0);
}

static void command_set_creature_fear_wounded(const char *crtr_name, long val)
{
    long crtr_id = get_rid(creature_desc, crtr_name);
    if (crtr_id == -1)
    {
        SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
        return;
  }
  if ((val < 0) || (val > UCHAR_MAX))
  {
    SCRPTERRLOG("Invalid '%s' fear value, %ld", crtr_name, val);
    return;
  }
  command_add_value(Cmd_SET_CREATURE_FEAR_WOUNDED, ALL_PLAYERS, crtr_id, val, 0);
}

static void command_set_creature_fear_stronger(const char *crtr_name, long val)
{
    long crtr_id = get_rid(creature_desc, crtr_name);
    if (crtr_id == -1)
    {
        SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
        return;
  }
  if ((val < 0) || (val > SHRT_MAX))
  {
    SCRPTERRLOG("Invalid '%s' fear value, %ld", crtr_name, val);
    return;
  }
  command_add_value(Cmd_SET_CREATURE_FEAR_STRONGER, ALL_PLAYERS, crtr_id, val, 0);
}

static void command_set_creature_fearsome_factor(const char* crtr_name, long val)
{
    long crtr_id = get_rid(creature_desc, crtr_name);
    if (crtr_id == -1)
    {
        SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
        return;
    }
    if ((val < 0) || (val > SHRT_MAX))
    {
        SCRPTERRLOG("Invalid '%s' fearsome value, %ld", crtr_name, val);
        return;
    }
    command_add_value(Cmd_SET_CREATURE_FEARSOME_FACTOR, ALL_PLAYERS, crtr_id, val, 0);
}

static void command_set_creature_property(const char* crtr_name, const char* property, short val)
{
    long crtr_id = get_rid(creature_desc, crtr_name);
    if (crtr_id == -1)
    {
        SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
        return;
    }
    long prop_id = get_rid(creatmodel_properties_commands, property);
    if (prop_id == -1)
    {
        SCRPTERRLOG("Unknown creature property kind, \"%s\"", property);
        return;
    }
    command_add_value(Cmd_SET_CREATURE_PROPERTY, ALL_PLAYERS, crtr_id, prop_id, val);
}

/**
 * Enables or disables an alliance between two players.
 *
 * @param plr1_range_id First player range identifier.
 * @param plr2_range_id Second player range identifier.
 * @param ally Controls whether the alliance is being created or being broken.
 */
static void command_ally_players(long plr1_range_id, long plr2_range_id, TbBool ally)
{
    // Verify enemy player
    long plr2_id = get_players_range_single(plr2_range_id);
    if (plr2_id < 0) {
        SCRPTERRLOG("Given second player is not supported in this command");
        return;
    }
    command_add_value(Cmd_ALLY_PLAYERS, plr1_range_id, plr2_id, ally, 0);
}

static void command_quick_objective(int idx, const char *msgtext, const char *where, long x, long y)
{
  TbMapLocation location;
  if ((idx < 0) || (idx >= QUICK_MESSAGES_COUNT))
  {
    SCRPTERRLOG("Invalid QUICK OBJECTIVE number (%d)", idx);
    return;
  }
  if (strlen(msgtext) >= MESSAGE_TEXT_LEN)
  {
      SCRPTWRNLOG("Objective TEXT too long; truncating to %d characters", MESSAGE_TEXT_LEN-1);
  }
  if ((game.quick_messages[idx][0] != '\0') && (strcmp(game.quick_messages[idx],msgtext) != 0))
  {
      SCRPTWRNLOG("Quick Objective no %d overwritten by different text", idx);
  }
  snprintf(game.quick_messages[idx], MESSAGE_TEXT_LEN, "%s", msgtext);
  if (!get_map_location_id(where, &location))
    return;
  command_add_value(Cmd_QUICK_OBJECTIVE, ALL_PLAYERS, idx, location, get_subtile_number(x,y));
}

static void command_quick_information(int idx, const char *msgtext, const char *where, long x, long y)
{
  TbMapLocation location;
  if ((idx < 0) || (idx >= QUICK_MESSAGES_COUNT))
  {
    SCRPTERRLOG("Invalid information ID number (%d)", idx);
    return;
  }
  if (strlen(msgtext) > MESSAGE_TEXT_LEN)
  {
      SCRPTWRNLOG("Information TEXT too long; truncating to %d characters", MESSAGE_TEXT_LEN-1);
  }
  if ((game.quick_messages[idx][0] != '\0') && (strcmp(game.quick_messages[idx],msgtext) != 0))
  {
      SCRPTWRNLOG("Quick Message no %d overwritten by different text", idx);
  }
  snprintf(game.quick_messages[idx], MESSAGE_TEXT_LEN, "%s", msgtext);
  if (!get_map_location_id(where, &location))
    return;
  command_add_value(Cmd_QUICK_INFORMATION, ALL_PLAYERS, idx, location, get_subtile_number(x,y));
}

static void command_add_gold_to_player(long plr_range_id, long amount)
{
    command_add_value(Cmd_ADD_GOLD_TO_PLAYER, plr_range_id, amount, 0, 0);
}

static void command_set_creature_tendencies(long plr_range_id, const char *tendency, long value)
{
    long tend_id = get_rid(tendency_desc, tendency);
    if (tend_id == -1)
    {
      SCRPTERRLOG("Unrecognized tendency type, '%s'", tendency);
      return;
    }
    command_add_value(Cmd_SET_CREATURE_TENDENCIES, plr_range_id, tend_id, value, 0);
}

static void command_reveal_map_rect(long plr_range_id, long x, long y, long w, long h)
{
    command_add_value(Cmd_REVEAL_MAP_RECT, plr_range_id, x, y, (h<<16)+w);
}

static const char *script_get_command_name(long cmnd_index)
{
    long i = 0;
    while (command_desc[i].textptr != NULL)
    {
        if (command_desc[i].index == cmnd_index)
            return command_desc[i].textptr;
        i++;
  }
  return NULL;
}

static void command_message(const char *msgtext, unsigned char kind)
{
  const char *cmd;
  if (kind == 80)
    cmd = script_get_command_name(Cmd_PRINT);
  else
    cmd = script_get_command_name(Cmd_MESSAGE);
  SCRPTWRNLOG("Command '%s' is only supported in Dungeon Keeper Beta", cmd);
}

static void command_kill_creature(long plr_range_id, const char *crtr_name, const char *criteria, int count)
{
    SCRIPTDBG(11, "Starting");
    if (count <= 0)
    {
        SCRPTERRLOG("Bad creatures count, %d", count);
        return;
  }
  long crtr_id = parse_creature_name(crtr_name);
  if (crtr_id == CREATURE_NONE) {
    SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
    return;
  }
  long select_id = parse_criteria(criteria);
  if (select_id == -1)
  {
    SCRPTERRLOG("Unknown select criteria, '%s'", criteria);
    return;
  }
  command_add_value(Cmd_KILL_CREATURE, plr_range_id, crtr_id, select_id, count);
}

static void command_level_up_creature(long plr_range_id, const char *crtr_name, const char *criteria, int count)
{
    SCRIPTDBG(11, "Starting");
    if (count == 0)
    {
        SCRPTERRLOG("Bad count, %d", count);
        return;
    }
    ThingModel crtr_id = parse_creature_name(crtr_name);
    if (crtr_id == CREATURE_NONE)
    {
        SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
        return;
    }
    long select_id = parse_criteria(criteria);
    if (select_id == -1) 
    {
        SCRPTERRLOG("Unknown select criteria, '%s'", criteria);
        return;
    }
    command_add_value(Cmd_LEVEL_UP_CREATURE, plr_range_id, crtr_id, select_id, count);
}

static void command_use_power_on_creature(long plr_range_id, const char *crtr_name, const char *criteria, long caster_plyr_idx, const char *magname, KeepPwrLevel power_level, const char *freestring)
{
  SCRIPTDBG(11, "Starting");
  if (power_level < 1)
  {
    SCRPTWRNLOG("Spell %s level too low: %d, setting to 1.", magname, power_level);
    power_level = 1;
  }
  if (power_level > MAGIC_OVERCHARGE_LEVELS)
  {
    SCRPTWRNLOG("Spell %s level too high: %d, setting to %d.", magname, power_level, MAGIC_OVERCHARGE_LEVELS);
    power_level = MAGIC_OVERCHARGE_LEVELS;
  }
  power_level--;
  long mag_id = get_rid(power_desc, magname);
  if (mag_id == -1)
  {
    SCRPTERRLOG("Unknown magic, '%s'", magname);
    return;
  }
  ThingModel crtr_id = parse_creature_name(crtr_name);
  if (crtr_id == CREATURE_NONE) {
    SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
    return;
  }
  long select_id = parse_criteria(criteria);
  if (select_id == -1) {
    SCRPTERRLOG("Unknown select criteria, '%s'", criteria);
    return;
  }
  char free;
  if (parameter_is_number(freestring))
  {
      free = atoi(freestring);
  }
  else
  {
      free = get_id(is_free_desc, freestring);
  }
  if ((free < 0) || (free > 1))
  {
      SCRPTERRLOG("Unknown free value '%s' not recognized", freestring);
      return;
  }

  // encode params: free, magic, caster, level -> into 4xbyte: FMCL
  long fmcl_bytes;
  {
      signed char f = free, m = mag_id, c = caster_plyr_idx, lvl = power_level;
      fmcl_bytes = (f << 24) | (m << 16) | (c << 8) | lvl;
  }
  command_add_value(Cmd_USE_POWER_ON_CREATURE, plr_range_id, crtr_id, select_id, fmcl_bytes);
}

static void command_use_power_at_pos(long plr_range_id, int stl_x, int stl_y, const char *magname, KeepPwrLevel power_level, const char *freestring)
{
  SCRIPTDBG(11, "Starting");
  if (power_level < 1)
  {
    SCRPTWRNLOG("Spell %s level too low: %d, setting to 1.", magname, power_level);
    power_level = 1;
  }
  if (power_level > MAGIC_OVERCHARGE_LEVELS)
  {
    SCRPTWRNLOG("Spell %s level too high: %d, setting to %d.", magname, power_level, MAGIC_OVERCHARGE_LEVELS);
    power_level = MAGIC_OVERCHARGE_LEVELS;
  }
  power_level--;
  long mag_id = get_rid(power_desc, magname);
  if (mag_id == -1)
  {
    SCRPTERRLOG("Unknown magic, '%s'", magname);
    return;
  }
  char free;
  if (parameter_is_number(freestring))
  {
      free = atoi(freestring);
  }
  else
  {
      free = get_id(is_free_desc, freestring);
  }
  if ((free < 0) || (free > 1))
  {
      SCRPTERRLOG("Unknown free value '%s' not recognized", freestring);
      return;
  }

  // encode params: free, magic, level -> into 3xbyte: FML
  long fml_bytes;
  {
      signed char f = free, m = mag_id, lvl = power_level;
      fml_bytes = (f << 16) | (m << 8) | lvl;
  }
  command_add_value(Cmd_USE_POWER_AT_POS, plr_range_id, stl_x, stl_y, fml_bytes);
}

static void command_use_power_at_location(long plr_range_id, const char *locname, const char *magname, KeepPwrLevel power_level, const char *freestring)
{
  SCRIPTDBG(11, "Starting");
  if (power_level < 1)
  {
    SCRPTWRNLOG("Spell %s level too low: %d, setting to 1.", magname, power_level);
    power_level = 1;
  }
  if (power_level > MAGIC_OVERCHARGE_LEVELS)
  {
    SCRPTWRNLOG("Spell %s level too high: %d, setting to %d.", magname, power_level, MAGIC_OVERCHARGE_LEVELS);
    power_level = MAGIC_OVERCHARGE_LEVELS;
  }
  power_level--;
  long mag_id = get_rid(power_desc, magname);
  if (mag_id == -1)
  {
    SCRPTERRLOG("Unknown magic, '%s'", magname);
    return;
  }

  TbMapLocation location;
  if (!get_map_location_id(locname, &location))
  {
    SCRPTWRNLOG("Use power script command at invalid location: %s", locname);
    return;
  }
  char free;
  if (parameter_is_number(freestring))
  {
      free = atoi(freestring);
  }
  else
  {
      free = get_id(is_free_desc, freestring);
  }
  if ((free < 0) || (free > 1))
  {
      SCRPTERRLOG("Unknown free value '%s' not recognized", freestring);
      return;
  }

  // encode params: free, magic, level -> into 3xbyte: FML
  long fml_bytes;
  {
      signed char f = free, m = mag_id, lvl = power_level;
      fml_bytes = (f << 16) | (m << 8) | lvl;
  }
  command_add_value(Cmd_USE_POWER_AT_LOCATION, plr_range_id, location, fml_bytes, 0);
}

static void command_use_power(long plr_range_id, const char *magname, const char *freestring)
{
    SCRIPTDBG(11, "Starting");
    long mag_id = get_rid(power_desc, magname);
    if (mag_id == -1)
    {
        SCRPTERRLOG("Unknown magic, '%s'", magname);
        return;
    }
    char free;
    if (parameter_is_number(freestring))
    {
        free = atoi(freestring);
    }
    else
    {
        free = get_id(is_free_desc, freestring);
    }
    if ((free < 0) || (free > 1))
    {
        SCRPTERRLOG("Unknown free value '%s' not recognized", freestring);
        return;
    }
    command_add_value(Cmd_USE_POWER, plr_range_id, mag_id, free, 0);
}

static void command_use_special_increase_level(long plr_range_id, long count)
{
    if (count == 0)
    {
        SCRPTWRNLOG("Invalid count: %ld, setting to 1.", count);
        count = 1;
    }

    if (count > 9)
    {
        SCRPTWRNLOG("Count too high: %ld, setting to 9.", count);
        count = 9;
    }

    if (count < -9)
    {
        SCRPTWRNLOG("Count too low: %ld, setting to -9.", count);
        count = -9;
    }
    command_add_value(Cmd_USE_SPECIAL_INCREASE_LEVEL, plr_range_id, count, 0, 0);
}

static void command_use_special_multiply_creatures(long plr_range_id, long count)
{
    if (count < 1)
    {
        SCRPTWRNLOG("Invalid count: %ld, setting to 1.", count);
        count = 1;
    }

    if (count > 9)
    {
        SCRPTWRNLOG("Count too high: %ld, setting to 9.", count);
        count = 9;
    }
    command_add_value(Cmd_USE_SPECIAL_MULTIPLY_CREATURES, plr_range_id, count, 0, 0);
}

static void make_safe(long plr_range_id)
{
    command_add_value(Cmd_MAKE_SAFE, plr_range_id, 0, 0, 0);
}

static void command_locate_hidden_world()
{
    command_add_value(Cmd_LOCATE_HIDDEN_WORLD, 0, 0, 0, 0);
}

static void command_change_creature_owner(long origin_plyr_idx, const char *crtr_name, const char *criteria, long dest_plyr_idx)
{
    SCRIPTDBG(11, "Starting");
    long crtr_id = parse_creature_name(crtr_name);
    if (crtr_id == CREATURE_NONE)
    {
        SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
        return;
  }
  long select_id = parse_criteria(criteria);
  if (select_id == -1) {
    SCRPTERRLOG("Unknown select criteria, '%s'", criteria);
    return;
  }
  command_add_value(Cmd_CHANGE_CREATURE_OWNER, origin_plyr_idx, crtr_id, select_id, dest_plyr_idx);
}

static void command_computer_dig_to_location(long plr_range_id, const char* origin, const char* destination)
{
    TbMapLocation orig_loc;
    if (!get_map_location_id(origin, &orig_loc))
    {
        SCRPTWRNLOG("Dig to location script command has invalid source location: %s", origin);
        return;
    }
    TbMapLocation dest_loc;
    if (!get_map_location_id(destination, &dest_loc))
    {
        SCRPTWRNLOG("Dig to location script command has invalid destination location: %s", destination);
        return;
    }

    command_add_value(Cmd_COMPUTER_DIG_TO_LOCATION, plr_range_id, orig_loc, dest_loc, 0);
}

static void command_set_campaign_flag(long plr_range_id, const char *cmpflgname, long val)
{
    long flg_id = get_rid(campaign_flag_desc, cmpflgname);
    if (flg_id == -1)
    {
        SCRPTERRLOG("Unknown campaign flag, '%s'", cmpflgname);
        return;
  }
  command_add_value(Cmd_SET_CAMPAIGN_FLAG, plr_range_id, flg_id, val, 0);
}

static void command_add_to_campaign_flag(long plr_range_id, const char *cmpflgname, long val)
{
    long flg_id = get_rid(campaign_flag_desc, cmpflgname);
    if (flg_id == -1)
    {
        SCRPTERRLOG("Unknown campaign flag, '%s'", cmpflgname);
        return;
  }
  command_add_value(Cmd_ADD_TO_CAMPAIGN_FLAG, plr_range_id, flg_id, val, 0);
}

static void command_export_variable(long plr_range_id, const char *varib_name, const char *cmpflgname)
{
    long src_type;
    long src_id;
    // Recognize flag
    long flg_id = get_rid(campaign_flag_desc, cmpflgname);
    if (flg_id == -1)
    {
        SCRPTERRLOG("Unknown CAMPAIGN FLAG, '%s'", cmpflgname);
        return;
    }
    if (!parse_get_varib(varib_name, &src_id, &src_type, level_file_version))
    {
        SCRPTERRLOG("Unknown VARIABLE, '%s'", varib_name);
        return;
    }
    command_add_value(Cmd_EXPORT_VARIABLE, plr_range_id, src_type, src_id, flg_id);
}

static void command_use_spell_on_creature(long plr_range_id, const char *crtr_name, const char *criteria, const char *magname, CrtrExpLevel spell_level)
{
    SCRIPTDBG(11, "Starting");
    long mag_id = get_rid(spell_desc, magname);
    if (mag_id == -1)
    {
        SCRPTERRLOG("Unknown magic, '%s'", magname);
        return;
    }
    long crtr_id = parse_creature_name(crtr_name);
    if (crtr_id == CREATURE_NONE)
    {
        SCRPTERRLOG("Unknown creature, '%s'", crtr_name);
        return;
    }
    long select_id = parse_criteria(criteria);
    if (select_id == -1)
    {
        SCRPTERRLOG("Unknown select criteria, '%s'", criteria);
        return;
    }
    struct SpellConfig *spconf = get_spell_config(mag_id);
    if (spconf->linked_power) // Only check for spells linked to a keeper power.
    {
        if (spell_level < 1)
        {
            SCRPTWRNLOG("Spell %s level too low: %d, setting to 1.", magname, spell_level);
            spell_level = 1;
        }
        if (spell_level > (MAGIC_OVERCHARGE_LEVELS + 1)) // Creatures cast spells from level 1 to 10.
        {
            SCRPTWRNLOG("Spell %s level too high: %d, setting to %d.", magname, spell_level, (MAGIC_OVERCHARGE_LEVELS + 1));
            spell_level = MAGIC_OVERCHARGE_LEVELS;
        }
    }
    spell_level--;
    // SpellKind sp = mag_id;
    // encode params: free, magic, caster, level -> into 4xbyte: FMCL
    long fmcl_bytes;
    {
        signed char m = mag_id, lvl = spell_level;
        fmcl_bytes = (m << 8) | lvl;
    }
    command_add_value(Cmd_USE_SPELL_ON_CREATURE, plr_range_id, crtr_id, select_id, fmcl_bytes);
}

static void command_creature_entrance_level(long plr_range_id, unsigned char val)
{
  command_add_value(Cmd_CREATURE_ENTRANCE_LEVEL, plr_range_id, val, 0, 0);
}

static void command_make_unsafe(long plr_range_id)
{
    command_add_value(Cmd_MAKE_UNSAFE, plr_range_id, 0, 0, 0);
}

static void command_randomise_flag(long plr_range_id, const char *flgname, long val)
{
    long flg_id;
    long flag_type;
    if (!parse_set_varib(flgname, &flg_id, &flag_type))
    {
        SCRPTERRLOG("Unknown flag, '%s'", flgname);
        return;
    }
  command_add_value(Cmd_RANDOMISE_FLAG, plr_range_id, flg_id, val, flag_type);
}

static void command_compute_flag(long plr_range_id, const char *flgname, const char *operator_name, long src_plr_range_id, const char *src_flgname, long alt)
{
    long flg_id;
    long flag_type;
    if (!parse_set_varib(flgname, &flg_id, &flag_type))
    {
        SCRPTERRLOG("Unknown target flag, '%s'", flgname);
        return;
    }

    long src_flg_id;
    long src_flag_type;
    // try to identify source flag as a power, if it agrees, change flag type to SVar_AVAILABLE_MAGIC, keep power id
    // with rooms, traps, doors, etc. parse_get_varib assumes we want the count flag of them. Change it later in 'alt' switch if 'available' flag is needed
    src_flg_id = get_id(power_desc, src_flgname);
    if (src_flg_id == -1)
    {
        if (!parse_get_varib(src_flgname, &src_flg_id, &src_flag_type, level_file_version))
        {
            SCRPTERRLOG("Unknown source flag, '%s'", src_flgname);
            return;
        }
    } else
    {
        src_flag_type = SVar_AVAILABLE_MAGIC;
    }

    long op_id = get_rid(script_operator_desc, operator_name);
    if (op_id == -1)
    {
        SCRPTERRLOG("Invalid operation for modifying flag's value: '%s'", operator_name);
        return;
    }

    if (alt != 0)
    {
        switch (src_flag_type)
        {
            case SVar_CREATURE_NUM:
                src_flag_type = SVar_CONTROLS_CREATURE;
                break;
            case SVar_TOTAL_CREATURES:
                src_flag_type = SVar_CONTROLS_TOTAL_CREATURES;
                break;
            case SVar_TOTAL_DIGGERS:
                src_flag_type = SVar_CONTROLS_TOTAL_DIGGERS;
                break;
            case SVar_GOOD_CREATURES:
                src_flag_type = SVar_CONTROLS_GOOD_CREATURES;
                break;
            case SVar_EVIL_CREATURES:
                src_flag_type = SVar_CONTROLS_EVIL_CREATURES;
                break;
            case SVar_DOOR_NUM:
                src_flag_type = SVar_AVAILABLE_DOOR;
                break;
            case SVar_TRAP_NUM:
                src_flag_type = SVar_AVAILABLE_TRAP;
                break;
            case SVar_ROOM_SLABS:
                src_flag_type = SVar_AVAILABLE_ROOM;
                break;
        }
    }
    // encode 4 byte params into 4xbyte integer (from high-order bit to low-order):
    // 1st byte: src player range idx
    // 2nd byte: operation id
    // 3rd byte: flag type
    // 4th byte: src flag type
    long srcplr_op_flagtype_srcflagtype = (src_plr_range_id << 24) | (op_id << 16) | (flag_type << 8) | src_flag_type;
    command_add_value(Cmd_COMPUTE_FLAG, plr_range_id, srcplr_op_flagtype_srcflagtype, flg_id, src_flg_id);
}

/** Adds a script command to in-game structures.
 *
 * @param cmd_desc
 * @param scline
 */
void script_add_command(const struct CommandDesc *cmd_desc, const struct ScriptLine *scline, long file_version)
{
    if (cmd_desc->check_fn != NULL)
    {
        cmd_desc->check_fn(scline);
        return;
    }
    switch (cmd_desc->index)
    {
    case Cmd_CREATE_PARTY:
        command_create_party(scline->tp[0]);
        break;
    case Cmd_ADD_PARTY_TO_LEVEL:
        command_add_party_to_level(scline->np[0], scline->tp[1], scline->tp[2], scline->np[3]);
        break;
    case Cmd_ADD_CREATURE_TO_LEVEL:
        command_add_creature_to_level(scline->np[0], scline->tp[1], scline->tp[2], scline->np[3], scline->np[4], scline->np[5], scline->tp[6]);
        break;
    case Cmd_ENDIF:
        pop_condition();
        break;
    case Cmd_SET_HATE:
        command_set_hate(scline->np[0], scline->np[1], scline->np[2]);
        break;
    case Cmd_START_MONEY:
        command_set_start_money(scline->np[0], scline->np[1]);
        break;
    case Cmd_ROOM_AVAILABLE:
        command_room_available(scline->np[0], scline->tp[1], scline->np[2], scline->np[3]);
        break;
    case Cmd_CREATURE_AVAILABLE:
        if (file_version > 0) {
            command_creature_available(scline->np[0], scline->tp[1], scline->np[2], scline->np[3]);
        } else {
            command_creature_available(scline->np[0], scline->tp[1], scline->np[3], 0);
        }
        break;
    case Cmd_MAGIC_AVAILABLE:
        command_magic_available(scline->np[0], scline->tp[1], scline->np[2], scline->np[3]);
        break;
    case Cmd_TRAP_AVAILABLE:
        command_trap_available(scline->np[0], scline->tp[1], scline->np[2], scline->np[3]);
        break;
    case Cmd_RESEARCH:
        command_research(scline->np[0], scline->tp[1], scline->tp[2], scline->np[3]);
        break;
    case Cmd_RESEARCH_ORDER:
        command_research_order(scline->np[0], scline->tp[1], scline->tp[2], scline->np[3]);
        break;
    case Cmd_SET_TIMER:
        command_set_timer(scline->np[0], scline->tp[1]);
        break;
    case Cmd_IF_ACTION_POINT:
        command_if_action_point(scline->np[0], scline->np[1]);
        break;
    case Cmd_ADD_TUNNELLER_TO_LEVEL:
        command_add_tunneller_to_level(scline->np[0], scline->tp[1], scline->tp[2], scline->np[3], scline->np[4], scline->np[5]);
        break;
    case Cmd_WIN_GAME:
        command_win_game();
        break;
    case Cmd_LOSE_GAME:
        command_lose_game();
        break;
    case Cmd_SET_FLAG:
        command_set_flag(scline->np[0], scline->tp[1], scline->np[2]);
        break;
    case Cmd_MAX_CREATURES:
        command_max_creatures(scline->np[0], scline->np[1]);
        break;
    case Cmd_NEXT_COMMAND_REUSABLE:
        next_command_reusable = 2;
        break;
    case Cmd_DOOR_AVAILABLE:
        command_door_available(scline->np[0], scline->tp[1], scline->np[2], scline->np[3]);
        break;
    case Cmd_DISPLAY_INFORMATION:
        if (file_version > 0)
          command_display_information(scline->np[0], scline->tp[1], 0, 0);
        else
          command_display_information(scline->np[0], "ALL_PLAYERS", 0, 0);
        break;
    case Cmd_ADD_TUNNELLER_PARTY_TO_LEVEL:
        command_add_tunneller_party_to_level(scline->np[0], scline->tp[1], scline->tp[2], scline->tp[3], scline->np[4], scline->np[5], scline->np[6]);
        break;
    case Cmd_ADD_CREATURE_TO_POOL:
        command_add_creature_to_pool(scline->tp[0], scline->np[1]);
        break;
    case Cmd_TUTORIAL_FLASH_BUTTON:
        command_tutorial_flash_button(scline->np[0], scline->np[1]);
        break;
    case Cmd_SET_CREATURE_HEALTH:
        command_set_creature_health(scline->tp[0], scline->np[1]);
        break;
    case Cmd_SET_CREATURE_STRENGTH:
        command_set_creature_strength(scline->tp[0], scline->np[1]);
        break;
    case Cmd_SET_CREATURE_ARMOUR:
        command_set_creature_armour(scline->tp[0], scline->np[1]);
        break;
    case Cmd_SET_CREATURE_FEAR_WOUNDED:
        if (file_version > 0)
            command_set_creature_fear_wounded(scline->tp[0], scline->np[1]);
        else
            command_set_creature_fear_wounded(scline->tp[0], 101*scline->np[1]/255); // old fear was scaled 0..255
        break;
    case Cmd_SET_CREATURE_FEAR_STRONGER:
        command_set_creature_fear_stronger(scline->tp[0], scline->np[1]);
        break;
    case Cmd_SET_CREATURE_FEARSOME_FACTOR:
        command_set_creature_fearsome_factor(scline->tp[0], scline->np[1]);
        break;
    case Cmd_SET_CREATURE_PROPERTY:
        command_set_creature_property(scline->tp[0], scline->tp[1], scline->np[2]);
        break;
    case Cmd_IF_SLAB_OWNER:
        command_if_slab_owner(scline->np[0], scline->np[1], scline->np[2]);
        break;
    case Cmd_IF_SLAB_TYPE:
        command_if_slab_type(scline->np[0], scline->np[1], scline->np[2]);
        break;
    case Cmd_ALLY_PLAYERS:
        if (file_version > 0)
            command_ally_players(scline->np[0], scline->np[1], scline->np[2]);
        else
            command_ally_players(scline->np[0], scline->np[1], true);
        break;
    case Cmd_DEAD_CREATURES_RETURN_TO_POOL:
        command_dead_creatures_return_to_pool(scline->np[0]);
        break;
    case Cmd_DISPLAY_INFORMATION_WITH_POS:
        command_display_information(scline->np[0], NULL, scline->np[1], scline->np[2]);
        break;
    case Cmd_BONUS_LEVEL_TIME:
        command_bonus_level_time(scline->np[0], scline->np[1]);
        break;
    case Cmd_QUICK_OBJECTIVE:
        command_quick_objective(scline->np[0], scline->tp[1], scline->tp[2], 0, 0);
        break;
    case Cmd_QUICK_INFORMATION:
        if (file_version > 0)
          command_quick_information(scline->np[0], scline->tp[1], scline->tp[2], 0, 0);
        else
          command_quick_information(scline->np[0], scline->tp[1], "ALL_PLAYERS", 0, 0);
        break;
    case Cmd_QUICK_OBJECTIVE_WITH_POS:
        command_quick_objective(scline->np[0], scline->tp[1], NULL, scline->np[2], scline->np[3]);
        break;
    case Cmd_QUICK_INFORMATION_WITH_POS:
        command_quick_information(scline->np[0], scline->tp[1], NULL, scline->np[2], scline->np[3]);
        break;
    case Cmd_PRINT:
        command_message(scline->tp[0],80);
        break;
    case Cmd_MESSAGE:
        command_message(scline->tp[0],68);
        break;
    case Cmd_ADD_GOLD_TO_PLAYER:
        command_add_gold_to_player(scline->np[0], scline->np[1]);
        break;
    case Cmd_SET_CREATURE_TENDENCIES:
        command_set_creature_tendencies(scline->np[0], scline->tp[1], scline->np[2]);
        break;
    case Cmd_REVEAL_MAP_RECT:
        command_reveal_map_rect(scline->np[0], scline->np[1], scline->np[2], scline->np[3], scline->np[4]);
        break;
    case Cmd_KILL_CREATURE:
        command_kill_creature(scline->np[0], scline->tp[1], scline->tp[2], scline->np[3]);
        break;
    case Cmd_LEVEL_UP_CREATURE:
        command_level_up_creature(scline->np[0], scline->tp[1], scline->tp[2], scline->np[3]);
        break;
    case Cmd_USE_POWER_ON_CREATURE:
        command_use_power_on_creature(scline->np[0], scline->tp[1], scline->tp[2], scline->np[3], scline->tp[4], scline->np[5], scline->tp[6]);
        break;
    case Cmd_USE_SPELL_ON_CREATURE:
        command_use_spell_on_creature(scline->np[0], scline->tp[1], scline->tp[2], scline->tp[3], scline->np[4]);
        break;
    case Cmd_USE_POWER_AT_POS:
        command_use_power_at_pos(scline->np[0], scline->np[1], scline->np[2], scline->tp[3], scline->np[4], scline->tp[5]);
        break;
    case Cmd_USE_POWER_AT_LOCATION:
        command_use_power_at_location(scline->np[0], scline->tp[1], scline->tp[2], scline->np[3], scline->tp[4]);
        break;
    case Cmd_USE_POWER:
        command_use_power(scline->np[0], scline->tp[1], scline->tp[2]);
        break;
    case Cmd_USE_SPECIAL_INCREASE_LEVEL:
        command_use_special_increase_level(scline->np[0], scline->np[1]);
        break;
    case Cmd_USE_SPECIAL_MULTIPLY_CREATURES:
        command_use_special_multiply_creatures(scline->np[0], scline->np[1]);
        break;
    case Cmd_MAKE_SAFE:
        make_safe(scline->np[0]);
        break;
    case Cmd_LOCATE_HIDDEN_WORLD:
        command_locate_hidden_world();
        break;
    case Cmd_CHANGE_CREATURE_OWNER:
        command_change_creature_owner(scline->np[0], scline->tp[1], scline->tp[2], scline->np[3]);
        break;
    case Cmd_LEVEL_VERSION:
        level_file_version = scline->np[0];
        SCRPTLOG("Level files version %ld.",level_file_version);
        break;
    case Cmd_ADD_TO_FLAG:
        command_add_to_flag(scline->np[0], scline->tp[1], scline->np[2]);
        break;
    case Cmd_SET_CAMPAIGN_FLAG:
        command_set_campaign_flag(scline->np[0], scline->tp[1], scline->np[2]);
        break;
    case Cmd_ADD_TO_CAMPAIGN_FLAG:
        command_add_to_campaign_flag(scline->np[0], scline->tp[1], scline->np[2]);
        break;
    case Cmd_EXPORT_VARIABLE:
        command_export_variable(scline->np[0], scline->tp[1], scline->tp[2]);
        break;
    case Cmd_RUN_AFTER_VICTORY:
        if (scline->np[0] == 1)
        {
            game.system_flags |= GSF_RunAfterVictory;
        }
        break;
    case Cmd_COMPUTER_DIG_TO_LOCATION:
        command_computer_dig_to_location(scline->np[0], scline->tp[1], scline->tp[2]);
        break;
    case Cmd_CREATURE_ENTRANCE_LEVEL:
        command_creature_entrance_level(scline->np[0], scline->np[1]);
        break;
    case Cmd_MAKE_UNSAFE:
        command_make_unsafe(scline->np[0]);
        break;
    case Cmd_RANDOMISE_FLAG:
        command_randomise_flag(scline->np[0], scline->tp[1], scline->np[2]);
        break;
    case Cmd_COMPUTE_FLAG:
        command_compute_flag(scline->np[0], scline->tp[1], scline->tp[2], scline->np[3], scline->tp[4], scline->np[5]);
        break;
    default:
        SCRPTERRLOG("Unhandled SCRIPT command '%s'", scline->tcmnd);
        break;
    }
}
