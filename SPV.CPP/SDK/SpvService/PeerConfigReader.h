// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PEERCONFIGREADER_H__
#define __ELASTOS_SDK_PEERCONFIGREADER_H__

#include <boost/filesystem.hpp>

#include <nlohmann/json.hpp>

#include "SharedWrapperList.h"
#include "Peer.h"

namespace Elastos {
	namespace SDK {

		class PeerConfigReader {
		public:
			SharedWrapperList<Peer, BRPeer *> readPeersFromJson(const nlohmann::json &j) const;

			SharedWrapperList<Peer, BRPeer *> readPeersFromFile(const boost::filesystem::path &path) const;

		private:
			PeerPtr convertToPeer(const nlohmann::json &peerJson, uint32_t magicNumber) const;

			UInt128 parseAddress(const std::string &address) const;

			UInt128 constructIpv4(const std::vector <uint8_t> &address) const;

			UInt128 constructIpv6(const std::vector <uint8_t> &address) const;
		};

	}
}

#endif //__ELASTOS_SDK_PEERCONFIGREADER_H__
