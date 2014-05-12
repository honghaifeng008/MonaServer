/*
Copyright 2014 Mona
mathieu.poux[a]gmail.com
jammetthomas[a]gmail.com

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License received along this program for more
details (or else see http://www.gnu.org/licenses/).

This file is a part of Mona.
*/

#pragma once

#include "Mona/TCPClient.h"
#include "Mona/TCPSender.h"
#include "Mona/AMFWriter.h"
#include "Mona/PacketReader.h"
#include "Mona/MapParameters.h"
#include "Mona/Event.h"

class ServerMessage : public Mona::TCPSender,public Mona::AMFWriter {
	friend class ServerConnection;
public:
	ServerMessage(const char* handler, const Mona::PoolBuffers& poolBuffers) : _shift(295), _handler(handler), AMFWriter(poolBuffers), TCPSender("ServerMessage") {
		packet.next(300);
	}
	
private:
	std::string		_handler;
	Mona::UInt16	_shift;

	const Mona::UInt8*	data() { return packet.data()+_shift; }
	Mona::UInt32		size() { return packet.size()-_shift; }
};


class ServerConnection;
namespace ServerEvents {
	struct OnHello : Mona::Event<void(ServerConnection&)> {};
	struct OnMessage : Mona::Event<void(ServerConnection&,const std::string&,Mona::PacketReader&)> {};
	struct OnDisconnection : Mona::Event<void(Mona::Exception&,ServerConnection&)> {};
}

class ServerConnection : public Mona::MapParameters,
		public ServerEvents::OnHello,
		public ServerEvents::OnMessage,
		public ServerEvents::OnDisconnection {
public:
	// Target version
	ServerConnection(const Mona::SocketAddress& address,const Mona::SocketManager& manager);
	// Initiator version
	ServerConnection(const Mona::SocketAddress& address, Mona::SocketFile& file, const Mona::SocketManager& manager);

	~ServerConnection();

	const Mona::SocketAddress	address;
	const bool					isTarget;
	Mona::MapParameters			properties;

	const Mona::PoolBuffers&  poolBuffers() const { return _pClient->manager().poolBuffers; }

	void			connect(const Mona::Parameters& configs);
	bool			connected() { return _connected; }

	void			close();

	void			send(const std::shared_ptr<ServerMessage>& pMessage);
	void			sendHello(const Mona::Parameters& configs);
	void			reject(const char* error);

private:

	Mona::UInt32	onData(Mona::PoolBuffer& pBuffer);
	void			onDisconnection();

	Mona::TCPClient::OnError::Type			_onError;
	Mona::TCPClient::OnData::Type			_onData;
	Mona::TCPClient::OnDisconnection::Type	_onDisconnection;


	std::map<std::string,Mona::UInt32>	_sendingRefs;
	std::map<Mona::UInt32,std::string>	_receivingRefs;

	bool								_connected;
	Mona::Exception						_ex;
	std::unique_ptr<Mona::TCPClient>	_pClient;
};
