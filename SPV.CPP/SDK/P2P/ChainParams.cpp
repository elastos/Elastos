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
#include "ChainParams.h"

namespace Elastos {
	namespace ElaWallet {

		CheckPoint::CheckPoint() :
			_height(0),
			_timestamp(0),
			_target(0) {}

		CheckPoint::CheckPoint(uint32_t height, const std::string &hash, time_t timestamp, uint32_t target) :
			_height(height),
			_timestamp(timestamp),
			_target(target) {
			_hash.SetHex(hash);
		}

		CheckPoint::CheckPoint(const CheckPoint &checkPoint) {
			this->operator=(checkPoint);
		}

		CheckPoint::~CheckPoint() {
		}

		CheckPoint &CheckPoint::operator=(const CheckPoint &checkpoint) {
			_height = checkpoint._height;
			_hash = checkpoint._hash;
			_timestamp = checkpoint._timestamp;
			_target = checkpoint._target;
			return *this;
		}

		const uint32_t &CheckPoint::Height() const {
			return _height;
		}

		const uint256 &CheckPoint::Hash() const {
			return _hash;
		}

		const time_t &CheckPoint::Timestamp() const {
			return _timestamp;
		}

		const uint32_t &CheckPoint::Target() const {
			return _target;
		}


		ChainParams::ChainParams() :
			_standardPort(0),
			_magicNumber(0),
			_services(0),
			_targetTimeSpan(0),
			_targetTimePerBlock(0) {}

		ChainParams::ChainParams(uint16_t standardPort, uint32_t magic,
								 const std::vector<std::string> &dnsSeeds,
								 const std::vector<CheckPoint> &checkpoints) :
			_standardPort(standardPort),
			_magicNumber(magic),
			_services(0),
			_targetTimeSpan(86400),
			_targetTimePerBlock(120) {

		}

		ChainParams::ChainParams(const ChainParams &chainParams) {
			this->operator=(chainParams);
		}

		ChainParams::~ChainParams() {
		}

		ChainParams &ChainParams::operator=(const ChainParams &params) {
			_dnsSeeds = params._dnsSeeds;
			_checkpoints = params._checkpoints;
			_standardPort = params._standardPort;
			_magicNumber = params._magicNumber;
			_services = params._services;
			_targetTimeSpan = params._targetTimeSpan;
			_targetTimePerBlock = params._targetTimePerBlock;
			return *this;
		}

		const std::vector<std::string> &ChainParams::DNSSeeds() const {
			return _dnsSeeds;
		}

		const CheckPoint &ChainParams::LastCheckpoint() const {
			return _checkpoints.back();
		}

		const CheckPoint &ChainParams::FirstCheckpoint() const {
			return _checkpoints.front();
		}

		const std::vector<CheckPoint> &ChainParams::Checkpoints() const {
			return _checkpoints;
		}

		const uint32_t &ChainParams::MagicNumber() const {
			return _magicNumber;
		}

		const uint16_t &ChainParams::StandardPort() const {
			return _standardPort;
		}

		const uint64_t &ChainParams::Services() const {
			return _services;
		}

		const uint32_t &ChainParams::TargetTimeSpan() const {
			return _targetTimeSpan;
		}

		const uint32_t &ChainParams::TargetTimePerBlock() const {
			return _targetTimePerBlock;
		}

	}
}