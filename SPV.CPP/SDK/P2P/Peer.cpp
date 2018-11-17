// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <float.h>
#include <sys/time.h>
#include <SDK/Common/Log.h>
#include <arpa/inet.h>
#include <boost/thread.hpp>
#include <CMemBlock.h>
#include <SDK/Common/Utils.h>
#include <SDK/P2P/Message/PingMessage.h>
#include <SDK/P2P/Message/VersionMessage.h>
#include <SDK/P2P/Message/VerackMessage.h>
#include <SDK/P2P/Message/AddressMessage.h>
#include <SDK/P2P/Message/InventoryMessage.h>
#include <SDK/P2P/Message/GetDataMessage.h>
#include <SDK/P2P/Message/NotFoundMessage.h>
#include <SDK/P2P/Message/GetBlocksMessage.h>
#include <SDK/P2P/Message/GetHeadersMessage.h>
#include <SDK/P2P/Message/TransactionMessage.h>
#include <SDK/P2P/Message/MerkleBlockMessage.h>
#include <SDK/P2P/Message/MempoolMessage.h>
#include <SDK/P2P/Message/PongMessage.h>
#include <SDK/P2P/Message/BloomFilterMessage.h>
#include <SDK/P2P/Message/GetAddressMessage.h>
#include <SDK/P2P/Message/HeadersMessage.h>

#include "Peer.h"
#include "PeerManager.h"

#define HEADER_LENGTH      24
#define MAX_MSG_LENGTH     0x02000000
#define MIN_PROTO_VERSION  70002 // peers earlier than this protocol version not supported (need v0.9 txFee relay rules)
#define LOCAL_HOST         ((UInt128) { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x01 })
#define CONNECT_TIMEOUT    3.0
#define MESSAGE_TIMEOUT    10.0

#define PTHREAD_STACK_SIZE  (512 * 1024)

namespace Elastos {
	namespace ElaWallet {

		Peer::Peer(PeerManager *manager, uint32_t magicNumber) :
				_status(Disconnected),
				_magicNumber(magicNumber),
				_pingTime(DBL_MAX),
				_mempoolTime(DBL_MAX),
				_disconnectTime(DBL_MAX),
				_manager(manager),
				_socket(-1),
				_waitingForNetwork(0),
				_needsFilterUpdate(false),
				_nonce(0),
				_feePerKb(0),
				_sentVerack(false),
				_gotVerack(false),
				_sentGetaddr(false),
				_sentFilter(false),
				_sentGetdata(false),
				_sentMempool(false),
				_sentGetblocks(false),
				_version(0),
				_lastblock(0),
				_earliestKeyTime(0),
				_currentBlockHeight(0),
				_startTime(0) {

			RegisterListner(_manager);
		}

		Peer::~Peer() {
		}

		const UInt128 &Peer::getAddress() const {
			return _info.Address;
		}

		void Peer::setAddress(const UInt128 &addr) {
			_info.Address = addr;
		}

		uint16_t Peer::GetPort() const {
			return _info.Port;
		}

		void Peer::SetPort(uint16_t port) {
			_info.Port = port;
		}

		uint64_t Peer::GetTimestamp() const {
			return _info.Timestamp;
		}

		void Peer::SetTimestamp(uint64_t timestamp) {
			_info.Timestamp = timestamp;
		}

		uint64_t Peer::GetServices() const {
			return _info.Services;
		}

		void Peer::SetServices(uint64_t services) {
			_info.Services = services;
		}

		uint64_t Peer::GetNonce() const {
			return _nonce;
		}

		void Peer::SetNonce(uint64_t nonce) {
			_nonce = nonce;
		}

		uint32_t Peer::GetEarliestKeyTime() const {
			return _earliestKeyTime;
		}

		void Peer::setEarliestKeyTime(uint32_t earliestKeyTime) {
			_earliestKeyTime = earliestKeyTime;
		}

		uint32_t Peer::GetCurrentBlockHeight() const {
			return _currentBlockHeight;
		}

		void Peer::SetCurrentBlockHeight(uint32_t currentBlockHeight) {
			_currentBlockHeight = currentBlockHeight;
		}

