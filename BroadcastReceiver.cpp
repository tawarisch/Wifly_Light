/**
 Copyright (C) 2012 Nils Weiss, Patrick Bruenn.
 
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

#include "BroadcastReceiver.h"

#include <cstring>
#include <iostream>
#include <stdio.h>
#include <sstream>

const char BroadcastReceiver::BROADCAST_MSG[] = "WiFly_Light";
const size_t BroadcastReceiver::BROADCAST_MSG_LENGTH = sizeof(BROADCAST_MSG);
const char BroadcastReceiver::STOP_MSG[] = "StopThread";
const size_t BroadcastReceiver::STOP_MSG_LENGTH = sizeof(STOP_MSG);

BroadcastReceiver::BroadcastReceiver(unsigned short port) : mPort(port), mThread(boost::ref(*this))
{
}

BroadcastReceiver::~BroadcastReceiver(void)
{
	Stop();
}

void BroadcastReceiver::operator()(void)
{
	boost::asio::io_service io_service;
	udp::socket sock(io_service, udp::endpoint(udp::v4(), mPort));
	
	for(;;)
	{
		char inputBuffer[32];
		udp::endpoint remoteEndpoint;
		size_t bytesRead = sock.receive_from(boost::asio::buffer(inputBuffer, sizeof(inputBuffer)), remoteEndpoint);

		// received a Wifly broadcast?
		if((BROADCAST_MSG_LENGTH == bytesRead) && (0 == memcmp(inputBuffer, BROADCAST_MSG, bytesRead)))
		{
			mMutex.lock();
			//TODO read address from endpoint!
			mIpTable.insert(std::pair<unsigned long, std::string>(0x7f000001, std::string("127.0.0.1")));
			mMutex.unlock();
		}
		// received a stop event?
		else
		{
			if((STOP_MSG_LENGTH == bytesRead) && (0 == memcmp(inputBuffer, STOP_MSG, bytesRead))
						/*TODO && (remoteEndpoint.isLocalhost())*/)
		{
			return;
		}}
	}
}

size_t BroadcastReceiver::GetIpTable(std::vector<std::string>& outputVector)
{
	size_t numElements = 0;
	std::map<unsigned long, std::string>::const_iterator it;
	mMutex.lock();
	for(it = mIpTable.begin(); it != mIpTable.end(); it++)
	{
		outputVector.push_back((*it).second);
		numElements++;
	}
	mMutex.unlock();
	return numElements;
}

void BroadcastReceiver::Stop(void)
{
	try {
		std::ostringstream converter;
		converter << mPort;
		boost::asio::io_service io_service;
		udp::socket sock(io_service, udp::endpoint(udp::v4(), 0));
		udp::resolver resolver(io_service);
		udp::resolver::query query(udp::v4(), "127.0.0.1", converter.str());
		udp::resolver::iterator it = resolver.resolve(query);
		sock.send_to(boost::asio::buffer(STOP_MSG, STOP_MSG_LENGTH), *it);
		mThread.join();
	} catch (std::exception& e) {
		std::cout << __FUNCTION__ << ':' <<  e.what() << '\n';
	}
}

