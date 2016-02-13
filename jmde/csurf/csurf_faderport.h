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

#include <Windows.h> 
#include <string>
#include <unordered_map>

#include "DeviceButton.h"
#include "automator.h"

using namespace std;
/*
 ** Todo: automation status, automation mode setting using "auto" button, more
 */


static bool g_csurf_mcpmode=false; // we may wish to allow an action to set this


// INI support (karbo 11.8.2011 )
#define BUFSIZE MAX_PATH

enum FaderPortDevice
{
    FPD_FADER = 0xb0,
    FPD_PAN_KNOB = 0xe0,
    FPD_BUTTONS = 0xa0
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


class CSurf_FaderPort : public IReaperControlSurface
{
    int m_midi_in_dev,m_midi_out_dev;
    midi_Output *m_midiout;
    midi_Input *m_midiin;
    
    
    //modifiers
    bool m_faderport_shift;
    bool m_faderport_bank;
    bool m_faderport_fxmode;
    bool m_faderport_fwd;
    bool m_faderport_rew;
    bool m_touch_latch;
    bool m_faderport_pause;
    
    std::unordered_map<uint32_t, DeviceButton> m_ButtonLookup;

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
    
    void ProcessFader(FaderPortAction* action);
    void ProcessPan(FaderPortAction* action);
    void ProcessButtonUp(FaderPortAction* action);
    void ProcessButtonDown(FaderPortAction* action);
    void OnMIDIEvent(MIDI_event_t *evt);
    void ReadINIfile();
    void AdjustBankOffset(int amt, bool dosel);
    
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
    
    int Extended(int call, void *parm1, void *parm2, void *parm3);
    
    void SetTrackTitle(MediaTrack *trackid, const char *title) { }
    
    bool GetTouchState(MediaTrack *trackid, int isPan);
    void SetAutoMode(int mode);
    void ResetCachedVolPanStates();
    void OnTrackSelection(MediaTrack *trackid);
    bool IsKeyDown(int key);
    
public:
    CSurf_FaderPort(int indev, int outdev, int *errStats);
   ~CSurf_FaderPort();
    
    
    void ToggleFXMode();
    void SetFXMode(bool set);
    bool GetFXMode() const;
    
    int GetBankOffset() const { return m_bank_offset; }
    
    void SetLight(FaderPortLights light, bool state);
    void AdjustFader(int val);
    void Notify(unsigned char button);
    
    Envelope_Automator& GetFXDevice() { return m_fxautomation; }
    
    static void RunCommand(const string& cmd);
    
    
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
    
};




#endif
