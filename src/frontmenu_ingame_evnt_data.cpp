/******************************************************************************/
// Free implementation of Bullfrog's Dungeon Keeper strategy game.
/******************************************************************************/
/** @file frontmenu_ingame_evnt_data.cpp
 *     In-game events GUI, visible during gameplay at bottom.
 * @par Purpose:
 *     Structures to show and maintain message menu appearing ingame.
 * @par Comment:
 *     None.
 * @author   KeeperFX Team
 * @date     05 Jan 2009 - 11 Feb 2013
 * @par  Copying and copyrights:
 *     This program is free software; you can redistribute it and/or modify
 *     it under the terms of the GNU General Public License as published by
 *     the Free Software Foundation; either version 2 of the License, or
 *     (at your option) any later version.
 */
/******************************************************************************/
#include "pre_inc.h"
#include "frontmenu_ingame_evnt.h"
#include "frontmenu_ingame_tabs.h"
#include "globals.h"
#include "bflib_basics.h"

#include "bflib_guibtns.h"
#include "bflib_sprite.h"
#include "bflib_sprfnt.h"
#include "bflib_vidraw.h"

#include "gui_frontbtns.h"
#include "gui_draw.h"
#include "frontend.h"
#include "frontmenu_saves.h"
#include "config_settings.h"
#include "frontmenu_options.h"
#include "game_legacy.h"
#include "sprites.h"
#include "post_inc.h"

#ifdef __cplusplus
extern "C" {
#endif
/******************************************************************************/
void gui_previous_battle(struct GuiButton *gbtn);
void gui_next_battle(struct GuiButton *gbtn);
void gui_get_creature_in_battle(struct GuiButton *gbtn);
void gui_go_to_person_in_battle(struct GuiButton *gbtn);
void gui_setup_friend_over(struct GuiButton *gbtn);
void gui_area_friendly_battlers(struct GuiButton *gbtn);
void gui_setup_enemy_over(struct GuiButton *gbtn);
void gui_area_enemy_battlers(struct GuiButton *gbtn);
/******************************************************************************/
struct GuiButtonInit text_info_buttons[] = {
  { 0, 0,                   0, 0, NULL,               NULL,        NULL,               0, 999,   4, 999,   4,400, 78, gui_area_scroll_window,            0, GUIStr_Empty,        0,{.ptr = &game.evntbox_scroll_window},0,NULL },
  { 1, BID_EVENT_ZOOM,      0, 0, gui_go_to_event,    NULL,        NULL,               0,   4,   4,   4,   4, 30, 24, gui_area_new_normal_button, GPS_message_message_btn_show_act, GUIStr_ZoomToArea,   0,       {0},            0, maintain_zoom_to_event },
  { 0, BID_OBJ_CLOSE,       0, 1, gui_close_objective,gui_close_objective,NULL,        0,   4,  56,   4,  56, 30, 24, gui_area_new_normal_button, GPS_message_message_btn_accept_act, GUIStr_CloseWindow,  0,       {0},            0, NULL },
  { 1, BID_OBJ_SCRL_UP,     0, 0, gui_scroll_text_up, NULL,        NULL,               0, 446,   4, 446,   4, 30, 24, gui_area_new_normal_button, GPS_message_message_btn_up_act, GUIStr_CtrlUp,       0,{.ptr = &game.evntbox_scroll_window},0,maintain_scroll_up },
  { 1, BID_OBJ_SCRL_DWN,    0, 0, gui_scroll_text_down,NULL,       NULL,               0, 446,  56, 446,  56, 30, 24, gui_area_new_normal_button, GPS_message_message_btn_down_act, GUIStr_CtrlDown,     0,{.ptr = &game.evntbox_scroll_window},0,maintain_scroll_down },
  {-1, 0,                   0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0, NULL,                              0,   0,                 0,       {0},            0, NULL },
};

struct GuiButtonInit battle_buttons[] = {
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 1, gui_close_objective,NULL,        NULL,               0,   4,  72,   4,  72, 30, 24,                gui_area_new_normal_button, GPS_message_message_btn_accept_act, GUIStr_CloseWindow,  0,       {0},            0, NULL },
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, gui_previous_battle,NULL,        NULL,               0, 446,   4, 446,   4, 30, 24,                gui_area_new_normal_button, GPS_message_message_btn_up_act, GUIStr_KeyUp,        0,       {0},            0, NULL },
  { LbBtnT_HoldableBtn,BID_DEFAULT, 0, 0, gui_next_battle    ,NULL,        NULL,               0, 446,  72, 446,  72, 30, 24,                gui_area_new_normal_button, GPS_message_message_btn_down_act, GUIStr_KeyDown,      0,       {0},            0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, gui_get_creature_in_battle,gui_go_to_person_in_battle,gui_setup_friend_over,0, 42,12, 42,12,160,24,gui_area_friendly_battlers,  0,GUIStr_Empty,        0,       {0},            0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, gui_get_creature_in_battle,gui_go_to_person_in_battle,gui_setup_enemy_over, 0,260,12,260,12,160,24,gui_area_enemy_battlers,     0,GUIStr_Empty,        0,       {0},            0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, gui_get_creature_in_battle,gui_go_to_person_in_battle,gui_setup_friend_over,1, 42,42, 42,42,160,24,gui_area_friendly_battlers,  0,GUIStr_Empty,        0,       {0},            0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, gui_get_creature_in_battle,gui_go_to_person_in_battle,gui_setup_enemy_over, 1,260,42,260,42,160,24,gui_area_enemy_battlers,     0,GUIStr_Empty,        0,       {0},            0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, gui_get_creature_in_battle,gui_go_to_person_in_battle,gui_setup_friend_over,2, 42,72, 42,72,160,24,gui_area_friendly_battlers,  0,GUIStr_Empty,        0,       {0},            0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, gui_get_creature_in_battle,gui_go_to_person_in_battle,gui_setup_enemy_over, 2,260,72,260,72,160,24,gui_area_enemy_battlers,     0,GUIStr_Empty,        0,       {0},            0, NULL },
  { LbBtnT_NormalBtn,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0, 214,  34, 214,  34, 32, 32,                gui_area_null, GBS_guisymbols_sym_fight,GUIStr_Empty,        0,       {0},            0, NULL },
  {-1,  BID_DEFAULT, 0, 0, NULL,               NULL,        NULL,               0,   0,   0,   0,   0,  0,  0,                NULL,                        0,   0,                0,       {0},            0, NULL },
};


struct GuiMenu text_info_menu =
// { 16, 0, 4, text_info_buttons,                        160, 316, 480,  86, gui_round_glass_background,  0, NULL,    reset_scroll_window,     0, 0, 0,};
 { GMnu_TEXT_INFO,   0, 4, text_info_buttons,                  160, POS_SCRBTM,480,  86, gui_round_glass_background,  0, NULL,    reset_scroll_window,     0, 0, 0,};
struct GuiMenu battle_menu =
// { 34, 0, 4, battle_buttons,                    160,        300, 480, 102, gui_round_glass_background,  0, NULL,    NULL,                    0, 0, 0,};
 { GMnu_BATTLE,      0, 4, battle_buttons,                    160, POS_SCRBTM, 480, 102, gui_round_glass_background,  0, NULL,    NULL,                    0, 0, 0,};

/******************************************************************************/
#ifdef __cplusplus
}
#endif
/******************************************************************************/
