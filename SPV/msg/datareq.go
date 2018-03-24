package msg

import (
	"bytes"
	. "SPVWallet/core"
	"SPVWallet/core/serialization"
)

type DataReq struct {
	Type uint8
	Hash Uint256
}

func NewDataReq(invType uint8, hash Uint256) *DataReq {
	dataReq := new(DataReq)
	dataReq.Type = invType
	dataReq.Hash = hash
	return dataReq
}

func (msg *DataReq) CMD() string {
	return "getdata"
}

func (msg *DataReq) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := serialization.WriteUint8(buf, msg.Type)
	if err != nil {
		return nil, err
	}

	_, err = msg.Hash.Serialize(buf)
	if err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}

func (msg *DataReq) Deserialize(body []byte) error {
	var err error
	buf := bytes.NewReader(body)
	msg.Type, err = serialization.ReadUint8(buf)
	if err != nil {
		return err
	}

	err = msg.Hash.Deserialize(buf)
	if err != nil {
		return err
	}

	return nil
}
