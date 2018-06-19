package node

import (
	"crypto/tls"
	"crypto/x509"
	"errors"
	"fmt"
	"io/ioutil"
	"net"
	"strings"
	"time"

	. "github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/log"
	. "github.com/elastos/Elastos.ELA/protocol"

	"github.com/elastos/Elastos.ELA.Utility/p2p"
)

type link struct {
	addr         string    // The address of the node
	conn         net.Conn  // Connect socket with the peer node
	port         uint16    // The server port of the node
	httpInfoPort uint16    // The node information server port of the node
	lastActive   time.Time // The latest time the node activity
	handshakeQueue
	*p2p.MsgHelper
}

func (link *link) CloseConn() {
	link.conn.Close()
}

func (node *node) UpdateLastActive() {
	node.lastActive = time.Now()
}

func (node *node) GetLastActiveTime() time.Time {
	return node.lastActive
}

func (node *node) initConnection() {
	go listenNodePort()
	// Listen open port if OpenService enabled
	if Parameters.OpenService {
		go listenNodeOpenPort()
	}
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
			log.Error("Error accepting", err.Error())
			return
		}
		log.Infof("Remote node %v connect with %v", conn.RemoteAddr(), conn.LocalAddr())

		node := NewNode(Parameters.Magic, conn)
		node.addr, err = parseIPaddr(conn.RemoteAddr().String())
		node.Read()
		LocalNode.AddToHandshakeQueue(node)
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

func (node *node) Connect(nodeAddr string) error {
	log.Debug()

	if node.IsAddrInNbrList(nodeAddr) {
		return nil
	}
	if !node.AddToConnectingList(nodeAddr) {
		return errors.New("node exist in connecting list, cancel")
	}

	isTls := Parameters.IsTLS
	var conn net.Conn
	var err error

	if isTls {
		conn, err = TLSDial(nodeAddr)
		if err != nil {
			node.RemoveFromConnectingList(nodeAddr)
			log.Error("TLS connect failed:", err)
			return err
		}
	} else {
		conn, err = NonTLSDial(nodeAddr)
		if err != nil {
			node.RemoveFromConnectingList(nodeAddr)
			log.Error("non TLS connect failed:", err)
			return err
		}
	}
	n := NewNode(Parameters.Magic, conn)
	n.addr, err = parseIPaddr(conn.RemoteAddr().String())

	log.Infof("Local node %s connect with %s with %s",
		conn.LocalAddr().String(), conn.RemoteAddr().String(),
		conn.RemoteAddr().Network())
	n.Read()

	n.SetState(p2p.HAND)
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

func (node *node) Send(msg p2p.Message) {
	if node.State() == p2p.INACTIVITY {
		return
	}

	node.MsgHelper.Write(msg)
}
