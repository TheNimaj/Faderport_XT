//
//  Helpers.h
//  reaper_csurf_fpxt
//
//  Created by Bryce Bias on 2/13/16.
//  Copyright (c) 2016 Phyersoft. All rights reserved.
//

#ifndef __reaper_csurf_fpxt__Helpers__
#define __reaper_csurf_fpxt__Helpers__

int paramToint14(double param);
double int14ToParam(unsigned char msb, unsigned char lsb);
double int14ToVol(unsigned char msb, unsigned char lsb);
double int14ToPan(unsigned char msb, unsigned char lsb);
int volToInt14(double vol);
int panToInt14(double pan);


#endif /* defined(__reaper_csurf_fpxt__Helpers__) */
