/**
 * ps3controller - Using a PS3 controller to accelerate, brake, and steer a vehicle.
 * Copyright (C) 2016 Christian Berger
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */

#include <stdint.h>
#include <iostream>
#include <cmath>
#include "PS3Controller.h"

#if !defined(WIN32) && !defined(__gnu_hurd__) && !defined(__APPLE__)
#include <linux/joystick.h>
#include <fcntl.h>
#endif

#include <odvdvehicle/GeneratedHeaders_ODVDVehicle.h> // for ActuationRequest

namespace opendlv {
namespace core {
namespace system {

using namespace std;
using namespace odcore::base;

PS3Controller::PS3Controller(const int &argc, char **argv)
    : TimeTriggeredConferenceClientModule(argc, argv, "ps3controller")
    , m_ps3controllerDevice(0)
    , m_axes(0) 
    {}

PS3Controller::~PS3Controller() {}

void PS3Controller::setUp() {
    string const PS3CONTROLLER_DEVICE_NODE =
    getKeyValueConfiguration().getValue<string>("tools-can-ps3controller.ps3controllerdevicenode");

    cout << "[PS3Controller] Trying to open ps3controller " << PS3CONTROLLER_DEVICE_NODE << endl;
    
    // Setup ps3controller control.
    #if !defined(WIN32) && !defined(__gnu_hurd__) && !defined(__APPLE__)
    int num_of_axes = 0;
    int num_of_buttons = 0;
    int name_of_ps3controller[80];

    if ((m_ps3controllerDevice = open(PS3CONTROLLER_DEVICE_NODE.c_str(), O_RDONLY)) == -1) {
        cerr << "[PS3Controller] Could not open ps3controller " << PS3CONTROLLER_DEVICE_NODE << endl;
        exit(1);
    }

    ioctl(m_ps3controllerDevice, JSIOCGAXES, &num_of_axes);
    ioctl(m_ps3controllerDevice, JSIOCGBUTTONS, &num_of_buttons);
    ioctl(m_ps3controllerDevice, JSIOCGNAME(80), &name_of_ps3controller);

    m_axes = (int *)calloc(num_of_axes, sizeof(int));
    cerr << "[PS3Controller] PS3Controller found " << name_of_ps3controller
    << ", number of axes: " << num_of_axes
    << ", number of buttons: " << num_of_buttons << endl;

    // Use non blocking reading.
    fcntl(m_ps3controllerDevice, F_SETFL, O_NONBLOCK);
    #else
    cerr << "[PS3Controller] This code will not work on this computer architecture " << endl;
    #endif
}

void PS3Controller::tearDown() {
    // send 0.0 for acceleration and steering
    sendActuationRequest(0.0, 0.0, true);
    
    // Deactivate ps3controller control.
    {
        close(m_ps3controllerDevice);
    }
}

void PS3Controller::sendActuationRequest(double acceleration, double steering, bool isValid)
{
    opendlv::proxy::ActuationRequest actuationRequest;
    actuationRequest.setAcceleration(acceleration);
    actuationRequest.setSteering(steering);
    actuationRequest.setIsValid(isValid);
    
    odcore::data::Container c(actuationRequest);
    getConference().send(c);
}

odcore::data::dmcp::ModuleExitCodeMessage::ModuleExitCode PS3Controller::body() {
    /* Lessons learned from live test:
    * - Activating the program takes exclusive control of acceleration 
    * (no manual override, unless the Emergency switch is pressed)
    * - Care must be taken when activating the brakes, since braking too much 
    * is *very* uncomfortable -and potentially dangerous-, on the other hand a 
    * smaller maximum value for the deceleration could be dangerous as well, since it
    * could impair the possiblity to perform an emergency brake while using the controller.
    */
    const double RANGE_ACCELERATION_MIN = 0; // acceleration can be between 0%...
    const double RANGE_ACCELERATION_MAX = 50; // ...and 50% 
    const double RANGE_DECELERATION_MIN = 0; // deceleration can be between 0 m/s^2...
    const double RANGE_DECELERATION_MAX = -10; // ...and -10 m/s^2
    const double RANGE_ROTATION_MIN = -10; // the torque can be between -10 Nm...
    const double RANGE_ROTATION_MAX = 10; // ...and 10 Nm

    double acceleration = 0;
    double steering = 0;

    while (getModuleStateAndWaitForRemainingTimeInTimeslice() == odcore::data::dmcp::ModuleStateMessage::RUNNING) {

        #if !defined(WIN32) && !defined(__gnu_hurd__) && !defined(__APPLE__)
        struct js_event js;

        while (read(m_ps3controllerDevice, &js, sizeof(struct js_event)) > 0) 
        {
            double percent=0.0;
            // Check event.
            switch (js.type & ~JS_EVENT_INIT)
            {
                case JS_EVENT_AXIS:
                    m_axes[js.number] = js.value;
                    if(js.number==0) // LEFT ANALOG STICK
                    {
                        CLOG3 << "[PS3Controller] Axis number " << (int)js.number << " with value " << (int)js.value;
                        // this will return a percent value over the whole range
                        percent=(double)(js.value-MIN_AXES_VALUE)/(double)(MAX_AXES_VALUE-MIN_AXES_VALUE)*100;
                        if(percent>49.95 && percent<50.05)
                        {
                            CLOG3<<" : going straight "<<endl;
                        }
                        else
                        {
                            // this will return values in the range [0-100] for both a left or right turn (instead of [0-50] for left and [50-100] for right)
                            CLOG3<<" : turning "<< (js.value<0?"left":"right") <<" at "<< (js.value<0?(100.0-2*percent):(2*percent-100.0)) <<"%"<<endl;
                        }

                        // map the steering from percentage to its range
                        steering=percent/100*(RANGE_ROTATION_MAX-RANGE_ROTATION_MIN)+RANGE_ROTATION_MIN;
                        
                        // modify in steps of 0.5
                        steering=round(2*steering)/2;
                    }

                    // no else-if as many of these events can occur simultaneously
                    if(js.number==3) // RIGHT ANALOG STICK
                    {
                        CLOG3 << "[PS3Controller] Axis number " << (int)js.number << " with value " << (int)js.value;
                        // this will return a percent value over the whole range
                        percent=(double)(js.value-MIN_AXES_VALUE)/(double)(MAX_AXES_VALUE-MIN_AXES_VALUE)*100;
                        // this will return values in the range [0-100] for both accelerating and braking (instead of [50-0] for accelerating and [50-100] for braking)
                        CLOG3<<" : "<< (js.value<0?"accelerating":"braking") <<" at "<< (js.value<0?(100.0-2*percent):(2*percent-100.0)) <<"%"<<endl;

                        if(js.value<0)
                        {
                            // map the acceleration from percentage to its range
                            acceleration=(100.0-2*percent)/100*(RANGE_ACCELERATION_MAX-RANGE_ACCELERATION_MIN)+RANGE_ACCELERATION_MIN;
                        }
                        else
                        {
                            // map the deceleration from percentage to its range
                            acceleration=(2*percent-100.0)/100*(RANGE_DECELERATION_MAX-RANGE_DECELERATION_MIN);
                        }

                        // modify in steps of 0.5
                        acceleration=round(2*acceleration)/2;

                        // to avoid showing "-0" (just "0" looks better imo)
                        if(acceleration<0.01 && acceleration>-0.01) acceleration=0.0;
                    }

                    // no else-if as many of these events can occur simultaneously
                    if(js.number==18) // X BUTTON AXIS
                    {
                        CLOG3 << "[PS3Controller] Axis number " << (int)js.number << " with value " << (int)js.value << " (X BUTTON AXIS)"<<endl;
                        // kills the steering
                        sendActuationRequest(acceleration, 0.0, true);
                        steering=0.0;
                    }

                    // no else-if as many of these events can occur simultaneously
                    if(js.number==17) // CIRCLE BUTTON AXIS
                    {
                        CLOG3 << "[PS3Controller] Axis number " << (int)js.number << " with value " << (int)js.value << " (CIRCLE BUTTON AXIS)"<<endl;
                        // kills the acceleration
                        sendActuationRequest(0.0, steering, true);
                        acceleration=0.0;
                    }

                    // no else-if as many of these events can occur simultaneously
                    if(js.number==19) // SQUARE BUTTON AXIS
                    {
                        CLOG3 << "[PS3Controller] Axis number " << (int)js.number << " with value " << (int)js.value << " (SQUARE BUTTON AXIS)"<<endl;
                        // kills the acceleration
                        acceleration=0.0;
                        sendActuationRequest(acceleration, steering, true);
                    }

                    // no else-if as many of these events can occur simultaneously
                    if(js.number==16) // TRIANGLE BUTTON AXIS
                    {
                        CLOG3 << "[PS3Controller] Axis number " << (int)js.number << " with value " << (int)js.value << " (TRIANGLE BUTTON AXIS)"<<endl;
                        // kills the steering
                        steering=0.0;
                        sendActuationRequest(acceleration, steering, true);
                    }
                    break;

                case JS_EVENT_BUTTON:
                    break;

                case JS_EVENT_INIT:
                    break;

                default:
                    break;
            }
        }

        /* EAGAIN is returned when the queue is empty */
        if (errno != EAGAIN) {
            cerr << "[PS3Controller] An error occurred in the PS3 joystick event handling" << endl;
            return odcore::data::dmcp::ModuleExitCodeMessage::SERIOUS_ERROR;
        }

        #else
        // Accelerating
        acceleration = 0.0;
        // Steering
        steering = 0.0;
        #endif

        CLOG2 << "[PS3Controller] Values: Acceleration: " << acceleration << ", Steering: " << steering << endl;

        {
            // send out the commands to the truck
            sendActuationRequest(acceleration, steering, true);
        }
    }
    return odcore::data::dmcp::ModuleExitCodeMessage::OKAY;
}
} // system
} // core
} // opendlv::core::system

