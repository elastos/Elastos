package msg

type AddrsReq struct{}

func (msg *AddrsReq) Serialize() ([]byte, error) {
	return NewHeader("getaddr", EmptyMsgSum, 0).Serialize()
}

func (msg *AddrsReq) Deserialize(body []byte) error {
	return nil
}
