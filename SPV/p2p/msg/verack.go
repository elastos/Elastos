package msg

type VerAck struct{}

func (msg *VerAck) CMD() string {
	return "verack"
}

func (msg *VerAck) Serialize() ([]byte, error) {
	return nil, nil
}

func (msg *VerAck) Deserialize(body []byte) error {
	return nil
}
