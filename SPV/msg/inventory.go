package msg

import (
	"bytes"
	"encoding/binary"
	. "github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/common/serialize"
)

type Inventory struct {
	Type  uint8
	Count uint32
	Data  []byte
}

func (msg *Inventory) CMD() string {
	return "inv"
}

func (msg *Inventory) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := serialize.WriteElements(buf, msg.Type, msg.Count, msg.Data)
	if err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}

func (msg *Inventory) Deserialize(body []byte) error {
	buf := bytes.NewReader(body)
	err := serialize.ReadElements(buf, &msg.Type, &msg.Count)
	if err != nil {
		return err
	}

	msg.Data = make([]byte, msg.Count*UINT256SIZE)
	err = binary.Read(buf, binary.LittleEndian, &msg.Data)
	if err != nil {
		return err
	}

	return nil
}
