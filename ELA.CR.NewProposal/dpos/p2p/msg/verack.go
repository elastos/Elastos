// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package msg

import (
	"io"

	"github.com/elastos/Elastos.ELA/p2p"
)

// Ensure VerAck implement p2p.Message interface.
var _ p2p.Message = (*VerAck)(nil)

type VerAck struct {
	Signature [64]byte
}

func (msg *VerAck) CMD() string {
	return p2p.CmdVerAck
}

func (msg *VerAck) MaxLength() uint32 {
	return 64
}

func (msg *VerAck) Serialize(w io.Writer) error {
	_, err := w.Write(msg.Signature[:])
	return err
}

func (msg *VerAck) Deserialize(r io.Reader) error {
	_, err := io.ReadFull(r, msg.Signature[:])
	return err
}

func NewVerAck(signature []byte) *VerAck {
	verAck := VerAck{}
	copy(verAck.Signature[:], signature)
	return &verAck
}