		Peer::ConnectStatus Peer::GetConnectStatus() const {
			return _status;
		}

		void Peer::SetConnectStatus(ConnectStatus status) {
			_status = status;
		}

		void Peer::Connect() {
			struct timeval tv;
			int error = 0;
			pthread_attr_t attr;

			if (_status == Peer::Disconnected || _waitingForNetwork) {
				_status = Peer::Connecting;

				if (0 && networkIsReachable()) { // delay until network is reachable
					if (!_waitingForNetwork) Pinfo("waiting for network reachability");
					_waitingForNetwork = 1;
				} else {
					Pinfo("connecting");
					_waitingForNetwork = 0;
					gettimeofday(&tv, NULL);
					_disconnectTime = tv.tv_sec + (double) tv.tv_usec / 1000000 + CONNECT_TIMEOUT;

//					if (pthread_attr_init(&attr) != 0) {
//						error = ENOMEM;
//						Log::traceWithTime("error creating thread");
//						status = BRPeerStatusDisconnected;
//						if (_listener) _listener->OnDisconnected(error);
//					} else if (pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED) != 0 ||
//							   pthread_attr_setstacksize(&attr, PTHREAD_STACK_SIZE) != 0 ||
//							   pthread_create(&_thread, &attr, &Peer::peerThreadRoutine, this) != 0) {
//						error = EAGAIN;
//						Log::traceWithTime("error creating thread");
//						pthread_attr_destroy(&attr);
//						status = BRPeerStatusDisconnected;
//						if (_listener) _listener->OnDisconnected(error);
//					}
					boost::thread workThread(boost::bind(&Peer::peerThreadRoutine, this));
				}
			}
		}

		void Peer::Disconnect() {
			int socket = _socket;

			if (socket >= 0) {
				_socket = -1;
				if (shutdown(socket, SHUT_RDWR) < 0) {
					Perror("peer shutdown error: {}", FormatError(errno));
				}
				close(socket);
			}
		}

		// sends a bitcoin protocol message to peer
		void Peer::SendMessage(const CMBlock &message, const std::string &type) {
			if (message.GetSize() > MAX_MSG_LENGTH) {
				Perror("failed to send {}, length {} is too long", type, message.GetSize());
			} else {
				uint8_t buf[HEADER_LENGTH + message.GetSize()], hash[32];
				size_t off = 0;
				ssize_t n = 0;
				struct timeval tv;
				int socket, error = 0;

				UInt32SetLE(&buf[off], _magicNumber);
				off += sizeof(uint32_t);
				strncpy((char *) &buf[off], type.c_str(), 12);
				off += 12;
				UInt32SetLE(&buf[off], (uint32_t) message.GetSize());
				off += sizeof(uint32_t);
				BRSHA256_2(hash, message, message.GetSize());
				memcpy(&buf[off], hash, sizeof(uint32_t));
				off += sizeof(uint32_t);
				memcpy(&buf[off], message, message.GetSize());

				size_t msgLen = 0;
				socket = _socket;
				if (socket < 0) error = ENOTCONN;

				while (socket >= 0 && !error && msgLen < sizeof(buf)) {
					n = send(socket, &buf[msgLen], sizeof(buf) - msgLen, MSG_NOSIGNAL);
					if (n >= 0) {
						msgLen += n;
					}
					if (n < 0 && errno != EWOULDBLOCK) error = errno;
					gettimeofday(&tv, NULL);
					if (!error && tv.tv_sec + (double) tv.tv_usec / 1000000 >= _disconnectTime) error = ETIMEDOUT;
					socket = _socket;
				}

				if (error) {
					Perror("sending {} message {}", type, FormatError(error));
					Disconnect();
				}
			}
		}

		void Peer::scheduleDisconnect(double seconds) {
			struct timeval tv;

			gettimeofday(&tv, NULL);
			_disconnectTime = (seconds < 0) ? DBL_MAX : tv.tv_sec + (double) tv.tv_usec / 1000000 + seconds;
		}

		bool Peer::NeedsFilterUpdate() const {
			return _needsFilterUpdate;
		}

