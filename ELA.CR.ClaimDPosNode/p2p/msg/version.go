// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
//

package msg

import (
	"io"
	"time"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/elanet/pact"
	"github.com/elastos/Elastos.ELA/p2p"
)

// Ensure Version implement p2p.Message interface.
var _ p2p.Message = (*Version)(nil)

type Version struct {
	Version     uint32
	Services    uint64
	Timestamp   time.Time
	Port        uint16
	Nonce       uint64
	Height      uint64
	Relay       bool
	NodeVersion string
}

func (msg *Version) CMD() string {
	return p2p.CmdVersion
}

func (msg *Version) MaxLength() uint32 {
	return 82
}

func (msg *Version) Serialize(w io.Writer) error {
	var timestamp = uint32(msg.Timestamp.Unix())
	err := common.WriteElements(w, msg.Version, msg.Services, timestamp,
		msg.Port, msg.Nonce, msg.Height, msg.Relay)
	if err != nil {
		return err
	}
	if msg.Version > pact.DPOSStartVersion {
		return common.WriteVarString(w, msg.NodeVersion)
	}
	return nil
}

func (msg *Version) Deserialize(r io.Reader) error {
	var timestamp uint32
	var err error
	err = common.ReadElements(r, &msg.Version, &msg.Services, &timestamp,
		&msg.Port, &msg.Nonce, &msg.Height, &msg.Relay)
	if err != nil {
		return err
	}
	if msg.Version > pact.DPOSStartVersion {
		msg.NodeVersion, err = common.ReadVarString(r)
		if err != nil {
			return err
		}
	}
	msg.Timestamp = time.Unix(int64(timestamp), 0)
	return nil
}

func NewVersion(pver uint32, port uint16, services, nonce, height uint64,
	disableRelayTx bool, nver string) *Version {
	return &Version{
		Version:     pver,
		Services:    services,
		Timestamp:   time.Unix(time.Now().Unix(), 0),
		Port:        port,
		Nonce:       nonce,
		Height:      height,
		Relay:       !disableRelayTx,
		NodeVersion: nver,
	}
}
