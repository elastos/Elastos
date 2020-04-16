package mempool

import (
	"crypto/rand"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"testing"

	"github.com/stretchr/testify/suite"

	bc "github.com/elastos/Elastos.ELA.SideChain.ID/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain.ID/params"
	"github.com/elastos/Elastos.ELA.SideChain.ID/types"
	"github.com/elastos/Elastos.ELA.SideChain.ID/types/base64url"
	"github.com/elastos/Elastos.ELA.SideChain/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain/mempool"
	types2 "github.com/elastos/Elastos.ELA.SideChain/types"
	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/crypto"
)

type txValidatorTestSuite struct {
	suite.Suite
	validator validator
	Chain     *blockchain.BlockChain
}

func (s *txValidatorTestSuite) SetupSuite() {
	idChainStore, err := bc.NewChainStore(params.
		GenesisBlock,
		"Chain_UnitTest")
	if err != nil {
		return
	}
	s.validator.Validator = &mempool.Validator{}
	s.validator.Store = idChainStore
}

func TestTxValidatorTest(t *testing.T) {
	suite.Run(t, new(txValidatorTestSuite))
}

func (s *txValidatorTestSuite) TestCheckDIDOperation() {
	//no create ------>update
	payloadUpdateDIDInfo := getPayloadUpdateDID()
	err := s.validator.checkDIDOperation(&payloadUpdateDIDInfo.Header,
		payloadUpdateDIDInfo.PayloadInfo.ID)
	s.Equal(err.Error(), "DID WRONG OPERATION NOT EXIST")

	//doubale create
	payloadCreate := getPayloadCreateDID()
	err = s.validator.checkDIDOperation(&payloadCreate.Header,
		payloadCreate.PayloadInfo.ID)
	s.NoError(err)
}

const (
	PayloadPrivateKey = "a38aa1f5f693a13ef0cf2f1c1c0155cbcdd9386f37b0000739f8cb50af601b7b"
	TxPrivateKey      = "5fe87de21fa55d751583bd0d74532c3cc679caf67919261e0c9b2a56f547c38d"
	publicKeyStr1     = "035d3adebb69db5fbd8005c37d225cd2fd9ec50ec7fcb38ff7c4fcf9b90455cf5f"
	publicKeyStr2     = "03bfd8bd2b10e887ec785360f9b329c2ae567975c784daca2f223cb19840b51914"
	publicKeyStr3     = "035d3adebb69db5fbd8005c37d225cd2fd9ec50ec7fcb38ff7c4fcf9b90455cf5f"
	ID                = "icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN"
	operationCreate   = "create"
	operationUpdate   = "update"
)

var didPayloadBytes = []byte(
	`{
        "id" : "did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN",
        "publicKey":[{ "id": "did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#default",
                       "type":"ECDSAsecp256r1",
                       "controller":"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN",
                       "publicKeyBase58":"zxt6NyoorFUFMXA8mDBULjnuH3v6iNdZm42PyG4c1YdC"
                      }
                    ],
        "authentication":["did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#default",
                          {
                               "id": "did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#default",
                               "type":"ECDSAsecp256r1",
                               "controller":"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN",
                               "publicKeyBase58":"zNxoZaZLdackZQNMas7sCkPRHZsJ3BtdjEvM2y5gNvKJ"
                           }
                         ],
        "authorization":["did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#default"],
        "expires" : "2023-02-10T17:00:00Z"
	}`)

var didPayloadInfoBytes = []byte(
	`{
		"header":{"operation":"create","specification":"elastos/did/1.0"},
		"payload":"eyJpZCI6ImRpZDplbGFzdG9zOmliRjdnTXo1c2FObzM5MlVkN3pTQVZSblFyc0E3cHgydEMiLCJwdWJsaWNLZXkiOlt7ImlkIjoiI3ByaW1hcnkiLCJwdWJsaWNLZXlCYXNlNTgiOiJyb1FHRWVNdU1LZjdFeUFWa3loZjdxSnN5cmtGVXBUZ296WEQ4VkpoS2hpQyJ9XSwiYXV0aGVudGljYXRpb24iOlsiI3ByaW1hcnkiXSwiZXhwaXJlcyI6IjIwMjQtMTEtMjVUMDI6MDA6MDBaIn0",
		"proof":{
			"signature":"nrbHEEysMLzBR1mMVRjan9yfQtNGmK6Rqy7v9rvUpsJNoIMsY5JtEUiJvW82jW4xNlvOOEDI-VpLK_GCgjoUdQ",
			"verificationMethod":"#primary"
			}
	 }
`)

var errDIDPayloadInfoBytes = []byte(
	`{
		"header":{"operation":"create","specification":"elastos/did/1.0"},
		"payload":"eyJpZCI6ImRpZDplbGFzdG9zOmlZUTZ1alBjd21UWmZqMmtOZmZXNEJDeXRKenlqbUpkRGQiLCJwdWJsaWNLZXkiOlt7ImlkIjoiI3ByaW1hcnkiLCJwdWJsaWNLZXlCYXNlNTgiOiJ6S1JYMWtOWGVYeTVuS3NyVTVtdVR3Z2Y3ZlhRYnhXZzdpUUtCdnBlS0dCUCJ9XSwiYXV0aGVudGljYXRpb24iOlsiI3ByaW1hcnkiXX0",
		"proof":{
			"signature":"nrbHEEysMLzBR1mMVRjan9yfQtNGmK6Rqy7v9rvUpsJNoIMsY5JtEUiJvW82jW4xNlvOOEDI-VpLK_GCgjoUdQ",
			"verificationMethod":"#primary"
			}
	 }
`)

