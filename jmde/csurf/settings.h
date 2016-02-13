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

int g_action_output;

string g_intro_string;

// Pan Actions
string g_action_pan_left;
string g_action_pan_right;
string g_action_pan_left_shift;
string g_action_pan_right_shift;


int g_action_bank_base;

bool g_isMaster;

#endif
