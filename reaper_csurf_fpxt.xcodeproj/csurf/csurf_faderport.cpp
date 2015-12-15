/*
** reaper_csurf
** FaderPort support
** Copyright (C) 2007-2008 Cockos Incorporated
** License: LGPL.
*/


#include "csurf.h"
#include "reaper_plugin_functions.h"

#include "MacCompatibility.h"
#include <Windows.h> //Just be sure this stays a capital 'W' for mac -- nimaj
#include <stdio.h> // ini support (karbo 11.8.2011)

/*
** Todo: automation status, automation mode setting using "auto" button, more
*/


static bool g_csurf_mcpmode=false; // we may wish to allow an action to set this


// INI support (karbo 11.8.2011 )
#define BUFSIZE MAX_PATH

// settings
bool g_selected_is_touched=false ;
bool g_shift_latch=true ;
bool g_auto_scroll=true;
bool g_override_automation_read;
bool g_pan_scroll_tracks;

// button Actions
int g_action_footswitch;
int g_action_footswitch_shift;

int g_action_output;
int g_action_output_shift;

int g_action_mix;
int g_action_mix_shift;

int g_action_project;
int g_action_project_shift;

int g_action_trans;
int g_action_trans_shift;

int g_action_undo = IDC_EDIT_UNDO;
int g_action_undo_shift = IDC_EDIT_REDO;

int g_action_bank_base;

int g_action_punch=ID_MARKER_PREV;
int g_action_punch_shift=ID_LOOP_SETSTART;

int g_action_loop=IDC_REPEAT;
int g_action_loop_shift=ID_INSERT_MARKER;

int g_action_user=ID_MARKER_NEXT;
int g_action_user_shift=ID_LOOP_SETEND;

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

struct FaderPortAction
{
    FaderPortAction(MIDI_event_t* evt)
    {
        device = (FaderPortDevice)evt->midi_message[0];
        id = (FaderPortButton)evt->midi_message[1];
        state = evt->midi_message[2];;
    }
    
