package msg

type VerAck struct{}

func NewVerAckMsg() ([]byte, error) {
	return NewHeader("verack", EmptyMsgSum, 0).Serialize()
}

func (msg *VerAck) Serialize() ([]byte, error) {
	return nil, nil
}

func (msg *VerAck) Deserialize(body []byte) error {
	return nil
}
