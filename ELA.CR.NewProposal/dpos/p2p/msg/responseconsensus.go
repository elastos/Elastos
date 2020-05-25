// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package msg

import (
	"io"
)

const DefaultResponseConsensusMessageDataSize = 8000000 * 10

type ResponseConsensus struct {
	Consensus ConsensusStatus
}

func (msg *ResponseConsensus) CMD() string {
	return CmdResponseConsensus
}

func (msg *ResponseConsensus) MaxLength() uint32 {
	return DefaultResponseConsensusMessageDataSize
}

func (msg *ResponseConsensus) Serialize(w io.Writer) error {
	return msg.Consensus.Serialize(w)
}

func (msg *ResponseConsensus) Deserialize(r io.Reader) error {
	return msg.Consensus.Deserialize(r)
}
