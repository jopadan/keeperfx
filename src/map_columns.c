/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file map_columns.c
 *     Column and columns array data management functions.
 * @par Purpose:
 *     Functions to support the column data array.
 * @par Comment:
 *     None.
 * @author   Tomasz Lis
 * @date     27 Oct 2009 - 09 Nov 2009
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "map_columns.h"
#include "globals.h"

#include "config_terrain.h"
#include "slab_data.h"
#include "game_legacy.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
struct Column *get_column(long idx)
{
  if ((idx < 1) || (idx >= COLUMNS_COUNT))
    return INVALID_COLUMN;
  return game.columns.lookup[idx];
}

struct Column *get_column_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
  struct Map *mapblk;
  mapblk = get_map_block_at(stl_x, stl_y);
  if (map_block_invalid(mapblk))
    return INVALID_COLUMN;
  return game.columns.lookup[get_mapblk_column_index(mapblk)];
}

struct Column *get_map_column(const struct Map *mapblk)
{
  if (map_block_invalid(mapblk))
    return INVALID_COLUMN;
  return game.columns.lookup[get_mapblk_column_index(mapblk)];
}

TbBool column_invalid(const struct Column *colmn)
{
  if (colmn == NULL)
    return true;
  if (colmn == INVALID_COLUMN)
    return true;
  return (colmn <= game.columns.lookup[0]) || (colmn > game.columns.lookup[COLUMNS_COUNT-1]) || (colmn == NULL);
}

/**
 * Returns amount of filled subtiles at bottom of given column.
 * @param col The column which filled height should be returned.
 */
long get_column_floor_filled_subtiles(const struct Column *col)
{
    return (col->bitfields & 0xF0) >> 4;
}

/**
 * Returns amount of filled subtiles at bottom of column at given map block.
 * @param mapblk The map block for which column height should be returned.
 */
long get_map_floor_filled_subtiles(const struct Map *mapblk)
{
    const struct Column *col;
    col = get_map_column(mapblk);
    if (column_invalid(col))
        return 0;
    return (col->bitfields & 0xF0) >> 4;
}

/**
 * Returns amount of filled subtiles at bottom of column at given coords.
 * @param stl_x Subtile for which column height should be returned, X coord.
 * @param stl_y Subtile for which column height should be returned, Y coord.
 */
long get_floor_filled_subtiles_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    const struct Column *col;
    col = get_column_at(stl_x, stl_y);
    if (column_invalid(col))
        return 0;
    return (col->bitfields & 0xF0) >> 4;
}

/**
 * Sets amount of filled subtiles at bottom of given column.
 * @param col The column which filled height should be set.
 * @param n Amount of subtiles.
 */
void set_column_floor_filled_subtiles(struct Column *col, MapSubtlCoord n)
{
    col->bitfields &= ~0xF0;
    col->bitfields |= (n<<4) & 0xF0;
}

/**
 * Sets amount of filled subtiles at bottom of a column at given map block.
 * @param mapblk The map block for which filled height should be set.
 * @param n Amount of subtiles.
 */
void set_map_floor_filled_subtiles(struct Map *mapblk, MapSubtlCoord n)
{
    struct Column *col;
    col = get_map_column(mapblk);
    if (column_invalid(col))
        return;
    col->bitfields &= ~0xF0;
    col->bitfields |= (n<<4) & 0xF0;
}

/**
 * Returns amount of filled subtiles at top of given column.
 * @param col The column which filled height should be returned.
 */
long get_column_ceiling_filled_subtiles(const struct Column *col)
{
    return (col->bitfields & CLF_CEILING_MASK) >> 1;
}

/**
 * Returns amount of filled subtiles at top of column at given map block.
 * @param mapblk The map block for which column height should be returned.
 */
long get_map_ceiling_filled_subtiles(const struct Map *mapblk)
{
    const struct Column *col;
    col = get_map_column(mapblk);
    if (column_invalid(col))
        return 0;
    return (col->bitfields & CLF_CEILING_MASK) >> 1;
}

/**
 * Returns amount of filled subtiles at top of column at given coords.
 * @param stl_x Subtile for which column height should be returned, X coord.
 * @param stl_y Subtile for which column height should be returned, Y coord.
 */
long get_ceiling_filled_subtiles_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    const struct Column *colmn;
    colmn = get_column_at(stl_x, stl_y);
    if (column_invalid(colmn))
        return 0;
    return (colmn->bitfields & CLF_CEILING_MASK) >> 1;
}

/**
 * Sets amount of filled subtiles at top of given column.
 * @param col The column which filled height should be set.
 * @param n Amount of subtiles.
 */
