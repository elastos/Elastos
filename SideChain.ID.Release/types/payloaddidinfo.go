package types

import (
	"bytes"
	"encoding/json"
	"errors"
	"io"
	"strings"

	"github.com/elastos/Elastos.ELA/common"

	"github.com/elastos/Elastos.ELA.SideChain.ID/types/base64url"
)

const DIDInfoVersion = 0x00
const DID_ELASTOS_PREFIX = "did:elastos:"
const (
	Create_DID_Operation     = "create"
	Update_DID_Operation     = "update"
	Deactivate_DID_Operation = "deactivate"
)

// header of DID transaction payload
type DIDHeaderInfo struct {
	Specification string `json:"specification"`
	Operation     string `json:"operation"`
	PreviousTxid  string `json:"previousTxid,omitempty"`
}

func IsURIHasPrefix(did string) bool {
	return strings.HasPrefix(did, DID_ELASTOS_PREFIX)
}
func GetDIDFromUri(idURI string) string {
	index := strings.LastIndex(idURI, ":")
	if index == -1 {
		return ""
	}
	return idURI[index+1:]
}
func (d *DIDHeaderInfo) Serialize(w io.Writer, version byte) error {
	if err := common.WriteVarString(w, d.Specification); err != nil {
		return errors.New("[DIDHeaderInfo], Specification serialize failed.")
	}

	if err := common.WriteVarString(w, d.Operation); err != nil {
		return errors.New("[DIDHeaderInfo], Operation serialize failed.")
	}
	if d.Operation == Update_DID_Operation {
		if err := common.WriteVarString(w, d.PreviousTxid); err != nil {
			return errors.New("[DIDHeaderInfo], PreviousTxid serialize failed.")
		}
	}

	return nil
}

func (d *DIDHeaderInfo) Deserialize(r io.Reader, version byte) error {
	var err error
	d.Specification, err = common.ReadVarString(r)
	if err != nil {
		return errors.New("[DIDHeaderInfo], Specification deserialize failed.")
	}

	d.Operation, err = common.ReadVarString(r)
	if err != nil {
		return errors.New("[DIDHeaderInfo], Operation deserialize failed.")
	}
	if d.Operation == Update_DID_Operation {
		d.PreviousTxid, err = common.ReadVarString(r)
		if err != nil {
			return errors.New("[DIDHeaderInfo], PreviousTxid deserialize failed.")
		}
	}

	return nil
}

// proof of DID transaction payload
type DIDProofInfo struct {
	Type               string `json:"type"`
	VerificationMethod string `json:"verificationMethod"`
	Signature          string `json:"signature"`
}

func (d *DIDProofInfo) Serialize(w io.Writer, version byte) error {
	if err := common.WriteVarString(w, d.Type); err != nil {
		return errors.New("[DIDProofInfo], Type serialize failed.")
	}

	if err := common.WriteVarString(w, d.VerificationMethod); err != nil {
		return errors.New("[DIDProofInfo], VerificationMethod serialize failed.")
	}

	if err := common.WriteVarString(w, d.Signature); err != nil {
		return errors.New("[DIDProofInfo], Signature serialize failed.")
	}
	return nil
}

func (d *DIDProofInfo) Deserialize(r io.Reader, version byte) error {
	var err error
	d.Type, err = common.ReadVarString(r)
	if err != nil {
		return errors.New("[DIDProofInfo], Type deserialize failed.")
	}

	d.VerificationMethod, err = common.ReadVarString(r)
	if err != nil {
		return errors.New("[DIDProofInfo], VerificationMethod deserialize failed.")
	}

	d.Signature, err = common.ReadVarString(r)
	if err != nil {
		return errors.New("[DIDProofInfo], Signature deserialize failed.")
	}
	return nil
}

// public keys of payload in DID transaction payload
type DIDPublicKeyInfo struct {
	ID              string `json:"id"`
	Type            string `json:"type"`
	Controller      string `json:"controller"`
	PublicKeyBase58 string `json:"publicKeyBase58"`
}

func (p *DIDPublicKeyInfo) Serialize(w io.Writer, version byte) error {

	if err := common.WriteVarString(w, p.ID); err != nil {
		return errors.New("[DIDPublicKeyInfo], ID serialize failed.")
	}
	if err := common.WriteVarString(w, p.Type); err != nil {
		return errors.New("[DIDPublicKeyInfo], Type serialize failed.")
	}
	if err := common.WriteVarString(w, p.Controller); err != nil {
		return errors.New("[DIDPublicKeyInfo], Controller serialize failed.")
	}
	if err := common.WriteVarString(w, p.PublicKeyBase58); err != nil {
		return errors.New("[DIDPublicKeyInfo], PublicKeyBase58 serialize failed.")
	}

	return nil
}

