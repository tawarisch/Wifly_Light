/*
                Copyright (C) 2012, 2013 Nils Weiss, Patrick Bruenn.

    This file is part of Wifly_Light.

    Wifly_Light is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    Wifly_Light is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with Wifly_Light.  If not, see <http://www.gnu.org/licenses/>. */

#ifndef _WIFLYCONTROL_H_
#define _WIFLYCONTROL_H_

#include <string>
#include "ComProxy.h"
#include "wifly_cmd.h"
#include "TelnetProxy.h"
#include "WiflyControlException.h"
#include "ConfigControl.h"
#include "BootloaderControl.h"
#include "FirmwareControl.h"

namespace WyLight
{
/******************************************************************************/
/*! \file WiflyControl.h
 * \author Nils Weiss, Patrick Bruenn
 *
 *! \cond
 * class - Control
 * \endcond
 *
 * \brief Class to communicate with a Wifly_Light Hardware.
 *
 *
 * The Control class allows the user to control the Wifly_Light Hardware.
 * There are three target's at the Wifly_Light Hardware.
 * - Bootloader<br>
 *           All methodes with Bl* relate to the bootloader part.
 * - Firmware<br>
 *           All methodes with Fw* relate to the firmware part.
 * - RN-171 Wifi Interface<br>
 *           All methodes witch Conf* relate to the communication module.
 *******************************************************************************/
class Control {
public:
    /*
     * Send a byte sequence to ident the current software running on PIC
     * @return mode of target: BL_IDENT for Bootloader mode, FW_IDENT for Firmware mode
     * @throw FatalError if synchronisation fails
     */
    virtual size_t GetTargetMode(void) const;

    /* ------------------------- VERSION EXTRACT METHODE ------------------------- */
    /**
     * Methode to extract the firmware version from a hex file
     * @return the version string from a given hex file
     */
    virtual uint16_t ExtractFwVersion(const std::string& pFilename) const;

    const BootloaderControl& mBootloader;
    const ConfigControl& mConfig;
    const FirmwareControl& mFirmware;

/* ------------------------- PRIVATE DECLARATIONS ------------------------- */
protected:
    Control(uint32_t                 addr,
            uint16_t                 port,
            const BootloaderControl& bootloader,
            const ConfigControl&     config,
            const FirmwareControl&   firmware);

    /*
     * Sockets used for communication with wifly device.
     * A reference to the TcpSocket is provided to the aggregated subobjects.
     */
    const TcpSocket mTcpSock;

    /**
     * The UdpSocket is used directly in WiflyControl, to send fast connectionless packets.
     */
    const UdpSocket mUdpSock;

    /**
     * Proxy object handling the low level communication with bootloader and firmware.
     */
    const ComProxy mProxy;

    /**
     * Proxy object handling the communication with the wlan module for its configuration
     */
    const TelnetProxy mTelnet;
};

class RN171Control : public Control {
public:
    RN171Control(uint32_t addr, uint16_t port);

private:
    const BootloaderControl mBootloaderInstance;
    const ConfigControl mConfigInstance;
    const RN171FirmwareControl mFirmwareInstance;
};

class CC3200Control : public Control {
public:
    CC3200Control(uint32_t addr, uint16_t port);

private:
    const BootloaderControl mBootloaderInstance;
    const ConfigControl mConfigInstance;
    const CC3200FirmwareControl mFirmwareInstance;
};
}
#endif /* #ifndef _WIFLYCONTROL_H_ */