    FaderPortDevice device;
    FaderPortButton id;
    int state;
};

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
    
  int m_vol_lastpos;
  int m_flipmode;
  int m_faderport_lasthw,m_faderport_buttonstates;
  
  char m_fader_touchstate;
  int m_bank_offset;

  DWORD m_frameupd_lastrun;
  DWORD m_buttonstate_lastrun;
  DWORD m_pan_lasttouch;

  WDL_String descspace;
  char configtmp[1024];
    
    void ReadINIfile()
    {
        char *INIFileName=new char[1024];
#ifdef __APPLE__
        /*
         In default case this would be "/Users/username/Library/Application Support/REAPER"
         I do it this way because it could cause permissions issues trying to copy it to the app package
        */
        sprintf(INIFileName,"%s/reaper_csurf_fpxt.ini",GetResourcePath());
#else
        sprintf(INIFileName,"%s\\Plugins\\reaper_csurf_fpxt.ini",GetExePath());
#endif
        OutputDebugString(INIFileName);
        char *resultString=new char[512];
        
        // shift_latch: default=true
        GetPrivateProfileString("FPCSURF","SHIFT_LATCH","1",resultString,512,INIFileName);
        int x=atoi(resultString);
        if (x==0)
            g_shift_latch=false; else g_shift_latch=true;
        
        // mcp_view; default = false
        GetPrivateProfileString("FPCSURF","MCP_VIEW","0",resultString,512,INIFileName);
        x=atoi(resultString);
        if (x==0)
            g_csurf_mcpmode=false; else g_csurf_mcpmode=true;
        
        // auto_scroll: default = true
        GetPrivateProfileString("FPCSURF","AUTO_SCROLL","1",resultString,512,INIFileName);
        x=atoi(resultString);
        if (x==0)
            g_auto_scroll=false; else g_auto_scroll=true;
        
        // punch: default = ??
        GetPrivateProfileString("FPCSURF","ACTION_PUNCH","40222",resultString,512,INIFileName);
        g_action_punch=atoi(resultString);
        
        // punch_shift: default = ??
        GetPrivateProfileString("FPCSURF","ACTION_PUNCH_SHIFT","40172",resultString,512,INIFileName);
        g_action_punch_shift=atoi(resultString);
        
        // user: default = ??
        GetPrivateProfileString("FPCSURF","ACTION_USER","40223",resultString,512,INIFileName);
        g_action_user=atoi(resultString);
        
        // user_shift: default = ??
        GetPrivateProfileString("FPCSURF","ACTION_USER_SHIFT","40173",resultString,512,INIFileName);
        g_action_user_shift=atoi(resultString);
        
        // footswitch: default = play/pause
        GetPrivateProfileString("FPCSURF","ACTION_FOOTSWITCH","40073",resultString,512,INIFileName);
        g_action_footswitch=atoi(resultString);
        
        // footswitch_shift: default = play/pause
        GetPrivateProfileString("FPCSURF","ACTION_FOOTSWITCH_SHIFT","1013",resultString,512,INIFileName);
        g_action_footswitch_shift=atoi(resultString);
        
        // mix: default = show/hide mixer
        GetPrivateProfileString("FPCSURF","ACTION_MIX","40078",resultString,512,INIFileName);
        g_action_mix=atoi(resultString);
        
        // mix_shift: default = show/hide mixer
        GetPrivateProfileString("FPCSURF","ACTION_MIX_SHIFT","40078",resultString,512,INIFileName);
        g_action_mix_shift=atoi(resultString);
        
        // project: default = -->
        GetPrivateProfileString("FPCSURF","ACTION_PROJECT","40861",resultString,512,INIFileName);
        g_action_project=atoi(resultString);
        
        // project_shift: default = <--
        GetPrivateProfileString("FPCSURF","ACTION_PROJECT_SHIFT","40862",resultString,512,INIFileName);
        g_action_project_shift=atoi(resultString);
        
        // trans: default = show/hide transport
        GetPrivateProfileString("FPCSURF","ACTION_TRANS","40259",resultString,512,INIFileName);
        g_action_trans=atoi(resultString);
        
        // project shift: ditto
        GetPrivateProfileString("FPCSURF","ACTION_TRANS_SHIFT","40259",resultString,512,INIFileName);
        g_action_trans_shift=atoi(resultString);
        
        // output
        GetPrivateProfileString("FPCSURF","ACTION_OUTPUT","0",resultString,512,INIFileName);
        g_action_output=atoi(resultString);
        
        // output shift
        GetPrivateProfileString("FPCSURF","ACTION_OUTPUT_SHIFT","40917",resultString,512,INIFileName);
        g_action_output_shift=atoi(resultString);
        
        // loop: default ??
        GetPrivateProfileString("FPCSURF","ACTION_LOOP","1068",resultString,512,INIFileName);
        g_action_loop=atoi(resultString);
        
        // loop_shift: default ??
        GetPrivateProfileString("FPCSURF","ACTION_LOOP_SHIFT","40157",resultString,512,INIFileName);
        g_action_loop_shift=atoi(resultString);
        
        // pan scrolls tracks: default 0
        GetPrivateProfileString("FPCSURF","PAN_SCROLLS_TRACKS","0",resultString,512,INIFileName);
        g_pan_scroll_tracks=atoi(resultString);
        
        //ignore mix/proj/trns buttons: default = false
        GetPrivateProfileString("FPCSURF","MTP_OVERRIDE","0",resultString,512,INIFileName);
        x=atoi(resultString);
        if (x==0)
            g_override_automation_read=false; else g_override_automation_read=true;
        
        // select_is_touched: default = false
        GetPrivateProfileString("FPCSURF","SELECT_IS_TOUCHED","0",resultString,512,INIFileName);
        x=atoi(resultString);
        if (x==0)
            g_selected_is_touched=false; else g_selected_is_touched=true;
        
        delete[] INIFileName;
        delete[] resultString;
        
        // debug output
        char buf[200];
        sprintf(buf,"MCP VIEW = %d",g_csurf_mcpmode);OutputDebugString(buf);
        sprintf(buf,"SHIFT LATCH = %d",g_shift_latch);OutputDebugString(buf);
        sprintf(buf,"AUTO SCROLL = %d",g_shift_latch);OutputDebugString(buf);
        sprintf(buf,"USER ACTION = %d",g_action_user);OutputDebugString(buf);
        sprintf(buf,"SHIFT/USER ACTION = %d",g_action_user_shift);OutputDebugString(buf);
        sprintf(buf,"PUNCH ACTION = %d",g_action_punch);OutputDebugString(buf);
        sprintf(buf,"SHIFT/PUNCH ACTION = %d",g_action_punch_shift);OutputDebugString(buf);
        sprintf(buf,"LOOP ACTION = %d",g_action_loop);OutputDebugString(buf);
        sprintf(buf,"SHIFT/LOOP ACTION = %d",g_action_loop_shift);OutputDebugString(buf);
        sprintf(buf,"FIRST SELECTED IS LAST TOUCHED = %d",g_selected_is_touched);OutputDebugString(buf);
        sprintf(buf,"TRANS = %d",g_action_trans);OutputDebugString(buf);
        sprintf(buf,"TRANS SHIFT = %d",g_action_trans_shift);OutputDebugString(buf);
        sprintf(buf,"PROJECT = %d",g_action_project);OutputDebugString(buf);
        sprintf(buf,"PROJECT SHIFT = %d",g_action_project_shift);OutputDebugString(buf);
        sprintf(buf,"MIX = %d",g_action_mix);OutputDebugString(buf);
        sprintf(buf,"MIX SHIFT = %d",g_action_mix_shift);OutputDebugString(buf);
    }
    
    void AdjustBankOffset(int amt, bool dosel)
    {
        if (!amt) return;
        if (amt<0)
        {
            if (m_bank_offset>0)
            {
                m_bank_offset += amt;
                if (m_bank_offset<0) m_bank_offset=0;
                
                if (dosel)
                {
                    int x;
                    MediaTrack *t=CSurf_TrackFromID(m_bank_offset,g_csurf_mcpmode);
                    for (x = 0; ; x ++)
                    {
                        int f=0;
                        if (!GetTrackInfo(x-1,&f)) break;
                        
                        MediaTrack *tt=CSurf_TrackFromID(x,false);
                        bool sel=tt == t;
                        if (tt && !(f&2) == sel)
                        {
                            
                            SetTrackSelected(tt,sel);
                            if (g_auto_scroll) CSurf_SetSurfaceSelected(tt,CSurf_OnSelectedChange(tt,sel),NULL);
                        }
                    }
                }
            }
        }
        else
        {
            int msize=CSurf_NumTracks(g_csurf_mcpmode);
            
            if (m_bank_offset<msize)
            {
                m_bank_offset += amt;
                if (m_bank_offset>msize) m_bank_offset=msize;
                
                if (dosel)
                {
                    int x;
                    MediaTrack *t=CSurf_TrackFromID(m_bank_offset,g_csurf_mcpmode);
                    for (x = 0; ; x ++)
                    {
                        int f=0;
                        if (!GetTrackInfo(x-1,&f)) break;
                        
                        MediaTrack *tt=CSurf_TrackFromID(x,false);
                        bool sel=tt == t;
                        if (tt && !(f&2) == sel)
                        {
                            
                            SetTrackSelected(tt,sel);
                            if (g_auto_scroll) CSurf_SetSurfaceSelected(tt,CSurf_OnSelectedChange(tt,sel),NULL);
                        }
                    }
                }
            }
        }
        
    }
    
    void ProcessFader(FaderPortAction* action)
    {
        if (action->id==0) m_faderport_lasthw=(int)action->state;
        else if (action->id == 0x20)
        {
            int tid=m_bank_offset;
            MediaTrack *tr=CSurf_TrackFromID(m_bank_offset,g_csurf_mcpmode);
            
            if (!tr) return;
            if (m_flipmode)
                CSurf_SetSurfacePan(tr,CSurf_OnPanChange(tr,int14ToPan(m_faderport_lasthw,action->state),false),NULL);
            else
                CSurf_SetSurfaceVolume(tr,CSurf_OnVolumeChange(tr,int14ToVol(m_faderport_lasthw,action->state),false),NULL);
            
        }
    }
    
    void ProcessPan(FaderPortAction* action)
    {
        if (action->id==0) //Not actually sure what this means, but it was there from the beginning
        {
            m_pan_lasttouch=timeGetTime();
            
            double adj=0.05;
            if (action->state>0x3f) adj=-adj;
            MediaTrack *tr=CSurf_TrackFromID(m_bank_offset,g_csurf_mcpmode);
            
            if (!tr) return;
            
            if (m_flipmode)
                CSurf_SetSurfaceVolume(tr,CSurf_OnVolumeChange(tr,adj*11.0,true),NULL);
            else
                CSurf_SetSurfacePan(tr,CSurf_OnPanChange(tr,adj,true),NULL);
            
        }
    }
    
    void ProcessButtonUp(FaderPortAction* action)
    {
        switch(action->id)
        {
            case FPB_FADER_TOUCH:
            {
                m_fader_touchstate=!!action->state;
                break;
            }
            case FPB_SHIFT:
            {
                if(!g_shift_latch)//Turn it off if we're not latching
                {
                    m_faderport_shift = !m_faderport_shift;
                    if (m_midiout) m_midiout->Send(0xa0,5,m_faderport_shift,-1);
                }
                break;
            }
            case FPB_FWD:
            {
                m_faderport_fwd = false;
                if (m_midiout) m_midiout->Send(0xa0,3,action->state,-1);
                break;
            }
            case FPB_REW:
            {
                m_faderport_rew = false;
                if (m_midiout) m_midiout->Send(0xa0,4,action->state,-1);
                break;
            }
            case FPB_CH_NEXT:
            {
                if (m_midiout) m_midiout->Send(0xa0,0x12,action->state,-1);
                break;
            }
            case FPB_CH_PREV:
            {
                if (m_midiout) m_midiout->Send(0xa0,0x14,action->state,-1);
                break;
            }
        }
    }
    void ProcessButtonDown(FaderPortAction* action)
    {
        //Do this outside of the switch so we can omit redundant checks to action->state
       
        
        switch (action->id)
        {
            case FPB_FADER_TOUCH:
            {
                m_fader_touchstate=!!action->state;
                break;
            }
            case FPB_SHIFT:
            {
                m_faderport_shift = !m_faderport_shift;
                if (m_midiout) m_midiout->Send(0xa0,5,m_faderport_shift,-1);
                break;
            }
                
            case FPB_BANK:
            {
                m_faderport_bank = !m_faderport_bank;
                if (m_midiout) m_midiout->Send(0xa0,0x13,m_faderport_bank,-1);
                break;
            }
                
            case FPB_FWD:
            {
                if ((m_faderport_shift)) CSurf_GoEnd();
                else m_faderport_fwd = true;
                if (m_midiout) m_midiout->Send(0xa0,3,action->state,-1);
                break;
            }
                
            case FPB_REW:
            {
                if ((m_faderport_shift)) CSurf_GoStart();
                else m_faderport_rew = true;
                if (m_midiout) m_midiout->Send(0xa0,4,action->state,-1);
                break;
            }
                
            case FPB_REC_ARM:
            {
                if (m_faderport_shift)
                    ClearAllRecArmed();
                else
                {
                    MediaTrack *tr=CSurf_TrackFromID(m_bank_offset,g_csurf_mcpmode);
                    if (tr) SetSurfaceRecArm(tr,CSurf_OnRecArmChange(tr,-1));
                }
                break;
            }
                
            case FPB_ATM_OFF:
            {
                MediaTrack *tr=CSurf_TrackFromID(m_bank_offset,g_csurf_mcpmode);
                if (tr)
                {
                    SetTrackAutomationMode(tr,0);
                    CSurf_SetAutoMode(-1,NULL);
                }
                break;
            }
                
            case FPB_ATM_READ:
            {
                MediaTrack *tr=CSurf_TrackFromID(m_bank_offset,g_csurf_mcpmode);
                if (tr)
                {
                    SetTrackAutomationMode(tr,1);
                    CSurf_SetAutoMode(-1,NULL);
                }
                break;
            }
                
            case FPB_ATM_WRITE:
            {
                MediaTrack *tr=CSurf_TrackFromID(m_bank_offset,g_csurf_mcpmode);
                if (tr)
                {
                    SetTrackAutomationMode(tr,3);
                    CSurf_SetAutoMode(-1,NULL);
                }
                break;
            }
                
            case FPB_ATM_TOUCH:
            {
                MediaTrack *tr=CSurf_TrackFromID(m_bank_offset,g_csurf_mcpmode);
                
                if (tr)
                {
                    SetTrackAutomationMode(tr,2);
                    CSurf_SetAutoMode(-1,NULL);
                }
                break;
            }
                
           
                
            case FPB_STOP:
            {
                CSurf_OnStop();
                break;
            }
                
            case FPB_PUNCH:
            {
                if ((m_faderport_shift))
                    SendMessage(g_hwnd,WM_COMMAND,g_action_punch_shift,0);// karbo: Add punch + default condition
                else
                    SendMessage(g_hwnd,WM_COMMAND,g_action_punch,0);
                break;
            }
                
            case FPB_USER:
            {
                if ((m_faderport_shift)) SendMessage(g_hwnd,WM_COMMAND,g_action_user_shift,0);
                else SendMessage(g_hwnd,WM_COMMAND,g_action_user,0);
                break;
            }
                
            case FPB_LOOP:
            {
                if ((m_faderport_shift))
                    SendMessage(g_hwnd,WM_COMMAND,g_action_loop_shift,0);
                else
                    SendMessage(g_hwnd,WM_COMMAND,g_action_loop,0);
                break;
            }
                
            case FPB_PLAY:
            {
                CSurf_OnPlay();
                break;
            }
                
            case FPB_REC:
            {
                CSurf_OnRecord();
                break;
            }
                
            case FPB_UNDO:
            {
                SendMessage(g_hwnd,WM_COMMAND,(m_faderport_shift)? g_action_undo_shift : g_action_undo,0);
                break;
            }
                
            case FPB_OUTPUT:
            {
    
                if(!g_pan_scroll_tracks && g_action_output == 0)
                {
                    m_flipmode=!m_flipmode;
                    if (m_midiout) m_midiout->Send(0xa0, 0x11,m_flipmode?1:0,-1);
                    CSurf_ResetAllCachedVolPanStates();
                    TrackList_UpdateAllExternalSurfaces();
                }else
                {
                    // if (m_midiout) m_midiout->Send(0xa0, 0x11,m_outFlip?1:0,-1); // light on/off
                    // m_outFlip=!m_outFlip;
                    if (m_midiout) m_midiout->Send(0xa0, 0x11,1,-1); // light on
                    
                    if(g_action_output == 1) // kw: special case for Master track selection via output button (need to refactor redundant code).
                    {
                        
                        int numTracks = CSurf_NumTracks(g_csurf_mcpmode);
                        MediaTrack *tr;
                        for(int i=0;i<numTracks;i++)
                        {
                            // deselect any currently selected tracks
                            tr=CSurf_TrackFromID(i+1,g_csurf_mcpmode);
                            CSurf_OnSelectedChange(tr, false);
                        }
                        
                        // if shift is used, run shift action
                        tr=CSurf_TrackFromID(0, false);
                        if ((m_faderport_buttonstates&2))
                        {
                            SendMessage(g_hwnd,WM_COMMAND,g_action_output_shift,0);
                        }
                        else
                        {
                            // get master and select it
                            CSurf_OnSelectedChange(tr, true);
                            CSurf_OnTrackSelection(tr);
                        }
                    }
                    else if(g_action_output == 2)
                    {
                        MediaTrack *tr;
                        tr=CSurf_TrackFromID(0, false);
                        
                        // if shift is used, run shift action 
                        if ((m_faderport_buttonstates&2))
                        {
                            SendMessage(g_hwnd,WM_COMMAND,g_action_output_shift,0);
                        }
                        else
                        {
                            // get master and select it but don't focus
                            CSurf_OnTrackSelection(tr);
                        }
                    }
                    else
                    {
                        // Run actions instead selecting master output
                        if ((m_faderport_buttonstates&2))
                        {
                            SendMessage(g_hwnd,WM_COMMAND,g_action_output_shift,0);
                        }
                        else
                        {
                            SendMessage(g_hwnd,WM_COMMAND,g_action_output,0);
                        }
                        
                    }
                }
                break;
            }
                
            case FPB_SOLO:
            {
                if (m_faderport_shift) SoloAllTracks(false);
                else
                {
                    MediaTrack *tr=CSurf_TrackFromID(m_bank_offset,g_csurf_mcpmode);
                    if (tr) SetSurfaceSolo(tr,CSurf_OnSoloChange(tr,-1));
                }
                break;
            }
                
            case FPB_CH_PREV:
            {
                AdjustBankOffset((m_faderport_bank)?-8:-1,true);
                TrackList_UpdateAllExternalSurfaces();
                if (m_midiout) m_midiout->Send(0xa0,0x14,action->state,-1);
                break;
            }
                
            case FPB_CH_NEXT:
            {
                AdjustBankOffset((m_faderport_bank)?8:1,true);
                TrackList_UpdateAllExternalSurfaces();
                if (m_midiout) m_midiout->Send(0xa0,0x12,action->state,-1);
                break;
            }
                
            case FPB_MUTE:
            {
                if (m_faderport_shift) MuteAllTracks(false);
                else
                {
                    MediaTrack *tr=CSurf_TrackFromID(m_bank_offset,g_csurf_mcpmode);
                    if (tr) SetSurfaceMute(tr,CSurf_OnMuteChange(tr,-1));
                }
                break;
            }
                
            case FPB_MIX:
            {
                if(g_override_automation_read)
                {
                    int a=(action->id-0xb)+8;
                    if (m_faderport_buttonstates&2) a+=3;
                    MIDI_event_t evt={0,3,{0xbf,static_cast<unsigned char>(a),0}};//clang hates implicit casts --nimaj
                    kbd_OnMidiEvent(&evt,-1);
                    break;
                }
                
                if(m_faderport_shift)  SendMessage(g_hwnd,WM_COMMAND,g_action_mix_shift,0);
                else SendMessage(g_hwnd,WM_COMMAND,g_action_mix,0);
                break;
            }
                
            case FPB_PROJ:
            {
                if(g_override_automation_read)
                {
                    int a=(action->id-0xb)+8;
                    if (m_faderport_buttonstates&2) a+=3;
                    MIDI_event_t evt={0,3,{0xbf,static_cast<unsigned char>(a),0}};//clang hates implicit casts --nimaj
                    kbd_OnMidiEvent(&evt,-1);
                    break;
                }
                if (m_faderport_shift) SendMessage(g_hwnd,WM_COMMAND,g_action_project_shift,0);
                else SendMessage(g_hwnd,WM_COMMAND,g_action_project,0);
                break;
            }
                
            case FPB_TRNS:
            {
                if(g_override_automation_read)
                {
                    int a=(action->id-0xb)+8;
                    if (m_faderport_buttonstates&2) a+=3;
                    MIDI_event_t evt={0,3,{0xbf,static_cast<unsigned char>(a),0}};//clang hates implicit casts --nimaj
                    kbd_OnMidiEvent(&evt,-1);
                    break;
                }
                if (m_faderport_shift) SendMessage(g_hwnd,WM_COMMAND,g_action_trans_shift,0);
                else SendMessage(g_hwnd,WM_COMMAND,g_action_trans,0);
                break;
            }
                
            case FPB_FOOTSWITCH:
            {
                if (m_faderport_shift) SendMessage(g_hwnd,WM_COMMAND,g_action_footswitch_shift,0);
                else  SendMessage(g_hwnd,WM_COMMAND,g_action_footswitch,0);
                break;
            }
        }
    }
    
  void OnMIDIEvent(MIDI_event_t *evt)
  {
      FaderPortAction action(evt);
      if(action.device == FPD_FADER) ProcessFader(&action);
      else if(action.device == FPD_PAN_KNOB) ProcessPan(&action);
      else if(action.device == FPD_BUTTONS) action.state == 1 ? ProcessButtonDown(&action) : ProcessButtonUp(&action);
  }



