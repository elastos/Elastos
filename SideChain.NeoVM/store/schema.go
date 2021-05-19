// Copyright 2018 The go-ethereum Authors
// This file is part of the go-ethereum library.
//
// The go-ethereum library is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// The go-ethereum library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with the go-ethereum library. If not, see <http://www.gnu.org/licenses/>.

// Package rawdb contains a collection of low level database accessors.
package store

import (
	"github.com/elastos/Elastos.ELA/common"
	"encoding/binary"
)

var (
	blockReceiptsPrefix = []byte("r") // blockReceiptsPrefix + num (uint64 big endian) + hash -> block receipts
	txLookupPrefix      = []byte("l") // txLookupPrefix + hash -> transaction/receipt lookup metadata
)

// encodeBlockNumber encodes a block number as big endian uint64
func encodeBlockNumber(number uint32) []byte {
	enc := make([]byte, 8)
	binary.BigEndian.PutUint32(enc, number)
	return enc
}

// blockReceiptsKey = blockReceiptsPrefix + num (uint64 big endian) + hash
func blockReceiptsKey(number uint32, hash common.Uint256) []byte {
	return append(append(blockReceiptsPrefix, encodeBlockNumber(number)...), hash.Bytes()...)
}

// txLookupKey = txLookupPrefix + hash
func txLookupKey(hash common.Uint256) []byte {
	return append(txLookupPrefix, hash.Bytes()...)
}