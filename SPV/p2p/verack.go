package p2p

type VerAck struct {
	Header
}

func NewVerAckMsg() ([]byte, error) {
	header := NewHeader("verack", EmptyMsgSum, 0)
	return header.Serialize()
}
