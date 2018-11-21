package core

import (
	"bytes"
	"errors"
	"io"

	. "github.com/elastos/Elastos.ELA.Utility/common"
)

type PayloadVoteProducer struct {
	Voter      string
	Stake      Fixed64
	PublicKeys []string
}

func (a *PayloadVoteProducer) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := a.Serialize(buf, version); err != nil {
		return []byte{0}
	}
	return buf.Bytes()
}

func (a *PayloadVoteProducer) Serialize(w io.Writer, version byte) error {
	err := WriteVarString(w, a.Voter)
	if err != nil {
		return errors.New("[PayloadVoteProducer], Voter serialize failed.")
	}

	err = a.Stake.Serialize(w)
	if err != nil {
		return errors.New("[PayloadVoteProducer], Stake serialize failed.")
	}

	if err := WriteVarUint(w, uint64(len(a.PublicKeys))); err != nil {
		return errors.New("[PayloadVoteProducer], PublicKeys length serialize failed.")
	}

	for _, pk := range a.PublicKeys {
		if err := WriteVarString(w, pk); err != nil {
			return errors.New("[PayloadVoteProducer], PublicKeys serialize failed.")
		}
	}
	return nil
}

func (a *PayloadVoteProducer) Deserialize(r io.Reader, version byte) error {
	voter, err := ReadVarString(r)
	if err != nil {
		return errors.New("[PayloadVoteProducer], Voter deserialize failed.")
	}
	a.Voter = voter

	err = a.Stake.Deserialize(r)
	if err != nil {
		return errors.New("[PayloadVoteProducer], Stake deserialize failed.")
	}

	length, err := ReadVarUint(r, 0)
	if err != nil {
		return errors.New("[PayloadVoteProducer], PublicKeys length deserialize failed.")
	}

	a.PublicKeys = make([]string, 0)
	for i := uint64(0); i < length; i++ {
		pk, err := ReadVarString(r)
		if err != nil {
			return errors.New("[PayloadVoteProducer], PublicKeys deserialize failed.")
		}
		a.PublicKeys = append(a.PublicKeys, pk)
	}
	return nil
}
