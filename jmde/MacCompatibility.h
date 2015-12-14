//
//  MacCompatibility.h
//  reaper_csurf
//
//  Created by Benjamin Bias on 12/4/15.
//

#ifndef MACCOMPITABILITY_H_
#define MACCOMPITABILITY_H_

#ifdef __APPLE__
#ifdef DEBUG
#include <asl.h>
#define OutputDebugString(x) asl_log(nullptr, nullptr, ASL_LEVEL_NOTICE, "%s\n", x)
#else
#define OutputDebugString(x)
#endif
#endif

#endif