void set_column_ceiling_filled_subtiles(struct Column *col, MapSubtlCoord n)
{
    col->bitfields &= ~CLF_CEILING_MASK;
    col->bitfields |= (n<<1) & CLF_CEILING_MASK;
}

/**
 * Sets amount of filled subtiles at top of a column at given map block.
 * @param mapblk The map block for which filled height should be set.
 * @param n Amount of subtiles.
 */
void set_map_ceiling_filled_subtiles(struct Map *mapblk, MapSubtlCoord n)
{
    struct Column *col;
    col = get_map_column(mapblk);
    if (column_invalid(col))
        return;
    col->bitfields &= ~CLF_CEILING_MASK;
    col->bitfields |= (n<<1) & CLF_CEILING_MASK;
}

TbBool map_pos_solid_at_ceiling(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    const struct Map *mapblk;
    mapblk = get_map_block_at(stl_x, stl_y);
    if ((mapblk->flags & SlbAtFlg_Blocking) != 0)
        return true;
    return get_map_ceiling_filled_subtiles(mapblk) > 0;
}

long get_top_cube_at_pos(SubtlCodedCoords stl_num)
{
    struct Column *col;
    struct Map *mapblk;
    unsigned long top_pos;
    long tcube;
    mapblk = get_map_block_at_pos(stl_num);
    col = get_map_column(mapblk);
    top_pos = get_column_floor_filled_subtiles(col);
    if (top_pos > 0)
        tcube = col->cubes[top_pos-1];
    else
        tcube = game.top_cube[col->floor_texture];
    return tcube;
}

long get_top_cube_at(MapSubtlCoord stl_x, MapSubtlCoord stl_y, long *cube_pos)
{
    struct Column *col;
    unsigned long top_pos;
    long tcube;
    col = get_column_at(stl_x, stl_y);
    top_pos = get_column_floor_filled_subtiles(col);
    if (top_pos > 0)
        tcube = col->cubes[top_pos-1];
    else
        tcube = game.top_cube[col->floor_texture];
    if (cube_pos != NULL)
        *cube_pos = top_pos;
    return tcube;
}

void make_solidmask(struct Column *col)
{
  int i;
  col->solidmask = 0;
  for (i=0; i<COLUMN_STACK_HEIGHT; i++)
  {
    if (col->cubes[i] != 0)
      col->solidmask |= (1 << i);
  }
}

unsigned short find_column_height(struct Column *col)
{
  unsigned short h;
  h = 0;
  if (col->solidmask == 0)
    return h;
  while (col->cubes[h] > 0)
  {
    h++;
    if (h >= COLUMN_STACK_HEIGHT)
      return COLUMN_STACK_HEIGHT;
  }
  return h;
}

/**
 * Returns height of a floor, in map coordinates.
 * @param mapblk The map block for which floor height should be returned.
 */
long get_map_floor_height(const struct Map *mapblk)
{
    const struct Column *colmn;
    long i;
    long cubes_height;
    colmn = get_map_column(mapblk);
    i = get_column_floor_filled_subtiles(colmn);
    if (i > 0) {
        cubes_height = i;
    } else {
        cubes_height = 0;
    }
    return cubes_height << 8;
}

/**
 * Returns height of a floor, in map coordinates.
 * @param pos The coordinates of a block for which floor height should be returned.
 */
long get_floor_height_at(const struct Coord3d *pos)
{
    struct Map *mapblk;
    mapblk = get_map_block_at(pos->x.val >> 8, pos->y.val >> 8);
    return get_map_floor_height(mapblk);
}

long get_floor_height(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Map *mapblk;
    mapblk = get_map_block_at(stl_x, stl_y);
    return get_map_floor_height(mapblk);
}

long get_map_ceiling_height(const struct Map *mapblk)
{
    const struct Column *colmn;
    long i;
    long cubes_height;
    colmn = get_map_column(mapblk);
    i = get_column_ceiling_filled_subtiles(colmn);
    if (i > 0) {
        cubes_height = 8 - i;
    } else {
        cubes_height = get_mapblk_filled_subtiles(mapblk);
    }
    return cubes_height << 8;
}

long get_ceiling_height_at(const struct Coord3d *pos)
{
    struct Map *mapblk;
    mapblk = get_map_block_at(pos->x.val >> 8, pos->y.val >> 8);
    return get_map_ceiling_height(mapblk);
}

long get_ceiling_height_at_subtile(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Map *mapblk;
    mapblk = get_map_block_at(stl_x, stl_y);
    return get_map_ceiling_height(mapblk);
}