		void Peer::SetNeedsFilterUpdate(bool needsFilterUpdate) {
			_needsFilterUpdate = needsFilterUpdate;
		}

		const std::string &Peer::getHost() const {
			if (_host.empty()) {
				char temp[INET6_ADDRSTRLEN];
				if (isIPv4()) {
					inet_ntop(AF_INET, &_info.Address.u32[3], temp, sizeof(temp));
				} else inet_ntop(AF_INET6, &_info.Address, temp, sizeof(temp));

				_host = temp;
			}

			return _host;
		}

		uint32_t Peer::GetVersion() const {
			return _version;
		}

		void Peer::SetVersion(uint32_t version) {
			_version = version;
		}

		const std::string &Peer::getUserAgent() const {
			return _useragent;
		}

		uint32_t Peer::GetLastBlock() const {
			return _lastblock;
		}

		void Peer::SetLastBlock(uint32_t height) {
			_lastblock = height;
		}

		uint64_t Peer::getFeePerKb() const {
			return _feePerKb;
		}

		bool Peer::IsEqual(const Peer *otherPeer) const {
			return (this == otherPeer || _info == otherPeer->GetPeerInfo());
		}

		bool Peer::networkIsReachable() const {
			//fixme [refactor]
			return true;
		}

		bool Peer::isIPv4() const {
			return (_info.Address.u64[0] == 0 && _info.Address.u16[4] == 0 && _info.Address.u16[5] == 0xffff);
		}

		void Peer::peerThreadRoutine() {

			int socket, error = 0;

			if (openSocket(PF_INET6, CONNECT_TIMEOUT, &error)) {
				struct timeval tv;
				double time = 0, msgTimeout;
				uint8_t header[HEADER_LENGTH];
				CMBlock payload;
				size_t len = 0;
				ssize_t n = 0;

				gettimeofday(&tv, NULL);
				_startTime = tv.tv_sec + (double) tv.tv_usec / 1000000;
				SendMessage(MSG_VERSION, Message::DefaultParam);

				while (_socket >= 0 && !error) {
					len = 0;
					socket = _socket;

					while (socket >= 0 && !error && len < HEADER_LENGTH) {
						n = read(socket, &header[len], sizeof(header) - len);
						if (n > 0) len += n;
						if (n == 0)
							error = ECONNRESET;
						if (n < 0 && errno != EWOULDBLOCK) {
							error = errno;
							if (error == EINTR) {
								error = 0;
								continue;
							}
						}

						gettimeofday(&tv, NULL);
						time = tv.tv_sec + (double) tv.tv_usec / 1000000;
						if (!error && time >= _disconnectTime) error = ETIMEDOUT;

						if (!error && time >= _mempoolTime) {
							Pinfo("done waiting for mempool response");
							PingParameter pingParameter;
							pingParameter.callback = _mempoolCallback;
							SendMessage(MSG_PING, pingParameter);
							_mempoolCallback = PeerCallback();
							_mempoolTime = DBL_MAX;
						}

						while (sizeof(uint32_t) <= len && UInt32GetLE(header) != _magicNumber) {
							memmove(header, &header[1],
									--len); // consume one byte at a time until we find the magic number
						}

						socket = _socket;
					}

					if (error) {
						Perror("read socket error: {}", FormatError(error));
					} else if (header[15] != 0) { // verify header type field is NULL terminated
						Perror("malformed message header: type not NULL terminated");
						error = EPROTO;
					} else if (len == HEADER_LENGTH) {
						std::string type = (const char *) (&header[4]);
						uint32_t msgLen = UInt32GetLE(&header[16]);
						uint32_t checksum = UInt32GetLE(&header[20]);
						UInt256 hash;

						if (msgLen > MAX_MSG_LENGTH) { // check message length
							Perror("error reading {}, message length {} is too long", type, msgLen);
							error = EPROTO;
						} else {
							payload.Resize(size_t(msgLen));
							len = 0;
							socket = _socket;
							msgTimeout = time + MESSAGE_TIMEOUT;

							while (socket >= 0 && !error && len < msgLen) {
								n = read(socket, &payload[len], msgLen - len);
								if (n > 0) len += n;
								if (n == 0)
									error = ECONNRESET;
								if (n < 0 && errno != EWOULDBLOCK) {
									error = errno;
									if (error == EINTR) {
										error = 0;
										continue;
									}
								}

								gettimeofday(&tv, NULL);
								time = tv.tv_sec + (double) tv.tv_usec / 1000000;
								if (n > 0) msgTimeout = time + MESSAGE_TIMEOUT;
								if (!error && time >= msgTimeout)
									error = ETIMEDOUT;
								socket = _socket;
							}

							if (error) {
								Perror("read socket error: {}", FormatError(error));
							} else if (len == msgLen) {
								BRSHA256_2(&hash, payload, msgLen);

								if (UInt32GetLE(&hash) != checksum) { // verify checksum
									Perror("reading {}, invalid checksum {:x}, expected {:x}, payload length:{},"
										   " SHA256_2: {}", type, UInt32GetLE(&hash), checksum, msgLen,
										   Utils::UInt256ToString(hash));
									error = EPROTO;
								} else if (!acceptMessage(payload, type)) error = EPROTO;
							}
						}
					}
				}
			}

			socket = _socket;
			_socket = -1;
			_status = Peer::Disconnected;
			if (socket >= 0) close(socket);
			Pinfo("disconnected");

			while (!_pongCallbackList.empty()) {
				Peer::PeerCallback pongCallback = popPongCallback();
				if (pongCallback) pongCallback(0);
			}

			if (!_mempoolCallback.empty()) _mempoolCallback(0);
			_mempoolCallback = PeerCallback();
			if (_listener) _listener->OnDisconnected(shared_from_this(), error);
		}

