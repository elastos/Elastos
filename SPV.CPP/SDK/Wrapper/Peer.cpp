// Copyright (c) 2012-2018 The Elastos Open Source Project
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include <float.h>
#include <sys/time.h>
#include <SDK/Common/Log.h>
#include <arpa/inet.h>
#include <boost/thread.hpp>
#include <CMemBlock.h>

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

		void Peer::setCurrentBlockHeight(uint32_t currentBlockHeight) {
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
					if (!_waitingForNetwork) Log::traceWithTime("waiting for network reachability");
					_waitingForNetwork = 1;
				} else {
					Log::traceWithTime("connecting");
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
					Log("peer shutdown error: {}", std::string(strerror(errno)));
				}
				close(socket);
			}
		}

		// sends a bitcoin protocol message to peer
		void Peer::SendMessage(const CMBlock &message, const std::string &type) {
			if (message.GetSize() > MAX_MSG_LENGTH) {
				Log("failed to send {}, length {} is too long", type, message.GetSize());
			} else {
//				BRPeerContext *ctx = (BRPeerContext *)peer;
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
				Log("sending {}", type);

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
					Log("ERROR: sending {} message {}", type, std::string(strerror(error)));
					Disconnect();
				}
			}
		}

		void Peer::scheduleDisconnect(double seconds) {
			struct timeval tv;

			gettimeofday(&tv, NULL);
			_disconnectTime = (seconds < 0) ? DBL_MAX : tv.tv_sec + (double) tv.tv_usec / 1000000 + seconds;
		}

		void Peer::setNeedsFilterUpdate(bool needsFilterUpdate) {
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
			return false;
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
				std::string payload;
				payload.resize(0x1000);
				size_t len = 0, payloadLen = 0x1000;
				ssize_t n = 0;

				gettimeofday(&tv, NULL);
				_startTime = tv.tv_sec + (double) tv.tv_usec / 1000000;
				_messages[MSG_VERSION]->Send(Message::DefaultParam);

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
							Log::traceWithTime("done waiting for mempool response");
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
//						Log::traceWithTime(peer, "read socket error: %s", strerror(error));
					} else if (header[15] != 0) { // verify header type field is NULL terminated
						Log::traceWithTime("malformed message header: type not NULL terminated");
						error = EPROTO;
					} else if (len == HEADER_LENGTH) {
						std::string type = (const char *) (&header[4]);
						uint32_t msgLen = UInt32GetLE(&header[16]);
						uint32_t checksum = UInt32GetLE(&header[20]);
						UInt256 hash;

						if (msgLen > MAX_MSG_LENGTH) { // check message length
							//fixme [refactor]
//							peer_log(peer, "error reading %s, message length %"PRIu32" is too long", type, msgLen);
							error = EPROTO;
						} else {
//                    peer_dbg(peer, "start read head: port %d", (int)port);
							if (msgLen > payloadLen) {
								payloadLen = msgLen;
								payload.resize(payloadLen);
							}
							len = 0;
							socket = _socket;
							msgTimeout = time + MESSAGE_TIMEOUT;

							while (socket >= 0 && !error && len < msgLen) {
								n = read(socket, &payload[len], msgLen - len);
								//peer_log(peer, "read socket n %ld", n);
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
								//fixme [refactor]
//								peer_log(peer, "read socket error: %s", strerror(error));
							} else if (len == msgLen) {
								BRSHA256_2(&hash, payload.data(), msgLen);

								if (UInt32GetLE(&hash) != checksum) { // verify checksum
									//fixme [refactor]
//									peer_log(peer,
//											 "error reading %s, invalid checksum %x, expected %x, payload length:%"PRIu32
//													 ", SHA256_2:%s", type, UInt32GetLE(&hash), checksum, msgLen,
//											 u256hex(hash));
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
			Log::traceWithTime("disconnected");

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
			if (_listener) _listener->OnDisconnected(error);
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

		bool Peer::acceptMessage(const std::string &msg, const std::string &type) {
			bool r = false;

			if (_currentBlock != nullptr && MSG_TX == type) { // if we receive a non-tx message, merkleblock is done
				//fixme [refactor]
//        peer_log(peer, "incomplete merkleblock %s, expected %zu more tx, got %s", u256hex(_currentBlock->blockHash),
//                 array_count(_currentBlockTxHashes), type);
				_currentBlockTxHashes.clear();
				_currentBlock.reset();
				r = 0;
			} else if (_messages.find(type) != _messages.end())
				r = _messages[type]->Accept(msg);
			//fixme [refactor]
//    else peer_log(peer, "dropping %s, length %zu, not implemented", type, msgLen);

			return r;
		}

		bool Peer::sentVerack() {
			return _sentVerack;
		}

		void Peer::setSentVerack(bool sent) {
			_sentVerack = sent;
		}

		bool Peer::gotVerack() {
			return _gotVerack;
		}

		void Peer::setGotVerack(bool got) {
			_gotVerack = got;
		}

		bool Peer::sentGetaddr() {
			return _sentGetaddr;
		}

		void Peer::setSentGetaddr(bool sent) {
			_sentGetaddr = sent;
		}

		bool Peer::sentFilter() {
			return _sentFilter;
		}

		void Peer::setSentFilter(bool sent) {
			_sentFilter = sent;
		}

		bool Peer::sentGetdata() {
			return _sentGetdata;
		}

		void Peer::setSentGetdata(bool sent) {
			_sentGetdata = sent;
		}

		bool Peer::sentMempool() {
			return _sentMempool;
		}

		void Peer::setSentMempool(bool sent) {
			_sentMempool = sent;
		}

		bool Peer::sentGetblocks() {
			return _sentGetblocks;
		}

		void Peer::setSentGetblocks(bool sent) {
			_sentGetblocks = sent;
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

				if (r) Log::traceWithTime("socket connected");
				fcntl(_socket, F_SETFL, arg); // restore socket non-blocking status
			}

			//fixme [refactor]
//			if (!r && err) peer_log(peer, "connect error: %s", strerror(err));
			if (error && err) *error = err;
			return r;
		}

	}
}
