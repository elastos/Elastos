package p2p

import (
	"SPVWallet/db"
)

type Pong struct {
	Ping
}

func NewPongMsg() ([]byte, error) {
	pong := new(Pong)

	pong.Height = uint64(db.GetBlockchain().Height())

	body, err := pong.Serialize()
	if err != nil {
		return nil, err
	}

	return BuildMessage("pong", body)
}
