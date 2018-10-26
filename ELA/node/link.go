package node

import (
	"crypto/tls"
	"crypto/x509"
	"errors"
	"fmt"
	"io"
	"io/ioutil"
	"net"
	"sync/atomic"
	"time"

	. "github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/log"
	. "github.com/elastos/Elastos.ELA/protocol"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
	"github.com/elastos/Elastos.ELA.Utility/p2p/msg"
)

const (
	// idleTimeout is the duration of inactivity before we time out a peer.
	idleTimeout = 5 * time.Minute
)

type link struct {
	magic        uint32
	inbound      bool
	disconnect   int32
	addr         string // The address of the node
	ip           net.IP
	conn         net.Conn // Connect socket with the peer node
	port         uint16   // The server port of the node
	httpInfoPort uint16   // The node information server port of the node
	handshakeQueue
	handler   Handler
	sendQueue chan p2p.Message
	quit      chan struct{}
}

func (node *node) String() string {
	direction := "outbound"
	if node.inbound {
		direction = "inbound"
	}
	return fmt.Sprintf("%s (%s)", node.addr, direction)
}

func (node *node) UpdateHandler(handler Handler) {
	node.handler = handler
}

func (node *node) initConnection() {
	go listenNodePort()
	// Listen open port if OpenService enabled
	if Parameters.OpenService {
		go listenNodeOpenPort()
	}
}

func (node *node) start() {
	go node.inHandler()
	go node.outHandler()
	go node.pingHandler()
}

func listenNodePort() {
	var err error
	var listener net.Listener

	if Parameters.IsTLS {
		listener, err = initTlsListen()
		if err != nil {
			log.Error("TLS listen failed")
			return
		}
	} else {
		listener, err = initNonTlsListen()
		if err != nil {
			log.Error("non TLS listen failed")
			return
		}
	}
	defer listener.Close()

	for {
		conn, err := listener.Accept()
		if err != nil {
			log.Errorf("Can't accept connection: %v", err)
			continue
		}

		node := NewNode(conn, true)
		LocalNode.AddToHandshakeQueue(conn.RemoteAddr().String(), node)
	}
}

func initNonTlsListen() (net.Listener, error) {
	listener, err := net.Listen("tcp", fmt.Sprint(":", Parameters.NodePort))
	if err != nil {
		log.Error("Error listening", err)
		return nil, err
	}
	return listener, nil
}

func initTlsListen() (net.Listener, error) {
	CertPath := Parameters.CertPath
	KeyPath := Parameters.KeyPath
	CAPath := Parameters.CAPath

	// load cert
	cert, err := tls.LoadX509KeyPair(CertPath, KeyPath)
	if err != nil {
		log.Error("load keys fail", err)
		return nil, err
	}
	// load root ca
	caData, err := ioutil.ReadFile(CAPath)
	if err != nil {
		log.Error("read ca fail", err)
		return nil, err
	}
	pool := x509.NewCertPool()
	ret := pool.AppendCertsFromPEM(caData)
	if !ret {
		return nil, errors.New("failed to parse root certificate")
	}

	tlsConfig := &tls.Config{
		Certificates: []tls.Certificate{cert},
		RootCAs:      pool,
		ClientAuth:   tls.RequireAndVerifyClientCert,
		ClientCAs:    pool,
	}

	log.Info("TLS listen port is", Parameters.NodePort)
	listener, err := tls.Listen("tcp", fmt.Sprint(":", Parameters.NodePort), tlsConfig)
	if err != nil {
		log.Error(err)
		return nil, err
	}
	return listener, nil
}

func resolveTCPAddr(addr string) (string, error) {
	tcpAddr, err := net.ResolveTCPAddr("tcp", addr)
	if err != nil {
		log.Debugf("Can not resolve address %s", addr)
		return addr, err
	}

	return tcpAddr.String(), nil
}

func (node *node) Connect(addr string) error {
	// Resolve tcpAddr address first
	tcpAddr, err := resolveTCPAddr(addr)
	if err != nil {
		return err
	}
	log.Debugf("Addr %s, resolved tcpAddr %s", addr, tcpAddr)

	if node.IsNeighborAddr(tcpAddr) {
		log.Debugf("addr %s in neighbor list, cancel", addr)
		return nil
	}
	if !node.AddToConnectingList(tcpAddr) {
		log.Debugf("addr %s in connecting list, cancel", addr)
		return nil
	}

	var conn net.Conn

	if Parameters.IsTLS {
		conn, err = TLSDial(addr)
		if err != nil {
			node.RemoveFromConnectingList(tcpAddr)
			log.Error("TLS connect failed:", err)
			return err
		}
	} else {
		conn, err = NonTLSDial(tcpAddr)
		if err != nil {
			node.RemoveFromConnectingList(tcpAddr)
			log.Error("non TLS connect failed:", err)
			return err
		}
	}

	n := NewNode(conn, false)
	n.SetState(HAND)
	n.SendMessage(NewVersion(node))

	node.AddToHandshakeQueue(tcpAddr, n)
	return nil
}

