/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file room_lair.c
 *     Lair room and creature lairs maintain functions.
 * @par Purpose:
 *     Functions to create and use lairs.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     07 Apr 2011 - 20 Nov 2012
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "room_lair.h"

#include "globals.h"
#include "bflib_basics.h"
#include "bflib_planar.h"
#include "room_data.h"
#include "player_data.h"
#include "dungeon_data.h"
#include "thing_data.h"
#include "thing_navigate.h"
#include "creature_control.h"
#include "config_creature.h"
#include "gui_soundmsgs.h"
#include "game_legacy.h"
#include "front_simple.h"
#include "thing_effects.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif

/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
struct Room *start_rooms;
struct Room *end_rooms;
/******************************************************************************/
long calculate_free_lair_space(struct Dungeon * dungeon)
{
    SYNCDBG(9,"Starting");
    long cap_used = 0;
    long cap_total = 0;
    unsigned long k = 0;
    long i;

    for (RoomKind rkind = 0; rkind < game.conf.slab_conf.room_types_count; rkind++)
    {
        if(room_role_matches(rkind,RoRoF_LairStorage))
        {
            i = dungeon->room_kind[rkind];
            while (i != 0)
            {
                struct Room* room = room_get(i);
                if (room_is_invalid(room))
                {
                    ERRORLOG("Jump to invalid room detected");
                    break;
                }
                i = room->next_of_owner;
                // Per-room code
                cap_total += room->total_capacity;
                cap_used += room->used_capacity;
                // Per-room code ends
                k++;
                if (k > ROOMS_COUNT)
                {
                    ERRORLOG("Infinite loop detected when sweeping rooms list");
                    break;
                }
            }
        }
    }
    long cap_required = 0;
    k = 0;
    i = dungeon->creatr_list_start;
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        struct CreatureControl* cctrl = creature_control_get_from_thing(thing);
        if (thing_is_invalid(thing) || creature_control_invalid(cctrl))
        {
            ERRORLOG("Jump to invalid creature detected");
            break;
        }
        i = cctrl->players_next_creature_idx;
        // Thing list loop body
        if (cctrl->lair_room_id == 0)
        {
            struct CreatureModelConfig* crconf = creature_stats_get_from_thing(thing);
            cap_required += crconf->lair_size;
        }
        // Thing list loop body ends
        k++;
        if (k > CREATURES_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping creatures list");
            break;
        }
    }
    SYNCDBG(9,"Total lair capacity %d, used %d, will need %d more",(int)cap_total,(int)cap_used,(int)cap_required);
    return cap_total - cap_used - cap_required;
}

static short get_lair_score(TbBool room_has_units_of_same_kind,TbBool room_has_units_of_different_kind,TbBool room_has_lair_enemy)
{
    if ( room_has_units_of_same_kind )
    {
        if ( room_has_units_of_different_kind )
        {
            if ( room_has_lair_enemy )
            {
                return 2;
            }
            else
            {
                return 5;
            }
        }
        else
        {
            if ( room_has_lair_enemy )
            {
                return 0;
            }
            else
            {
                return 6;
            }
        }
    }
    else if ( room_has_units_of_different_kind )
    {
        if ( room_has_lair_enemy )
        {
            return 1;
        }
        else
        {
            return 3;
        }
    }
    else
    {
        return 4;
    }
}

TbBool creature_model_is_lair_enemy(const ThingModel lair_enemy[LAIR_ENEMY_MAX], ThingModel crmodel)
{
    for (int i = 0; i < LAIR_ENEMY_MAX; i++)
    {
        if (lair_enemy[i] == crmodel)
        {
            return true;
        }
    }
    return false;
}

TbBool creature_model_is_hostile_towards(const ThingModel hostile_towards[CREATURE_TYPES_MAX], ThingModel crmodel)
{
    for (int i = 0; i < CREATURE_TYPES_MAX; i++)
    {
        if (hostile_towards[i] == crmodel)
        {
            return true;
        }
    }
    return false;
}

struct Room *get_best_new_lair_for_creature(struct Thing *creatng)
{
    struct Room *room;
    char best_score = 0;

    const struct CreatureModelConfig* crconf = creature_stats_get_from_thing(creatng);
    struct Dungeon* dungeon = get_dungeon(creatng->owner);

    short *room_scores = (short *)big_scratch;
    memset(big_scratch, 0, ROOMS_COUNT);


