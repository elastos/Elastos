package msg

type Pong struct {
	Ping
}

func NewPongMsg(height uint32) ([]byte, error) {
	pong := new(Pong)

	pong.Height = uint64(height)

	body, err := pong.Serialize()
	if err != nil {
		return nil, err
	}

	return BuildMessage("pong", body)
}
