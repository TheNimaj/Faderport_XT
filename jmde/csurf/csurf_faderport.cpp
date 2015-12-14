/*
 ** reaper_csurf
 ** FaderPort support
 ** Copyright (C) 2007-2008 Cockos Incorporated
 ** License: LGPL.
 */

#include "csurf_faderport.h"

#include "../reaper_plugin_functions.h"
 
#include "../MacCompatibility.h"
#include <stdio.h> // ini support (karbo 11.8.2011)
#include <sstream>

using namespace std;


CSurf_FaderPort::CSurf_FaderPort(int indev, int outdev, int *errStats)
{
	m_midi_in_dev=indev;
	m_midi_out_dev=outdev;
	
	
	m_faderport_lasthw=0;
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
	m_faderport_fxmode = false;
	m_fx_waiting = false;
	
	m_faderport_reload = 0;
    
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
CSurf_FaderPort::~CSurf_FaderPort()
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

void CSurf_FaderPort::RunCommand(const string& cmd) { Main_OnCommand(NamedCommandLookup(cmd.c_str()),-1); }
void CSurf_FaderPort::AdjustFader(int val)
{
	m_midiout->Send(0xb0,0x00,val>>7,-1);
	m_midiout->Send(0xb0,0x20,val&127,-1);
}

/*
 ** Todo: automation status, automation mode setting using "auto" button, more
 */

void CSurf_FaderPort::ProcessFader(FaderPortAction* action)
{
	stringstream ss;
	ss << action->id << ":" << action->state;
	//OutputDebugString(ss.str().c_str());
	if (action->id==0) m_faderport_lasthw=(int)action->state;
	else if (action->id == 0x20)
	{
		int tid=m_bank_offset;
		MediaTrack *tr=CSurf_TrackFromID(m_bank_offset,g_csurf_mcpmode);
		if (!tr) return;
		
		if(m_faderport_fxmode)
			m_fxautomation.SetParamNormalized(int14ToParam(m_faderport_lasthw, action->state));
		else
		{
			if (m_flipmode)
				CSurf_SetSurfacePan(tr,CSurf_OnPanChange(tr,int14ToPan(m_faderport_lasthw,action->state),false),NULL);
			else
				CSurf_SetSurfaceVolume(tr,CSurf_OnVolumeChange(tr,int14ToVol(m_faderport_lasthw,action->state),false),NULL);
		}
	}
}

void CSurf_FaderPort::ProcessPan(FaderPortAction* action)
{
	if (action->id==0) //Not actually sure what this means, but it was there from the beginning
	{
		m_pan_lasttouch=timeGetTime();
		
		double adj=0.05;
		if (action->state>0x3f) adj=-adj;
		MediaTrack *tr=CSurf_TrackFromID(m_bank_offset,g_csurf_mcpmode);
		
		if (!tr) return;
		
		
		if (m_faderport_fxmode)
		{
			if(action->state == 0x7E)
				m_fxautomation.SelectPrevParam();
			else
				m_fxautomation.SelectNextParam();
			
			m_fx_waiting = true;
		}
		// Pan tracks support: karbo 12.2.2012
		else if(g_pan_scroll_tracks && !m_faderport_shift) //Allow shift to adjust pan? --nimaj 12.4.2015
		{
            
			m_pan_dir = action->state == 0x1 ? true : false;
            
            if (m_pan_dir==false) // scroll prev track
                AdjustBankOffset((m_faderport_bank) ? -8 : -1, true);
            else  // scroll next track
                AdjustBankOffset((m_faderport_bank) ? 8 : 1, true);
            
			m_track_waiting = true;
		}
		else
		{
			if (m_flipmode)
				CSurf_SetSurfaceVolume(tr,CSurf_OnVolumeChange(tr,adj*11.0,true),NULL);
			else
				CSurf_SetSurfacePan(tr,CSurf_OnPanChange(tr,adj,true),NULL);
		}
		
		
	}
}

void CSurf_FaderPort::ProcessButtonUp(FaderPortAction* action)
{
	switch(action->id)
	{
		case FPB_FADER_TOUCH:
		{
			if(g_fader_controls_fx && m_faderport_fxmode)
			{
				bool isTouch = GetTrackAutomationMode(m_fxautomation.GetTrack()) == 2;
				bool isPlaying = GetPlayState() & 5;
				int val = paramToint14(m_fxautomation.EndTouch(isTouch && isPlaying));
				stringstream ss;
				ss << std::boolalpha << isPlaying;
				OutputDebugString(ss.str().c_str());
				if(m_midiout && isTouch && isPlaying) AdjustFader(val);
			}else
			{
				m_fader_touchstate=!!action->state;
			}
			break;
		}
		case FPB_SHIFT:
		{
			m_faderport_reload &= ~FPB_RFLAG1;

			if(!g_shift_latch)//Turn it off if we're not latching
			{
				m_faderport_shift = !m_faderport_shift;
				if (m_midiout) m_midiout->Send(0xa0,5,m_faderport_shift,-1);
			}
			break;
		}

		case FPB_STOP:
		{
			m_faderport_reload &= ~FPB_RFLAG2;
		}
		case FPB_PLAY:
		{
			m_faderport_reload &= ~FPB_RFLAG3;
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
void CSurf_FaderPort::ProcessButtonDown(FaderPortAction* action)
{
	switch (action->id)
	{
		case FPB_FADER_TOUCH:
		{
			if(g_fader_controls_fx && m_faderport_fxmode) m_fxautomation.BeginTouch();
			else m_fader_touchstate=!!action->state;
			break;
		}
		case FPB_SHIFT:
		{
			m_faderport_reload |= FPB_RFLAG1;
			m_faderport_shift = !m_faderport_shift;
			if (m_midiout) m_midiout->Send(0xa0,5,m_faderport_shift,-1);
			break;
		}
			
		case FPB_BANK:
		{
			m_faderport_bank = !m_faderport_bank;
			if(g_fader_controls_fx)
			{
				if(m_faderport_shift) m_faderport_fxmode = !m_faderport_fxmode;
				else m_faderport_fxmode = false;
				
				if(m_faderport_fxmode)
				{
					int val = paramToint14(m_fxautomation.GetParamNormalized());
					AdjustFader(val);
				}else
				{
					MediaTrack* tr = CSurf_TrackFromID(m_bank_offset, g_csurf_mcpmode);
					double vol = GetMediaTrackInfo_Value(tr, "D_VOL");
					int volint=volToInt14(vol) / 16;
					AdjustFader(volint);
				}
			}
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
			m_faderport_reload |= FPB_RFLAG2;
			CSurf_OnStop();
			break;
		}
			
		case FPB_PUNCH:
		{
			if ((m_faderport_shift))
				RunCommand(g_action_punch_shift);
			else
				RunCommand(g_action_punch);
			break;
		}
			
		case FPB_USER:
		{
			if ((m_faderport_shift)) RunCommand(g_action_user_shift);
			else RunCommand(g_action_user);
			break;
		}
			
		case FPB_LOOP:
		{
			if ((m_faderport_shift))
				RunCommand(g_action_loop_shift);
			else
				RunCommand(g_action_loop);
			break;
		}
			
		case FPB_PLAY:
		{
			m_faderport_reload |= FPB_RFLAG3;
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
			RunCommand((m_faderport_shift)? g_action_undo_shift : g_action_undo);
			break;
		}
			
		case FPB_OUTPUT:
		{
			
			if(!g_pan_scroll_tracks && g_action_output == "0")
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
				
				if(g_action_output == "1") // kw: special case for Master track selection via output button (need to refactor redundant code).
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
					if ((m_faderport_shift))
					{
						RunCommand(g_action_output_shift);
					}
					else
					{
						// get master and select it
						CSurf_OnSelectedChange(tr, true);
						CSurf_OnTrackSelection(tr);
					}
				}
				else if(g_action_output == "2")
				{
					MediaTrack *tr;
					tr=CSurf_TrackFromID(0, false);
					
					// if shift is used, run shift action
					if ((m_faderport_shift))
					{
						RunCommand(g_action_output_shift);
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
					if ((m_faderport_shift))
						RunCommand(g_action_output_shift);
					else
						RunCommand(g_action_output);
					
					
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
			if(m_faderport_fxmode)
			{
				m_fxautomation.SelectPrevFX();
				int val = paramToint14(m_fxautomation.GetParamNormalized());
				AdjustFader(val);
				
			}else
			{
				AdjustBankOffset((m_faderport_bank)?-8:-1,true);
				TrackList_UpdateAllExternalSurfaces();
			}
			
			if (m_midiout) m_midiout->Send(0xa0,0x14,action->state,-1);
			break;
		}
			
		case FPB_CH_NEXT:
		{
			if(m_faderport_fxmode)
			{
				m_fxautomation.SelectNextFX();
				int val = paramToint14(m_fxautomation.GetParamNormalized());
				AdjustFader(val);
			}else
			{
				AdjustBankOffset((m_faderport_bank)?8:1,true);
				TrackList_UpdateAllExternalSurfaces();
			}
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
				if (m_faderport_shift) a+=3;
				MIDI_event_t evt={0,3,{0xbf,static_cast<unsigned char>(a),0}};//clang hates implicit casts --nimaj
				kbd_OnMidiEvent(&evt,-1);
				break;
			}
			
			if(m_faderport_shift)  RunCommand(g_action_mix_shift);
			else RunCommand(g_action_mix);
			break;
		}
			
		case FPB_PROJ:
		{
			if(g_override_automation_read)
			{
				int a=(action->id-0xb)+8;
				if (m_faderport_shift) a+=3;
				MIDI_event_t evt={0,3,{0xbf,static_cast<unsigned char>(a),0}};//clang hates implicit casts --nimaj
				kbd_OnMidiEvent(&evt,-1);
				break;
			}
			if (m_faderport_shift) RunCommand(g_action_project_shift);
			else RunCommand(g_action_project);
			break;
		}
			
		case FPB_TRNS:
		{
			if(g_override_automation_read)
			{
				int a=(action->id-0xb)+8;
				if (m_faderport_shift) a+=3;
				MIDI_event_t evt={0,3,{0xbf,static_cast<unsigned char>(a),0}};//clang hates implicit casts --nimaj
				kbd_OnMidiEvent(&evt,-1);
				break;
			}
			if (m_faderport_shift) RunCommand(g_action_trans_shift);
			else RunCommand(g_action_trans);
			break;
		}
			
		case FPB_FOOTSWITCH:
		{
			if (m_faderport_shift) RunCommand(g_action_footswitch_shift);
			else  RunCommand(g_action_footswitch);
			break;
		}
	}
}

void CSurf_FaderPort::OnMIDIEvent(MIDI_event_t *evt)
{
	char eventBuf[200];
	if(evt->midi_message[0]);
	sprintf(eventBuf,"MIDI Messages[0:1:2:3] = %d:%d:%d:%d",evt->midi_message[0],evt->midi_message[1],evt->midi_message[2],evt->midi_message[3]);//OutputDebugString(eventBuf);
	FaderPortAction action(evt);
	if(action.device == FPD_FADER) ProcessFader(&action);
	else if(action.device == FPD_PAN_KNOB) ProcessPan(&action);
	else if(action.device == FPD_BUTTONS) action.state == 1 ? ProcessButtonDown(&action) : ProcessButtonUp(&action);
}



void CSurf_FaderPort::ReadINIfile()
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
	g_shift_latch = atoi(resultString) == 1 ? true : false;
	
	// mcp_view; default = false
	GetPrivateProfileString("FPCSURF","MCP_VIEW","0",resultString,512,INIFileName);
	g_csurf_mcpmode = atoi(resultString) == 1 ? true : false;
	
	// auto_scroll: default = true
	GetPrivateProfileString("FPCSURF","AUTO_SCROLL","1",resultString,512,INIFileName);
	g_auto_scroll = atoi(resultString) == 1 ? true : false;
	
	// punch: default = ??
	GetPrivateProfileString("FPCSURF","ACTION_PUNCH","40222",resultString,512,INIFileName);
	g_action_punch = resultString;
	
	// punch_shift: default = ??
	GetPrivateProfileString("FPCSURF","ACTION_PUNCH_SHIFT","40172",resultString,512,INIFileName);
	g_action_punch_shift= resultString;
	
	// user: default = ??
	GetPrivateProfileString("FPCSURF","ACTION_USER","40223",resultString,512,INIFileName);
	g_action_user= resultString;
	
	// user_shift: default = ??
	GetPrivateProfileString("FPCSURF","ACTION_USER_SHIFT","40173",resultString,512,INIFileName);
	g_action_user_shift= resultString;
	
	// footswitch: default = play/pause
	GetPrivateProfileString("FPCSURF","ACTION_FOOTSWITCH","40073",resultString,512,INIFileName);
	g_action_footswitch=(resultString);
	
	// footswitch_shift: default = play/pause
	GetPrivateProfileString("FPCSURF","ACTION_FOOTSWITCH_SHIFT","1013",resultString,512,INIFileName);
	g_action_footswitch_shift=(resultString);
	
	// mix: default = show/hide mixer
	GetPrivateProfileString("FPCSURF","ACTION_MIX","40078",resultString,512,INIFileName);
	g_action_mix=(resultString);
	
	// mix_shift: default = show/hide mixer
	GetPrivateProfileString("FPCSURF","ACTION_MIX_SHIFT","40078",resultString,512,INIFileName);
	g_action_mix_shift=(resultString);
	
	// project: default = -->
	GetPrivateProfileString("FPCSURF","ACTION_PROJECT","40861",resultString,512,INIFileName);
	g_action_project=(resultString);
	
	// project_shift: default = <--
	GetPrivateProfileString("FPCSURF","ACTION_PROJECT_SHIFT","40862",resultString,512,INIFileName);
	g_action_project_shift=(resultString);
	
	// trans: default = show/hide transport
	GetPrivateProfileString("FPCSURF","ACTION_TRANS","40259",resultString,512,INIFileName);
	g_action_trans=(resultString);
	
	// project shift: ditto
	GetPrivateProfileString("FPCSURF","ACTION_TRANS_SHIFT","40259",resultString,512,INIFileName);
	g_action_trans_shift=(resultString);
	
	// output
	GetPrivateProfileString("FPCSURF","ACTION_OUTPUT","0",resultString,512,INIFileName);
	g_action_output=(resultString);
	
	// output shift
	GetPrivateProfileString("FPCSURF","ACTION_OUTPUT_SHIFT","40917",resultString,512,INIFileName);
	g_action_output_shift=(resultString);
	
	// loop: default ??
	GetPrivateProfileString("FPCSURF","ACTION_LOOP","1068",resultString,512,INIFileName);
	g_action_loop=(resultString);
	
	// loop_shift: default ??
	GetPrivateProfileString("FPCSURF","ACTION_LOOP_SHIFT","40157",resultString,512,INIFileName);
	g_action_loop_shift=(resultString);
	
	// pan scrolls tracks: default 0
	GetPrivateProfileString("FPCSURF","PAN_SCROLLS_TRACKS","0",resultString,512,INIFileName);
	g_pan_scroll_tracks=atoi(resultString) == 1 ? true : false;
	
	//ignore mix/proj/trns buttons: default = false
	GetPrivateProfileString("FPCSURF","MTP_OVERRIDE","0",resultString,512,INIFileName);
	g_override_automation_read = atoi(resultString) == 1 ? true : false;
	
	// select_is_touched: default = false
	GetPrivateProfileString("FPCSURF","SELECT_IS_TOUCHED","0",resultString,512,INIFileName);
	g_selected_is_touched = atoi(resultString) == 1 ? true : false;
	
	// fader_controls_fx: default = false
	GetPrivateProfileString("FPCSURF","FADER_CONTROLS_FX","0",resultString,512,INIFileName);
	g_fader_controls_fx = atoi(resultString) == 1 ? true : false;
	
	delete[] INIFileName;
	delete[] resultString;
	
	// debug output
	char buf[200];
	sprintf(buf,"MCP VIEW = %d",g_csurf_mcpmode);OutputDebugString(buf);
	sprintf(buf,"SHIFT LATCH = %d",g_shift_latch);OutputDebugString(buf);
	sprintf(buf,"AUTO SCROLL = %d",g_auto_scroll);OutputDebugString(buf);
	sprintf(buf,"FADER CONTROLS FXPARAM = %d",g_fader_controls_fx);OutputDebugString(buf);
	sprintf(buf,"USER ACTION = %s",g_action_user.c_str());OutputDebugString(buf);
	sprintf(buf,"SHIFT/USER ACTION = %s",g_action_user_shift.c_str());OutputDebugString(buf);
	sprintf(buf,"PUNCH ACTION = %s",g_action_punch.c_str());OutputDebugString(buf);
	sprintf(buf,"SHIFT/PUNCH ACTION = %s",g_action_punch_shift.c_str());OutputDebugString(buf);
	sprintf(buf,"LOOP ACTION = %s",g_action_loop.c_str());OutputDebugString(buf);
	sprintf(buf,"SHIFT/LOOP ACTION = %s",g_action_loop_shift.c_str());OutputDebugString(buf);
	sprintf(buf,"FIRST SELECTED IS LAST TOUCHED = %d",g_selected_is_touched);OutputDebugString(buf);
	sprintf(buf,"TRANS = %s",g_action_trans.c_str());OutputDebugString(buf);
	sprintf(buf,"TRANS SHIFT = %s",g_action_trans_shift.c_str());OutputDebugString(buf);
	sprintf(buf,"PROJECT = %s",g_action_project.c_str());OutputDebugString(buf);
	sprintf(buf,"PROJECT SHIFT = %s",g_action_project_shift.c_str());OutputDebugString(buf);
	sprintf(buf,"MIX = %s",g_action_mix.c_str());OutputDebugString(buf);
	sprintf(buf,"MIX SHIFT = %s",g_action_mix_shift.c_str());OutputDebugString(buf);
}

void CSurf_FaderPort::AdjustBankOffset(int amt, bool dosel)
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

void CSurf_FaderPort::CloseNoReset()
{
	delete m_midiout;
	delete m_midiin;
	m_midiout=0;
	m_midiin=0;
}

void CSurf_FaderPort::Run()
{
	if (m_midiin)
	{
		m_midiin->SwapBufs(timeGetTime());
		int l=0;
		MIDI_eventlist *list=m_midiin->GetReadBuf();
		MIDI_event_t *evts;
		while ((evts=list->EnumItems(&l))) OnMIDIEvent(evts);
		
		//if shift+stop+play are pressed (at the same time) reload the ini
		if ((m_faderport_reload & FPB_RFLAG_MASK) == FPB_RFLAG_MASK)
		{
			
			ReadINIfile();
			//Reset flags that depend on features being active
			m_faderport_fxmode = false;
			m_track_waiting = false;
			m_faderport_shift = false;
			CSurf_OnStop(); //Stop playback (which is probably on)
			if (m_midiout) m_midiout->Send(0xa0, 5, 0, -1);//Turn shift light off just incase
			m_faderport_reload = 0;
		}
		
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
		
		static DWORD then = timeGetTime();
		DWORD now = timeGetTime();

		if (g_pan_scroll_tracks)
		{
			if (now >= m_pan_lasttouch + 250 && m_track_waiting)
			{
				TrackList_UpdateAllExternalSurfaces();
				m_track_waiting = false;
			}
		}

		if(m_faderport_fxmode)
		{
            //Cause Bank to blink if we're in fxmode
			if (now  >= then + 500)
			{
				then = timeGetTime();
				static bool blink = false;
				blink = !blink;
				if (m_midiout) m_midiout->Send(0xa0,0x13,blink,-1);
			}
			
			//Prevent the fader from doing weird/rapid jumps when selecting param with pan
			if (now >= m_pan_lasttouch + 250 && m_fx_waiting)
			{
				int val = paramToint14(m_fxautomation.GetParamNormalized());
				AdjustFader(val);
				m_fx_waiting = false;
			}
			
			if (now >= m_pan_lasttouch + 250 && m_track_waiting)
			{
				TrackList_UpdateAllExternalSurfaces();
				m_track_waiting = false;
			}
		}
	}
}


#define FIXID(id) int id=CSurf_TrackToID(trackid,g_csurf_mcpmode); int oid=id; id -= m_bank_offset;

void CSurf_FaderPort::SetSurfaceVolume(MediaTrack *trackid, double volume)
{
	//OutputDebugString("Setting New Volume");
	FIXID(id)
	if (m_midiout && !id && !m_flipmode && !m_faderport_fxmode)
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
void CSurf_FaderPort::SetSurfacePan(MediaTrack *trackid, double pan)
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

void CSurf_FaderPort::SetSurfaceMute(MediaTrack *trackid, bool mute)
{
	FIXID(id)
	if (!id && m_midiout) m_midiout->Send(0xa0,0x15,mute?1:0,-1);
}

void CSurf_FaderPort::SetSurfaceSelected(MediaTrack *trackid, bool selected)
{
	// kw: turn output light on/off
	if(g_action_output=="1" || g_action_output=="0")
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

void CSurf_FaderPort::SetSurfaceSolo(MediaTrack *trackid, bool solo)
{
	FIXID(id)
	if (!id && m_midiout)
	m_midiout->Send(0xa0,0x16,solo?1:0,-1);
}
void CSurf_FaderPort::SetSurfaceRecArm(MediaTrack *trackid, bool recarm)
{
	FIXID(id)
	if (!id && m_midiout)
	m_midiout->Send(0xa0,0x17,recarm?1:0,-1);
}
void CSurf_FaderPort::SetPlayState(bool play, bool pause, bool rec)
{
	if (m_midiout)
	{
		m_midiout->Send(0xa0, 0,rec?1:0,-1);
		m_midiout->Send(0xa0, 1,play?1:0,-1);
		m_midiout->Send(0xa0, 2,(!play&&!pause)?1:0,-1);
	}
}
void CSurf_FaderPort::SetRepeatState(bool rep)
{
	if (m_midiout) m_midiout->Send(0xa0, 8,rep?1:0,-1);
}

bool CSurf_FaderPort::GetTouchState(MediaTrack *trackid, int isPan)
{
	FIXID(id)
	if (!id)
	{
		if (!m_flipmode != !isPan)
		{
			DWORD now=timeGetTime();
			// fake touch, go for 3s after last movement
			if (m_pan_lasttouch==1 || (now<m_pan_lasttouch+3000 && now >= m_pan_lasttouch-1000))
				return true;
	  
			return false;
		}
		
		return !!m_fader_touchstate;
	}
	return false;
}

void CSurf_FaderPort::SetAutoMode(int mode)
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

void CSurf_FaderPort::ResetCachedVolPanStates()
{
	m_vol_lastpos=-1000;
}

void CSurf_FaderPort::OnTrackSelection(MediaTrack *trackid)
{
	int newpos=CSurf_TrackToID(trackid,g_csurf_mcpmode);
	if (newpos>=0 && newpos != m_bank_offset)
	{
		AdjustBankOffset(newpos-m_bank_offset,false);
		TrackList_UpdateAllExternalSurfaces();
	}
	
	if(g_fader_controls_fx)//Support for automating fx params (nimaj 12.5.2015)
	{
		if(trackid)
		{
			bool result = m_fxautomation.Poll(trackid);
			if(m_midiout && result && m_faderport_fxmode)
			{
				int val = paramToint14(m_fxautomation.GetParamNormalized());
				m_midiout->Send(0xb0,0x00,val>>7,-1);
				m_midiout->Send(0xb0,0x20,val&127,-1);
			}
		}
	}
}

int CSurf_FaderPort::Extended(int call, void *parm1, void *parm2, void *parm3)
{
	switch (call)
	{
		case CSURF_EXT_SETLASTTOUCHEDTRACK:
		{
			OutputDebugString("CSURF_EXT_SETLASTTOUCHEDTRACK");
			break;
		}
		
		case CSURF_EXT_SETFOCUSEDFX:
		{
			stringstream ss;
			if( !parm1 && !parm2 && !parm3)
			{
				ss << "CSURF_EXT_SETFOCUSEDFX: Cleared";OutputDebugString(ss.str().c_str());
				
				break;
			}
			else
			{
				MediaTrack* tr = (MediaTrack*) parm1;
				int fxid = *(int*)parm3;
				ss << "FX: " << fxid << " selected on MediaTrack: " << tr;
				OutputDebugString(ss.str().c_str());
				if(m_faderport_fxmode)
				{
					double val = m_fxautomation.SetSelectedTrackFX(tr, fxid);
					if(val > -1) AdjustFader(paramToint14(val));
				}
			}
			break;
		}
		case CSURF_EXT_SETFXPARAM:
		{
			MediaTrack* tr = (MediaTrack*) parm1;
			int fxpid = *(int*)parm2;
			double val = *(double*) parm3;
			int fx = (fxpid >> 16);
			int param = (fxpid & 0xFF);
			if(m_faderport_fxmode)
				if(m_fxautomation.IsSelected(tr, fx, param)) AdjustFader(paramToint14(val));
			
			break;
		}
			
		case CSURF_EXT_SETFXCHANGE:
		{
			MediaTrack* tr = (MediaTrack*) parm1;
			if(m_faderport_fxmode)
			{
				//todo: Update the internal state pof the automation controller
				if(m_fxautomation.GetTrack() == tr) (void) parm1;
				stringstream ss;
				ss << "Value of Parm2:Parm3 = " << (parm2 == nullptr ? "nullptr" : to_string(*(int*)parm2))
					<< " : " << (parm3 == nullptr ? "nullptr" : to_string(*(int*)parm3));
				OutputDebugString(ss.str().c_str());
			}
			break;
		}
			
		case CSURF_EXT_RESET:
		case CSURF_EXT_SETINPUTMONITOR:
		case CSURF_EXT_SETMETRONOME:
		case CSURF_EXT_SETAUTORECARM:
		case CSURF_EXT_SETRECMODE:
		case CSURF_EXT_SETSENDVOLUME:
		case CSURF_EXT_SETSENDPAN:
		case CSURF_EXT_SETFXENABLED:
		case CSURF_EXT_SETLASTTOUCHEDFX:
		case CSURF_EXT_SETMIXERSCROLL:
		case CSURF_EXT_SETBPMANDPLAYRATE:
		case CSURF_EXT_SETPAN_EX:
		case CSURF_EXT_SETRECVVOLUME:
		case CSURF_EXT_SETRECVPAN:
		case CSURF_EXT_SETFXOPEN:
			break;

		default:
		{
			stringstream ss;
			ss << "Unknown EXT Call: " << call << "( " << parm1 << ", " << parm2 << ", " << parm3 << " )";
			OutputDebugString(ss.str().c_str());
			break;
		}
	}
	return 0;
}


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
