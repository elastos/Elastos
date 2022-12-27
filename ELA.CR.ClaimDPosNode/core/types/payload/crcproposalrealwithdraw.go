// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package payload

import (
	"bytes"
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA/common"
)

const CRCProposalRealWithdrawVersion byte = 0x00

type CRCProposalRealWithdraw struct {
	// Hash of the proposal to withdrawal ela.
	WithdrawTransactionHashes []common.Uint256
}

func (p *CRCProposalRealWithdraw) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := p.Serialize(buf, version); err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (p *CRCProposalRealWithdraw) Serialize(w io.Writer, version byte) error {
	err := common.WriteVarUint(w, uint64(len(p.WithdrawTransactionHashes)))
	if err != nil {
		return errors.New(
			"failed to serialize length of WithdrawTransactionHashes")
	}

	for _, hash := range p.WithdrawTransactionHashes {
		if err := hash.Serialize(w); err != nil {
			return errors.New(
				"failed to serialize WithdrawTransactionHashes")
		}
	}

	return nil
}

func (p *CRCProposalRealWithdraw) Deserialize(r io.Reader, version byte) error {
	length, err := common.ReadVarUint(r, 0)
	if err != nil {
		return errors.New(
			"failed to deserialize length of WithdrawTransactionHashes")
	}

	for i := uint64(0); i < length; i++ {
		var hash common.Uint256
		err = hash.Deserialize(r)
		if err != nil {
			return errors.New(
				"failed to deserialize WithdrawTransactionHashes")
		}
		p.WithdrawTransactionHashes = append(p.WithdrawTransactionHashes, hash)
	}

	return nil
}
