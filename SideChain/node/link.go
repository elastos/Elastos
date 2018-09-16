package node

import (
	"crypto/tls"
	"crypto/x509"
	"errors"
	"fmt"
	"io/ioutil"
	"net"
	"strings"
	"sync"
	"time"

	. "github.com/elastos/Elastos.ELA.SideChain/config"
	"github.com/elastos/Elastos.ELA.SideChain/log"
	. "github.com/elastos/Elastos.ELA.SideChain/protocol"

	. "github.com/elastos/Elastos.ELA.Utility/p2p"
)

type link struct {
	addr         string   // The address of the Node
	conn         net.Conn // Connect socket with the peer Node
	port         uint16   // The server port of the Node
	httpInfoPort uint16   // The Node information server port of the Node
	activeLock   sync.RWMutex
	lastActive   time.Time // The latest time the Node activity
	handshakeQueue
	*MsgHelper
}

func (link *link) CloseConn() {
	link.conn.Close()
}

func (node *Node) UpdateLastActive() {
	node.activeLock.Lock()
	defer node.activeLock.Unlock()
	node.lastActive = time.Now()
}

func (node *Node) GetLastActiveTime() time.Time {
	node.activeLock.RLock()
	defer node.activeLock.RUnlock()
	return node.lastActive
}

func (node *Node) InitConnection() {
	go node.listenNodePort()
}

func (node *Node) listenNodePort() {
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

	node.listenConnections(listener)
}

func (n *Node) listenConnections(listener net.Listener) {
	defer listener.Close()

	for {
		conn, err := listener.Accept()
		if err != nil {
			log.Error("Error accepting", err.Error())
			continue
		}
		log.Infof("Remote Node %v connect with %v", conn.RemoteAddr(), conn.LocalAddr())

		node := NewNode(Parameters.Magic, conn)
		node.addr, err = parseIPaddr(conn.RemoteAddr().String())
		node.Read()
		LocalNode.AddToHandshakeQueue(node)
		LocalNode.RemoveFromHandshakeQueue(node)
	}
}

func initNonTlsListen() (net.Listener, error) {
	listener, err := net.Listen("tcp", fmt.Sprint(":", Parameters.NodePort))
	if err != nil {
		log.Error("Error listening\n", err.Error())
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

func parseIPaddr(s string) (string, error) {
	i := strings.Index(s, ":")
	if i < 0 {
		log.Warn("Split IP address&port error")
		return s, errors.New("Split IP address&port error")
	}
	return s[:i], nil
}

func (node *Node) Connect(nodeAddr string) error {
	log.Debug()

	if node.IsAddrInNbrList(nodeAddr) == true {
		return nil
	}
	if added := node.AddToConnectionList(nodeAddr); added == false {
		return errors.New("Node exist in connecting list, cancel")
	}

	isTls := Parameters.IsTLS
	var conn net.Conn
	var err error

	if isTls {
		conn, err = TLSDial(nodeAddr)
		if err != nil {
			node.RemoveFromConnectingList(nodeAddr)
			log.Error("TLS connect failed: ", err)
			return err
		}
	} else {
		conn, err = NonTLSDial(nodeAddr)
		if err != nil {
			node.RemoveFromConnectingList(nodeAddr)
			log.Error("non TLS connect failed: ", err)
			return err
		}
	}
	n := NewNode(Parameters.Magic, conn)
	n.addr, err = parseIPaddr(conn.RemoteAddr().String())

	log.Infof("Local Node %s connect with %s with %s",
		conn.LocalAddr().String(), conn.RemoteAddr().String(),
		conn.RemoteAddr().Network())
	n.Read()

	n.SetState(HAND)
	n.Send(NewVersion(node))

	node.AddToHandshakeQueue(n)
	return nil
}

func NonTLSDial(nodeAddr string) (net.Conn, error) {
	log.Debug()
	conn, err := net.DialTimeout("tcp", nodeAddr, time.Second*DialTimeout)
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
	dialer.Timeout = time.Second * DialTimeout
	conn, err := tls.DialWithDialer(&dialer, "tcp", nodeAddr, conf)
	if err != nil {
		return nil, err
	}
	return conn, nil
}

func (node *Node) Send(msg Message) {
	if node.State() == INACTIVITY {
		return
	}

	node.MsgHelper.Write(msg)
	node.UpdateLastActive()
}
