/*
** reaper_csurf
** Copyright (C) 2006-2008 Cockos Incorporated
** License: LGPL.
*/

//Disable this while working on Faderport XT


#define REAPERAPI_IMPLEMENT

#ifndef DEBUG
#define REAPERAPI_MINIMAL


#define REAPERAPI_WANT_DB2SLIDER
#define REAPERAPI_WANT_SLIDER2DB
#define REAPERAPI_WANT_GetNumMIDIInputs
#define REAPERAPI_WANT_GetNumMIDIOutputs
#define REAPERAPI_WANT_CreateMIDIInput
#define REAPERAPI_WANT_CreateMIDIOutput
#define REAPERAPI_WANT_GetMIDIOutputName
#define REAPERAPI_WANT_GetMIDIInputName
#define REAPERAPI_WANT_CSurf_TrackToID
#define REAPERAPI_WANT_CSurf_TrackFromID
#define REAPERAPI_WANT_CSurf_NumTracks
#define REAPERAPI_WANT_CSurf_SetTrackListChange
#define REAPERAPI_WANT_CSurf_SetSurfaceVolume
#define REAPERAPI_WANT_CSurf_SetSurfacePan
#define REAPERAPI_WANT_CSurf_SetSurfaceMute
#define REAPERAPI_WANT_CSurf_SetSurfaceSelected
#define REAPERAPI_WANT_CSurf_SetSurfaceSolo
#define REAPERAPI_WANT_CSurf_SetSurfaceRecArm
#define REAPERAPI_WANT_CSurf_GetTouchState
#define REAPERAPI_WANT_CSurf_SetAutoMode
#define REAPERAPI_WANT_CSurf_SetPlayState
#define REAPERAPI_WANT_CSurf_SetRepeatState
#define REAPERAPI_WANT_CSurf_OnVolumeChange
#define REAPERAPI_WANT_CSurf_OnPanChange
#define REAPERAPI_WANT_CSurf_OnMuteChange
#define REAPERAPI_WANT_CSurf_OnSelectedChange
#define REAPERAPI_WANT_CSurf_OnSoloChange
#define REAPERAPI_WANT_CSurf_OnFXChange
#define REAPERAPI_WANT_CSurf_OnRecArmChange
#define REAPERAPI_WANT_CSurf_OnPlay
#define REAPERAPI_WANT_CSurf_OnStop
#define REAPERAPI_WANT_CSurf_OnFwd
#define REAPERAPI_WANT_CSurf_OnRew
#define REAPERAPI_WANT_CSurf_OnRecord
#define REAPERAPI_WANT_CSurf_GoStart
#define REAPERAPI_WANT_CSurf_GoEnd
#define REAPERAPI_WANT_CSurf_OnArrow
#define REAPERAPI_WANT_CSurf_OnTrackSelection
#define REAPERAPI_WANT_CSurf_ResetAllCachedVolPanStates
#define REAPERAPI_WANT_CSurf_ScrubAmt
#define REAPERAPI_WANT_TrackList_UpdateAllExternalSurfaces
#define REAPERAPI_WANT_kbd_OnMidiEvent
#define REAPERAPI_WANT_GetMasterMuteSoloFlags
#define REAPERAPI_WANT_ClearAllRecArmed
#define REAPERAPI_WANT_SetTrackAutomationMode
#define REAPERAPI_WANT_GetTrackAutomationMode
#define REAPERAPI_WANT_SoloAllTracks
#define REAPERAPI_WANT_MuteAllTracks
#define REAPERAPI_WANT_BypassFxAllTracks
#define REAPERAPI_WANT_GetTrackInfo
#define REAPERAPI_WANT_SetTrackSelected
#define REAPERAPI_WANT_SetAutomationMode
#define REAPERAPI_WANT_UpdateTimeline
#define REAPERAPI_WANT_Main_UpdateLoopInfo
#define REAPERAPI_WANT_GetPlayState
#define REAPERAPI_WANT_GetPlayPosition
#define REAPERAPI_WANT_GetCursorPosition
#define REAPERAPI_WANT_format_timestr_pos
#define REAPERAPI_WANT_Track_GetPeakInfo
#define REAPERAPI_WANT_GetTrackUIVolPan
#define REAPERAPI_WANT_GetSetRepeat
#define REAPERAPI_WANT_mkvolpanstr
#define REAPERAPI_WANT_mkvolstr
#define REAPERAPI_WANT_mkpanstr
#define REAPERAPI_WANT_MoveEditCursor
#define REAPERAPI_WANT_adjustZoom
#define REAPERAPI_WANT_GetHZoomLevel

