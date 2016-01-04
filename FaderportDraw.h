//
//  FaderportDraw.h
//  reaper_csurf_fpxt
//
//  Created by Bryce Bias on 12/31/15.
//

#pragma once

#include <string>

class midi_Output;
void Display(std::string text, midi_Output* out, int clearDelay, int nextDelay);
void Draw(char ch, midi_Output* output);
void Clear(midi_Output* output);
