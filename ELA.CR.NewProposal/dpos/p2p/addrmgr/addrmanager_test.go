// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package addrmgr_test

import (
	"crypto/rand"
	"fmt"
	"net"
	"testing"

	"github.com/elastos/Elastos.ELA/dpos/p2p/addrmgr"
)

// Put some IP in here for convenience. Points to google.
var someIP = "173.194.115.66"

func TestStartStop(t *testing.T) {
	n := addrmgr.New("teststartstop")
	n.Start()
	n.Stop()
}

func TestGetAddress(t *testing.T) {
	n := addrmgr.New("testgetaddress")

	var pid [33]byte
	rand.Read(pid[:])

	// Get an address from an empty set (should error)
	if rv := n.GetAddress(pid); rv != nil {
		t.Errorf("GetAddress failed: got: %v want: %v\n", rv, nil)
	}

	// Add a new address and get it
	na := &net.TCPAddr{IP: net.ParseIP(someIP), Port: 8333}
	n.AddAddress(pid, na)
	ka := n.GetAddress(pid)
	if ka == nil {
		t.Fatalf("Did not get an address where there is one in the pool")
	}
	if ka.String() != fmt.Sprintf("%s:8333", someIP) {
		t.Errorf("Wrong IP: got %v, want %v", ka.String(),
			fmt.Sprintf("%s:8333", someIP))
	}
}

func TestSavePeers(t *testing.T) {
	am := addrmgr.New("./")
	am.Start()
	for i := 0; i < 100; i++ {
		var pid [33]byte
		rand.Read(pid[:])

		na := &net.TCPAddr{IP: net.ParseIP(someIP), Port: 8333 + i}
		am.AddAddress(pid, na)
	}

	am.Stop()
}

func TestLoadPeers(t *testing.T) {
	am := addrmgr.New("./")
	am.Start()
	for i := 0; i < 100; i++ {
		var pid [33]byte
		rand.Read(pid[:])

		na := &net.TCPAddr{IP: net.ParseIP(someIP), Port: 8333 + i}
		am.AddAddress(pid, na)
	}

	am.Stop()

	am.Start()
}
