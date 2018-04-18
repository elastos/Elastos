// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PEER_H__
#define __ELASTOS_SDK_PEER_H__

#include <boost/shared_ptr.hpp>

#include "Wrapper.h"
#include "BRPeer.h"
#include "ByteData.h"

namespace Elastos {
	namespace SDK {

		class Peer :
			public Wrapper<BRPeer> {

		public:
			enum ConnectStatus {
				Disconnected = 0,
				Connecting = 1,
				Connected = 2,
				Unknown = -2
			};

			struct Status {
				Status(int v) {
					value = v;
				}

				int value;

				int getValue() {
					return value;
				}

				static ConnectStatus fromValue(int value) {
					for (int i = 0; i <= 2; i++) {
						if (i == value)
							return ConnectStatus(value);
					}
					return Unknown;
				}

				static std::string toString(ConnectStatus status) {
					if (Disconnected == status) {
						return "Disconnected";
					} else if (Connecting == status) {
						return "Connecting";
					} else if (Connected == status) {
						return "Connected";
					}

					return "Unknown";
				}
			};


		public:
			//todo complete me
			virtual std::string toString() const;

			virtual BRPeer *getRaw() const;

			Peer();

			Peer(const BRPeer &peer);

			Peer(const UInt128 &addr, uint16_t port, uint64_t timestamp);

			Peer(uint32_t magicNumber);

			~Peer();

			Peer(const Peer &peer);

			Peer& operator=(const Peer& peer);

			UInt128 getAddress() const;

			uint16_t getPort() const;

			uint64_t getTimestamp() const;

			void setEarliestKeyTime(uint32_t earliestKeyTime);

			void setCurrentBlockHeight(uint32_t currentBlockHeight);

			ConnectStatus getConnectStatusValue() const;

			void connect() const;

			void disconnect() const;

			void scheduleDisconnect(double time);

			void setNeedsFilterUpdate(bool needsFilterUpdate);

			std::string getHost() const;

			uint32_t getVersion() const;

			std::string getUserAgent() const;

			uint32_t getLastBlock() const;

			uint64_t getFeePerKb() const;

			double getPingTime() const;

		private:
			BRPeer *_peer;
		};

		typedef boost::shared_ptr<Peer> PeerPtr;

	}
}

#endif //__ELASTOS_SDK_PEER_H__