public:
  CSurf_FaderPort(int indev, int outdev, int *errStats)
  {
    m_midi_in_dev=indev;
    m_midi_out_dev=outdev;
  

    m_faderport_lasthw=0;
    m_faderport_buttonstates=0;
    m_flipmode=0;
    m_vol_lastpos=-1000;

    m_fader_touchstate=0;

    m_bank_offset=0;
    m_frameupd_lastrun=0;
    m_buttonstate_lastrun=0;
    m_pan_lasttouch=0;
      
      m_faderport_bank = false;
      m_faderport_shift = false;
      m_faderport_fwd = false;
      m_faderport_rew = false;

    //create midi hardware access
    m_midiin = m_midi_in_dev >= 0 ? CreateMIDIInput(m_midi_in_dev) : NULL;
    m_midiout = m_midi_out_dev >= 0 ? CreateThreadedMIDIOutput(CreateMIDIOutput(m_midi_out_dev,false,NULL)) : NULL;

      // load ini  (karbo)
      ReadINIfile();
      
    if (errStats)
    {
      if (m_midi_in_dev >=0  && !m_midiin) *errStats|=1;
      if (m_midi_out_dev >=0  && !m_midiout) *errStats|=2;
    }

    if (m_midiin)
      m_midiin->start();

    if (m_midiout)
    {
      m_midiout->Send(0xb0,0x00,0x06,-1);
      m_midiout->Send(0xb0,0x20,0x27,-1);

      int x;
      for (x = 0; x < 0x30; x ++) // lights out
        m_midiout->Send(0xa0,x,0x00,-1);

      //m_midiout->Send(0xa0,0xf,m_flipmode?1:0);// fucko

      m_midiout->Send(0x91,0x00,0x64,-1); // send these every so often? 
    }

  }
  ~CSurf_FaderPort()
  {
    if (m_midiout)
    {
      int x;
      for (x = 0; x < 0x30; x ++) // lights out
        m_midiout->Send(0xa0,x,0x00,-1);
      Sleep(5);    
    }

    delete m_midiout;
    delete m_midiin;
  }

    const char *GetTypeString() { return "FADERPORT"; }
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

  void CloseNoReset() 
  { 
    delete m_midiout;
    delete m_midiin;
    m_midiout=0;
    m_midiin=0;
  }

  void Run()
  {
    if (m_midiin)
    {
      m_midiin->SwapBufs(timeGetTime());
      int l=0;
      MIDI_eventlist *list=m_midiin->GetReadBuf();
      MIDI_event_t *evts;
      while ((evts=list->EnumItems(&l))) OnMIDIEvent(evts);

        
        
      if (m_faderport_rew || m_faderport_fwd)
      {
        DWORD now=timeGetTime();
        if (now >= m_buttonstate_lastrun + 100)
        {
          m_buttonstate_lastrun=now;
        if(m_faderport_fwd) CSurf_OnFwd(1);
        if(m_faderport_rew) CSurf_OnRew(1);

        }
      }
    }
  }

  void SetTrackListChange() 
  { 
    SetAutoMode(0);
  } 

