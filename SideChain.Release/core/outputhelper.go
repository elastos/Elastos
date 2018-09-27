package core

import (
	"io"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

var OutputHelper *OutputHelperBase

type OutputHelperBase struct {
	Serialize   func(output *Output, w io.Writer) error
	Deserialize func(output *Output, r io.Reader) error
}

func InitOutputHelper() {
	OutputHelper = &OutputHelperBase{}
	OutputHelper.Init()
}

func (h *OutputHelperBase) Init() {
	h.Serialize = h.serialize
	h.Deserialize = h.deserialize
}

func (h *OutputHelperBase) serialize(output *Output, w io.Writer) error {
	err := output.AssetID.Serialize(w)
	if err != nil {
		return err
	}

	err = output.Value.Serialize(w)
	if err != nil {
		return err
	}

	WriteUint32(w, output.OutputLock)

	err = output.ProgramHash.Serialize(w)
	if err != nil {
		return err
	}

	return nil
}

func (h *OutputHelperBase) deserialize(output *Output, r io.Reader) error {
	err := output.AssetID.Deserialize(r)
	if err != nil {
		return err
	}

	err = output.Value.Deserialize(r)
	if err != nil {
		return err
	}

	temp, err := ReadUint32(r)
	if err != nil {
		return err
	}
	output.OutputLock = uint32(temp)

	err = output.ProgramHash.Deserialize(r)
	if err != nil {
		return err
	}

	return nil
}
