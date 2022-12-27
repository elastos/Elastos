// Copyright 2016 The Elastos.ELA.SideChain.ETH Authors
// This file is part of the Elastos.ELA.SideChain.ETH library.
//
// The Elastos.ELA.SideChain.ETH library is free software: you can redistribute it and/or modify
// it under the terms of the GNU Lesser General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// The Elastos.ELA.SideChain.ETH library is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
// GNU Lesser General Public License for more details.
//
// You should have received a copy of the GNU Lesser General Public License
// along with the Elastos.ELA.SideChain.ETH library. If not, see <http://www.gnu.org/licenses/>.

package api

import (
	"encoding/binary"
	"errors"

	"github.com/elastos/Elastos.ELA.SideChain.ETH/crypto/sha3"
	"github.com/elastos/Elastos.ELA.SideChain.ETH/swarm/storage/encryption"
)

type RefEncryption struct {
	refSize int
	span    []byte
}

func NewRefEncryption(refSize int) *RefEncryption {
	span := make([]byte, 8)
	binary.LittleEndian.PutUint64(span, uint64(refSize))
	return &RefEncryption{
		refSize: refSize,
		span:    span,
	}
}

func (re *RefEncryption) Encrypt(ref []byte, key []byte) ([]byte, error) {
	spanEncryption := encryption.New(key, 0, uint32(re.refSize/32), sha3.NewKeccak256)
	encryptedSpan, err := spanEncryption.Encrypt(re.span)
	if err != nil {
		return nil, err
	}
	dataEncryption := encryption.New(key, re.refSize, 0, sha3.NewKeccak256)
	encryptedData, err := dataEncryption.Encrypt(ref)
	if err != nil {
		return nil, err
	}
	encryptedRef := make([]byte, len(ref)+8)
	copy(encryptedRef[:8], encryptedSpan)
	copy(encryptedRef[8:], encryptedData)

	return encryptedRef, nil
}

func (re *RefEncryption) Decrypt(ref []byte, key []byte) ([]byte, error) {
	spanEncryption := encryption.New(key, 0, uint32(re.refSize/32), sha3.NewKeccak256)
	decryptedSpan, err := spanEncryption.Decrypt(ref[:8])
	if err != nil {
		return nil, err
	}

	size := binary.LittleEndian.Uint64(decryptedSpan)
	if size != uint64(len(ref)-8) {
		return nil, errors.New("invalid span in encrypted reference")
	}

	dataEncryption := encryption.New(key, re.refSize, 0, sha3.NewKeccak256)
	decryptedRef, err := dataEncryption.Decrypt(ref[8:])
	if err != nil {
		return nil, err
	}

	return decryptedRef, nil
}
