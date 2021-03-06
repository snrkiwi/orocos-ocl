// Copyright (C) 2006 Wim Meeussen <wim dot meeussen at mech dot kuleuven dot be>
//               2008 Ruben Smits < first dot last at mech etc>
// This program is free software; you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation; either version 2 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software
// Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
//

#ifndef __LASERSCANNER_HARDWARE__
#define __LASERSCANNER_HARDWARE__

#include <rtt/RTT.hpp>
#include <rtt/RTT.hpp>
#include <rtt/TaskContext.hpp>
#include <rtt/Ports.hpp>
#include <rtt/Event.hpp>
#include <rtt/Properties.hpp>
#include <rtt/dev/AnalogInput.hpp>
#include <rtt/NonPeriodicActivity.hpp>
#include <ocl/OCL.hpp>

// forward declaration
namespace SickDriver
{
    class SickLMS200;
}


namespace OCL {
    /**
     * This class implements a TaskContext which construct an
     * interface to the automated Laser initialising a link to the
     * digital IO PCI cards.
     */
    class LaserScanner
        :public RTT::TaskContext{
    public:
        /**
         * Constructor of the class
         *
         * @param name name of the TaskContext
         * @param propertyfilename location of the
         * propertyfile. Default: cpf/LaserScanner.cpf
         *
         */
        LaserScanner(std::string name);
        virtual~LaserScanner();
        virtual bool configureHook();
        virtual bool startHook();
        virtual void updateHook();  // taskcontext
        virtual void stopHook();

    protected:
        RTT::Property<std::string> port;
        RTT::Property<int>         range_mode;
        RTT::Property<double>      res_mode;
        RTT::Property<std::string> unit_mode;

        /// Dataport which contains the measurements
        RTT::WriteDataPort<std::vector<double> > distances, angles;

        /// Event which is fired if the distance is out of range
        RTT::Event< void(int,double)>  distanceOutOfRange;

    private:
        SickDriver::SickLMS200* sick_laserscanner;
        std::vector<double> distances_local, angles_local;

        const char* port_char;
        unsigned char range_mode_char, res_mode_char, unit_mode_char;
        unsigned int num_meas;

        unsigned char* buf;
        int datalen;

  }; // class
} // namespace

#endif
