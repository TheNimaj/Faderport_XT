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
#include <iostream>
#define OutputDebugString(x) std::cout << x << std::endl
#else
#define OutputDebugString(x)
#endif
#endif

#endif
