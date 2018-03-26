package _interface

import "SPVWallet/p2p"

type P2PClient interface {
	Start()
	HandleVersion(callback func(v *p2p.Version) error)
	PeerConnected(callback func(peer *p2p.Peer))
	MakeMessage(callback func(cmd string) (p2p.Message, error))
	HandleMessage(callback func(peer *p2p.Peer, msg p2p.Message) error)
	PeerManager() *p2p.PeerManager
}

func NewP2PClient(clientId uint64, magic uint32, port uint16, seeds []string) P2PClient {
	client := new(P2PClientImpl)
	client.id = clientId
	client.magic = magic
	client.port = port
	client.seeds = seeds
	return client
}