#define FIXID(id) int id=CSurf_TrackToID(trackid,g_csurf_mcpmode); int oid=id; id -= m_bank_offset;

  void SetSurfaceVolume(MediaTrack *trackid, double volume) 
  {
    FIXID(id)
    if (m_midiout && !id && !m_flipmode)
    {
      int volint=volToInt14(volume);
      volint/=16;

      if (m_vol_lastpos!=volint)
      {
        m_vol_lastpos=volint;
        m_midiout->Send(0xb0,0x00,volint>>7,-1);
        m_midiout->Send(0xb0,0x20,volint&127,-1);
        
      }
    }
  }
  void SetSurfacePan(MediaTrack *trackid, double pan) 
  {
    FIXID(id)
    if (m_midiout && !id && m_flipmode)
    {
      int volint=panToInt14(pan);
        volint/=16;

      if (m_vol_lastpos!=volint)
      {
        m_vol_lastpos=volint;
        m_midiout->Send(0xb0,0x00,volint>>7,-1);
        m_midiout->Send(0xb0,0x20,volint&127,-1);     
      }
    }
  }
  void SetSurfaceMute(MediaTrack *trackid, bool mute) 
  { 
    FIXID(id)
    if (!id && m_midiout)
      m_midiout->Send(0xa0,0x15,mute?1:0,-1);
  }
  void SetSurfaceSelected(MediaTrack *trackid, bool selected) 
  {
      // kw: turn output light on/off
      if(g_action_output==1 || g_action_output==0)
      {
          if(m_bank_offset==0)
          {
              if (m_midiout) m_midiout->Send(0xa0, 0x11,1,-1);
          }
          else
          {
              if (m_midiout) m_midiout->Send(0xa0, 0x11,0,-1);
          }
      }
  }
  void SetSurfaceSolo(MediaTrack *trackid, bool solo) 
  { 
    FIXID(id)
    if (!id && m_midiout)
      m_midiout->Send(0xa0,0x16,solo?1:0,-1);
  }
  void SetSurfaceRecArm(MediaTrack *trackid, bool recarm) 
  { 
    FIXID(id)
    if (!id && m_midiout)
      m_midiout->Send(0xa0,0x17,recarm?1:0,-1);
  }
  void SetPlayState(bool play, bool pause, bool rec) 
  { 
    if (m_midiout)
    {
      m_midiout->Send(0xa0, 0,rec?1:0,-1);
      m_midiout->Send(0xa0, 1,play?1:0,-1);
      m_midiout->Send(0xa0, 2,(!play&&!pause)?1:0,-1);
    }
  }
  void SetRepeatState(bool rep) 
  { 
    if (m_midiout) m_midiout->Send(0xa0, 8,rep?1:0,-1);

  }

  void SetTrackTitle(MediaTrack *trackid, const char *title) { }

  bool GetTouchState(MediaTrack *trackid, int isPan) 
  { 
    FIXID(id)
    if (!id)
    {
      if (!m_flipmode != !isPan) 
      {
        DWORD now=timeGetTime();
        if (m_pan_lasttouch==1 || (now<m_pan_lasttouch+3000 && now >= m_pan_lasttouch-1000)) // fake touch, go for 3s after last movement
        {
          return true;
        }
        return false;
      }
      return !!m_fader_touchstate;
    }
    return false;
  }

  void SetAutoMode(int mode) 
  { 
    if (m_midiout)
    {
      MediaTrack *tr=CSurf_TrackFromID(m_bank_offset,g_csurf_mcpmode);
      if (tr) mode=GetTrackAutomationMode(tr);

      if (mode<0) return;

      m_midiout->Send(0xa0,0x10,0,-1);//mode==0); // dont set off light, it disables the fader?!
      m_midiout->Send(0xa0,0xf,mode==2||mode==4,-1);
      m_midiout->Send(0xa0,0xe,mode==3,-1);
      m_midiout->Send(0xa0,0xd,mode==1,-1);
    }
  }

  void ResetCachedVolPanStates() 
  {
    m_vol_lastpos=-1000;
  }

  void OnTrackSelection(MediaTrack *trackid) 
  { 
    int newpos=CSurf_TrackToID(trackid,g_csurf_mcpmode);
    if (newpos>=0 && newpos != m_bank_offset)
    {
      AdjustBankOffset(newpos-m_bank_offset,false);
      TrackList_UpdateAllExternalSurfaces();
    }
  }
  
  bool IsKeyDown(int key) 
  { 
    return false; 
  }


};


