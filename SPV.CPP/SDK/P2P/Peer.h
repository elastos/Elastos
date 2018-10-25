// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#ifndef __ELASTOS_SDK_PEER_H__
#define __ELASTOS_SDK_PEER_H__

#include <deque>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <SDK/Common/Log.h>

#include "Wrapper.h"
#include "CMemBlock.h"
#include "PeerInfo.h"
#include "SDK/P2P/Message/Message.h"
#include "SDK/Common/ElementSet.h"
#include "UInt256ValueSet.h"


#define REJECT_INVALID     0x10 // transaction is invalid for some reason (invalid signature, output value > input, etc)
#define REJECT_SPENT       0x12 // an input is already spent
#define REJECT_NONSTANDARD 0x40 // not mined/relayed because it is "non-standard" (type or version unknown by server)
#define REJECT_DUST        0x41 // one or more output amounts are below the 'dust' threshold
#define REJECT_LOWFEE      0x42 // transaction does not have enough fee/priority to be relayed or mined

#ifndef MSG_NOSIGNAL   // linux based systems have a MSG_NOSIGNAL send flag, useful for supressing SIGPIPE signals
#define MSG_NOSIGNAL 0 // set to 0 if undefined (BSD has the SO_NOSIGPIPE sockopt, and windows has no signals at all)
#endif

namespace Elastos {
	namespace ElaWallet {

		class Peer;

		class PeerManager;

		typedef boost::shared_ptr<Peer> PeerPtr;

		class Peer : public boost::enable_shared_from_this<Peer> {
		public:
			enum ConnectStatus {
				Disconnected = 0,
				Connecting = 1,
				Connected = 2,
				Unknown = -2
			};

			class Listener {
			public:
				virtual void OnConnected(const PeerPtr &peer) = 0;

				virtual void OnDisconnected(const PeerPtr &peer, int error) = 0;

				virtual void
				OnRelayedPeers(const PeerPtr &peer, const std::vector<PeerInfo> &peers,
							   size_t peersCount) = 0;

				virtual void OnRelayedTx(const PeerPtr &peer, const TransactionPtr &tx) = 0;

				virtual void OnHasTx(const PeerPtr &peer, const UInt256 &txHash) = 0;

				virtual void OnRejectedTx(const PeerPtr &peer, const UInt256 &txHash, uint8_t code) = 0;

				virtual void OnRelayedBlock(const PeerPtr &peer, const MerkleBlockPtr &block) = 0;

				virtual void OnRelayedPingMsg(const PeerPtr &peer) = 0;

				virtual void
				OnNotfound(const PeerPtr &peer, const std::vector<UInt256> &txHashes,
						   const std::vector<UInt256> &blockHashes) = 0;

				virtual void OnSetFeePerKb(const PeerPtr &peer, uint64_t feePerKb) = 0;

				virtual const TransactionPtr &OnRequestedTx(const PeerPtr &peer, const UInt256 &txHash) = 0;

				virtual bool OnNetworkIsReachable(const PeerPtr &peer) = 0;

				virtual void OnThreadCleanup(const PeerPtr &peer) = 0;
			};

			typedef boost::function<void(int)> PeerCallback;

			struct UInt256Compare {
				bool operator()(const UInt256 *first, const UInt256 *second) const {
					return UInt256LessThan(first, second) == 1;
				}
			};

		public:
			Peer(PeerManager *manager, uint32_t magicNumber);

			~Peer();

			void RegisterListner(Listener *listener);

			void UnRegisterListener();

			void SendMessage(const std::string &msgType, const SendMessageParameter &parameter);

			const UInt128 &getAddress() const;

			void setAddress(const UInt128 &addr);

			uint16_t GetPort() const;

			void SetPort(uint16_t port);

			uint64_t GetTimestamp() const;

			void SetTimestamp(uint64_t timestamp);

			uint64_t GetServices() const;

			void SetServices(uint64_t services);

			uint64_t GetNonce() const;

			void SetNonce(uint64_t nonce);

			uint32_t GetEarliestKeyTime() const;

			void setEarliestKeyTime(uint32_t earliestKeyTime);

			uint32_t GetCurrentBlockHeight() const;

			void SetCurrentBlockHeight(uint32_t currentBlockHeight);

			ConnectStatus GetConnectStatus() const;

			void SetConnectStatus(ConnectStatus status);

			void Connect();

			void Disconnect();

			void SendMessage(const CMBlock &message, const std::string &type);

			void scheduleDisconnect(double time);

			bool NeedsFilterUpdate() const;

			void SetNeedsFilterUpdate(bool needsFilterUpdate);

			const std::string &getHost() const;

			uint32_t GetVersion() const;

			void SetVersion(uint32_t version);

			const std::string &getUserAgent() const;

			uint32_t GetLastBlock() const;

			void SetLastBlock(uint32_t height);

			uint64_t getFeePerKb() const;

			bool IsEqual(const Peer *peer) const;

			bool SentVerack();

			void SetSentVerack(bool sent);

			bool GotVerack();

			void SetGotVerack(bool got);

			bool SentGetaddr();

			void SetSentGetaddr(bool sent);

			bool SentFilter();

			void SetSentFilter(bool sent);

			bool SentGetdata();

			void SetSentGetdata(bool sent);

			bool SentMempool();

			void SetSentMempool(bool sent);

			bool SentGetblocks();

			void SetSentGetblocks(bool sent);

			const std::vector<UInt256> &CurrentBlockTxHashes() const;

			void AddCurrentBlockTxHash(const UInt256 &hash);

			void CurrentBlockTxHashesRemove(const UInt256 &hash);

			const std::vector<UInt256> &GetKnownBlockHashes() const;