static TbBool column_is_equivalent(struct Column * const src, struct Column *dst)
{
    if ((src->floor_texture != dst->floor_texture) ||
        (src->solidmask != dst->solidmask) ||
        (src->orient != dst->orient))
        return 0;
    return 0 == memcmp(src->cubes, dst->cubes, sizeof(src->cubes));
}

long find_column(struct Column *srccol)
{
    int i;
    for (i=1; i < COLUMNS_COUNT; i++) {
        struct Column *col;
        col = get_column(i);
        if (column_is_equivalent(srccol, col)) {
          return i;
        }
    }
    return 0;
}

long create_column(struct Column *col)
{
    long result;
    struct Column *dst;
    unsigned char v6;
    unsigned char top_of_floor;

    // Find an empty column
    result = 1;
    dst = &game.columns_data[1];
    while (dst->use || dst->bitfields & CLF_ACTIVE )
    {
        ++result;
        ++dst;
        if ( result >= COLUMNS_COUNT )
        {
            ERRORLOG("Could not create column: None free");
            return 0;
        }
    }
    // Copy data
    memcpy(dst, col, sizeof(struct Column));
    // Create cubemask
    make_solidmask(dst);

    // Find lowest cube
    for (v6 = 0; v6 < COLUMN_STACK_HEIGHT;v6++)
    {
        if ( dst->cubes[v6] == 0)
            break;
    }
    top_of_floor = v6;
    // set lowest cube info
    dst->bitfields &= ~CLF_FLOOR_MASK;
    dst->bitfields |= (top_of_floor << 4);
    ///
    if (top_of_floor >= COLUMN_STACK_HEIGHT )
    {
        dst->bitfields &= CLF_FLOOR_MASK | CLF_ACTIVE;
    }
    else
    {
        // Search for ceiling
        unsigned char ceiling = top_of_floor;
        for (; ceiling < COLUMN_STACK_HEIGHT; ceiling++)
        {
            if (dst->cubes[ceiling])
                break;
        }

        if (ceiling >= COLUMN_STACK_HEIGHT)
        {
            dst->bitfields &= CLF_FLOOR_MASK | CLF_ACTIVE;
        }
        else
        {
            unsigned short *v13 = &dst->cubes[7];
            unsigned char v12 = 0;
            // Counting ceiling height
            for (int i = 0; i < COLUMN_STACK_HEIGHT-1; i++)
            {
                if (*v13)
                    dst->bitfields ^= (v12 ^ dst->bitfields) & CLF_CEILING_MASK;
                --v13;
                v12 += 2;
            }
        }
    }
    return result;
}

void clear_columns(void)
{
  struct Column *colmn;
  int i;
  for (i=0; i < COLUMNS_COUNT; i++)
  {
    colmn = &game.columns_data[i];
    memset(colmn, 0, sizeof(struct Column));
    colmn->floor_texture = 1;
    make_solidmask(colmn);
  }
  game.unrevealed_column_idx = 0;
  for (i=0; i < 18; i++)
  {
    game.col_static_entries[i] = 0;
  }
}

void init_columns(void)
{
    int i;
    for (i=1; i < COLUMNS_COUNT; i++)
    {
        struct Column *col;
        col = get_column(i);
        if (col->use)
        {
            unsigned long mskbit;
            mskbit = 1;
            col->solidmask = 0;
            int n;
            for (n=0; n < COLUMN_STACK_HEIGHT; n++)
            {
                if (col->cubes[n] != 0) {
                    col->solidmask |= mskbit;
                }
                mskbit *= 2;
            }
            if (col->solidmask)
            {
                for (n=0; n < COLUMN_STACK_HEIGHT; n++)
                {
                    if (col->cubes[n] == 0) {
                        break;
                    }
                }
            } else
            {
                n = 0;
            }
            set_column_floor_filled_subtiles(col, n);
            n = get_column_floor_filled_subtiles(col);
            for (;n < COLUMN_STACK_HEIGHT; n++)
            {
                if (col->cubes[n] != 0) {
                  break;
                }
            }
            if (n >= COLUMN_STACK_HEIGHT)
            {
                col->bitfields &= ~0x0E;
            } else
            {
                mskbit = 0;
                for (n=COLUMN_STACK_HEIGHT-1; n > 0; n--)
                {
                    if (col->cubes[n] != 0) {
                        col->bitfields ^= (mskbit ^ col->bitfields) & 0xE;
                    }
                    mskbit += 2;
                }
            }
        }
    }
}

