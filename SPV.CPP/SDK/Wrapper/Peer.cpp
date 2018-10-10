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

#include "Peer.h"

namespace Elastos {
	namespace ElaWallet {

		const uint32_t DEFAULT_MAGICNUMBER = uint32_t(0);

		Peer::Peer(const UInt128 &addr, uint16_t port, uint64_t timestamp) :
				_status(Unknown),
				_magicNumber(DEFAULT_MAGICNUMBER) {
			_info.address = addr;
			_info.port = port;
			_info.timestamp = timestamp;
			_info.services = SERVICES_NODE_NETWORK;
			_info.flags = 0;

			initDefaultMessages();
		}

		Peer::Peer(uint32_t magicNumber) :
				_status(Unknown),
				_magicNumber(magicNumber),
				_pingTime(DBL_MAX),
				_mempoolTime(DBL_MAX),
				_disconnectTime(DBL_MAX),
				_socket(-1) {

			initDefaultMessages();
		}

		Peer::Peer(const UInt128 &addr, uint16_t port, uint64_t timestamp, uint64_t services) :
				_status(Unknown),
				_magicNumber(DEFAULT_MAGICNUMBER) {

			_info.address = addr;
			_info.port = port;
			_info.timestamp = timestamp;
			_info.services = services;
			_info.flags = 0;

			initDefaultMessages();
		}

		Peer::Peer(const Peer &peer) {
			operator=(peer);
		}

		Peer::~Peer() {
		}

		Peer &Peer::operator=(const Peer &peer) {
			_info.timestamp = peer._info.timestamp;
			_info.address = peer._info.address;
			_info.port = peer._info.port;
			_info.flags = peer._info.flags;
			_info.services = peer._info.services;

			return *this;
		}

		UInt128 Peer::getAddress() const {
			return _info.address;
		}

		void Peer::setAddress(const UInt128 &addr) {
			_info.address = addr;
		}

		uint16_t Peer::getPort() const {
			return _info.port;
		}

		void Peer::setPort(uint16_t port) {
			_info.port = port;
		}

		uint64_t Peer::getTimestamp() const {
			return _info.timestamp;
		}

		void Peer::setTimestamp(uint64_t timestamp) {
			_info.timestamp = timestamp;
		}

		uint64_t Peer::getServices() const {
			return _info.services;
		}

		void Peer::setServices(uint64_t services) {
			_info.services = services;
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

		Peer::ConnectStatus Peer::getConnectStatusValue() const {
			return _status;
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
				strncpy((char *)&buf[off], type.c_str(), 12);
				off += 12;
				UInt32SetLE(&buf[off], (uint32_t)message.GetSize());
				off += sizeof(uint32_t);
				BRSHA256_2(hash, message, message.GetSize());
				memcpy(&buf[off], hash, sizeof(uint32_t));
				off += sizeof(uint32_t);
				memcpy(&buf[off], message, message.GetSize());
				Pinfo("sending {}", type);

				size_t msgLen = 0;
				socket = _socket;
				if (socket < 0) error = ENOTCONN;

				while (socket >= 0 && ! error && msgLen < sizeof(buf)) {
					n = send(socket, &buf[msgLen], sizeof(buf) - msgLen, MSG_NOSIGNAL);
					if (n >= 0) msgLen += n;
					if (n < 0 && errno != EWOULDBLOCK) error = errno;
					gettimeofday(&tv, NULL);
					if (! error && tv.tv_sec + (double)tv.tv_usec/1000000 >= _disconnectTime) error = ETIMEDOUT;
					socket = _socket;
				}

				if (error) {
					Perror("ERROR: sending {} message {}", type, FormatError(error));
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
					inet_ntop(AF_INET, &_info.address.u32[3], temp, sizeof(temp));
				} else inet_ntop(AF_INET6, &_info.address, temp, sizeof(temp));

				_host = temp;
			}

			return _host;
		}

		uint32_t Peer::getVersion() const {
			return _version;
		}

		const std::string &Peer::getUserAgent() const {
			return _useragent;
		}

		uint32_t Peer::getLastBlock() const {
			return _lastblock;
		}

