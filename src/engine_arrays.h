/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file engine_arrays.h
 *     Header file for engine_arrays.c.
 * @par Purpose:
 *     Helper arrays for the engine.
 * @par Comment:
 *     Just a header file - #defines, typedefs, function prototypes etc.
 * @author   Tomasz Lis
 * @date     02 Apr 2010 - 06 Nov 2010
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/

#ifndef DK_ENGNARR_H
#define DK_ENGNARR_H

#include "bflib_basics.h"
#include "globals.h"
#include "engine_render.h"

// All anims below this have two different sprites in 3d and map view
#define TD_ISO_POINTS        982
#define RANDOMISORS_LEN      512
#define RANDOMISORS_MASK   0x1ff
#define RANDOMISORS_RANGE     63

#define WIBBLE_TABLE_SIZE   128

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
#pragma pack(1)

struct WibbleTable {
  long field_0;
  long field_4;
  long field_8;
  long field_C;
  long field_10;
  long field_14;
  long field_18;
  long field_1C;
};
/******************************************************************************/
extern long randomisors[512];
extern struct WibbleTable wibble_table[WIBBLE_TABLE_SIZE];
extern long floor_height_table[256];
extern long lintel_top_height[256];
extern long lintel_bottom_height[256];

#pragma pack()

extern short iso_td_add[KEEPERSPRITE_ADD_NUM];
extern short td_iso_add[KEEPERSPRITE_ADD_NUM];
/******************************************************************************/
extern unsigned short floor_to_ceiling_map[TEXTURE_BLOCKS_COUNT];
extern struct WibbleTable blank_wibble_table[WIBBLE_TABLE_SIZE];
/******************************************************************************/
short convert_td_iso(short n);
short straight_td_iso(short n);
short straight_iso_td(short n);

void init_iso_3d_conversion_tables(void);
void setup_3d(void);

TbBool load_ceiling_table(void);
void generate_wibble_table(void);
void fill_floor_heights_table(void);

/******************************************************************************/
#ifdef __cplusplus
}
#endif
#endif