		void Peer::RegisterListner(Peer::Listener *listener) {
			_listener = listener;
		}

		void Peer::UnRegisterListener() {
			_listener = nullptr;
		}

		void Peer::initDefaultMessages() {
			initSingleMessage(new VersionMessage(shared_from_this()));
			initSingleMessage(new VerackMessage(shared_from_this()));
			initSingleMessage(new AddressMessage(shared_from_this()));
			initSingleMessage(new InventoryMessage(shared_from_this()));
			initSingleMessage(new GetDataMessage(shared_from_this()));
			initSingleMessage(new NotFoundMessage(shared_from_this()));
			initSingleMessage(new GetBlocksMessage(shared_from_this()));
			initSingleMessage(new GetHeadersMessage(shared_from_this()));
			initSingleMessage(new TransactionMessage(shared_from_this()));
			initSingleMessage(new HeadersMessage(shared_from_this()));
			initSingleMessage(new MempoolMessage(shared_from_this()));
			initSingleMessage(new PingMessage(shared_from_this()));
			initSingleMessage(new PongMessage(shared_from_this()));
			initSingleMessage(new BloomFilterMessage(shared_from_this()));
			initSingleMessage(new MerkleBlockMessage(shared_from_this()));
			initSingleMessage(new GetAddressMessage(shared_from_this()));
		}

		bool Peer::acceptMessage(const CMBlock &msg, const std::string &type) {
			bool r = false;

			if (_currentBlock != nullptr && MSG_TX == type) { // if we receive a non-tx message, merkleblock is done
				Perror("incomplete merkleblock {}, expected {} more tx, got {}",
					   Utils::UInt256ToString(_currentBlock->getHash()),
					   _currentBlockTxHashes.size(), type);
				_currentBlockTxHashes.clear();
				_currentBlock.reset();
				r = 0;
			} else if (_messages.find(type) != _messages.end())
				r = _messages[type]->Accept(msg);
			else Perror("dropping {}, length {}, not implemented", type, msg.GetSize());

			return r;
		}

		bool Peer::SentVerack() {
			return _sentVerack;
		}

		void Peer::SetSentVerack(bool sent) {
			_sentVerack = sent;
		}

		bool Peer::GotVerack() {
			return _gotVerack;
		}

		void Peer::SetGotVerack(bool got) {
			_gotVerack = got;
		}

		bool Peer::SentGetaddr() {
			return _sentGetaddr;
		}

		void Peer::SetSentGetaddr(bool sent) {
			_sentGetaddr = sent;
		}

