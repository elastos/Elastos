// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <fstream>
#include <Core/BRPeer.h>
#include "Core/BRPeer.h"

#include "PeerConfigReader.h"

namespace Elastos {
	namespace ElaWallet {

		SharedWrapperList<Peer, BRPeer *> PeerConfigReader::readPeersFromJson(const nlohmann::json &j) const {
			SharedWrapperList<Peer, BRPeer *> result;

			uint32_t magicNumber = j["MagicNumber"].get<uint32_t>();
			std::vector<nlohmann::json> knowingPeers = j["KnowingPeers"];
			for (std::vector<nlohmann::json>::iterator it = knowingPeers.begin(); it != knowingPeers.end(); ++it) {
				result.push_back(convertToPeer(*it, magicNumber));
			}

			return result;
		}

		SharedWrapperList<Peer, BRPeer *> PeerConfigReader::readPeersFromFile(const boost::filesystem::path &path) const {

			if (!boost::filesystem::exists(path))
				return SharedWrapperList<Peer, BRPeer *>();

			std::ifstream i(path.string());
			nlohmann::json j;
			i >> j;

			return readPeersFromJson(j);
		}

		PeerPtr PeerConfigReader::convertToPeer(const nlohmann::json &peerJson, uint32_t magicNumber) const {
			PeerPtr result = PeerPtr(new Peer(magicNumber));
			BRPeer *peer = result->getRaw();
			peer->address = parseAddress(peerJson["Address"].get<std::string>());
			peer->port = peerJson["Port"].get<uint16_t>();
			peer->timestamp = peerJson["Timestamp"].get<uint64_t>();
			peer->services = peerJson["Services"].get<uint64_t>();
			peer->flags = peerJson["Flags"].get<uint8_t>();

			peer->timestamp = (uint64_t)time(nullptr);
			return result;
		}

		UInt128 PeerConfigReader::parseAddress(const std::string &address) const {
			UInt128 result;
			std::istringstream f(address);
			std::vector<uint8_t> addressBytes;
			std::string s;
			while (getline(f, s, '.')) {
				addressBytes.push_back(uint8_t(std::stoul(s)));
			}

			return addressBytes.size() == 4 ? constructIpv4(addressBytes) : constructIpv6(addressBytes);
		}

		UInt128 PeerConfigReader::constructIpv4(const std::vector<uint8_t> &address) const {
			UInt128 result;
			memset(result.u64, 0, 2 * sizeof(uint64_t));
			result.u8[10] = 0xff;
			result.u8[11] = 0xff;
			for (int i = 0; i < address.size(); ++i) {
				result.u8[i + 12] = address[i];
			}
			return result;
		}

		UInt128 PeerConfigReader::constructIpv6(const std::vector<uint8_t> &address) const {
			UInt128 result;
			//todo complete me
			return result;
		}
	}
}