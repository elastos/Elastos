package types

import (
	"io"
	"bytes"

	"github.com/elastos/Elastos.ELA/common"

	"github.com/elastos/Elastos.ELA.SideChain.NeoVM/avm"
)


type PayloadInvoke struct {
	CodeHash    common.Uint168
	Code        []byte
	ProgramHash common.Uint168
	Gas         common.Fixed64
}

func (ic *PayloadInvoke) Data(version byte) []byte{
	buf := new(bytes.Buffer)
	err := ic.Serialize(buf, version)
	if err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (ic *PayloadInvoke) Serialize(w io.Writer, version byte) error {
	err := ic.CodeHash.Serialize(w)
	if err != nil {
		return err
	}
	err = common.WriteVarBytes(w, ic.Code)
	if err != nil {
		return err
	}
	err = ic.ProgramHash.Serialize(w)
	if err != nil {
		return err
	}
	err = ic.Gas.Serialize(w)
	if err != nil {
		return err
	}
	return nil
}

func (ic *PayloadInvoke) Deserialize(r io.Reader, version byte) error {
	codeHash := common.Uint168{}
	err := codeHash.Deserialize(r)
	if err != nil {
		return err
	}
	ic.CodeHash = codeHash

	ic.Code, err = common.ReadVarBytes(r, avm.MaxItemSize, "PayloadInvoke Deserialize Code")
	if err != nil {
		return err
	}

	programHash := common.Uint168{}
	err = programHash.Deserialize(r)
	if err != nil {
		return err
	}
	ic.ProgramHash = programHash

	gas := common.Fixed64(0)
	err = gas.Deserialize(r)
	if err != nil {
		return err
	}
	ic.Gas = gas

	return nil
}