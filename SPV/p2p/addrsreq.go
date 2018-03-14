package p2p

type AddrsReq struct {
	Header
}

func NewAddrsReqMsg() ([]byte, error) {
	addrReq := new(AddrsReq)

	addrReq.Header = *NewHeader("getaddr", EmptyMsgSum, 0)

	return addrReq.Serialize()
}
