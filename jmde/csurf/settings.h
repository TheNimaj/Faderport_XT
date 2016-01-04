//
//  settings.h
//  reaper_csurf_fpxt
//
//  Created by Bryce Bias on 12/31/15.
//  Copyright (c) 2015 Phyersoft. All rights reserved.
//

#ifndef reaper_csurf_fpxt_settings_h
#define reaper_csurf_fpxt_settings_h

// settings
bool g_selected_is_touched=false ;
bool g_shift_latch=true;
bool g_auto_scroll=true;
bool g_override_automation_read;
bool g_fader_controls_fx=false;
bool g_select_touched_param=false;
bool g_enable_intro=false;

int g_pan_touch_reset_time=500;
int g_pan_scroll_fader_time=250;

int g_pan_min_turns = 3;
int g_pan_resolution;

int g_action_pan;
int g_action_pan_shift;

string g_intro_string;

// button Actions
string g_action_footswitch;
string g_action_footswitch_shift;

string g_action_output;
string g_action_output_shift;

string g_action_mix;
string g_action_mix_shift;

string g_action_project;
string g_action_project_shift;

string g_action_trans;
string g_action_trans_shift;

string g_action_pan_left;
string g_action_pan_right;
string g_action_pan_left_shift;
string g_action_pan_right_shift;

string g_action_undo = to_string(IDC_EDIT_UNDO);
string g_action_undo_shift = to_string(IDC_EDIT_REDO);

int g_action_bank_base;

string g_action_punch=to_string(ID_MARKER_PREV);
string g_action_punch_shift=to_string(ID_LOOP_SETSTART);

string g_action_loop=to_string(IDC_REPEAT);
string g_action_loop_shift=to_string(ID_INSERT_MARKER);

string g_action_user=to_string(ID_MARKER_NEXT);
string g_action_user_shift=to_string(ID_LOOP_SETEND);

bool g_isMaster;

#endif
