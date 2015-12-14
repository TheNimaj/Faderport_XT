//
//  automator.cpp
//  reaper_csurf
//
//  Created by Bryce Bias on 12/6/15.
//  Copyright (c) 2015 Phyersoft. All rights reserved.
//

#include "automator.h"
#include "../reaper_plugin_functions.h"

#include "../MacCompatibility.h"

using namespace std;

bool Envelope_Automator::Poll(MediaTrack* tr)
{
    if(m_ParentTrack == tr) { m_Valid = false; return false; }
    m_ParentTrack = tr;
    m_FXCount = TrackFX_GetCount(m_ParentTrack);
    m_Valid = m_FXCount != 0;
    if(m_Valid)
    {
        m_FX = 0;
        m_Param = 0;
        m_MaxParam = TrackFX_GetNumParams(m_ParentTrack, m_FX);
    }
    
    return m_Valid;
}

double Envelope_Automator::SetSelectedTrackFX(MediaTrack* tr, int fx)
{
    m_ParentTrack = tr;
    m_FX = fx;
    m_Param = 0;
    m_FXCount = TrackFX_GetCount(m_ParentTrack);
    m_MaxParam = TrackFX_GetNumParams(m_ParentTrack, m_FX);
    m_Valid = m_FXCount != 0;
    if(m_Valid)
    {
        //Display the name of the fx
        string fxName = GetCurrentFXName();
        //Display the name of the param
        string paramName = GetCurrentParamName();
        TrackCtl_SetToolTip((fxName + ": " + paramName).c_str(), 0, 0, true);
        return TrackFX_GetParamNormalized(m_ParentTrack, m_FX, m_Param);
    }
    
    return -1;
}

void Envelope_Automator::SelectNextFX()
{
    if(!m_Valid) return;
    m_FX = ++m_FX % m_FXCount;
    m_MaxParam = TrackFX_GetNumParams(m_ParentTrack, m_FX);
    m_Param = 0;
    
    //Display the name of the fx
    string name = GetCurrentFXName();
    TrackCtl_SetToolTip(name.c_str(), 0, 0, true);
}

void Envelope_Automator::SelectPrevFX()
{
    if(!m_Valid) return;
    m_FX = --m_FX % m_FXCount;
    m_MaxParam = TrackFX_GetNumParams(m_ParentTrack, m_FX);
    m_Param = 0;
    
    //Display the name of the fx
    string name = GetCurrentFXName();
    TrackCtl_SetToolTip(name.c_str(), 0, 0, true);
}

double Envelope_Automator::SelectNextParam()
{
    if(!m_Valid) return 0.0f;
    m_Param = ++m_Param % m_MaxParam;
    
    //Display the name of the param
    string name = GetCurrentParamName();
    TrackCtl_SetToolTip(name.c_str(), 0, 0, true);
    
    return TrackFX_GetParamNormalized(m_ParentTrack, m_FX, m_Param);
}

double Envelope_Automator::SelectPrevParam()
{
    if(!m_Valid) return 0.0f;
    m_Param = --m_Param % m_MaxParam;
    
    //Display the name of the param
    string name = GetCurrentParamName();
    TrackCtl_SetToolTip(name.c_str(), 0, 0, true);
    
    return TrackFX_GetParamNormalized(m_ParentTrack, m_FX, m_Param);
}

bool Envelope_Automator::HasValidEnvelope() const { return m_Valid; }

void Envelope_Automator::Reset()
{
    m_Valid = false;
    m_Envelope = nullptr;
}

void Envelope_Automator::BeginTouch()
{
    if(m_Valid)
        m_LastVal = TrackFX_GetParamNormalized(m_ParentTrack, m_FX, m_Param);
    m_TouchState = true;
}

double Envelope_Automator::EndTouch(bool touchMode)
{
    if(m_Valid)
    {
        if(touchMode)
        {
            TrackFX_SetParamNormalized(m_ParentTrack, m_FX, m_Param, m_LastVal);
            TrackFX_EndParamEdit(m_ParentTrack, m_FX, m_Param);
        }
        
    }
    m_TouchState = false;
    return m_LastVal;
}

int Envelope_Automator::GetParamByName(MediaTrack* tr, int fx, const string& name)
{
    char buf[256];
    for (int x = 0; x < TrackFX_GetNumParams(tr, fx); ++x)
    {
        TrackFX_GetParamName(tr,fx,x, buf, 256);
        if(string(buf).compare(name) == 0) return x;
    }
    return -1;
}

std::string Envelope_Automator::GetCurrentFXName()
{
    if(!m_Valid) return "";
    char buf[256];
    TrackFX_GetFXName(m_ParentTrack, m_FX, buf, 256);
    return buf;
}
std::string Envelope_Automator::GetCurrentParamName()
{
    if(!m_Valid) return "";
    char buf[256];
    TrackFX_GetParamName(m_ParentTrack, m_FX, m_Param, buf, 256);
    return buf;
}

void Envelope_Automator::SetParamNormalized(double val)
{
     if(m_Valid)
         TrackFX_SetParamNormalized(m_ParentTrack,m_FX, m_Param, val);
}

double Envelope_Automator::GetParamNormalized() const
{
     if(m_Valid) return TrackFX_GetParamNormalized(m_ParentTrack, m_FX, m_Param);
    return 0.0f;
}

bool Envelope_Automator::IsSelected(MediaTrack* tr, int fx, int param) const
{
    if(m_Valid) return m_ParentTrack == tr && m_FX == fx && m_Param == param;
    return false;
}