#define REAPERAPI_WANT_TrackFX_GetCount
#define REAPERAPI_WANT_TrackFX_GetNumParams
#define REAPERAPI_WANT_TrackFX_GetParam
#define REAPERAPI_WANT_TrackFX_SetParam
#define REAPERAPI_WANT_TrackFX_GetParamName
#define REAPERAPI_WANT_TrackFX_FormatParamValue
#define REAPERAPI_WANT_TrackFX_GetFXName

#define REAPERAPI_WANT_get_config_var
#define REAPERAPI_WANT_projectconfig_var_addr
#define REAPERAPI_WANT_projectconfig_var_getoffs


#define REAPERAPI_WANT_GetTrackGUID

#define REAPERAPI_WANT_GetTrack
#define REAPERAPI_WANT_GetMasterTrack
#define REAPERAPI_WANT_ShowConsoleMsg
#define REAPERAPI_WANT_Main_OnCommand
#define REAPERAPI_WANT_GetMediaTrackInfo_Value
#define REAPERAPI_WANT_GetExePath
#define REAPERAPI_WANT_GetResourcePath
#define REAPERAPI_WANT_NamedCommandLookup

#define REAPERAPI_WANT_TimeMap2_timeToBeats

#endif


#include "../reaper_plugin_functions.h"
#include "csurf.h"

extern reaper_csurf_reg_t csurf_bcf_reg,csurf_faderport_reg,csurf_hui_reg,
     csurf_mcu_reg,csurf_mcuex_reg,csurf_tranzport_reg,csurf_alphatrack_reg,csurf_01X_reg;

REAPER_PLUGIN_HINSTANCE g_hInst; // used for dialogs, if any
HWND g_hwnd;


int *g_config_csurf_rate,*g_config_zoommode;

int __g_projectconfig_timemode2, __g_projectconfig_timemode;
int __g_projectconfig_measoffs;
int __g_projectconfig_timeoffs; // double

extern "C"
{

REAPER_PLUGIN_DLL_EXPORT int REAPER_PLUGIN_ENTRYPOINT(REAPER_PLUGIN_HINSTANCE hInstance, reaper_plugin_info_t *rec)
{
    g_hInst=hInstance;

    if (!rec || rec->caller_version != REAPER_PLUGIN_VERSION || !rec->GetFunc)
      return 0;

    g_hwnd = rec->hwnd_main;
  
    if(REAPERAPI_LoadAPI(rec->GetFunc)) return 0;
    
    int sztmp;
    int errcnt = 0;
#define IMPVAR(x,nm) if (!((*(void **)&(x)) = get_config_var(nm,&sztmp)) || sztmp != sizeof(*x)) errcnt++;
#define IMPVARP(x,nm,type) if (!((x) = projectconfig_var_getoffs(nm,&sztmp)) || sztmp != sizeof(type)) errcnt++;
  
  IMPVAR(g_config_csurf_rate,"csurfrate")
  IMPVAR(g_config_zoommode,"zoommode")

  IMPVARP(__g_projectconfig_timemode,"projtimemode",int)
  IMPVARP(__g_projectconfig_timemode2,"projtimemode2",int)
  IMPVARP(__g_projectconfig_timeoffs,"projtimeoffs",double);
  IMPVARP(__g_projectconfig_measoffs,"projmeasoffs",int);


  if (errcnt) return 0;


    //Only register faderport on mac because I've only copied the dialog for it
    rec->Register("csurf",&csurf_faderport_reg);
    rec->Register("csurf",&csurf_hui_reg);
    rec->Register("csurf",&csurf_tranzport_reg);
    rec->Register("csurf",&csurf_alphatrack_reg);
    rec->Register("csurf",&csurf_01X_reg);
    rec->Register("csurf",&csurf_bcf_reg);
    rec->Register("csurf",&csurf_mcu_reg);
    rec->Register("csurf",&csurf_mcuex_reg);

  return 1;

}

};





