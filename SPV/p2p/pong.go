package p2p

import (
	"bytes"

	"SPVWallet/core/serialization"
	"SPVWallet/db"
)

type Pong struct {
	Ping
}

func NewPongMsg() ([]byte, error) {
	ping := new(Ping)

	ping.Height = uint64(db.GetBlockchain().Height())

	buf := new(bytes.Buffer)
	serialization.WriteUint64(buf, ping.Height)

	ping.Header = *BuildHeader("pong", buf.Bytes())

	return ping.Serialize()
}