static void parseParms(const char *str, int parms[4])
{
  parms[0]=0;
  parms[1]=9;
  parms[2]=parms[3]=-1;

  const char *p=str;
  if (p)
  {
    int x=0;
    while (x<4)
    {
      while (*p == ' ') p++;
      if ((*p < '0' || *p > '9') && *p != '-') break;
      parms[x++]=atoi(p);
      while (*p && *p != ' ') p++;
    }
  }  
}

static IReaperControlSurface *createFunc(const char *type_string, const char *configString, int *errStats)
{
  int parms[4];
  parseParms(configString,parms);

  return new CSurf_FaderPort(parms[2],parms[3],errStats);
}


static WDL_DLGRET dlgProc(HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
  switch (uMsg)
  {
    case WM_INITDIALOG:
      {
        int parms[4];
        parseParms((const char *)lParam,parms);

        ShowWindow(GetDlgItem(hwndDlg,IDC_EDIT1),SW_HIDE);
        ShowWindow(GetDlgItem(hwndDlg,IDC_EDIT1_LBL),SW_HIDE);
        ShowWindow(GetDlgItem(hwndDlg,IDC_EDIT2),SW_HIDE);
        ShowWindow(GetDlgItem(hwndDlg,IDC_EDIT2_LBL),SW_HIDE);
        ShowWindow(GetDlgItem(hwndDlg,IDC_EDIT2_LBL2),SW_HIDE);

        int n=GetNumMIDIInputs();
        int x=SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_ADDSTRING,0,(LPARAM)"None");
        SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_SETITEMDATA,x,-1);
        x=SendDlgItemMessage(hwndDlg,IDC_COMBO3,CB_ADDSTRING,0,(LPARAM)"None");
        SendDlgItemMessage(hwndDlg,IDC_COMBO3,CB_SETITEMDATA,x,-1);
        for (x = 0; x < n; x ++)
        {
          char buf[512];
          if (GetMIDIInputName(x,buf,sizeof(buf)))
          {
            int a=SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_ADDSTRING,0,(LPARAM)buf);
            SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_SETITEMDATA,a,x);
            if (x == parms[2]) SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_SETCURSEL,a,0);
          }
        }
        n=GetNumMIDIOutputs();
        for (x = 0; x < n; x ++)
        {
          char buf[512];
          if (GetMIDIOutputName(x,buf,sizeof(buf)))
          {
            int a=SendDlgItemMessage(hwndDlg,IDC_COMBO3,CB_ADDSTRING,0,(LPARAM)buf);
            SendDlgItemMessage(hwndDlg,IDC_COMBO3,CB_SETITEMDATA,a,x);
            if (x == parms[3]) SendDlgItemMessage(hwndDlg,IDC_COMBO3,CB_SETCURSEL,a,0);
          }
        }
      }
    break;
    case WM_USER+1024:
      if (wParam > 1 && lParam)
      {
        char tmp[512];

        int indev=-1, outdev=-1, offs=0, size=9;
        int r=SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_GETCURSEL,0,0);
        if (r != CB_ERR) indev = SendDlgItemMessage(hwndDlg,IDC_COMBO2,CB_GETITEMDATA,r,0);
        r=SendDlgItemMessage(hwndDlg,IDC_COMBO3,CB_GETCURSEL,0,0);
        if (r != CB_ERR)  outdev = SendDlgItemMessage(hwndDlg,IDC_COMBO3,CB_GETITEMDATA,r,0);

        sprintf(tmp,"0 0 %d %d",indev,outdev);
        lstrcpyn((char *)lParam, tmp,wParam);
        
      }
    break;
  }
  return 0;
}

static HWND configFunc(const char *type_string, HWND parent, const char *initConfigString)
{
    return CreateDialogParam(g_hInst,MAKEINTRESOURCE(IDD_SURFACEEDIT_MCU),parent,dlgProc,(LPARAM)initConfigString);
}


reaper_csurf_reg_t csurf_faderport_reg =
{
    "FADERPORT",
    "PreSonus FaderPort XT",
    createFunc,
    configFunc,
    
};