#ifndef _WIN32 // MAC resources
#include "../../WDL/swell/swell-dlggen.h"
#include "res.rc_mac_dlg"
#undef BEGIN
#undef END
#include "../../WDL/swell/swell-menugen.h"
#include "res.rc_mac_menu"
#endif


#ifndef _WIN32 // let OS X use this threading step

#include "../../WDL/mutex.h"
#include "../../WDL/ptrlist.h"



class threadedMIDIOutput : public midi_Output
{
public:
  threadedMIDIOutput(midi_Output *out) 
  { 
    m_output=out;
    m_quit=false;
    DWORD id;
    m_hThread=CreateThread(NULL,0,threadProc,this,0,&id);
  }
  virtual ~threadedMIDIOutput() 
  {
    if (m_hThread)
    {
      m_quit=true;
      WaitForSingleObject(m_hThread,INFINITE);
      CloseHandle(m_hThread);
      m_hThread=0;
      Sleep(30);
    }

    delete m_output;
    m_empty.Empty(true);
    m_full.Empty(true);
  }

  virtual void SendMsg(MIDI_event_t *msg, int frame_offset) // frame_offset can be <0 for "instant" if supported
  {
    if (!msg) return;

    WDL_HeapBuf *b=NULL;
    if (m_empty.GetSize())
    {
      m_mutex.Enter();
      b=m_empty.Get(m_empty.GetSize()-1);
      m_empty.Delete(m_empty.GetSize()-1);
      m_mutex.Leave();
    }
    if (!b && m_empty.GetSize()+m_full.GetSize()<500)
      b=new WDL_HeapBuf(256);

    if (b)
    {
      int sz=msg->size;
      if (sz<3)sz=3;
      int len = msg->midi_message + sz - (unsigned char *)msg;
      memcpy(b->Resize(len,false),msg,len);
      m_mutex.Enter();
      m_full.Add(b);
      m_mutex.Leave();
    }
  }

  virtual void Send(unsigned char status, unsigned char d1, unsigned char d2, int frame_offset) // frame_offset can be <0 for "instant" if supported
  {
    MIDI_event_t evt={0,3,status,d1,d2};
    SendMsg(&evt,frame_offset);
  }

  ///////////

  static DWORD WINAPI threadProc(LPVOID p)
  {
    WDL_HeapBuf *lastbuf=NULL;
    threadedMIDIOutput *_this=(threadedMIDIOutput*)p;
    unsigned int scnt=0;
    for (;;)
    {
      if (_this->m_full.GetSize()||lastbuf)
      {
        _this->m_mutex.Enter();
        if (lastbuf) _this->m_empty.Add(lastbuf);
        lastbuf=_this->m_full.Get(0);
        _this->m_full.Delete(0);
        _this->m_mutex.Leave();

        if (lastbuf) _this->m_output->SendMsg((MIDI_event_t*)lastbuf->Get(),-1);
        scnt=0;
      }
      else 
      {
        Sleep(1);
        if (_this->m_quit&&scnt++>3) break; //only quit once all messages have been sent
      }
    }
    delete lastbuf;
    return 0;
  }

  WDL_Mutex m_mutex;
  WDL_PtrList<WDL_HeapBuf> m_full,m_empty;

  HANDLE m_hThread;
  bool m_quit;
  midi_Output *m_output;
};




midi_Output *CreateThreadedMIDIOutput(midi_Output *output)
{
  if (!output) return output;
  return new threadedMIDIOutput(output);
}

#else

// windows doesnt need it since we have threaded midi outputs now
midi_Output *CreateThreadedMIDIOutput(midi_Output *output)
{
  return output;
}

#endif
