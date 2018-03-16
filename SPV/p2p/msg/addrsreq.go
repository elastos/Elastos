package msg

type AddrsReq struct {
	Header
}

func NewAddrsReqMsg() ([]byte, error) {
	return NewHeader("getaddr", EmptyMsgSum, 0).Serialize()
}
