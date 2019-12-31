// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "PeerInfo.h"

#include <stddef.h>
#include <netinet/in.h>
#include <arpa/inet.h>

namespace Elastos {
	namespace ElaWallet {

		const uint32_t DEFAULT_MAGICNUMBER = uint32_t(0);

		PeerInfo::PeerInfo() :
				Port(0),
				Timestamp(0),
				Services(SERVICES_NODE_NETWORK | SERVICES_NODE_BLOOM),
				Flags(0) {

		}

		PeerInfo::PeerInfo(const uint128 &addr, uint16_t port, uint64_t timestamp) :
				Port(port),
				Timestamp(timestamp),
				Address(addr),
				Services(SERVICES_NODE_NETWORK | SERVICES_NODE_BLOOM),
				Flags(0) {
		}

		PeerInfo::PeerInfo(const uint128 &addr, uint16_t port, uint64_t timestamp, uint64_t services) :
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
			return (Address == info.Address &&
					Port == info.Port);
		}

		bool PeerInfo::operator<(const PeerInfo &info) const {
			if (Address < info.Address) {
				return true;
			} else if (Address > info.Address) {
				return false;
			} else {
				return Port < info.Port;
			}
		}

		bool PeerInfo::operator!=(const PeerInfo &info) const {
			return !operator==(info);
		}

		size_t PeerInfo::GetHash() const {
			uint32_t address = Address.Get32(3), port = Port;

			// (((FNV_OFFSET xor address)*FNV_PRIME) xor port)*FNV_PRIME
			return (size_t) ((((0x811C9dc5 ^ address) * 0x01000193) ^ port) * 0x01000193);
		}

		bool PeerInfo::IsIPv4() const {
			return (Address.Get64(0) == 0 && Address.Get32(2) == 0xffff0000);
		}

		std::string PeerInfo::GetHost() const {
			char temp[INET6_ADDRSTRLEN];
			if (IsIPv4()) {
				inet_ntop(AF_INET, Address.end() - 4 , temp, sizeof(temp));
			} else {
				inet_ntop(AF_INET6, &Address, temp, sizeof(temp));
			}

			return std::string(temp);
		}

	}
}
