package msg

type VerAck struct {
	Header
}

func NewVerAckMsg() ([]byte, error) {
	return NewHeader("verack", EmptyMsgSum, 0).Serialize()
}
