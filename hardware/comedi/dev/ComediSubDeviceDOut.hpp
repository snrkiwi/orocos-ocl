// $Id: ComediSubDeviceDOut.hpp,v 1.4 2003/08/14 09:17:13 kgadeyne Exp $
// Copyright (C) 2003 Klaas Gadeyne

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


/* Based completely on ComediSubDeviceDin.hpp,  basically I didn't do
   anything but M-x query-replace [Enter] in [Enter] out :-)
   And changed the api to that of DigitalOutInterface offcourse.
   And the implementation: M-x query-replace [Enter] read [Enter] write :-)
   And some other things...
   And ...
*/

#ifndef COMEDISUBDEVICEDOUT_HPP
#define COMEDISUBDEVICEDOUT_HPP

#include <rtt/dev/DigitalOutInterface.hpp>
#include "ComediDevice.hpp"

namespace OCL
{

  /**
   * This logical device represents one subdevice of a Comedi device.
   */
  class ComediSubDeviceDOut
    : public RTT::DigitalOutInterface
  {

  public:
    /**
     * Create a new ComediSubDeviceDOut with a given ComediDevice, subdevice number and
     * a name for this sub device
     * The standard constructors assumes the whole subdevice is already configured for output.
     * For DIO (reconfigurable IO devices) use the extra constructor argument to specify
     * that this should not be assumed and use the useBit() method to indicate which bits
     * should be used as output bits.
     *
     * @param cd The ComediDevice to use for output
     * @param subdevice The subdevice number for this comedi device
     * @param name The name of this instance
     * @param configure_all_bits Set to false to not configure all bits of this subdevice as outputs.
     * @see useBit() for configuring bit-per-bit.
     */
    ComediSubDeviceDOut( ComediDevice* cd, const std::string& name, unsigned int subdevice, bool configure_all_bits = true);

    ComediSubDeviceDOut( ComediDevice* cd, unsigned int subdevice, bool configure_all_bits = true );

    /**
     * If \a configure_all_bits was false, use this method (for each bit) to specify
     * which bits must be configured as input.
     */
    bool useBit( unsigned int bit );

    virtual void switchOn( unsigned int bit);

    virtual void switchOff( unsigned int bit);

    virtual void setBit( unsigned int bit, bool value);

    virtual void setSequence(unsigned int start_bit, unsigned int stop_bit, unsigned int value);

    virtual bool checkBit( unsigned int bit) const;

    virtual unsigned int checkSequence (unsigned int start_bit, unsigned int stop_bit) const;

    virtual unsigned int nbOfOutputs() const;

  protected:
    void init(bool all_bits);

    /**
     * The output device to write to
     */
    ComediDevice* myCard;

    /**
     * The subdevice number of this instance in \a myCard
     */
    unsigned int subDevice;
    unsigned int channels;
  };

}

#endif
