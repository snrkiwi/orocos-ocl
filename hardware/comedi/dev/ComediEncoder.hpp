// Copyright (C) 2003,2007 Klaas Gadeyne
//
/***************************************************************************
 *   This library is free software; you can redistribute it and/or         *
 *   modify it under the terms of the GNU Lesser General Public            *
 *   License as published by the Free Software Foundation; either          *
 *   version 2.1 of the License, or (at your option) any later version.    *
 *                                                                         *
 *   This library is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU     *
 *   Lesser General Public License for more details.                       *
 *                                                                         *
 *   You should have received a copy of the GNU Lesser General Public      *
 *   License along with this library; if not, write to the Free Software   *
 *   Foundation, Inc., 59 Temple Place,                                    *
 *   Suite 330, Boston, MA  02111-1307  USA                                *
 *                                                                         *
 ***************************************************************************/

#ifndef __COMEDI_ENCODER_INCREMENTAL_HPP
#define __COMEDI_ENCODER_INCREMENTAL_HPP


#include <rtt/dev/EncoderInterface.hpp>
#include "ComediDevice.hpp"
#include <string>

namespace OCL
{
  /**
   * A class for reading an encoder using the comedi hardware
   * abstraction layer.  Tested with the NI660X card.
   * @todo Currently this wrapper does not support all functionality.
   * It always uses X4 encoding (maximum resolution), you cannot choose
   * when to take into account the indexpulse or reset the counter
   * when the index pulse arrives.  Also see the Comedi gpct_encoder.c demo program
   * @todo subdevice locking
   * @bug upcounting is always true
   * @note The current implementation does not consider the _turn
   * parameter, since an overflow of the 32 bit register is unlikely
   * to occur in our case.
   */
  class ComediEncoder :
     public RTT::EncoderInterface
  {
  public:
    /**
     * Create a nameserved encoder.
     *
     * @param cd The comedi device your are using
     * @param subd The comedi subdevice where the COUNTER is situated.
     */
    ComediEncoder(ComediDevice * cd, unsigned int subd, const std::string& name);

    /**
     * Create an encoder.
     *
     * @param cd The comedi device your are using
     * @param subd The comedi subdevice where the COUNTER is situated.
     */
    ComediEncoder(ComediDevice * cd, unsigned int subd);

    virtual ~ComediEncoder();

    // Redefinition of Pure virtuals
    virtual int positionGet() const;
    virtual int turnGet() const;
    virtual void positionSet( int p);
    virtual void turnSet( int t );
    virtual int resolution() const;
    virtual bool upcounting() const;

  protected:
    void init();

    // Is this enough?
    ComediDevice * _myCard;
    unsigned int _subDevice;

    int _turn;
    int _resolution;
    bool _upcounting;
  };

};

#endif

