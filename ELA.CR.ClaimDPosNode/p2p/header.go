// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package p2p

import (
	"bytes"
	"encoding/binary"
	"errors"
	"fmt"

	"github.com/elastos/Elastos.ELA/common"
)

type Header struct {
	Magic    uint32
	CMD      [CMDSize]byte
	Length   uint32
	Checksum [ChecksumSize]byte
}

func BuildHeader(magic uint32, cmd string, body []byte) *Header {
	// Calculate Checksum
	checksum := common.Sha256D(body)

	header := new(Header)
	// Write Magic
	header.Magic = magic
	// Write CMD
	copy(header.CMD[:len(cmd)], cmd)
	// Write Length
	header.Length = uint32(len(body))
	// Write Checksum
	copy(header.Checksum[:], checksum[:ChecksumSize])

	return header
}

func (header *Header) Verify(buf []byte) error {
	// Verify Checksum
	sum := common.Sha256D(buf)
	checksum := sum[:ChecksumSize]
	if !bytes.Equal(header.Checksum[:], checksum) {
		return fmt.Errorf("unmatched body checksum")
	}

	return nil
}

func (header *Header) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := binary.Write(buf, binary.LittleEndian, header)
	if err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}

func (header *Header) Deserialize(buf []byte) error {
	cmd := buf[CMDOffset : CMDOffset+CMDSize]
	end := bytes.IndexByte(cmd, 0)
	if end < 0 || end >= CMDSize {
		return errors.New("unexpected length of CMD")
	}

	hdr := bytes.NewReader(buf[:HeaderSize])
	return binary.Read(hdr, binary.LittleEndian, header)
}

func (header *Header) GetCMD() string {
	return string(bytes.TrimRight(header.CMD[:], string(0)))
}
