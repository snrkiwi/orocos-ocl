// Copyright (C) 2006 Klaas Gadeyne <klaas dot gadeyne at mech dot kuleuven dot be>
//                    Ruben Smits <ruben dot smits at mech dot kuleuven dot be>
//                    Wim Meeussen <wim dot meeussen at mech dot kuleuven dot be>
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

#include "KryptonK600Sensor.hpp"
#include <rtt/Logger.hpp>
#include <kdl/frames_io.hpp>

namespace OCL
{
    using namespace RTT;
    using namespace KDL;
    using namespace std;


  // constructor
    KryptonK600Sensor::KryptonK600Sensor(string name, unsigned int num_leds, unsigned int priority)
      : TaskContext(name),
#if defined (OROPKG_OS_LXRT)
	NonPeriodicActivity(priority),
	_keep_running(true),
#endif
	_num_leds(num_leds),
	_ledPositions_local(num_leds),
	_ledPositions("LedPositions")
    {
        ports()->addPort(&_ledPositions);

	// set ledpositions to zero
	for (unsigned int i=0; i<_num_leds; i++)
	  _ledPositions_local[i] = Vector(1.0,1.0,1.0);
	_ledPositions.Set(_ledPositions_local);

#if defined (OROPKG_OS_LXRT)
        // If kernel Module is not loaded yet, Print error message
        if (! ((udp_message_arrived = (SEM *) rt_get_adr(nam2num("KEDSEM"))) &&
               (udp_message = (MBX *) rt_get_adr(nam2num("KEDMBX")))	      ))
	  log(Info) << "KryptonK600Sensor: Can't find sem/mbx" << endlog();

        for (unsigned int i =0; i<_num_leds;i++)
	  _ledPositions_local[i] = Vector(-99999, -99999, -99999);

	this->NonPeriodicActivity::start();
#endif
    }



  // destructor
    KryptonK600Sensor::~KryptonK600Sensor()
    {
#if defined (OROPKG_OS_LXRT)
      _keep_running = false;

      // In update, we might be waiting for the sem, so first post it...
      SEM * udp_message_arrived;
      udp_message_arrived = (SEM *) rt_get_adr(nam2num("KEDSEM"));
      rt_sem_signal (udp_message_arrived);

      this->NonPeriodicActivity::stop();
#endif
    }




#if defined (OROPKG_OS_LXRT)
    void KryptonK600Sensor::loop()
    {
      char msg[MAX_MESSAGE_LENGTH];

      while (_keep_running){
	// Wait until kernel Module posts semaphore
	rt_sem_wait(udp_message_arrived);

	int ret = rt_mbx_receive_if(udp_message,(void *) &msg, MAX_MESSAGE_LENGTH);
	if (ret != 0 && ret !=MAX_MESSAGE_LENGTH)
	  log(Info) << "KryptonK600Sensor: Error receiving message from mbx: error "
			<< ret << " EINVAL is " << EINVAL << endlog();
	else{
	  // Interprete Message
	  if ( interprete_K600_Msg(msg))
	    // Copy Data into write buffer
	    _ledPositions.Set(_ledPositions_local);
	  else log(Info) << "KryptonK600Sensor: Bad message, "
			     << "or something went wrong in decoding" << endlog();
	}
	_ledPositions.Set(_ledPositions_local);
      }
    }


    bool KryptonK600Sensor::interprete_K600_Msg(char* msg)
      {
        unsigned int index = 15;
	double* temp_double;
	unsigned short* temp_short;

	temp_short = (unsigned short*)&msg[0]; _length_msg = *temp_short;
        temp_short = (unsigned short*)&msg[2]; _type_msg = *temp_short;
        temp_short = (unsigned short*)&msg[4]; _nr_msg= *temp_short;
        temp_short = (unsigned short*)&msg[6]; _type_body_msg = *temp_short;
        _msg_valid     = msg[8];
        temp_short = (unsigned short*)&msg[9]; _type_body_msg = *temp_short;
        temp_short = (unsigned short*)&msg[11]; _cycle_nr = *temp_short;
        temp_short = (unsigned short*)&msg[13]; _nr_markers = *temp_short;


        if (_nr_markers < MAX_NUM_LEDS && _nr_markers==_num_leds){
	  for (unsigned int i = 0; i < _num_leds ; i++){
	    for (unsigned int j=0; j<3; j++){
	      temp_double = (double*)&msg[index];
	      _ledPositions_local[i](j) = *temp_double;
	      index += 8; // double is 8 bytes (chars)
	    }
	  }
	  return true;
        }
	else {
	  log(Error) << "K600PositionInterface: The Krypton system has " << _nr_markers
			<< " leds registered, but you said there are " << _num_leds << " leds" << endlog();
	  log(Error) << "K600PositionInterface: Prepare for Segfault :-)" << endlog();
	  return false;
	}
      }
#endif

}//namespace

