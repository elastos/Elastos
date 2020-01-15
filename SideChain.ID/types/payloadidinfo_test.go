package types

import (
	"bytes"
	"crypto/rand"
	"crypto/sha256"
	"encoding/hex"
	"encoding/json"
	"fmt"
	mathrand "math/rand"
	"strings"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/stretchr/testify/assert"

	"github.com/elastos/Elastos.ELA.SideChain.ID/types/base64url"
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

var didPayloadBytesNoAuth = []byte(
	`{
        "id" : "did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN",
        "publicKey":[{ "id": "did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#default",
                       "type":"ECDSAsecp256r1",
                       "controller":"did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN",
                       "publicKeyBase58":"27bqfhMew6TjL4NMz2u8b2cFCvGovaELqr19Xytt1rDmd"
                      }
                    ],
        "authorization":["did:elastos:icJ4z2DULrHEzYSvjKNJpKyhqFDxvYV7pN#default"],
        "expires" : "2023-02-10T17:00:00Z"
	}`)

func TestDIDPayloadInfo(t *testing.T) {
	// test for unmarshal did payload from bytes
	info := new(DIDPayloadInfo)
	err := json.Unmarshal(didPayloadBytes, info)
	assert.True(t, err == nil)

	infoNoAuth := new(DIDPayloadInfo)
	err = json.Unmarshal(didPayloadBytesNoAuth, infoNoAuth)
	assert.True(t, err == nil)

}

func TestPayloadDID_Serialize(t *testing.T) {
	// test for payloadDIDInfo serialize and deserialize
	payload1 := randomPayloadDID()
	buf := new(bytes.Buffer)
	payload1.Serialize(buf, DIDInfoVersion)
	payload2 := &Operation{}
	payload2.Deserialize(buf, DIDInfoVersion)
	assert.True(t, paylaodDIDInfoEqual(payload1, payload2))
}

func TestJavaDigest(t *testing.T) {
	targetDigest := "B7943F86927374CA7A7ECFBAF8F2F2405BEBF781AD8843A551012A2B188FA5A5"
	var payloadDid Operation
	payloadDid.Header.Specification = "elastos/did/1.0"
	payloadDid.Header.Operation = "create"
	payloadDid.Payload = "ICAiZG9jIjogewogICAgImlkIjogImRpZDplbGFzdG9zOmljSjR6MkRVTHJIRXpZU3ZqS05KcEt5aH\n" +
		"FGRHh2WVY3cE4iLAogICAgInB1YmxpY0tleSI6IFt7CiAgICAgICJpZCI6ICIjbWFzdGVyLWtleSIsC\n" +
		"iAgICAgICJwdWJsaWNLZXlCYXNlNTgiOiAiek54b1phWkxkYWNrWlFOTWFzN3NDa1BSSFpzSjNCdGRq\n" +
		"RXZNMnk1Z052S0oiCiAgICB9LCB7CiAgICAgICJpZCI6ICIja2V5LTIiLAogICAgICAicHVibGljS2V\n" +
		"5QmFzZTU4IjogIjI3M2o4ZlExWlpWTTZVNmQ1WEUzWDhTeVVMdUp3anlZWGJ4Tm9wWFZ1ZnRCZSIKIC\n" +
		"AgIH0sIHsKICAgICAgImlkIjogIiNyZWNvdmVyeS1rZXkiLAogICAgICAiY29udHJvbGxlciI6ICJka\n" +
		"WQ6ZWxhc3RvczppcDdudERvMm1ldEduVTh3R1A0Rm55S0NVZGJIbTRCUERoIiwKICAgICAgInB1Ymxp\n" +
		"Y0tleUJhc2U1OCI6ICJ6cHB5MzNpMnIzdUMxTFQzUkZjTHFKSlBGcFl1WlBEdUtNZUtaNVRkQXNrTSI\n" +
		"KICAgIH1dLAogICAgImF1dGhlbnRpY2F0aW9uIjogWwogICAgICAibWFzdGVyLWtleXMiLAogICAgIC\n" +
		"AiI2tleS0yIiwKICAgIF0sCiAgICAuLi4KICB9LA"

	dataString := payloadDid.Header.Specification + payloadDid.Header.
		Operation + payloadDid.Payload
	digest := sha256.Sum256([]byte(dataString))
	digestHexStr := hex.EncodeToString(digest[:])
	upperDigestHexStr := strings.ToUpper(digestHexStr)
	assert.Equal(t, upperDigestHexStr, targetDigest)
	fmt.Println(upperDigestHexStr)
	fmt.Println(digestHexStr)
}

func paylaodDIDInfoEqual(first *Operation, second *Operation) bool {
	return didHeaderEqual(&first.Header, &second.Header) &&
		first.Payload == second.Payload &&
		didProofEqual(&first.Proof, &second.Proof) &&
		didPayloadEqual(first.PayloadInfo, second.PayloadInfo)
}

func didHeaderEqual(first *DIDHeaderInfo, second *DIDHeaderInfo) bool {
	return first.Specification == second.Specification &&
		first.Operation == second.Operation
}

func didProofEqual(first *DIDProofInfo, second *DIDProofInfo) bool {
	return first.Type == second.Type &&
		first.VerificationMethod == second.VerificationMethod &&
		first.Signature == second.Signature
}

func didPayloadEqual(first *DIDPayloadInfo, second *DIDPayloadInfo) bool {
	return first.ID == second.ID &&
		didPublicKeysEqual(first.PublicKey, second.PublicKey) &&
		didAuthEqual(first.Authentication, second.Authentication) &&
		didAuthEqual(first.Authorization, second.Authorization) &&
		first.Expires == second.Expires
}

func didPublicKeysEqual(first []DIDPublicKeyInfo, second []DIDPublicKeyInfo) bool {
	if len(first) != len(second) {
		return false
	}
	for i := 0; i < len(first); i++ {
		if !didPublicKeyEqual(&first[i], &second[i]) {
			return false
		}
	}
	return true
}

func didPublicKeyEqual(first *DIDPublicKeyInfo, second *DIDPublicKeyInfo) bool {
	return first.ID == second.ID && first.Type == second.Type &&
		first.Controller == second.Controller &&
		first.PublicKeyBase58 == second.PublicKeyBase58
}

func didAuthEqual(first []interface{}, second []interface{}) bool {
	if len(first) != len(second) {
		return false
	}
	for i := 0; i < len(first); i++ {
		switch first[i].(type) {
		case string:
			if first[i] != second[i] {
				return false
			}
		case map[string]interface{}:
			data1, _ := json.Marshal(first[i])
			pk1 := new(DIDPublicKeyInfo)
			json.Unmarshal(data1, pk1)

			data2, _ := json.Marshal(second[i])
			pk2 := new(DIDPublicKeyInfo)
			json.Unmarshal(data2, pk2)

			if !didPublicKeyEqual(pk1, pk2) {
				return false
			}
		default:
			return false
		}

	}
	return true
}

func randomPayloadDID() *Operation {
	info := new(DIDPayloadInfo)
	json.Unmarshal(didPayloadBytes, info)

	return &Operation{
		Header: DIDHeaderInfo{
			Specification: "elastos/did/1.0",
			Operation:     getRandomOperation(),
		},
		Payload: base64url.EncodeToString(didPayloadBytes),
		Proof: DIDProofInfo{
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

func TestRandomPlayDID(t *testing.T) {
	payLoadDidInfo := randomPayloadDIDAll()
	fmt.Printf("payLoadDidInfo %+v \n", payLoadDidInfo)

	data, err := json.Marshal(payLoadDidInfo)
	assert.NoError(t, err)
	fmt.Printf("payLoadDidInfo %s\n", data)
}

func randomPayloadDIDNoAuth() *Operation {
	info := &DIDPayloadInfo{
		ID: randomString(),
		PublicKey: []DIDPublicKeyInfo{
			{
				ID:              randomString(),
				Type:            randomString(),
				Controller:      randomString(),
				PublicKeyBase58: randomString(),
			},
		},
	}
	fmt.Printf("randomPayloadDIDAll DIDPayloadInfo %+v \n", info)
	return &Operation{
		Header: DIDHeaderInfo{
			Specification: "elastos/did/1.0",
			Operation:     getRandomOperation(),
		},
		Payload: randomString(),
		Proof: DIDProofInfo{
			Type:               randomString(),
			VerificationMethod: randomString(),
			Signature:          randomString(),
		},
		PayloadInfo: info,
	}
}

func randomPayloadDIDAll() *Operation {
	info := &DIDPayloadInfo{
		ID: randomString(),
		PublicKey: []DIDPublicKeyInfo{
			{
				ID:              randomString(),
				Type:            randomString(),
				Controller:      randomString(),
				PublicKeyBase58: randomString(),
			},
		},
		Authentication: []interface{}{
			randomString(),
		},
		Authorization: []interface{}{
			randomString(),
		},
	}

	fmt.Printf("randomPayloadDIDAll DIDPayloadInfo %+v \n", info)
	return &Operation{
		Header: DIDHeaderInfo{
			Specification: "elastos/did/1.0",
			Operation:     getRandomOperation(),
		},
		Payload: randomString(),
		Proof: DIDProofInfo{
			Type:               randomString(),
			VerificationMethod: randomString(),
			Signature:          randomString(),
		},
		PayloadInfo: info,
	}
}

func randomPayloadNoContrller() *Operation {
	info := &DIDPayloadInfo{
		ID: randomString(),
		PublicKey: []DIDPublicKeyInfo{
			{
				ID:              randomString(),
				Type:            randomString(),
				PublicKeyBase58: randomString(),
			},
		},
		Authentication: []interface{}{
			randomString(),
		},
		Authorization: []interface{}{
			randomString(),
		},
	}
	fmt.Printf("randomPayloadDIDAll DIDPayloadInfo %+v \n", info)
	return &Operation{
		Header: DIDHeaderInfo{
			Specification: "elastos/did/1.0",
			Operation:     getRandomOperation(),
		},
		Payload: randomString(),
		Proof: DIDProofInfo{
			Type:               randomString(),
			VerificationMethod: randomString(),
			Signature:          randomString(),
		},
		PayloadInfo: info,
	}
}

func getRandomOperation() string {
	operations := []string{"create", "update"}
	index := mathrand.Int() % 2
	return operations[index]
}
