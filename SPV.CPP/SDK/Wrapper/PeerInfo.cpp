// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PeerInfo.h"

namespace Elastos {
	namespace ElaWallet {

		const uint32_t DEFAULT_MAGICNUMBER = uint32_t(0);

		PeerInfo::PeerInfo() :
				Port(0),
				Timestamp(0),
				Services(SERVICES_NODE_NETWORK),
				Flags(0) {

		}

		PeerInfo::PeerInfo(const UInt128 &addr, uint16_t port, uint64_t timestamp) :
				Port(port),
				Timestamp(timestamp),
				Address(addr),
				Services(SERVICES_NODE_NETWORK),
				Flags(0) {
		}

		PeerInfo::PeerInfo(const UInt128 &addr, uint16_t port, uint64_t timestamp, uint64_t services) :
				Port(port),
				Timestamp(timestamp),
				Services(services),
				Address(addr),
				Flags(0) {
		}

		PeerInfo::PeerInfo(const PeerInfo &peerInfo) {
			operator=(peerInfo);
		}

		PeerInfo &PeerInfo::operator=(const PeerInfo &peerInfo) {
			Timestamp = peerInfo.Timestamp;
			Address = peerInfo.Address;
			Port = peerInfo.Port;
			Flags = peerInfo.Flags;
			Services = peerInfo.Services;

			return *this;
		}

		bool PeerInfo::operator==(const PeerInfo &info) const {
			return (UInt128Eq(&Address, &info.Address) &&
					 Port == info.Port);
		}

		bool PeerInfo::operator!=(const PeerInfo &info) const {
			return !operator==(info);
		}
	}
}
