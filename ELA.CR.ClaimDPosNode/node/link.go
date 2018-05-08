package node

import (
	"crypto/tls"
	"crypto/x509"
	"errors"
	"fmt"
	"io/ioutil"
	"net"
	"strconv"
	"strings"
	"time"

	. "github.com/elastos/Elastos.ELA/config"
	"github.com/elastos/Elastos.ELA/log"
	"github.com/elastos/Elastos.ELA/events"
	. "github.com/elastos/Elastos.ELA/protocol"

	. "github.com/elastos/Elastos.ELA.Utility/p2p"
)

type link struct {
	addr         string    // The address of the node
	conn         net.Conn  // Connect socket with the peer node
	port         uint16    // The server port of the node
	localPort    uint16    // The local port witch the node connected with
	httpInfoPort uint16    // The node information server port of the node
	lastActive   time.Time // The latest time the node activity
	connCnt      uint64    // The connection count
	*MsgHelper
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
	go node.listenNodePort()
	go node.listenSPVPort()
}

func (node *node) listenNodePort() {
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

func (n *node) listenConnections(listener net.Listener) {
	defer listener.Close()

	for {
		conn, err := listener.Accept()
		if err != nil {
			log.Error("Error accepting ", err.Error())
			return
		}
		log.Info("Remote node connect with ", conn.RemoteAddr(), conn.LocalAddr())

		n.link.connCnt++

		node := NewNode(Parameters.Magic, conn)
		node.addr, err = parseIPaddr(conn.RemoteAddr().String())
		node.localPort = localPortFromConn(conn)
		go node.Read()
	}
}

func localPortFromConn(conn net.Conn) uint16 {
	// Get node connection port
	addr := conn.LocalAddr().String()
	portIndex := strings.LastIndex(addr, ":")
	port, _ := strconv.ParseUint(string([]byte(addr)[portIndex+1:]), 10, 16)
	return uint16(port)
}

func initNonTlsListen() (net.Listener, error) {
	listener, err := net.Listen("tcp", ":"+strconv.Itoa(Parameters.NodePort))
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

	log.Info("TLS listen port is ", strconv.Itoa(Parameters.NodePort))
	listener, err := tls.Listen("tcp", ":"+strconv.Itoa(Parameters.NodePort), tlsConfig)
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

	if node.IsAddrInNbrList(nodeAddr) == true {
		return nil
	}
	if added := node.SetAddrInConnectingList(nodeAddr); added == false {
		return errors.New("node exist in connecting list, cancel")
	}

	isTls := Parameters.IsTLS
	var conn net.Conn
	var err error

	if isTls {
		conn, err = TLSDial(nodeAddr)
		if err != nil {
			node.RemoveAddrInConnectingList(nodeAddr)
			log.Error("TLS connect failed: ", err)
			return err
		}
	} else {
		conn, err = NonTLSDial(nodeAddr)
		if err != nil {
			node.RemoveAddrInConnectingList(nodeAddr)
			log.Error("non TLS connect failed: ", err)
			return err
		}
	}
	node.link.connCnt++
	n := NewNode(Parameters.Magic, conn)
	n.addr, err = parseIPaddr(conn.RemoteAddr().String())

	log.Info(fmt.Sprintf("Connect node %s connect with %s with %s",
		conn.LocalAddr().String(), conn.RemoteAddr().String(),
		conn.RemoteAddr().Network()))
	go n.Read()

	n.SetState(HAND)
	go n.Send(NewVersion(node))

	return nil
}

func NonTLSDial(nodeAddr string) (net.Conn, error) {
	log.Debug()
	conn, err := net.DialTimeout("tcp", nodeAddr, time.Second*DIALTIMEOUT)
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
	dialer.Timeout = time.Second * DIALTIMEOUT
	conn, err := tls.DialWithDialer(&dialer, "tcp", nodeAddr, conf)
	if err != nil {
		return nil, err
	}
	return conn, nil
}

func (node *node) Send(msg Message) {
	if node.State() == INACTIVITY {
		return
	}

	buf, err := node.MsgHelper.Build(msg)
	if err != nil {
		log.Error("Serialize message failed, ", err)
		return
	}

	_, err = node.conn.Write(buf)
	if err != nil {
		log.Error("Error sending message to node ", err)
		LocalNode.eventQueue.GetEvent("disconnect").Notify(events.EventNodeDisconnect, node)
	}
}
