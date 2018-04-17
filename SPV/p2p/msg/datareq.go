package msg

import (
	"bytes"
	. "github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/common/serialize"
)

type DataReq struct {
	Type uint8
	Hash Uint256
}

func (msg *DataReq) CMD() string {
	return "getdata"
}

func (msg *DataReq) Serialize() ([]byte, error) {
	buf := new(bytes.Buffer)
	err := serialize.WriteElements(buf, msg.Type, msg.Hash)
	if err != nil {
		return nil, err
	}

	return buf.Bytes(), nil
}

func (msg *DataReq) Deserialize(body []byte) error {
	buf := bytes.NewReader(body)
	err := serialize.ReadElements(buf, &msg.Type, &msg.Hash)
	if err != nil {
		return err
	}

	return nil
}
