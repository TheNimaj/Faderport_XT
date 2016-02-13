//
//  FaderportDraw.cpp
//  reaper_csurf_fpxt
//
//  Created by Bryce Bias on 12/31/15.
//

#include "FaderportDraw.h"
#include "csurf_faderport.h"

#include <algorithm>
using namespace std;
void Display(std::string text, midi_Output* out, int d1, int d2)
{
    text.erase(std::remove(text.begin(),text.end(),' '),text.end());
    std::transform(text.begin(),text.end(),text.begin(), ::toupper);
    for (auto ch : text)
    {
        Draw(ch, out);
        Sleep(d1);
        Clear(out);
        Sleep(d2);
    }
}

void Clear(midi_Output* output)
{
    for (int i = 0; i < FPL_COUNT; ++i)
        output->Send(0xa0, i, 0x0, -1);
}

void Draw(char ch, midi_Output* output)
{
    switch (ch)
    {
        case 'A':
        {
            output->Send(0xa0, FPL_RWD, 0x1, -1);
            output->Send(0xa0, FPL_SHIFT, 0x1, -1);
            output->Send(0xa0, FPL_MIX, 0x1, -1);
            output->Send(0xa0, FPL_READ, 0x1, -1);
            output->Send(0xa0, FPL_BANK, 0x1, -1);
            output->Send(0xa0, FPL_CH_NEXT, 0x1, -1);
            output->Send(0xa0, FPL_OFF, 0x1, -1);
            output->Send(0xa0, FPL_UNDO, 0x1, -1);
            output->Send(0xa0, FPL_LOOP, 0x1, -1);
            output->Send(0xa0, FPL_REC, 0x1, -1);
            output->Send(0xa0, FPL_PROJ, 0x1, -1);
            output->Send(0xa0, FPL_TRNS, 0x1, -1);
            break;
        }
            
        case 'B':
        {
            output->Send(0xa0, FPL_RWD, 0x1, -1);
            output->Send(0xa0, FPL_SHIFT, 0x1, -1);
            output->Send(0xa0, FPL_MIX, 0x1, -1);
            output->Send(0xa0, FPL_READ, 0x1, -1);
            output->Send(0xa0, FPL_CH_PREV, 0x1, -1);
            output->Send(0xa0, FPL_BANK, 0x1, -1);
            output->Send(0xa0, FPL_CH_NEXT, 0x1, -1);
            output->Send(0xa0, FPL_OUTPUT, 0x1, -1);
            output->Send(0xa0, FPL_OFF, 0x1, -1);
            output->Send(0xa0, FPL_TRNS, 0x1, -1);
            output->Send(0xa0, FPL_PROJ, 0x1, -1);
            output->Send(0xa0, FPL_LOOP, 0x1, -1);
            output->Send(0xa0, FPL_REC, 0x1, -1);
            output->Send(0xa0, FPL_PLAY, 0x1, -1);
            output->Send(0xa0, FPL_STOP, 0x1, -1);
            output->Send(0xa0, FPL_FWD, 0x1, -1);
            break;
        }
          
        case 'C':
        {
             output->Send(0xa0, FPL_REC, 0x1, -1);
             output->Send(0xa0, FPL_PLAY, 0x1, -1);
             output->Send(0xa0, FPL_STOP, 0x1, -1);
             output->Send(0xa0, FPL_FWD, 0x1, -1);
             output->Send(0xa0, FPL_SHIFT, 0x1, -1);
             output->Send(0xa0, FPL_MIX, 0x1, -1);
             output->Send(0xa0, FPL_READ, 0x1, -1);
             output->Send(0xa0, FPL_BANK, 0x1, -1);
             output->Send(0xa0, FPL_CH_NEXT, 0x1, -1);
             output->Send(0xa0, FPL_OUTPUT, 0x1, -1);
            break;
        }
            
        case 'D':
        {
            //output->Send(0xa0, FPL_REC, 0x1, -1);
            output->Send(0xa0, FPL_PLAY, 0x1, -1);
            output->Send(0xa0, FPL_STOP, 0x1, -1);
            output->Send(0xa0, FPL_FWD, 0x1, -1);
            output->Send(0xa0, FPL_RWD, 0x1, -1);
            output->Send(0xa0, FPL_SHIFT, 0x1, -1);
            output->Send(0xa0, FPL_MIX, 0x1, -1);
            output->Send(0xa0, FPL_READ, 0x1, -1);
            output->Send(0xa0, FPL_CH_PREV, 0x1, -1);
            output->Send(0xa0, FPL_BANK, 0x1, -1);
            output->Send(0xa0, FPL_CH_NEXT, 0x1, -1);
            output->Send(0xa0, FPL_OFF, 0x1, -1);
            output->Send(0xa0, FPL_UNDO, 0x1, -1);
            output->Send(0xa0, FPL_LOOP, 0x1, -1);
            output->Send(0xa0, FPL_PLAY, 0x1, -1);
            output->Send(0xa0, FPL_STOP, 0x1, -1);
            output->Send(0xa0, FPL_FWD, 0x1, -1);
            break;
        }
            
        case 'E':
        {
            output->Send(0xa0, FPL_REC, 0x1, -1);
            output->Send(0xa0, FPL_PLAY, 0x1, -1);
            output->Send(0xa0, FPL_STOP, 0x1, -1);
            output->Send(0xa0, FPL_FWD, 0x1, -1);
            output->Send(0xa0, FPL_RWD, 0x1, -1);
            output->Send(0xa0, FPL_SHIFT, 0x1, -1);
            output->Send(0xa0, FPL_MIX, 0x1, -1);
            output->Send(0xa0, FPL_READ, 0x1, -1);
            output->Send(0xa0, FPL_CH_PREV, 0x1, -1);
            output->Send(0xa0, FPL_BANK, 0x1, -1);
            output->Send(0xa0, FPL_CH_NEXT, 0x1, -1);
            output->Send(0xa0, FPL_OUTPUT, 0x1, -1);
            output->Send(0xa0, FPL_PROJ, 0x1, -1);
            output->Send(0xa0, FPL_TRNS, 0x1, -1);
            
            break;
        }
            
        case 'F':
        {
            output->Send(0xa0, FPL_RWD, 0x1, -1);
            output->Send(0xa0, FPL_SHIFT, 0x1, -1);
            output->Send(0xa0, FPL_MIX, 0x1, -1);
            output->Send(0xa0, FPL_READ, 0x1, -1);
            output->Send(0xa0, FPL_CH_PREV, 0x1, -1);
            output->Send(0xa0, FPL_BANK, 0x1, -1);
            output->Send(0xa0, FPL_CH_NEXT, 0x1, -1);
            output->Send(0xa0, FPL_OUTPUT, 0x1, -1);
            output->Send(0xa0, FPL_PROJ, 0x1, -1);
            output->Send(0xa0, FPL_TRNS, 0x1, -1);
            break;
        }
            
        case 'G':
        {
            
            output->Send(0xa0, FPL_REC, 0x1, -1);
            output->Send(0xa0, FPL_PLAY, 0x1, -1);
            output->Send(0xa0, FPL_STOP, 0x1, -1);
            output->Send(0xa0, FPL_FWD, 0x1, -1);
            output->Send(0xa0, FPL_RWD, 0x1, -1);
            output->Send(0xa0, FPL_SHIFT, 0x1, -1);
            output->Send(0xa0, FPL_MIX, 0x1, -1);
            output->Send(0xa0, FPL_READ, 0x1, -1);
            output->Send(0xa0, FPL_CH_PREV, 0x1, -1);
            output->Send(0xa0, FPL_BANK, 0x1, -1);
            output->Send(0xa0, FPL_CH_NEXT, 0x1, -1);
            output->Send(0xa0, FPL_OUTPUT, 0x1, -1);
            output->Send(0xa0, FPL_LOOP, 0x1, -1);
            output->Send(0xa0, FPL_UNDO, 0x1, -1);
            output->Send(0xa0, FPL_TRNS, 0x1, -1);
            break;
        }
            
        case 'H':
        {
            output->Send(0xa0, FPL_RWD, 0x1, -1);
            output->Send(0xa0, FPL_SHIFT, 0x1, -1);
            output->Send(0xa0, FPL_MIX, 0x1, -1);
            output->Send(0xa0, FPL_READ, 0x1, -1);
            output->Send(0xa0, FPL_CH_PREV, 0x1, -1);
            output->Send(0xa0, FPL_PROJ, 0x1, -1);
            output->Send(0xa0, FPL_TRNS, 0x1, -1);
            output->Send(0xa0, FPL_OUTPUT, 0x1, -1);
            output->Send(0xa0, FPL_OFF, 0x1, -1);
            output->Send(0xa0, FPL_UNDO, 0x1, -1);
            output->Send(0xa0, FPL_LOOP, 0x1, -1);
            output->Send(0xa0, FPL_REC, 0x1, -1);
            break;
        }
            
        case 'I':
        {
            output->Send(0xa0, FPL_CH_PREV, 0x1, -1);
            output->Send(0xa0, FPL_BANK, 0x1, -1);
            output->Send(0xa0, FPL_CH_NEXT, 0x1, -1);
            output->Send(0xa0, FPL_OUTPUT, 0x1, -1);
            output->Send(0xa0, FPL_WRITE, 0x1, -1);
            output->Send(0xa0, FPL_TOUCH, 0x1, -1);
            output->Send(0xa0, FPL_PROJ, 0x1, -1);
            output->Send(0xa0, FPL_TRNS, 0x1, -1);
            output->Send(0xa0, FPL_PUNCH, 0x1, -1);
            output->Send(0xa0, FPL_USER, 0x1, -1);
            output->Send(0xa0, FPL_RWD, 0x1, -1);
            output->Send(0xa0, FPL_FWD, 0x1, -1);
            output->Send(0xa0, FPL_STOP, 0x1, -1);
            output->Send(0xa0, FPL_PLAY, 0x1, -1);
            output->Send(0xa0, FPL_REC, 0x1, -1);
            break;
        }
            
        case 'J':
        {
            
            output->Send(0xa0, FPL_BANK, 0x1, -1);
            output->Send(0xa0, FPL_CH_NEXT, 0x1, -1);
            output->Send(0xa0, FPL_OUTPUT, 0x1, -1);
            output->Send(0xa0, FPL_OFF, 0x1, -1);
            output->Send(0xa0, FPL_UNDO, 0x1, -1);
            output->Send(0xa0, FPL_LOOP, 0x1, -1);
            output->Send(0xa0, FPL_PLAY, 0x1, -1);
            output->Send(0xa0, FPL_STOP, 0x1, -1);
            output->Send(0xa0, FPL_FWD, 0x1, -1);
            output->Send(0xa0, FPL_SHIFT, 0x1, -1);
            break;
        }
            
        case 'K':
        {
            output->Send(0xa0, FPL_RWD, 0x1, -1);
            output->Send(0xa0, FPL_SHIFT, 0x1, -1);
            output->Send(0xa0, FPL_MIX, 0x1, -1);
            output->Send(0xa0, FPL_READ, 0x1, -1);
            output->Send(0xa0, FPL_CH_PREV, 0x1, -1);
            output->Send(0xa0, FPL_PROJ, 0x1, -1);
            output->Send(0xa0, FPL_TOUCH, 0x1, -1);
            output->Send(0xa0, FPL_OUTPUT, 0x1, -1);
            output->Send(0xa0, FPL_USER, 0x1, -1);
            output->Send(0xa0, FPL_REC, 0x1, -1);
            break;
        }
            
        case 'L':
        {
            output->Send(0xa0, FPL_RWD, 0x1, -1);
            output->Send(0xa0, FPL_SHIFT, 0x1, -1);
            output->Send(0xa0, FPL_MIX, 0x1, -1);
            output->Send(0xa0, FPL_READ, 0x1, -1);
            output->Send(0xa0, FPL_CH_PREV, 0x1, -1);
            output->Send(0xa0, FPL_RWD, 0x1, -1);
            output->Send(0xa0, FPL_FWD, 0x1, -1);
            output->Send(0xa0, FPL_STOP, 0x1, -1);
            output->Send(0xa0, FPL_PLAY, 0x1, -1);
            output->Send(0xa0, FPL_REC, 0x1, -1);
            break;
        }
            
        case 'M':
        {
            output->Send(0xa0, FPL_RWD, 0x1, -1);
            output->Send(0xa0, FPL_SHIFT, 0x1, -1);
            output->Send(0xa0, FPL_MIX, 0x1, -1);
            output->Send(0xa0, FPL_READ, 0x1, -1);
            output->Send(0xa0, FPL_CH_PREV, 0x1, -1);
            output->Send(0xa0, FPL_WRITE, 0x1, -1);
            output->Send(0xa0, FPL_TOUCH, 0x1, -1);
            output->Send(0xa0, FPL_OUTPUT, 0x1, -1);
            output->Send(0xa0, FPL_OFF, 0x1, -1);
            output->Send(0xa0, FPL_UNDO, 0x1, -1);
            output->Send(0xa0, FPL_LOOP, 0x1, -1);
            output->Send(0xa0, FPL_REC, 0x1, -1);
            break;
        }
            
        case 'N':
        {
            output->Send(0xa0, FPL_RWD, 0x1, -1);
            output->Send(0xa0, FPL_SHIFT, 0x1, -1);
            output->Send(0xa0, FPL_MIX, 0x1, -1);
            output->Send(0xa0, FPL_READ, 0x1, -1);
            output->Send(0xa0, FPL_CH_PREV, 0x1, -1);
            output->Send(0xa0, FPL_WRITE, 0x1, -1);
            output->Send(0xa0, FPL_TRNS, 0x1, -1);
            output->Send(0xa0, FPL_LOOP, 0x1, -1);
            output->Send(0xa0, FPL_OUTPUT, 0x1, -1);
            output->Send(0xa0, FPL_OFF, 0x1, -1);
            output->Send(0xa0, FPL_UNDO, 0x1, -1);
            output->Send(0xa0, FPL_LOOP, 0x1, -1);
            output->Send(0xa0, FPL_REC, 0x1, -1);
            break;
        }
            
        case 'O':
        {
            output->Send(0xa0, FPL_REC, 0x1, -1);
            output->Send(0xa0, FPL_PLAY, 0x1, -1);
            output->Send(0xa0, FPL_STOP, 0x1, -1);
            output->Send(0xa0, FPL_FWD, 0x1, -1);
            output->Send(0xa0, FPL_RWD, 0x1, -1);
            output->Send(0xa0, FPL_SHIFT, 0x1, -1);
            output->Send(0xa0, FPL_MIX, 0x1, -1);
            output->Send(0xa0, FPL_READ, 0x1, -1);
            output->Send(0xa0, FPL_CH_PREV, 0x1, -1);
            output->Send(0xa0, FPL_BANK, 0x1, -1);
            output->Send(0xa0, FPL_OUTPUT, 0x1, -1);
            output->Send(0xa0, FPL_CH_NEXT, 0x1, -1);
            output->Send(0xa0, FPL_OFF, 0x1, -1);
            output->Send(0xa0, FPL_UNDO, 0x1, -1);
            output->Send(0xa0, FPL_LOOP, 0x1, -1);
            output->Send(0xa0, FPL_REC, 0x1, -1);
            output->Send(0xa0, FPL_PLAY, 0x1, -1);
            output->Send(0xa0, FPL_STOP, 0x1, -1);
            output->Send(0xa0, FPL_FWD, 0x1, -1);
            break;
        }
        case 'P':
        {
            output->Send(0xa0, FPL_RWD, 0x1, -1);
            output->Send(0xa0, FPL_SHIFT, 0x1, -1);
            output->Send(0xa0, FPL_MIX, 0x1, -1);
            output->Send(0xa0, FPL_READ, 0x1, -1);
            output->Send(0xa0, FPL_CH_PREV, 0x1, -1);
            output->Send(0xa0, FPL_BANK, 0x1, -1);
            output->Send(0xa0, FPL_CH_NEXT, 0x1, -1);
            output->Send(0xa0, FPL_OUTPUT, 0x1, -1);
            output->Send(0xa0, FPL_OFF, 0x1, -1);
            output->Send(0xa0, FPL_TRNS, 0x1, -1);
            output->Send(0xa0, FPL_PROJ, 0x1, -1);
            
            break;
        }
            
        case 'Q':
        {
            output->Send(0xa0, FPL_REC, 0x1, -1);
            output->Send(0xa0, FPL_PLAY, 0x1, -1);
            output->Send(0xa0, FPL_STOP, 0x1, -1);
            output->Send(0xa0, FPL_FWD, 0x1, -1);
            output->Send(0xa0, FPL_RWD, 0x1, -1);
            output->Send(0xa0, FPL_SHIFT, 0x1, -1);
            output->Send(0xa0, FPL_MIX, 0x1, -1);
            output->Send(0xa0, FPL_READ, 0x1, -1);
            output->Send(0xa0, FPL_CH_PREV, 0x1, -1);
            output->Send(0xa0, FPL_BANK, 0x1, -1);
            output->Send(0xa0, FPL_OUTPUT, 0x1, -1);
            output->Send(0xa0, FPL_CH_NEXT, 0x1, -1);
            output->Send(0xa0, FPL_OFF, 0x1, -1);
            output->Send(0xa0, FPL_UNDO, 0x1, -1);
            output->Send(0xa0, FPL_LOOP, 0x1, -1);
            output->Send(0xa0, FPL_REC, 0x1, -1);
            output->Send(0xa0, FPL_PLAY, 0x1, -1);
            output->Send(0xa0, FPL_STOP, 0x1, -1);
            output->Send(0xa0, FPL_FWD, 0x1, -1);
             output->Send(0xa0, FPL_USER, 0x1, -1);

            break;
        }
            
        case 'R':
        {
            output->Send(0xa0, FPL_RWD, 0x1, -1);
            output->Send(0xa0, FPL_SHIFT, 0x1, -1);
            output->Send(0xa0, FPL_MIX, 0x1, -1);
            output->Send(0xa0, FPL_READ, 0x1, -1);
            output->Send(0xa0, FPL_CH_PREV, 0x1, -1);
            output->Send(0xa0, FPL_BANK, 0x1, -1);
            output->Send(0xa0, FPL_CH_NEXT, 0x1, -1);
            output->Send(0xa0, FPL_OUTPUT, 0x1, -1);
            output->Send(0xa0, FPL_OFF, 0x1, -1);
            output->Send(0xa0, FPL_TRNS, 0x1, -1);
            output->Send(0xa0, FPL_PROJ, 0x1, -1);
            output->Send(0xa0, FPL_USER, 0x1, -1);
            output->Send(0xa0, FPL_REC, 0x1, -1);
            break;
        }
            
        case 'S':
        {
            output->Send(0xa0, FPL_REC, 0x1, -1);
            output->Send(0xa0, FPL_PLAY, 0x1, -1);
            output->Send(0xa0, FPL_STOP, 0x1, -1);
            output->Send(0xa0, FPL_FWD, 0x1, -1);
            output->Send(0xa0, FPL_RWD, 0x1, -1);
            output->Send(0xa0, FPL_LOOP, 0x1, -1);
            output->Send(0xa0, FPL_UNDO, 0x1, -1);
            output->Send(0xa0, FPL_TRNS, 0x1, -1);
            output->Send(0xa0, FPL_PROJ, 0x1, -1);
            output->Send(0xa0, FPL_MIX, 0x1, -1);
            output->Send(0xa0, FPL_READ, 0x1, -1);
            output->Send(0xa0, FPL_CH_PREV, 0x1, -1);
            output->Send(0xa0, FPL_BANK, 0x1, -1);
            output->Send(0xa0, FPL_CH_NEXT, 0x1, -1);
            output->Send(0xa0, FPL_OUTPUT, 0x1, -1);
            break;
        }
            
        case 'T':
        {
            output->Send(0xa0, FPL_CH_PREV, 0x1, -1);
            output->Send(0xa0, FPL_BANK, 0x1, -1);
            output->Send(0xa0, FPL_CH_NEXT, 0x1, -1);
            output->Send(0xa0, FPL_OUTPUT, 0x1, -1);
            output->Send(0xa0, FPL_WRITE, 0x1, -1);
            output->Send(0xa0, FPL_TOUCH, 0x1, -1);
            output->Send(0xa0, FPL_PROJ, 0x1, -1);
            output->Send(0xa0, FPL_TRNS, 0x1, -1);
            output->Send(0xa0, FPL_PUNCH, 0x1, -1);
            output->Send(0xa0, FPL_USER, 0x1, -1);
            output->Send(0xa0, FPL_STOP, 0x1, -1);
            break;
        }
            
        case 'U':
        {
            output->Send(0xa0, FPL_REC, 0x1, -1);
            output->Send(0xa0, FPL_PLAY, 0x1, -1);
            output->Send(0xa0, FPL_STOP, 0x1, -1);
            output->Send(0xa0, FPL_FWD, 0x1, -1);
            output->Send(0xa0, FPL_RWD, 0x1, -1);
            output->Send(0xa0, FPL_SHIFT, 0x1, -1);
            output->Send(0xa0, FPL_MIX, 0x1, -1);
            output->Send(0xa0, FPL_READ, 0x1, -1);
            output->Send(0xa0, FPL_CH_PREV, 0x1, -1);
            output->Send(0xa0, FPL_OUTPUT, 0x1, -1);
            output->Send(0xa0, FPL_OFF, 0x1, -1);
            output->Send(0xa0, FPL_UNDO, 0x1, -1);
            output->Send(0xa0, FPL_LOOP, 0x1, -1);
            output->Send(0xa0, FPL_REC, 0x1, -1);
            output->Send(0xa0, FPL_PLAY, 0x1, -1);
            output->Send(0xa0, FPL_STOP, 0x1, -1);
            output->Send(0xa0, FPL_FWD, 0x1, -1);
            break;
        }
            
        case 'V':
        {
            output->Send(0xa0, FPL_STOP, 0x1, -1);
            output->Send(0xa0, FPL_MIX, 0x1, -1);
            output->Send(0xa0, FPL_READ, 0x1, -1);
            output->Send(0xa0, FPL_CH_PREV, 0x1, -1);
            output->Send(0xa0, FPL_OUTPUT, 0x1, -1);
            output->Send(0xa0, FPL_OFF, 0x1, -1);
            output->Send(0xa0, FPL_UNDO, 0x1, -1);
            output->Send(0xa0, FPL_STOP, 0x1, -1);
            output->Send(0xa0, FPL_PUNCH, 0x1, -1);
            output->Send(0xa0, FPL_USER, 0x1, -1);

            break;
        }
            
        case 'W':
        {
            output->Send(0xa0, FPL_REC, 0x1, -1);
            output->Send(0xa0, FPL_RWD, 0x1, -1);
            output->Send(0xa0, FPL_SHIFT, 0x1, -1);
            output->Send(0xa0, FPL_MIX, 0x1, -1);
            output->Send(0xa0, FPL_READ, 0x1, -1);
            output->Send(0xa0, FPL_CH_PREV, 0x1, -1);
            output->Send(0xa0, FPL_OUTPUT, 0x1, -1);
            output->Send(0xa0, FPL_OFF, 0x1, -1);
            output->Send(0xa0, FPL_UNDO, 0x1, -1);
            output->Send(0xa0, FPL_LOOP, 0x1, -1);
            output->Send(0xa0, FPL_REC, 0x1, -1);
            output->Send(0xa0, FPL_PUNCH, 0x1, -1);
            output->Send(0xa0, FPL_USER, 0x1, -1);
            break;
        }
            
        case 'X':
        {
             output->Send(0xa0, FPL_CH_PREV, 0x1, -1);
             output->Send(0xa0, FPL_WRITE, 0x1, -1);
             output->Send(0xa0, FPL_TRNS, 0x1, -1);
             output->Send(0xa0, FPL_LOOP, 0x1, -1);
             output->Send(0xa0, FPL_REC, 0x1, -1);
             output->Send(0xa0, FPL_RWD, 0x1, -1);
             output->Send(0xa0, FPL_SHIFT, 0x1, -1);
             output->Send(0xa0, FPL_PROJ, 0x1, -1);
             output->Send(0xa0, FPL_TOUCH, 0x1, -1);
             output->Send(0xa0, FPL_OUTPUT, 0x1, -1);
            break;
        }
            
        case 'Y':
        {
            output->Send(0xa0, FPL_REC, 0x1, -1);
            output->Send(0xa0, FPL_PLAY, 0x1, -1);
            output->Send(0xa0, FPL_STOP, 0x1, -1);
            output->Send(0xa0, FPL_FWD, 0x1, -1);
            output->Send(0xa0, FPL_RWD, 0x1, -1);
            output->Send(0xa0, FPL_LOOP, 0x1, -1);
            output->Send(0xa0, FPL_UNDO, 0x1, -1);
            output->Send(0xa0, FPL_MIX, 0x1, -1);
            output->Send(0xa0, FPL_TRNS, 0x1, -1);
            output->Send(0xa0, FPL_OFF, 0x1, -1);
            output->Send(0xa0, FPL_PROJ, 0x1, -1);
            output->Send(0xa0, FPL_CH_PREV, 0x1, -1);
            output->Send(0xa0, FPL_READ, 0x1, -1);
            output->Send(0xa0, FPL_OUTPUT, 0x1, -1);
            break;
        }
            
        case 'Z':
        {
            output->Send(0xa0, FPL_CH_PREV, 0x1, -1);
            output->Send(0xa0, FPL_BANK, 0x1, -1);
            output->Send(0xa0, FPL_OUTPUT, 0x1, -1);
            output->Send(0xa0, FPL_CH_NEXT, 0x1, -1);
            output->Send(0xa0, FPL_TOUCH, 0x1, -1);
            output->Send(0xa0, FPL_PROJ, 0x1, -1);
            output->Send(0xa0, FPL_SHIFT, 0x1, -1);
            output->Send(0xa0, FPL_RWD, 0x1, -1);
            output->Send(0xa0, FPL_FWD, 0x1, -1);
            output->Send(0xa0, FPL_STOP, 0x1, -1);
            output->Send(0xa0, FPL_PLAY, 0x1, -1);
            output->Send(0xa0, FPL_REC, 0x1, -1);
            break;
        }
    }
}