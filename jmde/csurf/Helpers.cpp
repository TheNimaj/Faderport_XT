//
//  Helpers.cpp
//  reaper_csurf_fpxt
//
//  Created by Bryce Bias on 2/13/16.
//  Copyright (c) 2016 Phyersoft. All rights reserved.
//

#include "Helpers.h"
#include "csurf.h"

int paramToint14(double param)
{
    return param / 16.0f * 16383.0;
}

double int14ToParam(unsigned char msb, unsigned char lsb)
{
    int val = msb * 128 + lsb;
    return val / 16383.0;
}

double int14ToVol(unsigned char msb, unsigned char lsb)
{
    int val=lsb | (msb<<7);
    double pos=((double)val*1000.0)/16383.0;
    pos=SLIDER2DB(pos);
    return DB2VAL(pos);
}

double int14ToPan(unsigned char msb, unsigned char lsb)
{
    int val=lsb | (msb<<7);
    return 1.0 - (val/(16383.0*0.5));
}

int volToInt14(double vol)
{
    double d=(DB2SLIDER(VAL2DB(vol))*16383.0/1000.0);
    if (d<0.0)d=0.0;
    else if (d>16383.0)d=16383.0;
    
    return (int)(d+0.5);
}

int panToInt14(double pan)
{
    double d=((1.0-pan)*16383.0*0.5);
    if (d<0.0)d=0.0;
    else if (d>16383.0)d=16383.0;
    
    return (int)(d+0.5);
}