    for (RoomKind rkind = 0; rkind < game.conf.slab_conf.room_types_count; rkind++)
    {
        if(room_role_matches(rkind,RoRoF_LairStorage))
        {
            room = room_get(dungeon->room_kind[rkind]);
            while (!room_is_invalid(room))
            {
                if ( room_has_enough_free_capacity_for_creature_job(room, creatng, Job_TAKE_SLEEP) && creature_can_head_for_room(creatng, room, 0) )
                {
                    TbBool room_has_units_of_same_kind = false;
                    TbBool room_has_units_of_different_kind = false;
                    TbBool room_has_lair_enemy = false;
                    for ( ThingModel model = 0; model < game.conf.crtr_conf.model_count; ++model )
                    {
                        if ( room_has_units_of_same_kind && room_has_units_of_different_kind && room_has_lair_enemy )
                            break;
                        if ( room->content_per_model[model] > 0) 
                        {
                            if ( creatng->model == model )
                            {
                                room_has_units_of_same_kind = true;
                            }
                            else
                            {
                                room_has_units_of_different_kind = true;
                            }
                            if (creature_model_is_lair_enemy(crconf->lair_enemy, model)
                            || creature_model_is_hostile_towards(crconf->hostile_towards, model))
                            {
                                room_has_lair_enemy = true;
                            }
                        }
                    }
                    char lair_score = get_lair_score(room_has_units_of_same_kind,room_has_units_of_different_kind,room_has_lair_enemy);
                    room_scores[room->index] = lair_score;
                    if (best_score < lair_score)
                        best_score = lair_score;
                }
                room = room_get(room->next_of_owner);
            }
        }
    }
        
    if (best_score == 0)
    {
        return INVALID_ROOM;
    }

    struct Room *nearest_room = INVALID_ROOM;
    MapCoordDelta distance;
    MapCoordDelta min_distance = INT_MAX;
    struct Coord3d room_center_pos;

    for (RoomKind rkind = 0; rkind < game.conf.slab_conf.room_types_count; rkind++)
    {
        if(room_role_matches(rkind,RoRoF_LairStorage))
        {
            room = room_get(dungeon->room_kind[rkind]);
            while (!room_is_invalid(room))
            {
                if ( room_scores[room->index] == best_score )
                {
                    room_center_pos.x.val = subtile_coord_center(room->central_stl_x);
                    room_center_pos.y.val = subtile_coord_center(room->central_stl_y);
                    room_center_pos.z.val = get_floor_height_at(&room_center_pos);
                    distance = get_chessboard_distance(&creatng->mappos, &room_center_pos);

                    if ( min_distance > distance )
                    {
                        min_distance = distance;
                        nearest_room = room;
                    }
                }
                room = room_get(room->next_of_owner);
            }
        }
    }
    return nearest_room;
}

void count_lair_occupants_on_slab(struct Room *room,MapSlabCoord slb_x, MapSlabCoord slb_y)
{
    SYNCDBG(17,"Starting for %s index %d at %d,%d",room_code_name(room->kind),(int)room->index,(int)slb_x,(int)slb_y);
    for (int n = 0; n < MID_AROUND_LENGTH; n++)
    {
        MapSubtlDelta ssub_x = 1 + start_at_around[n].delta_x;
        MapSubtlDelta ssub_y = 1 + start_at_around[n].delta_y;
        struct Thing* lairtng = find_lair_totem_at(slab_subtile(slb_x, ssub_x), slab_subtile(slb_y, ssub_y));
        if (!thing_is_invalid(lairtng))
        {
            struct Thing* creatng = thing_get(lairtng->lair.belongs_to);
            int required_cap = get_required_room_capacity_for_object(RoRoF_LairStorage, 0, creatng->model);
            if (room->used_capacity + required_cap > room->total_capacity)
            {
                create_effect(&lairtng->mappos, imp_spangle_effects[get_player_color_idx(lairtng->owner)], lairtng->owner);
                delete_lair_totem(lairtng);
            } else
            {
                struct CreatureControl* cctrl = creature_control_get_from_thing(creatng);
                cctrl->lair_room_id = room->index;
                room->content_per_model[creatng->model]++;
                room->used_capacity += required_cap;
            }
        }
    }
}

void count_lair_occupants(struct Room *room)
{
    room->used_capacity = 0;
    memset(room->content_per_model, 0, sizeof(room->content_per_model));
    unsigned long k = 0;
    unsigned long i = room->slabs_list;
    while (i > 0)
    {
        MapSubtlCoord slb_x = slb_num_decode_x(i);
        MapSubtlCoord slb_y = slb_num_decode_y(i);
        struct SlabMap* slb = get_slabmap_direct(i);
        if (slabmap_block_invalid(slb))
        {
            ERRORLOG("Jump to invalid room slab detected");
            break;
        }
        i = get_next_slab_number_in_room(i);
        // Per slab code
        count_lair_occupants_on_slab(room, slb_x, slb_y);
        // Per slab code ends
        k++;
        if (k > game.map_tiles_x * game.map_tiles_y)
        {
            ERRORLOG("Infinite loop detected when sweeping room slabs");
            break;
        }
    }
}

struct Thing *find_lair_totem_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Map* mapblk = get_map_block_at(stl_x, stl_y);
    unsigned long k = 0;
    long i = get_mapwho_thing_index(mapblk);
    while (i != 0)
    {
        struct Thing* thing = thing_get(i);
        TRACE_THING(thing);
        if (thing_is_invalid(thing))
        {
            ERRORLOG("Jump to invalid thing detected");
            break;
        }
        i = thing->next_on_mapblk;
        // Per thing code start
        if (thing_is_lair_totem(thing)) {
            return thing;
        }
        // Per thing code end
        k++;
        if (k > THINGS_COUNT)
        {
            ERRORLOG("Infinite loop detected when sweeping things list");
            break_mapwho_infinite_chain(mapblk);
            break;
        }
    }
    return INVALID_THING;
}
/******************************************************************************/