		bool Peer::SentFilter() {
			return _sentFilter;
		}

		void Peer::SetSentFilter(bool sent) {
			_sentFilter = sent;
		}

		bool Peer::SentGetdata() {
			return _sentGetdata;
		}

		void Peer::SetSentGetdata(bool sent) {
			_sentGetdata = sent;
		}

		bool Peer::SentMempool() {
			return _sentMempool;
		}

		void Peer::SetSentMempool(bool sent) {
			_sentMempool = sent;
		}

		bool Peer::SentGetblocks() {
			return _sentGetblocks;
		}

		void Peer::SetSentGetblocks(bool sent) {
			_sentGetblocks = sent;
		}

		const std::vector<UInt256> &Peer::CurrentBlockTxHashes() const {
			return _currentBlockTxHashes;
		}

		void Peer::AddCurrentBlockTxHash(const UInt256 &hash) {
			_currentBlockTxHashes.push_back(hash);
		}

		void Peer::CurrentBlockTxHashesRemove(const UInt256 &hash) {
			for (size_t i = 0; i < _currentBlockTxHashes.size(); ++i) {
				if (UInt256Eq(&hash, &_currentBlockTxHashes[i])) {
					_currentBlockTxHashes.erase(_currentBlockTxHashes.begin() + i);
					break;
				}
			}
		}

		const std::vector<UInt256> &Peer::GetKnownBlockHashes() const {
			return _knownBlockHashes;
		}

		void Peer::KnownBlockHashesRemoveRange(size_t index, size_t len) {
			for (size_t i = len; i > index; --i) {
				_knownBlockHashes.erase(_knownBlockHashes.begin() + i - 1);
			}
		}

		void Peer::AddKnownBlockHash(const UInt256 &hash) {
			_knownBlockHashes.push_back(hash);
		}


		const std::vector<UInt256>& Peer::KnownTxHashes() const {
			return _knownTxHashes;
		}

		const UInt256 &Peer::LastBlockHash() const {
			return _lastBlockHash;
		}

		void Peer::SetLastBlockHash(const UInt256 &hash) {
			_lastBlockHash = hash;
		}

		const UInt256ValueSet &Peer::KnownTxHashSet() const {
			return _knownTxHashSet;
		}

		void Peer::AddKnownTxHashes(const std::vector<UInt256> &txHashes) {
			for (size_t i = 0; i < txHashes.size(); i++) {
				if (! _knownTxHashSet.Contains(txHashes[i])) {
					_knownTxHashes.push_back(txHashes[i]);
					_knownTxHashSet.Insert(txHashes[i]);
				}
			}
		}

		std::string Peer::FormatError(int errnum) {
			return std::string(strerror(errnum));
		}

		int Peer::openSocket(int domain, double timeout, int *error) {
			struct sockaddr_storage addr;
			struct timeval tv;
			fd_set fds;
			socklen_t addrLen, optLen;
			int count, arg = 0, err = 0, on = 1, r = 1;

			_socket = socket(domain, SOCK_STREAM, 0);

			if (_socket < 0) {
				err = errno;
				r = 0;
			} else {
				tv.tv_sec = 1; // one second timeout for send/receive, so thread doesn't block for too long
				tv.tv_usec = 0;
				setsockopt(_socket, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv));
				setsockopt(_socket, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv));
				setsockopt(_socket, SOL_SOCKET, SO_KEEPALIVE, &on, sizeof(on));
#ifdef SO_NOSIGPIPE // BSD based systems have a SO_NOSIGPIPE socket option to supress SIGPIPE signals
				setsockopt(_socket, SOL_SOCKET, SO_NOSIGPIPE, &on, sizeof(on));
#endif
				arg = fcntl(_socket, F_GETFL, NULL);
				if (arg < 0 || fcntl(_socket, F_SETFL, arg | O_NONBLOCK) < 0)
					r = 0; // temporarily set socket non-blocking
				if (!r) err = errno;
			}

