package types

import (
	"bytes"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
)

func TestPayloadRegisterIdentification_Deserialize(t *testing.T) {
	payload := &PayloadRegisterIdentification{
		ID:   "ij8rfb6A4Ri7c5CRE1nDVdVCUMuUxkk2c6",
		Sign: []byte{1, 1, 1},
		Contents: []RegisterIdentificationContent{
			RegisterIdentificationContent{
				Path: "kyc/person/identityCard",
				Values: []RegisterIdentificationValue{RegisterIdentificationValue{
					DataHash: common.Uint256{2, 2, 2},
					Proof:    "testproof1",
				}}},
			RegisterIdentificationContent{
				Path: "kyc/person/phone",
				Values: []RegisterIdentificationValue{RegisterIdentificationValue{
					DataHash: common.Uint256{3, 3, 3},
					Proof:    "testproof2",
				}}},
		},
	}

	buf := new(bytes.Buffer)
	if err := payload.Serialize(buf, 0); err != nil {
		t.Error("ID serialize error!")
	}

	r := bytes.NewReader(buf.Bytes())
	payload2 := PayloadRegisterIdentification{}
	payload2.Deserialize(r, RegisterIdentificationVersion)

	if payload2.ID != "ij8rfb6A4Ri7c5CRE1nDVdVCUMuUxkk2c6" {
		t.Error("ID deserialize error!")
	}

	if len(payload2.Contents) != 2 {
		t.Error("ID contents deserialize error!")
	}

	if payload2.Contents[0].Path != "kyc/person/identityCard" ||
		payload2.Contents[1].Path != "kyc/person/phone" {
		t.Error("ID content path deserialize error!")
	}

	if len(payload2.Contents[0].Values) != 1 || len(payload2.Contents[1].Values) != 1 {
		t.Error("ID content value deserialize error!")
	}

	if !payload2.Contents[0].Values[0].DataHash.IsEqual(common.Uint256{2, 2, 2}) ||
		!payload2.Contents[1].Values[0].DataHash.IsEqual(common.Uint256{3, 3, 3}) {
		t.Error("ID content values data hash deserialize error!")
	}

	if payload2.Contents[0].Values[0].Proof != "testproof1" ||
		payload2.Contents[1].Values[0].Proof != "testproof2" {
		t.Error("ID content values proof deserialize error!")
	}
}