func (p *DIDPublicKeyInfo) Deserialize(r io.Reader, version byte) error {
	id, err := common.ReadVarString(r)
	if err != nil {
		return errors.New("[DIDPublicKeyInfo], ID deserialize failed")
	}
	p.ID = id

	typePkInfo, err := common.ReadVarString(r)
	if err != nil {
		return errors.New("[DIDPublicKeyInfo], Type deserialize failed")
	}
	p.Type = typePkInfo

	controller, err := common.ReadVarString(r)
	if err != nil {
		return errors.New("[DIDPublicKeyInfo], Controller deserialize failed")
	}
	p.Controller = controller

	pkBase58, err := common.ReadVarString(r)
	if err != nil {
		return errors.New("[DIDPublicKeyInfo], PublicKeyBase58 deserialize failed")
	}
	p.PublicKeyBase58 = pkBase58

	return nil
}

// payload in DID transaction payload
type DIDPayloadInfo struct {
	ID             string             `json:"id"`
	PublicKey      []DIDPublicKeyInfo `json:"publicKey"`
	Authentication []interface{}      `json:"authentication"`
	Authorization  []interface{}      `json:"authorization"`
	Expires        string             `json:"expires"`
}

// payload of DID transaction
type Operation struct {
	Header  DIDHeaderInfo `json:"header"`
	Payload string        `json:"payload"`
	Proof   DIDProofInfo  `json:"proof"`

	PayloadInfo *DIDPayloadInfo
}

type TranasactionData struct {
	TXID      string    `json:"txid"`
	Timestamp string    `json:"timestamp"`
	Operation Operation `json:"operation"`
}

func (p *TranasactionData) Serialize(w io.Writer, version byte) error {
	if err := common.WriteVarString(w, p.TXID); err != nil {
		return errors.New("[TranasactionData], TXID serialize failed")
	}

	if err := common.WriteVarString(w, p.Timestamp); err != nil {
		return errors.New("[TranasactionData], Timestamp serialize failed")
	}

	if err := p.Operation.Serialize(w, version); err != nil {
		return errors.New("[TranasactionData] Operation serialize failed," +
			"" + err.Error())
	}

	return nil
}

func (p *Operation) Data(version byte) []byte {
	buf := new(bytes.Buffer)
	if err := p.Header.Serialize(buf, version); err != nil {
		return nil
	}
	if err := common.WriteVarString(buf, p.Payload); err != nil {
		return nil
	}
	return buf.Bytes()
}

func (p *Operation) Serialize(w io.Writer, version byte) error {
	if err := p.Header.Serialize(w, version); err != nil {
		return errors.New("[Operation], Header serialize failed," + err.Error())
	}

	if err := common.WriteVarString(w, p.Payload); err != nil {
		return errors.New("[Operation], Payload serialize failed")
	}

	if err := p.Proof.Serialize(w, version); err != nil {
		return errors.New("[Operation], Proof serialize failed," + err.Error())
	}

	return nil
}

func (p *Operation) Deserialize(r io.Reader, version byte) error {
	if err := p.Header.Deserialize(r, version); err != nil {
		return errors.New("[DIDInfo], Header deserialize failed" + err.Error())
	}

	payload, err := common.ReadVarString(r)
	if err != nil {
		return errors.New("[DIDInfo], payload deserialize failed")
	}
	p.Payload = payload

	if err := p.Proof.Deserialize(r, version); err != nil {
		return errors.New("[DIDInfo], Proof deserialize failed," + err.Error())
	}

	// get DIDPayloadInfo from payload data
	pBytes, err := base64url.DecodeString(p.Payload)
	if err != nil {
		return errors.New("[DIDInfo], payload decode failed")
	}
	payloadInfo := new(DIDPayloadInfo)
	if err := json.Unmarshal(pBytes, payloadInfo); err != nil {
		return errors.New("[DIDInfo], payload unmarshal failed")
	}
	p.PayloadInfo = payloadInfo
	return nil
}

func (p *Operation) GetData() []byte {
	var dataString string
	if p.Header.Operation == Update_DID_Operation {
		dataString = p.Header.Specification + p.Header.Operation + p.Header.
			PreviousTxid + p.Payload

	} else {
		dataString = p.Header.Specification + p.Header.Operation + p.Payload

	}
	return []byte(dataString)
}
