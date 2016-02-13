//
//  DeviceButton.cpp
//  reaper_csurf_fpxt
//
//  Created by Bryce Bias on 2/13/16.
//  Copyright (c) 2016 Phyersoft. All rights reserved.
//

#include "DeviceButton.h"
DeviceButton::DeviceButton(FaderPortButton button, FaderPortLights light, const std::string& name,
                           const std::string& defaultAction, const std::string& defaultActionShift) :
    buttonId(button), lightId(light), buttonName(name), action(defaultAction), action_shift(defaultActionShift)
{
    
}