//
//  automator.h
//  reaper_csurf
//
//  Created by Bryce Bias on 12/6/15.
//  Copyright (c) 2015 Phyersoft. All rights reserved.
//

#ifndef __reaper_csurf__automator__
#define __reaper_csurf__automator__

#include <stdio.h>
#include <string>

class MediaTrack;
class TrackEnvelope;

class Envelope_Automator
{
    bool m_Valid = false;
    bool m_Touched = false;
    bool m_TouchState = false;
    MediaTrack* m_ParentTrack = nullptr;
    TrackEnvelope* m_Envelope = nullptr;
    
    int m_FX = -1;
    int m_Param = 0;
    int m_FXCount = 0;
    float m_LastVal = 0.0f;
    int m_MaxParam = 0;
    
protected:
    std::string GetCurrentFXName();
    std::string GetCurrentParamName();
public:
    bool Poll(MediaTrack* tr);
    
    MediaTrack* GetTrack() const { return m_ParentTrack; }
    TrackEnvelope* GetTrackEnvelope() const { return m_Envelope; }
    
    /**
        Sets currently selected Track, FX, and Param (0).
        Returns normalized value of fx param
     */
    double SetSelectedTrackFX(MediaTrack* tr, int fx);
    
    void SelectNextFX();
    void SelectPrevFX();
    
    double SelectNextParam();
    double SelectPrevParam();
    
    bool HasValidEnvelope() const;
    bool GetTouchState() const { return m_TouchState; }
    
    void Reset();
    void BeginTouch();
    double EndTouch(bool touchMode);
    
    int GetParamByName(MediaTrack* tr, int fx, const std::string& name);
    
    void SetParamNormalized(double val);
    double GetParamNormalized() const;
    
    int GetCurrentParam() const { return m_Param; }
    int GetCurrentFX() const { return m_FX; }
    
    bool IsSelected(MediaTrack* tr, int fx, int param) const;
};

#endif /* defined(__reaper_csurf__automator__) */
