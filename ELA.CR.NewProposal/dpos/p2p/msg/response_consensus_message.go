package msg

import (
	"io"
)

const DefaultResponseConsensusMessageDataSize = 8000000 * 10

type ResponseConsensusMessage struct {
	Consensus ConsensusStatus
}

func (msg *ResponseConsensusMessage) CMD() string {
	return ResponseConsensus
}

func (msg *ResponseConsensusMessage) MaxLength() uint32 {
	return DefaultResponseConsensusMessageDataSize
}

func (msg *ResponseConsensusMessage) Serialize(w io.Writer) error {
	return msg.Consensus.Serialize(w)
}

func (msg *ResponseConsensusMessage) Deserialize(r io.Reader) error {
	return msg.Consensus.Deserialize(r)
}
