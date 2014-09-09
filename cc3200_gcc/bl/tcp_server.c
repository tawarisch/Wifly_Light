/*
 Copyright (C) 2014 Nils Weiss, Patrick Bruenn.

 This file is part of WyLight.

 WyLight is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 WyLight is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICUL<AR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with WyLight.  If not, see <http://www.gnu.org/licenses/>. */

#include "string.h"

//common
#include "uart_if.h"

//WyLight
#include "bootloader.h"
#include "firmware_loader.h"

#ifndef SIMULATOR
#include "simplelink.h"
#include "wy_bl_network_if.h"
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#define SUCCESS 0
#define UART_PRINT Report
#endif /*SIMULATOR */

#define BUFFERSIZE 1024

static const unsigned short SERVER_PORT = 2000;

#ifdef SIMULATOR

static uint8_t memory[0x3FFFF];

#undef FIRMWARE_ORIGIN
#define FIRMWARE_ORIGIN ((uint8_t*)&memory[0])

#endif


/**
 * @return 0 on success, if new firmware was saved and validated
 */
int ReceiveFw(int SocketTcpChild)
{
	uint8_t *pFirmware = (uint8_t *) FIRMWARE_ORIGIN;
	UART_PRINT(" Start writing Firmware at 0x%x \r\n", pFirmware);

	for(;;) {
		int bytesReceived = recv(SocketTcpChild, pFirmware, BUFFERSIZE, 0);

		if (bytesReceived > 0) {
			// Received some bytes
			pFirmware += bytesReceived;
			UART_PRINT("Tcp: Received %d bytes\r\n", bytesReceived);
#ifdef SIMULATOR
			if (bytesReceived < BUFFERSIZE) {
    			const size_t length = (size_t) (pFirmware - FIRMWARE_ORIGIN);
				return SaveSRAMContentAsFirmware((uint8_t *) FIRMWARE_ORIGIN, length);
			}
#endif
			continue;
		}

		if (EAGAIN == bytesReceived) {
			_SlNonOsMainLoopTask();
			continue;
		}

		if (bytesReceived < 0) {
			return bytesReceived;
		}

		const size_t length = (size_t) (pFirmware - FIRMWARE_ORIGIN);
		return SaveSRAMContentAsFirmware((uint8_t *) FIRMWARE_ORIGIN, length);
	}
}

/**
 * @return if new firmware was received and validated
 */
extern void TcpServer(void)
{
	char welcome[] = "\0\0\0\0WyLightBootloader";
	uint32_t nBootloaderVersion = htonl(BOOTLOADER_VERSION);

	memcpy(welcome, &nBootloaderVersion, sizeof(uint32_t));

	struct sockaddr_in RemoteAddr;
	socklen_t RemoteAddrLen = sizeof(struct sockaddr_in);

	struct sockaddr_in LocalAddr;
	LocalAddr.sin_family = AF_INET;
	LocalAddr.sin_port = htons(SERVER_PORT);
	LocalAddr.sin_addr.s_addr = htonl(0);
	int SocketTcpServer = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (SocketTcpServer < 0) {
		UART_PRINT(" Socket Error: %d \r\n", SocketTcpServer);
		return;
	}
#ifndef SIMULATOR
	int nonBlocking = 1;
	if (SUCCESS != setsockopt(SocketTcpServer, SOL_SOCKET, SO_NONBLOCKING, &nonBlocking, sizeof(nonBlocking))) {
		UART_PRINT(" Setsockopt ERROR \r\n");
		close(SocketTcpServer);
		return;
	}
#endif /* SIMULATOR */
	if (SUCCESS != bind(SocketTcpServer, (struct sockaddr *) &LocalAddr, sizeof(LocalAddr))) {
		UART_PRINT(" Bind Error\n\r");
		close(SocketTcpServer);
		return;
	}
	// Backlog = 1 to accept maximal 1 connection
	if (SUCCESS != listen(SocketTcpServer, 1)) {
		UART_PRINT(" Listen Error\n\r");
		close(SocketTcpServer);
		return;
	}

	while (1) {
		const int SocketTcpChild = accept(SocketTcpServer, (struct sockaddr *) &RemoteAddr, &RemoteAddrLen);

		if (SocketTcpChild == EAGAIN) {
			_SlNonOsMainLoopTask();
			continue;
		} else if (SocketTcpChild < 0) {
			UART_PRINT("Error: %d occured on accept", SocketTcpChild);
			continue;
		}

		UART_PRINT(" Connected TCP Client\r\n");

		if (sizeof(welcome) !=  send(SocketTcpChild, welcome, sizeof(welcome), 0)) {
			close(SocketTcpChild);
			continue;
		}

		const int fwStatus = ReceiveFw(SocketTcpChild);
		close(SocketTcpChild);
		if (0 == fwStatus)
			return;
	}
}
