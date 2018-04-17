package msg

type AddrsReq struct{}

func (msg *AddrsReq) CMD() string {
	return "getaddr"
}

func (msg *AddrsReq) Serialize() ([]byte, error) {
	return nil, nil
}

func (msg *AddrsReq) Deserialize(body []byte) error {
	return nil
}