void init_whole_blocks(void)
{
    struct Column *colmn;
    struct Column lcolmn;
    long i;
    memset(&lcolmn, 0, sizeof(struct Column));
    // Prepare the local column
    lcolmn.floor_texture = 22;
    lcolmn.cubes[0] = 10;
    lcolmn.cubes[1] = 1;
    lcolmn.cubes[2] = 1;
    lcolmn.cubes[3] = 1;
    lcolmn.cubes[4] = 141;
    make_solidmask(&lcolmn);
    // Find it or add to column list
    i = find_column(&lcolmn);
    if (i == 0)
      i = create_column(&lcolmn);
    colmn = get_column(i);
    // Update its parameters
    colmn->bitfields |= CLF_ACTIVE;
    game.unrevealed_column_idx = i;
}

void init_top_texture_to_cube_table(void)
{
    memset(game.top_cube, 0, sizeof(game.top_cube));
    int n;
    for (n=1; n < TEXTURE_BLOCKS_COUNT; n++)
    {
        int i;
        for (i=1; i < CUBE_ITEMS_MAX; i++)
        {
            struct CubeConfigStats * cubed;
            cubed = get_cube_model_stats(i);
            if (cubed->texture_id[4] == n) {
                game.top_cube[n] = i;
                break;
            }
        }
    }
}

/* Returns if given cube is lava.
 * @param cube_id
 * @return */
TbBool cube_is_lava(long cube_id)
{
    struct CubeConfigStats *cubest = get_cube_model_stats(cube_id);
    return flag_is_set(cubest->properties_flags, CPF_IsLava);
}

/* Returns if given cube is water.
 * @param cube_id
 * @return */
TbBool cube_is_water(long cube_id)
{
    struct CubeConfigStats *cubest = get_cube_model_stats(cube_id);
    return flag_is_set(cubest->properties_flags, CPF_IsWater);
}

/* Returns if given cube is a sacrificial ground or magic door surface.
 * @param cube_id
 * @return */
TbBool cube_is_sacrificial(long cube_id)
{
    struct CubeConfigStats *cubest = get_cube_model_stats(cube_id);
    return flag_is_set(cubest->properties_flags, CPF_IsSacrificial);
}

/* Returns if given cube is unclaimed path.
 * @param cube_id
 * @return */
TbBool cube_is_unclaimed_path(long cube_id)
{
    struct CubeConfigStats *cubest = get_cube_model_stats(cube_id);
    return flag_is_set(cubest->properties_flags, CPF_IsUnclaimedPath);
}

/**
 * Returns if given subtile has lava cube on its top.
 * @param stl_x Subtile X coordinate.
 * @param stl_y Subtile Y coordinate.
 * @return True if the top cube is lava; false otherwise.
 */
TbBool subtile_has_lava_on_top(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    long i;
    i = get_top_cube_at(stl_x, stl_y, NULL);
    return cube_is_lava(i);
}

/**
 * Returns if given subtile has water cube on its top.
 * @param stl_x Subtile X coordinate.
 * @param stl_y Subtile Y coordinate.
 * @return True if the top cube is water; false otherwise.
 */
TbBool subtile_has_water_on_top(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    long i;
    i = get_top_cube_at(stl_x, stl_y, NULL);
    return cube_is_water(i);
}

/**
 * Returns if given subtile has sacrificial (temple) cube on its top.
 * @param stl_x Subtile X coordinate.
 * @param stl_y Subtile Y coordinate.
 * @return True if the top cube is sacrificial; false otherwise.
 */
TbBool subtile_has_sacrificial_on_top(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    long i;
    long cube_pos;
    i = get_top_cube_at(stl_x, stl_y, &cube_pos);
    // Only low ground cubes are really sacrificial - high ground is most likely magic door.
    return cube_pos<4 && cube_is_sacrificial(i);
}

TbBool subtile_is_liquid(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    return ((subtile_has_water_on_top(stl_x, stl_y)) || (subtile_has_lava_on_top(stl_x, stl_y)));
}

TbBool subtile_is_unclaimed_path(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Room *room = subtile_room_get(stl_x, stl_y);
    if (!room_is_invalid(room))
    {
        return false;
    }
    long i;
    i = get_top_cube_at(stl_x, stl_y, NULL);
    return cube_is_unclaimed_path(i);
}

TbBool subtile_is_wall(MapSubtlCoord stl_x, MapSubtlCoord stl_y)
{
    struct Column* col = get_column_at(stl_x, stl_y);
    return (get_column_floor_filled_subtiles(col) >= COLUMN_WALL_HEIGHT);
}

/******************************************************************************/
#ifdef __cplusplus
}
#endif
