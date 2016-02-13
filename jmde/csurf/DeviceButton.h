//
//  DeviceButton.h
//  reaper_csurf_fpxt
//
//  Created by Bryce Bias on 2/13/16.
//  Copyright (c) 2016 Phyersoft. All rights reserved.
//

#ifndef __reaper_csurf_fpxt__DeviceButton__
#define __reaper_csurf_fpxt__DeviceButton__

#include <string>

#define IDC_REPEAT                      1068
#define ID_FILE_SAVEAS                  40022
#define ID_FILE_NEWPROJECT              40023
#define ID_FILE_OPENPROJECT             40025
#define ID_FILE_SAVEPROJECT             40026
#define IDC_EDIT_UNDO                   40029
#define IDC_EDIT_REDO                   40030
#define ID_MARKER_PREV                  40172
#define ID_MARKER_NEXT                  40173
#define ID_INSERT_MARKERRGN             40174
#define ID_INSERT_MARKER                40157
#define ID_LOOP_SETSTART                40222
#define ID_LOOP_SETEND                  40223
#define ID_METRONOME                    40364
#define ID_GOTO_MARKER1                 40161
#define ID_SET_MARKER1                  40657

enum FaderPortLights
{
    FPL_INVALID = -1,
    FPL_REC = 0x00,
    FPL_PLAY,
    FPL_STOP,
    FPL_FWD,
    FPL_RWD,
    FPL_SHIFT,
    FPL_PUNCH,
    FPL_USER,
    FPL_LOOP,
    FPL_UNDO,
    FPL_TRNS,
    FPL_PROJ,
    FPL_MIX,
    FPL_READ,
    FPL_WRITE,
    FPL_TOUCH,
    FPL_OFF,
    FPL_OUTPUT,
    FPL_CH_NEXT,
    FPL_BANK,
    FPL_CH_PREV,
    FPL_MUTE,
    FPL_SOLO,
    FPL_REC_ARM,
    FPL_COUNT
};


enum FaderPortButton : uint32_t
{
    FPB_MUTE = 18,
    FPB_SOLO = 17,
    FPB_REC_ARM = 16,
    FPB_CH_PREV = 19,
    FPB_BANK = 20,
    FPB_CH_NEXT = 21,
    FPB_OUTPUT = 22,
    FPB_ATM_READ = 10,
    FPB_ATM_WRITE = 9,
    FPB_ATM_TOUCH = 8,
    FPB_ATM_OFF = 23,
    FPB_MIX = 11,
    FPB_PROJ = 12,
    FPB_TRNS = 13,
    FPB_UNDO = 14,
    FPB_SHIFT = 2,
    FPB_PUNCH = 1,
    FPB_USER = 0,
    FPB_LOOP = 15,
    FPB_REW = 3,
    FPB_FWD = 4,
    FPB_STOP = 5,
    FPB_PLAY = 6,
    FPB_REC = 7,
    FPB_FADER_TOUCH = 127,
    FPB_FOOTSWITCH = 126
};

const uint32_t FPButtonCount = 26;

enum ActionFlags
{
    AF_SHIFT = (1<<0),
    AF_FXMODE = (1<<1),
};

struct DeviceButton
{
    FaderPortButton buttonId;
    FaderPortLights lightId;
    std::string buttonName;
    std::string action;
    std::string action_shift;
    DeviceButton(FaderPortButton button, FaderPortLights lightId, const std::string& name,
                 const std::string& defaultAction = "", const std::string& defaultActionShift = "");
    DeviceButton(){}
};



#endif /* defined(__reaper_csurf_fpxt__DeviceButton__) */
