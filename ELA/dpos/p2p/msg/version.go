// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package msg

import (
	"errors"
	"io"
	"time"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/dpos/dtime"
	"github.com/elastos/Elastos.ELA/p2p"
)

// Ensure Version implement p2p.Message interface.
var _ p2p.Message = (*Version)(nil)

type Version struct {
	PID       [33]byte
	Target    [16]byte
	Nonce     [16]byte
	Port      uint16
	Timestamp time.Time
}

func (msg *Version) CMD() string {
	return p2p.CmdVersion
}

func (msg *Version) MaxLength() uint32 {
	return 75 // 33+16+16+2+8
}

func (msg *Version) Serialize(w io.Writer) error {
	return common.WriteElements(w, msg.PID, msg.Target, msg.Nonce, msg.Port,
		msg.Timestamp.UnixNano())
}

func (msg *Version) Deserialize(r io.Reader) error {
	var timestamp int64
	err := common.ReadElements(r, &msg.PID, &msg.Target, &msg.Nonce, &msg.Port,
		&timestamp)
	if err != nil {
		return err
	}

	// A version timestamp must not have greater precision than one million
	// second.
	if timestamp%dtime.Millisecond > 0 {
		return errors.New("version timestamp have a higher precision" +
			" than one million second")
	}

	msg.Timestamp = dtime.Int64ToTime(timestamp)
	return nil
}

func NewVersion(pid [33]byte, target, nonce [16]byte, port uint16) *Version {
	return &Version{PID: pid, Target: target, Nonce: nonce, Port: port,
		Timestamp:dtime.Now()}
}
