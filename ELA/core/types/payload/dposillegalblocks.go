package payload

import (
	"bytes"
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/p2p/msg"
)

type CoinType uint32

const (
	ELACoin = CoinType(0)

	PayloadIllegalBlockVersion byte = 0x00
)

type BlockEvidence struct {
	Block        []byte
	BlockConfirm []byte
	Signers      [][]byte

	hash *common.Uint256
}

type DPOSIllegalBlocks struct {
	CoinType        CoinType
	BlockHeight     uint32
	Evidence        BlockEvidence
	CompareEvidence BlockEvidence

	hash *common.Uint256
}

func (b *BlockEvidence) Serialize(w io.Writer) error {
	if b.Block == nil {
		return errors.New("block data can not be null")
	}
	if err := common.WriteVarBytes(w, b.Block); err != nil {
		return err
	}
	return nil
}

func (b *BlockEvidence) Deserialize(r io.Reader) error {
	var err error
	if b.Block, err = common.ReadVarBytes(r, msg.MaxBlockSize,
		"block data"); err != nil {
		return err
	}
	return nil
}

func (b *BlockEvidence) BlockHash() common.Uint256 {
	if b.hash == nil {
		buf := new(bytes.Buffer)
		b.Serialize(buf)
		hash := common.Uint256(common.Sha256D(buf.Bytes()))
		b.hash = &hash
	}
	return *b.hash
}

func (d *DPOSIllegalBlocks) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := d.Serialize(buf, version); err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (d *DPOSIllegalBlocks) Serialize(w io.Writer, version byte) error {
	if err := common.WriteUint32(w, uint32(d.CoinType)); err != nil {
		return err
	}

	if err := common.WriteUint32(w, d.BlockHeight); err != nil {
		return err
	}

	if err := d.Evidence.Serialize(w); err != nil {
		return err
	}

	if err := d.CompareEvidence.Serialize(w); err != nil {
		return err
	}

	return nil
}

func (d *DPOSIllegalBlocks) Deserialize(r io.Reader, version byte) error {
	var err error
	var coinType uint32
	if coinType, err = common.ReadUint32(r); err != nil {
		return err
	}
	d.CoinType = CoinType(coinType)

	if d.BlockHeight, err = common.ReadUint32(r); err != nil {
		return err
	}

	if err = d.Evidence.Deserialize(r); err != nil {
		return err
	}

	if err = d.CompareEvidence.Deserialize(r); err != nil {
		return err
	}

	return nil
}

func (d *DPOSIllegalBlocks) Hash() common.Uint256 {
	if d.hash == nil {
		buf := new(bytes.Buffer)
		d.Serialize(buf, PayloadIllegalBlockVersion)
		hash := common.Uint256(common.Sha256D(buf.Bytes()))
		d.hash = &hash
	}
	return *d.hash
}

func (d *DPOSIllegalBlocks) GetBlockHeight() uint32 {
	return d.BlockHeight
}

func (d *DPOSIllegalBlocks) Type() IllegalDataType {
	return IllegalBlock
}
