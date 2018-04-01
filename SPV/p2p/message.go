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

// Handle the message creation, allocation etc.
type MessageHandler interface {
	// Create a message instance by the given cmd parameter
	MakeMessage(cmd string) (Message, error)

	// A handshake message received
	OnHandshake(v *Version) error

	// VerAck message received from a connected peer
	// which means the connected peer is established
	OnPeerEstablish(*Peer)

	// Handle messages received from the connected peer
	HandleMessage(*Peer, Message) error
}
