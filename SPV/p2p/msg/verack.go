package msg

type VerAck struct{}

func (msg *VerAck) Serialize() ([]byte, error) {
	return NewHeader("verack", EmptyMsgSum, 0).Serialize()
}

func (msg *VerAck) Deserialize(body []byte) error {
	return nil
}
