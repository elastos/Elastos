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
#include "Peer.h"
#include "PeerManager.h"
#include "Message/PingMessage.h"
#include "Message/VersionMessage.h"
#include "Message/VerackMessage.h"
#include "Message/AddressMessage.h"
#include "Message/InventoryMessage.h"
#include "Message/GetDataMessage.h"
#include "Message/NotFoundMessage.h"
#include "Message/GetBlocksMessage.h"
#include "Message/TransactionMessage.h"
#include "Message/MerkleBlockMessage.h"
#include "Message/MempoolMessage.h"
#include "Message/PongMessage.h"
#include "Message/FilterLoadMessage.h"
#include "Message/GetAddressMessage.h"
#include "Message/RejectMessage.h"

#include <Common/Log.h>
#include <Common/Utils.h>
#include <Common/hash.h>

#include <arpa/inet.h>
#include <cfloat>
#include <sys/time.h>
#include <boost/thread.hpp>

#define HEADER_LENGTH      24
#define MAX_MSG_LENGTH     0x02000000
#define MIN_PROTO_VERSION  70002 // peers earlier than this protocol version not supported (need v0.9 txFee relay rules)
#define LOCAL_HOST         ((UInt128) { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0xff, 0xff, 0x7f, 0x00, 0x00, 0x01 })
#define CONNECT_TIMEOUT    3.0
#define MESSAGE_TIMEOUT    40.0

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
				_startTime(0),
				_downloadStartTime(0),
				_downloadBytes(0) {

			_managerID = manager->GetID();
			RegisterListner(_manager);
		}

		Peer::~Peer() {
		}

		const uint128 &Peer::getAddress() const {
			return _info.Address;
		}

		void Peer::setAddress(const uint128 &addr) {
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

		time_t Peer::GetDownloadStartTime() const {
			return _downloadStartTime;
		}

		void Peer::ScheduleDownloadStartTime() {
			struct timeval tv;
			gettimeofday(&tv, NULL);

			_downloadStartTime = tv.tv_sec * 1000 + tv.tv_usec / 1000;
		}

		uint32_t Peer::GetDownloadBytes() const {
			return _downloadBytes;
		}

		void Peer::SetDownloadBytes(uint32_t bytes) {
			_downloadBytes = bytes;
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

				if (0 && !NetworkIsReachable()) { // delay until network is reachable
					if (!_waitingForNetwork) info("waiting for network reachability");
					_waitingForNetwork = 1;
				} else {
					info("connecting");
					_waitingForNetwork = 0;
					gettimeofday(&tv, NULL);
					_disconnectTime = tv.tv_sec + (double) tv.tv_usec / 1000000 + CONNECT_TIMEOUT;

					boost::thread workThread(boost::bind(&Peer::PeerThreadRoutine, this));
				}
			}
		}

		void Peer::Disconnect() {
			int socket = _socket;

			if (socket >= 0) {
				_socket = -1;
				if (shutdown(socket, SHUT_RDWR) < 0) {
					this->error("peer shutdown error: {}", FormatError(errno));
				}
				close(socket);
			}
		}

		// sends a bitcoin protocol message to peer
		void Peer::SendMessage(const bytes_t &message, const std::string &type) {
			if (message.size() > MAX_MSG_LENGTH) {
				this->error("failed to send {}, length {} is too long", type, message.size());
			} else {
				ssize_t n = 0;
				struct timeval tv;
				int socket, error = 0;
				ByteStream stream;

				stream.WriteUint32(_magicNumber);
				stream.WriteBytes(bytes_t(type.c_str(), type.size()));
				if (type.size() < 12)
					stream.WriteBytes(bytes_t(12 - type.size(), 0));
				stream.WriteUint32(message.size());
				bytes_t hash = sha256_2(message);
				stream.WriteUint32(*(uint32_t *)hash.data());
				stream.WriteBytes(message);

				const bytes_t &buf = stream.GetBytes();

				this->info("sending {}", type);
				size_t msgLen = 0;
				socket = _socket;
				if (socket < 0) error = ENOTCONN;

				gettimeofday(&tv, NULL);
				double disconnectTime = tv.tv_sec + (double) tv.tv_usec / 1000000 + MESSAGE_TIMEOUT;

				while (socket >= 0 && !error && msgLen < buf.size()) {
					n = send(socket, &buf[msgLen], buf.size() - msgLen, MSG_NOSIGNAL);
					if (n >= 0) {
						msgLen += n;
					}
					if (n < 0 && errno != EWOULDBLOCK) error = errno;
					gettimeofday(&tv, NULL);
					if (!error && tv.tv_sec + (double) tv.tv_usec / 1000000 >= disconnectTime) error = ETIMEDOUT;
					socket = _socket;
				}

				if (error) {
					this->error("sending {} message {}", type, FormatError(error));
					Disconnect();
				}
			}
		}

		void Peer::RerequestBlocks(const uint256 &fromBlock) {
			size_t i = _knownBlockHashes.size();

			while (i > 0 && _knownBlockHashes[i - 1] != fromBlock) i--;

			if (i > 0) {
				_knownBlockHashes.erase(_knownBlockHashes.begin(), _knownBlockHashes.begin() + i - 1);
				info("re-requesting {} block(s)", _knownBlockHashes.size());
				GetDataParameter getDataParameter({}, _knownBlockHashes);
				SendMessage(MSG_GETDATA, getDataParameter);
			}
		}

		void Peer::ScheduleDisconnect(double seconds) {
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

		const std::string &Peer::GetHost() const {
			if (_host.empty()) {
				_host = _info.GetHost();
			}

			return _host;
		}

		uint32_t Peer::GetVersion() const {
			return _version;
		}

		void Peer::SetVersion(uint32_t version) {
			_version = version;
		}

		const std::string &Peer::GetUserAgent() const {
			return _useragent;
		}

		uint32_t Peer::GetLastBlock() const {
			return _lastblock;
		}

		void Peer::SetLastBlock(uint32_t height) {
			_lastblock = height;
		}

		uint64_t Peer::GetFeePerKb() const {
			return _feePerKb;
		}

		bool Peer::IsEqual(const Peer *otherPeer) const {
			return (this == otherPeer || _info == otherPeer->GetPeerInfo());
		}

		bool Peer::NetworkIsReachable() const {
			//fixme [refactor]
			return true;
		}

		bool Peer::IsIPv4() const {
			return _info.IsIPv4();
		}

		void Peer::PeerThreadRoutine() {

			int socket, error = 0;

			if (OpenSocket(PF_INET6, CONNECT_TIMEOUT, &error)) {
				struct timeval tv;
				double time = 0, msgTimeout;
				uint8_t header[HEADER_LENGTH];
				bytes_t payload;
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
							info("done waiting for mempool response");
							PingParameter pingParameter(_manager->GetLastBlockHeight(), _mempoolCallback);
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
						if (_socket != -1) {
							this->error("read header error: {}", FormatError(error));
						}
					} else if (header[15] != 0) { // verify header type field is NULL terminated
						this->error("malformed message header: type not NULL terminated");
						error = EPROTO;
					} else if (len == HEADER_LENGTH) {
						std::string type = (const char *) (&header[4]);
						uint32_t msgLen = *(uint32_t*)&header[16];
						uint32_t checksum = *(uint32_t*)(&header[20]);

						if (msgLen > MAX_MSG_LENGTH) { // check message length
							this->error("error reading {}, message length {} is too long", type, msgLen);
							error = EPROTO;
						} else {
							payload.resize(size_t(msgLen));
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
								if (_socket != -1) {
									this->error("read message error: {}", FormatError(error));
								}
							} else if (len == msgLen) {
								bytes_t hash = sha256_2(payload);;

								if (*(uint32_t *)(&hash[0]) != checksum) { // verify checksum
									this->error("reading {}, invalid checksum {:x}, expected {:x}, payload length:{},",
												type, UInt32GetLE(&hash), checksum, msgLen);
									error = EPROTO;
								} else if (!AcceptMessage(payload, type)) error = EPROTO;
							}
						}
					}
				}
			}

			if (_socket == -1)
				error = 0;

			socket = _socket;
			_socket = -1;
			_status = Peer::Disconnected;
			if (socket >= 0) close(socket);
			info("disconnected");

			while (!_pongCallbackList.empty()) {
				Peer::PeerCallback pongCallback = PopPongCallback();
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

		void Peer::InitDefaultMessages() {
			InitSingleMessage(new VersionMessage(shared_from_this()));
			InitSingleMessage(new VerackMessage(shared_from_this()));
			InitSingleMessage(new AddressMessage(shared_from_this()));
			InitSingleMessage(new InventoryMessage(shared_from_this()));
			InitSingleMessage(new GetDataMessage(shared_from_this()));
			InitSingleMessage(new NotFoundMessage(shared_from_this()));
			InitSingleMessage(new GetBlocksMessage(shared_from_this()));
			InitSingleMessage(new TransactionMessage(shared_from_this()));
			InitSingleMessage(new MempoolMessage(shared_from_this()));
			InitSingleMessage(new PingMessage(shared_from_this()));
			InitSingleMessage(new PongMessage(shared_from_this()));
			InitSingleMessage(new FilterLoadMessage(shared_from_this()));
			InitSingleMessage(new MerkleBlockMessage(shared_from_this()));
			InitSingleMessage(new GetAddressMessage(shared_from_this()));
			InitSingleMessage(new RejectMessage(shared_from_this()));
		}

		bool Peer::AcceptMessage(const bytes_t &msg, const std::string &type) {
			bool r = false;

			if (_currentBlock != nullptr && MSG_TX != type) { // if we receive a non-tx message, merkleblock is done
				this->error("incomplete merkleblock {}, expected {} more tx, got {}",
							_currentBlock->GetHash().GetHex(), _currentBlockTxHashes.size(), type);
				_currentBlockTxHashes.clear();
				_currentBlock.reset();
				r = 0;
			} else if (_messages.find(type) != _messages.end()) {
				_downloadBytes += msg.size();
				r = _messages[type]->Accept(msg);
			} else {
				this->error("dropping {}, length {}, not implemented", type, msg.size());
			}

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

		bool Peer::WaitingBlocks() const {
			return _waitingBlocks;
		}

		void Peer::SetWaitingBlocks(bool wait) {
			_waitingBlocks = wait;
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

		const std::vector<uint256> &Peer::CurrentBlockTxHashes() const {
			return _currentBlockTxHashes;
		}

		void Peer::AddCurrentBlockTxHash(const uint256 &hash) {
			_currentBlockTxHashes.push_back(hash);
		}

		void Peer::CurrentBlockTxHashesRemove(const uint256 &hash) {
			for (std::vector<uint256>::iterator it = _currentBlockTxHashes.begin(); it != _currentBlockTxHashes.end();) {
				if (hash == (*it)) {
					it = _currentBlockTxHashes.erase(it);
				} else {
					++it;
				}
			}
		}

		const std::vector<uint256> &Peer::GetKnownBlockHashes() const {
			return _knownBlockHashes;
		}

		void Peer::KnownBlockHashesRemoveRange(size_t index, size_t len) {
			for (size_t i = len; i > index; --i) {
				_knownBlockHashes.erase(_knownBlockHashes.begin() + i - 1);
			}
		}

		void Peer::AddKnownBlockHash(const uint256 &hash) {
			_knownBlockHashes.push_back(hash);
		}


		const std::vector<uint256>& Peer::KnownTxHashes() const {
			return _knownTxHashes;
		}

		const uint256 &Peer::LastBlockHash() const {
			return _lastBlockHash;
		}

		void Peer::SetLastBlockHash(const uint256 &hash) {
			_lastBlockHash = hash;
		}

		const std::set<uint256> &Peer::KnownTxHashSet() const {
			return _knownTxHashSet;
		}

		void Peer::AddKnownTxHashes(const std::vector<uint256> &txHashes) {
			for (size_t i = 0; i < txHashes.size(); i++) {
				if (_knownTxHashSet.find(txHashes[i]) == _knownTxHashSet.end()) {
					_knownTxHashes.push_back(txHashes[i]);
					_knownTxHashSet.insert(txHashes[i]);
				}
			}
		}

		void Peer::RemoveKnownTxHashes(const std::vector<uint256> &txHashes) {
			for (size_t i = 0; i < txHashes.size(); ++i) {
				if (_knownTxHashSet.find(txHashes[i]) != _knownTxHashSet.end()) {
					_knownTxHashSet.erase(txHashes[i]);
				}

				for (std::vector<uint256>::iterator it = _knownTxHashes.begin(); it != _knownTxHashes.end();) {
					if ((*it) == txHashes[i]) {
						it = _knownTxHashes.erase(it);
					} else {
						++it;
					}
				}
			}
		}

		std::string Peer::FormatError(int errnum) {
			return std::string(strerror(errnum));
		}

		int Peer::OpenSocket(int domain, double timeout, int *error) {
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
					((struct sockaddr_in *) &addr)->sin_addr = *(struct in_addr *) &_info.Address.begin()[12];
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
				} else if (err && domain == PF_INET6 && IsIPv4()) {
					return OpenSocket(PF_INET, timeout, error); // fallback to IPv4
				} else if (err) r = 0;

				if (r) info("socket connected");
				fcntl(_socket, F_SETFL, arg); // restore socket non-blocking status
			}

			if (!r && err) this->error("connect error: {}", FormatError(err));
			if (error && err) *error = err;
			return r;
		}

		void Peer::SendMessage(const std::string &msgType, const SendMessageParameter &parameter) {
			if (_messages.find(msgType) == _messages.end()) {
				warn("sending unknown type message, message type: {}", msgType);
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

		void Peer::AddPongCallback(const PeerCallback &callback) {
			_pongCallbackList.push_back(callback);
		}

		PeerManager *Peer::GetPeerManager() const {
			return _manager;
		}

		const std::deque<Peer::PeerCallback> &Peer::GetPongCallbacks() const {
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

		Peer::PeerCallback Peer::PopPongCallback() {
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

		const Peer::PeerCallback &Peer::GetMemPoolCallback() const {
			return _mempoolCallback;
		}

		void Peer::ResetMemPool() {
			_mempoolCallback = PeerCallback();
			_mempoolTime = DBL_MAX;
		}

		void Peer::SetMempoolCallback(const Peer::PeerCallback &callback) {
			_mempoolCallback = callback;
		}

		void Peer::SetMempoolTime(double time) {
			_mempoolTime = time;
		}

		void Peer::InitSingleMessage(Message *message) {
			_messages[message->Type()] = MessagePtr(message);
		}

	}
}
