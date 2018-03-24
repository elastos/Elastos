package msg

type AddrsReq struct{}

func NewAddrsReqMsg() ([]byte, error) {
	return NewHeader("getaddr", EmptyMsgSum, 0).Serialize()
}

func (msg *AddrsReq) Serialize() ([]byte, error) {
	return nil, nil
}

func (msg *AddrsReq) Deserialize(body []byte) error {
	return nil
}
