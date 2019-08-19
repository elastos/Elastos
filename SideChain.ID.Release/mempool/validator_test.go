package mempool

import (
	"crypto/rand"
	"encoding/hex"
	"encoding/json"
	"fmt"
	"math/big"
	"testing"

	"github.com/btcsuite/btcutil/base58"
	"github.com/stretchr/testify/suite"

	bc "github.com/elastos/Elastos.ELA.SideChain.ID/blockchain"
	"github.com/elastos/Elastos.ELA.SideChain.ID/params"
	"github.com/elastos/Elastos.ELA.SideChain.ID/types"
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
	PayloadPrivateKey = "5fe87de21fa55d751583bd0d74532c3cc679caf67919261e0c9b2a56f547c38d"
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
                       "publicKeyBase58":"27bqfhMew6TjL4NMz2u8b2cFCvGovaELqr19Xytt1rDmd"
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
	fmt.Println(len(data))
	i, _ := getDIDByPublicKey(data)
	didAddress, _ := i.ToAddress()
	fmt.Println("didAddress", didAddress)

	bi := new(big.Int).SetBytes(data).String()
	base58PK := base58.Encode([]byte(bi))

	fmt.Println("base58PK", string(base58PK))
	fmt.Println("len(base58PK）", len(base58PK))

	err2 := s.validator.checkRegisterDID(tx)
	s.NoError(err2)
}

func (s *txValidatorTestSuite) TestGetIDFromUri() {
	validUriFormat := "did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN"
	id := s.validator.Store.GetIDFromUri(validUriFormat)
	s.Equal(id, "icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN")

	InvalidUriFormat := "icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN"
	id = s.validator.Store.GetIDFromUri(InvalidUriFormat)
	s.Equal(id, "")
}

func getPayloadCreateDID() *types.PayloadDIDInfo {
	info := new(types.DIDPayloadInfo)
	json.Unmarshal(didPayloadBytes, info)

	p := &types.PayloadDIDInfo{
		Header: types.DIDHeaderInfo{
			Specification: "elastos/did/1.0",
			Operation:     "create",
		},
		Payload: hex.EncodeToString(didPayloadBytes),
		Proof: types.DIDProofInfo{
			Type:               "ECDSAsecp256r1",
			VerificationMethod: "did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#default",
		},
		PayloadInfo: info,
	}

	privateKey1, _ := common.HexStringToBytes(PayloadPrivateKey)
	sign, _ := crypto.Sign(privateKey1, p.Data(types.DIDInfoVersion))
	p.Proof.Signature = hex.EncodeToString(sign)

	return p
}

func getPayloadUpdateDID() *types.PayloadDIDInfo {
	info := new(types.DIDPayloadInfo)
	json.Unmarshal(didPayloadBytes, info)

	return &types.PayloadDIDInfo{
		Header: types.DIDHeaderInfo{
			Specification: "elastos/did/1.0",
			Operation:     "update",
		},
		Payload: hex.EncodeToString(didPayloadBytes),
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

func getPayloadDIDInfo(id string, didOperation string) *types.PayloadDIDInfo {
	pBytes := getDIDPayloadBytes(id)
	info := new(types.DIDPayloadInfo)
	json.Unmarshal(pBytes, info)
	p := &types.PayloadDIDInfo{
		Header: types.DIDHeaderInfo{
			Specification: "elastos/did/1.0",
			Operation:     didOperation,
		},
		Payload: hex.EncodeToString(pBytes),
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
