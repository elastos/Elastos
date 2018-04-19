package p2p

// The message flying in the peer to peer network
type Message interface {
	// Get the message CMD parameter which is the type of this message
	CMD() string
	// Serialize the message content
	Serialize() ([]byte, error)
	// Deserialize the message content through bytes
	Deserialize(msg []byte) error
}

// BuildMessage create the message header and return serialized message bytes
func BuildMessage(msg Message) ([]byte, error) {
	body, err := msg.Serialize()
	if err != nil {
		return nil, err
	}
	hdr, err := BuildHeader(msg.CMD(), body).Serialize()
	if err != nil {
		return nil, err
	}

	return append(hdr, body...), nil
}
