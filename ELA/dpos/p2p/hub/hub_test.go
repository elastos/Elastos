package hub

import (
	"crypto/rand"
	"fmt"
	"net"
	"strconv"
	"testing"

	"github.com/elastos/Elastos.ELA/dpos/p2p/addrmgr"
	"github.com/elastos/Elastos.ELA/dpos/p2p/msg"
	"github.com/elastos/Elastos.ELA/p2p"
	"github.com/stretchr/testify/assert"
)

// mockConn creates a connection to the target port.
func mockConn(port int) (net.Conn, error) {
	conn, err := net.Dial("tcp", "localhost:"+strconv.Itoa(port))
	if err != nil {
		return nil, err
	}
	return conn, nil
}

// mockService mocks a service that accepting connections.
func mockService(port int) (chan net.Conn, error) {
	listener, err := net.Listen("tcp", ":"+strconv.Itoa(port))
	if err != nil {
		return nil, err
	}
	connChan := make(chan net.Conn)
	go func() {
		for {
			conn, err := listener.Accept()
			if err != nil {
				continue
			}
			connChan <- conn
		}
	}()
	return connChan, nil
}

// sendVersion write a version message to the connection.
func sendVersion(conn net.Conn, magic int, pid, target [33]byte, port int) error {
	v := msg.Version{PID: pid, Target: PIDTo16(target), Port: uint16(port)}
	return p2p.WriteMessage(conn, uint32(magic), &v)
}

func readVersion(conn net.Conn, magic int, pid, target [33]byte, port int) error {
	m, err := p2p.ReadMessage(conn, uint32(magic),
		func(cmd string) (m p2p.Message, e error) {
			if cmd != msg.CmdVersion {
				return nil, fmt.Errorf("not a version message")
			}
			return &msg.Version{}, nil
		})
	if err != nil {
		return err
	}
	v := m.(*msg.Version)
	if v.PID != pid {
		return fmt.Errorf("PID not match")
	}
	if v.Target != PIDTo16(target) {
		return fmt.Errorf("target not match")
	}
	if v.Port != uint16(port) {
		return fmt.Errorf("port not match")
	}
	return nil
}

func TestPidTo16(t *testing.T) {
	var pid [33]byte
	rand.Read(pid[:])

	key := PIDTo16(pid)
	assert.Equal(t, pid[17:], key[:])
}

// TestHub_Intercept test 3 things:
// 1. Inbound connection to main service not intercepted.
// 2. Inbound connection to sub service is intercepted and redirect to the sub
// service.
// 3. Outbound connection from sub service is intercepted and redirect to remote
// sub service.
func TestHub_Intercept(t *testing.T) {
	var mainID, subID, someID [33]byte
	rand.Read(mainID[:])
	rand.Read(subID[:])
	rand.Read(someID[:])
	var mainMagic, subMagic = 123123, 321321
	var mainPort, subPort, remotePort, somePort = 8200, 8300, 8301, 2222
	var hub = New(uint32(mainMagic), mainID, addrmgr.New("./"))
	hub.queue <- peerList{mainID, subID, someID}
	hub.admgr.AddAddress(subID, &net.TCPAddr{
		IP:   net.ParseIP("localhost"),
		Port: remotePort,
	})

	// Mock main local sub and remote sub services.
	mainSvr, err := mockService(mainPort)
	if !assert.NoError(t, err) {
		t.Fatal(err)
	}
	subSvr, err := mockService(subPort)
	if !assert.NoError(t, err) {
		t.Fatal(err)
	}
	remoteSvr, err := mockService(remotePort)
	if !assert.NoError(t, err) {
		t.Fatal(err)
	}

	// Create a connection and send a version message to main service.
	mainConn, err := mockConn(mainPort)
	if !assert.NoError(t, err) {
		t.Fatal(err)
	}
	err = sendVersion(mainConn, mainMagic, someID, mainID, somePort)
	if !assert.NoError(t, err) {
		t.Fatal(err)
	}
	mainSvrConn := hub.Intercept(<-mainSvr)
	err = readVersion(mainSvrConn, mainMagic, someID, mainID, somePort)
	if !assert.NoError(t, err) {
		t.Fatal(err)
	}

	// Create a connection and send a version message to remote sub service.
	remoteConn, err := mockConn(mainPort)
	if !assert.NoError(t, err) {
		t.Fatal(err)
	}
	err = sendVersion(remoteConn, subMagic, mainID, subID, subPort)
	if !assert.NoError(t, err) {
		t.Fatal(err)
	}
	if !assert.Nil(t, hub.Intercept(<-mainSvr)) {
		t.Fatal(err)
	}
	remoteSvrConn := <-remoteSvr
	err = readVersion(remoteSvrConn, subMagic, mainID, subID, subPort)
	if !assert.NoError(t, err) {
		t.Fatal(err)
	}

	// Create a connection and send a version message to local sub service.
	localSubConn, err := mockConn(mainPort)
	if !assert.NoError(t, err) {
		t.Fatal(err)
	}
	err = sendVersion(localSubConn, subMagic, someID, subID, somePort)
	if !assert.NoError(t, err) {
		t.Fatal(err)
	}
	if !assert.Nil(t, hub.Intercept(<-mainSvr)) {
		t.Fatal(err)
	}
	localSubSvrConn := <-subSvr
	err = readVersion(localSubSvrConn, subMagic, someID, subID, somePort)
	if !assert.NoError(t, err) {
		t.Fatal(err)
	}

	// Test sending and reading message after connection intercepted.
	for i := 0; i < 10; i++ {
		err = sendVersion(localSubConn, subMagic, someID, subID, somePort)
		if !assert.NoError(t, err) {
			t.Fatal(err)
		}
		err = readVersion(localSubSvrConn, subMagic, someID, subID, somePort)
		if !assert.NoError(t, err) {
			t.Fatal(err)
		}
	}

}