			void KnownBlockHashesRemoveRange(size_t index, size_t len);

			void AddKnownBlockHash(const UInt256 &hash);

			const std::vector<UInt256> &KnownTxHashes() const;

			const UInt256 &LastBlockHash() const;

			void SetLastBlockHash(const UInt256 &hash);

			const UInt256ValueSet &KnownTxHashSet() const;

			void AddKnownTxHashes(const std::vector<UInt256> &txHashes);

			bool isIPv4() const;

			double GetStartTime() const;

			void SetStartTime(double time);

			double GetPingTime() const;

			void SetPingTime(double time);

			double GetDisconnectTime() const;

			void SetDisconnectTime(double time);

			void addPongCallback(const PeerCallback &callback);

			PeerCallback popPongCallback();

			const std::deque<PeerCallback> &getPongCallbacks() const;

			const PeerCallback &getMemPoolCallback() const;

			void setMempoolCallback(const PeerCallback &callback);

			void setMempoolTime(double time);

			void resetMemPool();

			PeerManager *getPeerManager() const;

			uint8_t GetFlags() const;

			void SetFlags(uint8_t flags);

			const PeerInfo &GetPeerInfo() const;

			void SetPeerInfo(const PeerInfo &info);

			void SetCurrentBlock(const MerkleBlockPtr &block);

			const MerkleBlockPtr &CurrentBlock() const;

			std::string FormatError(int errnum);

			template<typename Arg1, typename... Args>
			inline void Ptrace(const std::string &fmt, const Arg1 &arg1, const Args &... args) {
				std::string peerFmt = "{}:{} ";
				peerFmt += fmt;
				Log::trace(peerFmt.c_str(), getHost(), GetPort(), arg1, args...);
			}

			template<typename Arg1, typename... Args>
			inline void Pdebug(const std::string &fmt, const Arg1 &arg1, const Args &... args) {
				std::string peerFmt = "{}:{} ";
				peerFmt += fmt;
				Log::debug(peerFmt.c_str(), getHost(), GetPort(), arg1, args...);
			}

			template<typename Arg1, typename... Args>
			inline void Pinfo(const std::string &fmt, const Arg1 &arg1, const Args &... args) {
				std::string peerFmt = "{}:{} ";
				peerFmt += fmt;
				Log::info(peerFmt.c_str(), getHost(), GetPort(), arg1, args...);
			}

			template<typename Arg1, typename... Args>
			inline void Pwarn(const std::string &fmt, const Arg1 &arg1, const Args &... args) {
				std::string peerFmt = "{}:{} ";
				peerFmt += fmt;
				Log::warn(peerFmt.c_str(), getHost(), GetPort(), arg1, args...);
			}

			template<typename Arg1, typename... Args>
			inline void Perror(const std::string &fmt, const Arg1 &arg1, const Args &... args) {
				std::string peerFmt = "{}:{} ";
				peerFmt += fmt;
				Log::error(peerFmt.c_str(), getHost(), GetPort(), arg1, args...);
			}

			template<typename Arg1, typename... Args>
			inline void Pcritical(const std::string &fmt, const Arg1 &arg1, const Args &... args) {
				std::string peerFmt = "{}:{} ";
				peerFmt += fmt;
				Log::critical(peerFmt.c_str(), getHost(), GetPort(), arg1, args...);
			}

			template<typename T>
			inline void Ptrace(const T &msg) {
				Log::trace("{}:{} {}", getHost(), GetPort(), msg);
			}

			template<typename T>
			inline void Pdebug(const T &msg) {
				Log::debug("{}:{} {}", getHost(), GetPort(), msg);
			}

			template<typename T>
			inline void Pinfo(const T &msg) {
				Log::info("{}:{} {}", getHost(), GetPort(), msg);
			}

			template<typename T>
			inline void Pwarn(const T &msg) {
				Log::warn("{}:{} {}", getHost(), GetPort(), msg);
			}

			template<typename T>
			inline void Perror(const T &msg) {
				Log::error("{}:{} {}", getHost(), GetPort(), msg);
			}

			template<typename T>
			inline void Pcritical(const T &msg) {
				Log::critical("{}:{} {}", getHost(), GetPort(), msg);
			}

		private:
			void initDefaultMessages();

			void initSingleMessage(Message *message);

			bool networkIsReachable() const;

			bool acceptMessage(const CMBlock &msg, const std::string &type);

			int openSocket(int domain, double timeout, int *error);

			void peerThreadRoutine();

		private:
			friend class Message;

			PeerInfo _info;

			uint32_t _magicNumber;
			mutable std::string _host;
			ConnectStatus _status;
			int _waitingForNetwork;
			volatile bool _needsFilterUpdate;
			uint64_t _nonce, _feePerKb;
			std::string _useragent;
			uint32_t _version, _lastblock, _earliestKeyTime, _currentBlockHeight;
			double _startTime, _pingTime;
			volatile double _disconnectTime, _mempoolTime;
			bool _sentVerack, _gotVerack, _sentGetaddr, _sentFilter, _sentGetdata, _sentMempool, _sentGetblocks;
			UInt256 _lastBlockHash;
			MerkleBlockPtr _currentBlock;
			std::vector<UInt256> _currentBlockTxHashes, _knownBlockHashes, _knownTxHashes;
			UInt256ValueSet _knownTxHashSet;
			volatile int _socket;

			PeerCallback _mempoolCallback;
			std::deque<PeerCallback> _pongCallbackList;

			typedef boost::shared_ptr<Message> MessagePtr;
			std::map<std::string, MessagePtr> _messages;
			PeerManager *_manager;
			Listener *_listener;
		};


	}
}

#endif //__ELASTOS_SDK_PEER_H__
