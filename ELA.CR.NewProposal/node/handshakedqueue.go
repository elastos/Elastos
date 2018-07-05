package node

import (
	"net"
	"sync"
	"time"

	"github.com/elastos/Elastos.ELA/protocol"
)

/*
Handshake queue is a connection queue to handle all connections in
handshake process. When a tcp connection has been started or accepted,
it must finish the handshake progress in less than two seconds, or it
will be disconnected for handshake timeout. And the handshake queue has
a capacity that limited by the DefaultMaxPeers according to P2P protocol.
*/
type handshakeQueue struct {
	sync.Mutex
	capChan chan protocol.Noder
	conns   map[protocol.Noder]net.Conn
}

func (q *handshakeQueue) init() {
	q.capChan = make(chan protocol.Noder, protocol.DefaultMaxPeers)
	q.conns = make(map[protocol.Noder]net.Conn, protocol.DefaultMaxPeers)
}

func (q *handshakeQueue) AddToHandshakeQueue(node protocol.Noder) {
	q.capChan <- node

	q.Lock()
	q.conns[node] = node.GetConn()
	q.Unlock()

	// Close handshake timeout connections
	go q.handleTimeout(node)
}

func (q *handshakeQueue) RemoveFromHandshakeQueue(node protocol.Noder) {
	q.Lock()
	if _, ok := q.conns[node]; ok {
		delete(q.conns, node)
		<-q.capChan
	}
	q.Unlock()
}

func (q *handshakeQueue) handleTimeout(node protocol.Noder) {
	time.Sleep(time.Second * protocol.HandshakeTimeout)
	q.Lock()
	if conn, ok := q.conns[node]; ok {
		conn.Close()
		delete(q.conns, node)
		<-q.capChan
	}
	q.Unlock()
}