		uint64_t Peer::getFeePerKb() const {
			return _feePerKb;
		}

		double Peer::getPingTime() const {
			return _pingTime;
		}

		bool Peer::IsEqual(const Peer *otherPeer) const {
			return (this == otherPeer ||
					(UInt128Eq(&_info.address, &otherPeer->_info.address) &&
					_info.port == otherPeer->_info.port));
		}

		bool Peer::networkIsReachable() const {
			//fixme [refactor]
			return true;
		}

		bool Peer::isIPv4() const {
			return (_info.address.u64[0] == 0 && _info.address.u16[4] == 0 && _info.address.u16[5] == 0xffff);
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
							//fixme [refactor]
//							manager->peerMessages->BRPeerSendPingMessage(peer, mempoolInfo,
//																			   mempoolCallback);
//							mempoolCallback = NULL;
							_mempoolTime = DBL_MAX;
						}

						while (sizeof(uint32_t) <= len && UInt32GetLE(header) != _magicNumber) {
							memmove(header, &header[1],
									--len); // consume one byte at a time until we find the magic number
						}

						socket = _socket;
					}

					if (error) {
						//fixme [refactor]
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
							Pdebug("start read head: len = {}", msgLen);
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

			//fixme [refactor]
//			while (array_count(pongCallback) > 0) {
//				void (*pongCallback)(void *, int) = pongCallback[0];
//				void *pongInfo = pongInfo[0];
//
//				array_rm(pongCallback, 0);
//				array_rm(pongInfo, 0);
//				if (pongCallback) pongCallback(pongInfo, 0);
//			}

			//fixme [refactor]
//			if (mempoolCallback) mempoolCallback(mempoolInfo, 0);
//			mempoolCallback = NULL;
			if (_listener) _listener->OnDisconnected(this, error);
			//fixme [refactor]
//		pthread_cleanup_pop(1)
		}

		void Peer::RegisterListner(Peer::Listener *listener) {
			_listener = listener;
		}

		void Peer::UnRegisterListener() {
			_listener = nullptr;
		}

		void Peer::initDefaultMessages() {
			//fixme [refactor]
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

		const std::vector<UInt256>& Peer::GetCurrentBlockTxHashes() const {
			return _currentBlockTxHashes;
		}

		void Peer::AddCurrentBlockTxHash(const UInt256 &hash) {
			_currentBlockTxHashes.push_back(hash);
		}

		const std::vector<UInt256>& Peer::GetKnownBlockHashes() const {
			return _knownBlockHashes;
		}

		void Peer::KnownBlockHashesRemoveRange(size_t index, size_t len) {
			for (int i = int(len - 1); i >= index; --i) {
				_knownBlockHashes.erase(_knownBlockHashes.begin() + i);
			}
		}

		void Peer::AddKnownBlockHash(const UInt256 &hash) {
			_knownBlockHashes.push_back(hash);
		}



		const std::vector<UInt256>& Peer::GetKnownTxHashes() const {
			return _knownTxHashes;
		}

		void Peer::AddKnownTxHash(const UInt256 &hash) {
			_knownTxHashes.push_back(hash);
		}

		const UInt256 &Peer::GetLastBlockHash() const {
			return _lastBlockHash;
		}

		void Peer::SetLastBlockHash(const UInt256 &hash) {
			_lastBlockHash = hash;
		}

//		const TransactionSet& Peer::GetKnownTxHashSet() const {
//			return _knownTxHashSet;
//		}

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
					((struct sockaddr_in6 *) &addr)->sin6_addr = *(struct in6_addr *) &_info.address;
					((struct sockaddr_in6 *) &addr)->sin6_port = htons(_info.port);
					addrLen = sizeof(struct sockaddr_in6);
				} else {
					((struct sockaddr_in *) &addr)->sin_family = AF_INET;
					((struct sockaddr_in *) &addr)->sin_addr = *(struct in_addr *) &_info.address.u32[3];
					((struct sockaddr_in *) &addr)->sin_port = htons(_info.port);
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
				Pwarn("Sending unknown type message, message type: {}", msgType);
				return;
			}
			_messages[msgType]->Send(parameter);
		}

	}
}
