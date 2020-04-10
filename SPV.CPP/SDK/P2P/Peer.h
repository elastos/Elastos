/*
 * Copyright (c) 2019 Elastos Foundation
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 */
#ifndef __ELASTOS_SDK_PEER_H__
#define __ELASTOS_SDK_PEER_H__

#include "PeerInfo.h"
#include "Message/Message.h"

#include <Common/Log.h>
#include <Common/ElementSet.h>
#include <Common/uint256.h>

#include <deque>
#include <boost/shared_ptr.hpp>
#include <boost/enable_shared_from_this.hpp>
#include <boost/function.hpp>
#include <sys/types.h>
#include <sys/socket.h>

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

#define time_after(a,b)  ((long)(b) - (long)(a) < 0)
#define PEER_DEBUG(p, ...) 	SPVLOG_DEBUG("{} {}:{} " __va_first(__VA_ARGS__, NULL), (p)->GetPeerManager()->GetID(), (p)->GetHost(), (p)->GetPort(), __va_rest(__VA_ARGS__, NULL))
#define PEER_INFO(p, ...) 	SPVLOG_INFO("{} {}:{} " __va_first(__VA_ARGS__, NULL), (p)->GetPeerManager()->GetID(), (p)->GetHost(), (p)->GetPort(), __va_rest(__VA_ARGS__, NULL))


		class Peer : public boost::enable_shared_from_this<Peer> {
		public:
			enum ConnectStatus {
				Disconnected = 0,
				Connecting = 1,
				Connected = 2,
			};

			class Listener {
			public:
				virtual void OnConnected(const PeerPtr &peer) = 0;

				virtual void OnDisconnected(const PeerPtr &peer, int error) = 0;

				virtual void
				OnRelayedPeers(const PeerPtr &peer, const std::vector<PeerInfo> &peers) = 0;

				virtual void OnRelayedTx(const PeerPtr &peer, const TransactionPtr &tx) = 0;

				virtual void OnHasTx(const PeerPtr &peer, const uint256 &txHash) = 0;

				virtual void OnRejectedTx(const PeerPtr &peer, const uint256 &txHash, uint8_t code, const std::string &reason) = 0;

				virtual void OnRelayedBlock(const PeerPtr &peer, const MerkleBlockPtr &block) = 0;

				virtual void OnRelayedPing(const PeerPtr &peer) = 0;

				virtual void
				OnNotfound(const PeerPtr &peer, const std::vector<uint256> &txHashes,
						   const std::vector<uint256> &blockHashes) = 0;

				virtual void OnSetFeePerKb(const PeerPtr &peer, uint64_t feePerKb) = 0;

				virtual TransactionPtr OnRequestedTx(const PeerPtr &peer, const uint256 &txHash) = 0;

				virtual bool OnNetworkIsReachable(const PeerPtr &peer) = 0;

				virtual void OnThreadCleanup(const PeerPtr &peer) = 0;
			};

			typedef boost::function<void(int)> PeerCallback;

			typedef boost::function<void(const uint256 &, int, const std::string &)> PeerPubTxCallback;

		public:
			Peer(PeerManager *manager, uint32_t magicNumber);

			void InitDefaultMessages();

			~Peer();

			void RegisterListner(Listener *listener);

			void UnRegisterListener();

			void SendMessage(const std::string &msgType, const SendMessageParameter &parameter);

			const uint128 &getAddress() const;

			void setAddress(const uint128 &addr);

			uint16_t GetPort() const;

			void SetPort(uint16_t port);

			uint64_t GetTimestamp() const;

			void SetTimestamp(uint64_t timestamp);

			time_t GetDownloadStartTime() const;

			void ScheduleDownloadStartTime();

			uint32_t GetDownloadBytes() const;

			void SetDownloadBytes(uint32_t bytes);

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

			void SendMessage(const bytes_t &message, const std::string &type);

			void RerequestBlocks(const uint256 &fromBlock);

			void ScheduleDisconnect(double time);

			bool NeedsFilterUpdate() const;

			void SetNeedsFilterUpdate(bool needsFilterUpdate);

			const std::string &GetHost() const;

			uint32_t GetVersion() const;

			void SetVersion(uint32_t version);

			const std::string &GetUserAgent() const;

			uint32_t GetLastBlock() const;

			void SetLastBlock(uint32_t height);

			uint64_t GetFeePerKb() const;

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

			bool WaitingBlocks() const;

			void SetWaitingBlocks(bool wait);

			bool SentMempool();

			void SetSentMempool(bool sent);

			bool SentGetblocks();

			void SetSentGetblocks(bool sent);

			const std::vector<uint256> &CurrentBlockTxHashes() const;

			void AddCurrentBlockTxHash(const uint256 &hash);

			void CurrentBlockTxHashesRemove(const uint256 &hash);

			const std::vector<uint256> &GetKnownBlockHashes() const;

			void KnownBlockHashesRemoveRange(size_t index, size_t len);

			void AddKnownBlockHash(const uint256 &hash);

			const std::vector<uint256> &KnownTxHashes() const;

			const uint256 &LastBlockHash() const;

			void SetLastBlockHash(const uint256 &hash);

			const std::set<uint256> &KnownTxHashSet() const;

			void AddKnownTxHashes(const std::vector<uint256> &txHashes);

			void RemoveKnownTxHashes(const std::vector<uint256> &txHashes);

			bool IsIPv4() const;

			double GetStartTime() const;

			void SetStartTime(double time);

			double GetPingTime() const;

			void SetPingTime(double time);

			double GetDisconnectTime() const;

			void SetDisconnectTime(double time);

			void AddPongCallback(const PeerCallback &callback);

			PeerCallback PopPongCallback();

			const std::deque<PeerCallback> &GetPongCallbacks() const;

			const PeerCallback &GetMemPoolCallback() const;

			void SetMempoolCallback(const PeerCallback &callback);

			void SetMempoolTime(double time);

			void ResetMemPool();

			PeerManager *GetPeerManager() const;

			uint8_t GetFlags() const;

			void SetFlags(uint8_t flags);

			const PeerInfo &GetPeerInfo() const;

			void SetPeerInfo(const PeerInfo &info);

			void SetCurrentBlock(const MerkleBlockPtr &block);

			const MerkleBlockPtr &CurrentBlock() const;

			std::string FormatError(int errnum);

			template<typename Arg1, typename... Args>
			inline void trace(const std::string &fmt, const Arg1 &arg1, const Args &... args) {
				std::string peerFmt = "{} {}:{} ";
				peerFmt += fmt;
				Log::trace(peerFmt.c_str(), _managerID, GetHost(), GetPort(), arg1, args...);
			}

			template<typename Arg1, typename... Args>
			inline void debug(const std::string &fmt, const Arg1 &arg1, const Args &... args) {
				std::string peerFmt = "{} {}:{} ";
				peerFmt += fmt;
				Log::debug(peerFmt.c_str(), _managerID, GetHost(), GetPort(), arg1, args...);
			}

			template<typename Arg1, typename... Args>
			inline void info(const std::string &fmt, const Arg1 &arg1, const Args &... args) {
				std::string peerFmt = "{} {}:{} ";
				peerFmt += fmt;
				Log::info(peerFmt.c_str(), _managerID, GetHost(), GetPort(), arg1, args...);
			}

			template<typename Arg1, typename... Args>
			inline void warn(const std::string &fmt, const Arg1 &arg1, const Args &... args) {
				std::string peerFmt = "{} {}:{} ";
				peerFmt += fmt;
				Log::warn(peerFmt.c_str(), _managerID, GetHost(), GetPort(), arg1, args...);
			}

			template<typename Arg1, typename... Args>
			inline void error(const std::string &fmt, const Arg1 &arg1, const Args &... args) {
				std::string peerFmt = "{} {}:{} ";
				peerFmt += fmt;
				Log::error(peerFmt.c_str(), _managerID, GetHost(), GetPort(), arg1, args...);
			}

			template<typename Arg1, typename... Args>
			inline void critical(const std::string &fmt, const Arg1 &arg1, const Args &... args) {
				std::string peerFmt = "{} {}:{} ";
				peerFmt += fmt;
				Log::critical(peerFmt.c_str(), _managerID, GetHost(), GetPort(), arg1, args...);
			}

			template<typename T>
			inline void trace(const T &msg) {
				Log::trace("{} {}:{} {}", _managerID, GetHost(), GetPort(), msg);
			}

			template<typename T>
			inline void debug(const T &msg) {
				Log::debug("{} {}:{} {}", _managerID, GetHost(), GetPort(), msg);
			}

			template<typename T>
			inline void info(const T &msg) {
				Log::info("{} {}:{} {}", _managerID, GetHost(), GetPort(), msg);
			}

			template<typename T>
			inline void warn(const T &msg) {
				Log::warn("{} {}:{} {}", _managerID, GetHost(), GetPort(), msg);
			}

			template<typename T>
			inline void error(const T &msg) {
				Log::error("{} {}:{} {}", _managerID, GetHost(), GetPort(), msg);
			}

			template<typename T>
			inline void critical(const T &msg) {
				Log::critical("{} {}:{} {}", _managerID, GetHost(), GetPort(), msg);
			}

		private:

			void InitSingleMessage(Message *message);

			bool NetworkIsReachable() const;

			bool AcceptMessage(const bytes_t &msg, const std::string &type);

			int OpenSocket(int domain, double timeout, int *error);

			void PeerThreadRoutine();

		private:
			friend class Message;

			PeerInfo _info;

			std::string _managerID;
			uint32_t _magicNumber;
			mutable std::string _host;
			ConnectStatus _status;
			int _waitingForNetwork;
			volatile bool _needsFilterUpdate;
			uint64_t _nonce, _feePerKb;
			std::string _useragent;
			uint32_t _version, _lastblock, _earliestKeyTime, _currentBlockHeight;
			double _startTime, _pingTime;
			uint64_t _downloadStartTime; // millisecond
			uint32_t _downloadBytes;
			volatile double _disconnectTime, _mempoolTime;
			bool _sentVerack, _gotVerack, _sentGetaddr, _sentFilter, _sentGetdata, _sentMempool, _sentGetblocks, _waitingBlocks;
			uint256 _lastBlockHash;
			MerkleBlockPtr _currentBlock;
			std::vector<uint256> _currentBlockTxHashes, _knownBlockHashes, _knownTxHashes;
			std::set<uint256> _knownTxHashSet;
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