func NonTLSDial(nodeAddr string) (net.Conn, error) {
	log.Debug()
	conn, err := net.DialTimeout("tcp", nodeAddr, dialTimeout)
	if err != nil {
		return nil, err
	}
	return conn, nil
}

func TLSDial(nodeAddr string) (net.Conn, error) {
	CertPath := Parameters.CertPath
	KeyPath := Parameters.KeyPath
	CAPath := Parameters.CAPath

	clientCertPool := x509.NewCertPool()

	cacert, err := ioutil.ReadFile(CAPath)
	cert, err := tls.LoadX509KeyPair(CertPath, KeyPath)
	if err != nil {
		return nil, err
	}

	ret := clientCertPool.AppendCertsFromPEM(cacert)
	if !ret {
		return nil, errors.New("failed to parse root certificate")
	}

	conf := &tls.Config{
		RootCAs:      clientCertPool,
		Certificates: []tls.Certificate{cert},
	}

	var dialer net.Dialer
	dialer.Timeout = dialTimeout
	conn, err := tls.DialWithDialer(&dialer, "tcp", nodeAddr, conf)
	if err != nil {
		return nil, err
	}
	return conn, nil
}

func (node *node) readMessage() (p2p.Message, error) {
	return p2p.ReadMessage(node.conn, node.magic, node.handler.MakeEmptyMessage)
}

// shouldHandleReadError returns whether or not the passed error, which is
// expected to have come from reading from the remote peer in the inHandler,
// should be logged and responded to with a reject message.
func (node *node) shouldHandleReadError(err error) bool {
	// No logging or reject message when the peer is being forcibly
	// disconnected.
	if atomic.LoadInt32(&node.disconnect) != 0 {
		return false
	}

	// No logging or reject message when the remote peer has been
	// disconnected.
	if err == io.EOF {
		return false
	}
	if opErr, ok := err.(*net.OpError); ok && !opErr.Temporary() {
		return false
	}

	return true
}

func (node *node) inHandler() {
	// The timer is stopped when a new message is received and reset after it
	// is processed.
	idleTimer := time.AfterFunc(idleTimeout, func() {
		log.Warnf("Peer %s no answer for %s -- disconnecting", node, idleTimeout)
		node.Disconnect()
	})
out:
	for atomic.LoadInt32(&node.disconnect) == 0 {
		// Read a message and stop the idle timer as soon as the read
		// is done.  The timer is reset below for the next iteration if
		// needed.
		rmsg, err := node.readMessage()
		idleTimer.Stop()
		if err != nil {
			// Only log the error and send reject message if the
			// local peer is not forcibly disconnecting and the
			// remote peer has not disconnected.
			if node.shouldHandleReadError(err) {
				errMsg := fmt.Sprintf("Can't read message from %s: %v", node, err)
				if err != io.ErrUnexpectedEOF {
					log.Errorf(errMsg)
				}

				// Push a reject message for the malformed message and wait for
				// the message to be sent before disconnecting.
				//
				// NOTE: Ideally this would include the command in the header if
				// at least that much of the message was valid, but that is not
				// currently exposed by wire, so just used malformed for the
				// command.
				reject := msg.NewReject("malformed", msg.RejectMalformed, errMsg)
				node.SendMessage(reject)
			}
			break out
		}

		node.handler.HandleMessage(rmsg)

		// A message was received so reset the idle timer.
		idleTimer.Reset(idleTimeout)
	}

	// Ensure the idle timer is stopped to avoid leaking the resource.
	idleTimer.Stop()

	// Ensure connection is closed.
	node.Disconnect()

	log.Debugf("Peer input handler done for %s", node)
}

func (node *node) outHandler() {
out:
	for {
		select {
		case smsg := <-node.sendQueue:
			err := p2p.WriteMessage(node.conn, node.magic, smsg)
			if err != nil {
				node.Disconnect()
				continue
			}

		case <-node.quit:
			break out
		}
	}

	// Drain any wait channels before going away so there is nothing left
	// waiting on this goroutine.
cleanup:
	for {
		select {
		case <-node.sendQueue:
		default:
			break cleanup
		}
	}
	log.Debugf("Peer output handler done for %s", node)
}

func (node *node) SendMessage(msg p2p.Message) {
	if atomic.LoadInt32(&node.disconnect) != 0 {
		return
	}

	node.sendQueue <- msg
}

func (node *node) Connected() bool {
	return atomic.LoadInt32(&node.disconnect) == 0
}

func (node *node) Disconnect() {
	if atomic.AddInt32(&node.disconnect, 1) != 1 {
		return
	}
	node.SetState(INACTIVITY)

	log.Debugf("Disconnecting %s", node)
	node.conn.Close()
	close(node.quit)
}