func (s *txValidatorTestSuite) TestIDChainStore_CreateDIDTx() {
	tx := &types2.Transaction{
		TxType:         0x0a,
		PayloadVersion: 0,
		Payload:        getPayloadCreateDID(),
		Inputs:         nil,
		Outputs:        nil,
		LockTime:       0,
		Programs:       nil,
		Fee:            0,
		FeePerKB:       0,
	}
	fmt.Println(tx)
	data, _ := hex.DecodeString(publicKeyStr1)
	fmt.Println(data)

	fmt.Println(len(data))
	i, _ := getDIDByPublicKey(data)
	didAddress, _ := i.ToAddress()
	fmt.Println("didAddress", didAddress)
	err := s.validator.checkRegisterDID(tx)
	s.NoError(err)

	info := new(types.Operation)
	json.Unmarshal(didPayloadInfoBytes, info)

	payloadBase64, _ := base64url.DecodeString(info.Payload)
	payloadInfo := new(types.DIDPayloadInfo)
	json.Unmarshal(payloadBase64, payloadInfo)
	info.PayloadInfo = payloadInfo

	tx.Payload = info
	err = s.validator.checkRegisterDID(tx)
	s.NoError(err)

	info.PayloadInfo.Expires = "Mon Jan _2 15:04:05 2006"
	err = s.validator.checkRegisterDID(tx)
	s.Error(err, "invalid Expires")

	info.PayloadInfo.Expires = "2006-01-02T15:04:05Z07:00"
	err = s.validator.checkRegisterDID(tx)
	s.Error(err, "invalid Expires")

	info.PayloadInfo.Expires = "2018-06-30T12:00:00Z"
	err = s.validator.checkRegisterDID(tx)
	s.NoError(err)

	info = new(types.Operation)
	json.Unmarshal(errDIDPayloadInfoBytes, info)

	payloadBase64, _ = base64url.DecodeString(info.Payload)
	payloadInfo = new(types.DIDPayloadInfo)
	json.Unmarshal(payloadBase64, payloadInfo)
	info.PayloadInfo = payloadInfo

	tx.Payload = info
	err = s.validator.checkRegisterDID(tx)
	s.Error(err, "invalid Expires")
}

func (s *txValidatorTestSuite) TestGetIDFromUri() {
	validUriFormat := "did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN"
	id := s.validator.Store.GetDIDFromUri(validUriFormat)
	s.Equal(id, "icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN")

	InvalidUriFormat := "icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN"
	id = s.validator.Store.GetDIDFromUri(InvalidUriFormat)
	s.Equal(id, "")
}

func getPayloadCreateDID() *types.Operation {
	info := new(types.DIDPayloadInfo)
	json.Unmarshal(didPayloadBytes, info)

	p := &types.Operation{
		Header: types.DIDHeaderInfo{
			Specification: "elastos/did/1.0",
			Operation:     "create",
		},
		Payload: base64url.EncodeToString(didPayloadBytes),
		Proof: types.DIDProofInfo{
			Type:               "ECDSAsecp256r1",
			VerificationMethod: "did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#default",
		},
		PayloadInfo: info,
	}

	privateKey1, _ := common.HexStringToBytes(PayloadPrivateKey)
	sign, _ := crypto.Sign(privateKey1, p.GetData())
	p.Proof.Signature = base64url.EncodeToString(sign)
	return p
}

func getPayloadUpdateDID() *types.Operation {
	info := new(types.DIDPayloadInfo)
	json.Unmarshal(didPayloadBytes, info)

	return &types.Operation{
		Header: types.DIDHeaderInfo{
			Specification: "elastos/did/1.0",
			Operation:     "update",
		},
		Payload: base64url.EncodeToString(didPayloadBytes),
		Proof: types.DIDProofInfo{
			Type:               randomString(),
			VerificationMethod: randomString(),
			Signature:          randomString(),
		},
		PayloadInfo: info,
	}
}

func randomString() string {
	a := make([]byte, 20)
	rand.Read(a)
	return common.BytesToHexString(a)
}

func getDIDPayloadBytes(id string) []byte {
	return []byte(
		"{" +
			"\"id\": \"did:elastos:" + id + "\"," +
			"\"publicKey\": [{" +
			"\"id\": \"did:elastos:" + id + "\"," +
			"\"type\": \"ECDSAsecp256r1\"," +
			"\"controller\": \"did:elastos:" + id + "\"," +
			"\"publicKeyBase58\": \"zxt6NyoorFUFMXA8mDBULjnuH3v6iNdZm42PyG4c1YdC\"" +
			"}]," +
			"\"authentication\": [" +
			"\"did:elastos:" + id + "\"" +
			"]," +
			"\"authorization\": [" +
			"\"did:elastos:" + id + "\"" +
			"]," +
			"\"expires\": \"2020-08-15T17:00:00Z\"" +
			"}",
	)
}

func getPayloadDIDInfo(id string, didOperation string) *types.Operation {
	pBytes := getDIDPayloadBytes(id)
	info := new(types.DIDPayloadInfo)
	json.Unmarshal(pBytes, info)
	p := &types.Operation{
		Header: types.DIDHeaderInfo{
			Specification: "elastos/did/1.0",
			Operation:     didOperation,
		},
		Payload: base64url.EncodeToString(pBytes),
		Proof: types.DIDProofInfo{
			Type:               "ECDSAsecp256r1",
			VerificationMethod: "did:elastos:" + id,
		},
		PayloadInfo: info,
	}
	return p
}

//didOperation must be create or update
func getDIDTx(id, didOperation string) *types2.Transaction {

	payloadDidInfo := getPayloadDIDInfo(id, didOperation)
	txn := new(types2.Transaction)
	txn.TxType = types.RegisterDID
	txn.Payload = payloadDidInfo
	return txn
}
