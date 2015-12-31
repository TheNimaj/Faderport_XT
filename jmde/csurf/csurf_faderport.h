//
//  csurf_faderport.h
//  reaper_csurf
//
//  Created by Bryce Bias on 12/6/15.
//

/*
 ** reaper_csurf
 ** FaderPort support
 ** Copyright (C) 2007-2008 Cockos Incorporated
 ** License: LGPL.
 */


#ifndef reaper_csurf_csurf_faderport_h
#define reaper_csurf_csurf_faderport_h

#include "csurf.h"

#include <Windows.h> //Just be sure this stays a capital 'W' for mac -- nimaj
#include <string>

#include "automator.h"

using namespace std;
/*
 ** Todo: automation status, automation mode setting using "auto" button, more
 */


static bool g_csurf_mcpmode=false; // we may wish to allow an action to set this


// INI support (karbo 11.8.2011 )
#define BUFSIZE MAX_PATH

// settings
bool g_selected_is_touched=false ;
bool g_shift_latch=true;
bool g_auto_scroll=true;
bool g_override_automation_read;
bool g_fader_controls_fx=false;
bool g_select_touched_param=false;

int g_pan_touch_reset_time=500;
int g_pan_scroll_fader_time=250;

int g_pan_min_turns = 3;
int g_pan_resolution;

int g_action_pan;
int g_action_pan_shift;

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

enum FaderPortDevice
{
    FPD_FADER = 0xb0,
    FPD_PAN_KNOB = 0xe0,
    FPD_BUTTONS = 0xa0
};
enum FaderPortButton
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

enum FP_RFlags
{
    FPB_RFLAG1 = (1<<0),
    FPB_RFLAG2 = (1<<1),
    FPB_RFLAG3 = (1<<2),
    FPB_RFLAG_MASK = FPB_RFLAG1 | FPB_RFLAG2 | FPB_RFLAG3
};

enum PanDirection
{
    PD_UNCHANGED,
    PD_LEFT,
    PD_RIGHT
};

struct FaderPortAction
{
    FaderPortAction(MIDI_event_t* evt)
    {
        device = (FaderPortDevice)evt->midi_message[0];
        id = (FaderPortButton)evt->midi_message[1];
        state = evt->midi_message[2];
    }
    
    FaderPortDevice device;
    FaderPortButton id;
    int state;
};

static int paramToint14(double param)
{
    return param / 16.0f * 16383.0;
}

static double int14ToParam(unsigned char msb, unsigned char lsb)
{
    int val = msb * 128 + lsb;
    return val / 16383.0;
}

static double int14ToVol(unsigned char msb, unsigned char lsb)
{
    int val=lsb | (msb<<7);
    double pos=((double)val*1000.0)/16383.0;
    pos=SLIDER2DB(pos);
    return DB2VAL(pos);
}
static double int14ToPan(unsigned char msb, unsigned char lsb)
{
    int val=lsb | (msb<<7);
    return 1.0 - (val/(16383.0*0.5));
}

static int volToInt14(double vol)
{
    double d=(DB2SLIDER(VAL2DB(vol))*16383.0/1000.0);
    if (d<0.0)d=0.0;
    else if (d>16383.0)d=16383.0;
    
    return (int)(d+0.5);
}
static  int panToInt14(double pan)
{
    double d=((1.0-pan)*16383.0*0.5);
    if (d<0.0)d=0.0;
    else if (d>16383.0)d=16383.0;
    
    return (int)(d+0.5);
}



class CSurf_FaderPort : public IReaperControlSurface
{
    int m_midi_in_dev,m_midi_out_dev;
    midi_Output *m_midiout;
    midi_Input *m_midiin;
    
    //modifiers
    bool m_faderport_shift;
    bool m_faderport_bank;
    bool m_faderport_fwd;
    bool m_faderport_rew;
    bool m_faderport_fxmode;
    bool m_touch_latch;

	//Stores flags for shift, mute, and rec (arm)
	unsigned m_faderport_reload;
    
    bool m_fx_waiting;
    bool m_track_waiting;
	PanDirection m_pan_dir;//left = false; true = right
    
    int m_pan_left_turns;
    int m_pan_right_turns;
    int m_fader_val;
    
    Envelope_Automator m_fxautomation;
    
    int m_vol_lastpos;
    int m_flipmode;
    int m_faderport_lasthw;
    
    unsigned char m_fader_touchstate;
    int m_bank_offset;
    
    DWORD m_frameupd_lastrun;
    DWORD m_buttonstate_lastrun;
    DWORD m_pan_lasttouch;
    
    WDL_String descspace;
    char configtmp[1024];
    
protected:
    
    //Action ID string support (nimaj 12.4.2015)
    void RunCommand(const string& cmd);
    void AdjustFader(int val);
    
    void ProcessFader(FaderPortAction* action);
    void ProcessPan(FaderPortAction* action);
    void ProcessButtonUp(FaderPortAction* action);
    void ProcessButtonDown(FaderPortAction* action);
    void OnMIDIEvent(MIDI_event_t *evt);
    void ReadINIfile();
    void AdjustBankOffset(int amt, bool dosel);
    
    void Notify(unsigned char button);
public:
    CSurf_FaderPort(int indev, int outdev, int *errStats);
    ~CSurf_FaderPort();
    
    const char *GetTypeString() { return "FADERPORTXT"; }
    const char *GetDescString()
    {
        descspace.Set("PreSonus FaderPort XT");
        char tmp[512];
        sprintf(tmp," (dev %d,%d)",m_midi_in_dev,m_midi_out_dev);
        descspace.Append(tmp);
        return descspace.Get();
    }
    const char *GetConfigString() // string of configuration data
    {
        sprintf(configtmp,"0 0 %d %d",m_midi_in_dev,m_midi_out_dev);
        return configtmp ;
        
    }
    
    void CloseNoReset();
    
    void Run();
    
    void SetTrackListChange(){ SetAutoMode(0); }
    
    void SetSurfaceVolume(MediaTrack *trackid, double volume);
    void SetSurfacePan(MediaTrack *trackid, double pan);
    void SetSurfaceMute(MediaTrack *trackid, bool mute);
    void SetSurfaceSelected(MediaTrack *trackid, bool selected);
    void SetSurfaceSolo(MediaTrack *trackid, bool solo);
    void SetSurfaceRecArm(MediaTrack *trackid, bool recarm);
    void SetPlayState(bool play, bool pause, bool rec);
    void SetRepeatState(bool rep);
    void SetTrackTitle(MediaTrack *trackid, const char *title) { }
    
    bool GetTouchState(MediaTrack *trackid, int isPan);
    void SetAutoMode(int mode);
    void ResetCachedVolPanStates();
    void OnTrackSelection(MediaTrack *trackid);
    bool IsKeyDown(int key) { return false; }
    
    int Extended(int call, void *parm1, void *parm2, void *parm3);
};




#endif
