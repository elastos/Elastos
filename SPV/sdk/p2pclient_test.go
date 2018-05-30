package sdk

import (
	"fmt"
	"testing"
	"time"

	"github.com/elastos/Elastos.ELA.SPV/log"
	"github.com/elastos/Elastos.ELA.SPV/net"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

func TestP2PClient(t *testing.T) {
	log.Init(log.LevelInfo)

	var clientNumber = 100

	var clients []*Client
	var seeds = []string{
		"127.0.0.1:50000",
		"127.0.0.1:50001",
		"127.0.0.1:50002",
		"127.0.0.1:50003",
		"127.0.0.1:50004",
	}
	for i := 0; i < clientNumber; i++ {
		client, err := NewClient(i, seeds)
		if err != nil {
			t.Errorf("GetP2PClient failed %s", err.Error())
		}
		clients = append(clients, client)
		client.Start()
	}

	go monitorConnections(clients)
	select {}
}

func monitorConnections(clients []*Client) {
	ticker := time.NewTicker(time.Second * 5)
	for range ticker.C {
		var min = -1
		var max = -1
		var total = 0
		for _, c := range clients {
			connections := c.PeerManager().PeersCount()
			total += connections
			if min == -1 {
				min = connections
			}
			if connections < min {
				min = connections
			}
			if connections > max {
				max = connections
			}
			fmt.Printf("Client %d connections %d known addresses %d\n",
				c.id, c.PeerManager().PeersCount(), len(c.PeerManager().KnownAddresses()))
		}
		fmt.Printf("Min: %d, Max: %d, Avg: %d, Total: %d\n", min, max, total/len(clients), total)
	}
}

type Client struct {
	id uint64
	P2PClient
}

func NewClient(i int, seeds []string) (*Client, error) {
	var minOutbound = 10
	var maxConnections = 20

	var clientIdBase = 100000
	var clientPortBase = 50000

	client := new(Client)
	client.id = uint64(clientIdBase + i)
	var err error
	client.P2PClient, err = GetP2PClient(
		987654321,
		client.id,
		seeds,
		uint16(clientPortBase+i),
		minOutbound,
		maxConnections,
	)
	client.P2PClient.PeerManager().SetMessageHandler(client)

	return client, err
}

// A handshake message received
func (c *Client) OnHandshake(v *msg.Version) error {
	return nil
}

// Create a message instance by the given cmd parameter
func (c *Client) MakeMessage(cmd string) (p2p.Message, error) {
	var message p2p.Message
	switch cmd {
	case p2p.CmdPing:
		message = new(msg.Ping)
	case p2p.CmdPong:
		message = new(msg.Pong)
	}
	return message, nil
}

// VerAck message received from a connected peer
// which means the connected peer is established
func (c *Client) OnPeerEstablish(peer *net.Peer) {
	go c.pingWithPeer(peer)
}

// Handle messages received from the connected peer
func (c *Client) HandleMessage(peer *net.Peer, message p2p.Message) error {
	switch message.(type) {
	case *msg.Ping:
		peer.Send(msg.NewPong(uint32(c.id)))
	case *msg.Pong:
	}
	return nil
}

func (c *Client) pingWithPeer(peer *net.Peer) {
	ticker := time.NewTicker(time.Second * 5)
	for range ticker.C {
		peer.Send(msg.NewPing(uint32(c.id)))
	}
}
