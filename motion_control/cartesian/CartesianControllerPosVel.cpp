// $Id: nAxisGeneratorCartesianPos.hpp,v 1.1.1.1 2003/12/02 20:32:06 kgadeyne Exp $
// Copyright (C) 2003 Klaas Gadeyne <klaas.gadeyne@mech.kuleuven.ac.be>
//                    Wim Meeussen  <wim.meeussen@mech.kuleuven.ac.be>
// Copyright (C) 2006 Ruben Smits <ruben.smits@mech.kuleuven.be>
//
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

#include "CartesianControllerPosVel.hpp"
#include <rtt/Logger.hpp>
#include <assert.h>
#include <ocl/ComponentLoader.hpp>

ORO_LIST_COMPONENT_TYPE( OCL::CartesianControllerPosVel );

namespace OCL
{
    using namespace RTT;
    using namespace KDL;
    using namespace std;

    CartesianControllerPosVel::CartesianControllerPosVel(string name)
        : TaskContext(name,PreOperational),
          _gain_local(6,0.0),
          _position_meas("CartesianSensorPosition"),
          _position_desi("CartesianDesiredPosition"),
          _velocity_desi("CartesianDesiredVelocity"),
          _velocity_out("CartesianOutputVelocity"),
          _controller_gain("K", "Proportional Gain",vector<double>(6,0.0))
    {
        //Creating TaskContext

        //Adding Ports
        this->ports()->addPort(&_position_meas);
        this->ports()->addPort(&_position_desi);
        this->ports()->addPort(&_velocity_desi);
        this->ports()->addPort(&_velocity_out);

        //Adding Properties
        this->properties()->addProperty(&_controller_gain);

    }


    CartesianControllerPosVel::~CartesianControllerPosVel(){};

    bool CartesianControllerPosVel::configureHook()
    {
        //        if(!marshalling()->readProperties(this->getName()+".cpf"))
        //            return false;
        //Check if size is correct
        if(_controller_gain.value().size()!=6)
            return false;
        //copy property values in local variable
        _gain_local=_controller_gain.value();
        return true;
    }


    bool CartesianControllerPosVel::startHook()
    {
        return true;
    }

    void CartesianControllerPosVel::updateHook()
    {
        // copy Input and Setpoint to local values
        _position_meas_local = _position_meas.Get();
        _position_desi_local = _position_desi.Get();
        _velocity_desi_local = _velocity_desi.Get();

        // feedback on position
        _velocity_feedback = diff(_position_meas_local, _position_desi_local);
        for(unsigned int i=0; i<6; i++)
            _velocity_feedback(i) *= _gain_local[i];

        //Add desired velocity as feedforward
        _velocity_out_local = _velocity_desi_local + _velocity_feedback;

        _velocity_out.Set(_velocity_out_local);
    }


    void CartesianControllerPosVel::stopHook()
    {
    }

    void CartesianControllerPosVel::cleanupHook()
    {
    }

}//namespace