			if (r) {
				memset(&addr, 0, sizeof(addr));

				if (domain == PF_INET6) {
					((struct sockaddr_in6 *) &addr)->sin6_family = AF_INET6;
					((struct sockaddr_in6 *) &addr)->sin6_addr = *(struct in6_addr *) &_info.Address;
					((struct sockaddr_in6 *) &addr)->sin6_port = htons(_info.Port);
					addrLen = sizeof(struct sockaddr_in6);
				} else {
					((struct sockaddr_in *) &addr)->sin_family = AF_INET;
					((struct sockaddr_in *) &addr)->sin_addr = *(struct in_addr *) &_info.Address.u32[3];
					((struct sockaddr_in *) &addr)->sin_port = htons(_info.Port);
					addrLen = sizeof(struct sockaddr_in);
				}

				if (connect(_socket, (struct sockaddr *) &addr, addrLen) < 0) err = errno;

				if (err == EINPROGRESS) {
					err = 0;
					optLen = sizeof(err);
					tv.tv_sec = timeout;
					tv.tv_usec = (long) (timeout * 1000000) % 1000000;
					FD_ZERO(&fds);
					FD_SET(_socket, &fds);
					count = select(_socket + 1, NULL, &fds, NULL, &tv);

					if (count <= 0 || getsockopt(_socket, SOL_SOCKET, SO_ERROR, &err, &optLen) < 0 || err) {
						if (count == 0) err = ETIMEDOUT;
						if (count < 0 || !err) err = errno;
						r = 0;
					}
				} else if (err && domain == PF_INET6 && isIPv4()) {
					return openSocket(PF_INET, timeout, error); // fallback to IPv4
				} else if (err) r = 0;

				if (r) Pinfo("socket connected");
				fcntl(_socket, F_SETFL, arg); // restore socket non-blocking status
			}

			if (!r && err) Perror("connect error: {}", FormatError(err));
			if (error && err) *error = err;
			return r;
		}

		void Peer::SendMessage(const std::string &msgType, const SendMessageParameter &parameter) {
			if (_messages.find(msgType) == _messages.end()) {
				Pwarn("sending unknown type message, message type: {}", msgType);
				return;
			}
			_messages[msgType]->Send(parameter);
		}

		double Peer::GetStartTime() const {
			return _startTime;
		}

		void Peer::SetStartTime(double time) {
			_startTime = time;
		}

		void Peer::addPongCallback(const PeerCallback &callback) {
			_pongCallbackList.push_back(callback);
		}

		PeerManager *Peer::getPeerManager() const {
			return _manager;
		}

		const std::deque<Peer::PeerCallback> &Peer::getPongCallbacks() const {
			return _pongCallbackList;
		}

		double Peer::GetPingTime() const {
			return _pingTime;
		}

		void Peer::SetPingTime(double time) {
			_pingTime = time;
		}

		double Peer::GetDisconnectTime() const {
			return _disconnectTime;
		}

		void Peer::SetDisconnectTime(double time) {
			_disconnectTime = time;
		}

		Peer::PeerCallback Peer::popPongCallback() {
			PeerCallback callback = _pongCallbackList.front();
			_pongCallbackList.pop_front();
			return callback;
		}

		uint8_t Peer::GetFlags() const {
			return _info.Flags;
		}

		void Peer::SetFlags(uint8_t flags) {
			_info.Flags = flags;
		}

		const PeerInfo &Peer::GetPeerInfo() const {
			return _info;
		}

		void Peer::SetPeerInfo(const PeerInfo &info) {
			_info = info;
		}

		void Peer::SetCurrentBlock(const MerkleBlockPtr &block) {
			_currentBlock = block;
		}

		const MerkleBlockPtr &Peer::CurrentBlock() const {
			return _currentBlock;
		}

		const Peer::PeerCallback &Peer::getMemPoolCallback() const {
			return _mempoolCallback;
		}

		void Peer::resetMemPool() {
			_mempoolCallback = PeerCallback();
			_mempoolTime = DBL_MAX;
		}

		void Peer::setMempoolCallback(const Peer::PeerCallback &callback) {
			_mempoolCallback = callback;
		}

		void Peer::setMempoolTime(double time) {
			_mempoolTime = time;
		}

		void Peer::initSingleMessage(Message *message) {
			_messages[message->Type()] = MessagePtr(message);
		}

	}
}
