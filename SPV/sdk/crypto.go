package sdk

import (
	"crypto/ecdsa"
	"crypto/elliptic"
	"crypto/rand"

	. "github.com/elastos/Elastos.ELA/crypto"
)

// Generate a ECC private key by the given curve
func GeneratePrivateKey(curve elliptic.Curve) []byte {
	privateKey, _ := ecdsa.GenerateKey(curve, rand.Reader)
	return privateKey.D.Bytes()
}

// Get the public key of the given private key on the specified ECC curve
func GetPublicKey(curve elliptic.Curve, privateKey []byte) *PublicKey {
	publicKey := new(PublicKey)
	publicKey.X, publicKey.Y = curve.ScalarBaseMult(privateKey)
	return publicKey
}

// Get a standard ELA account by the given private key and ECC curve
func GetAccount(curve elliptic.Curve, privateKey []byte) *Account {
	account, _ := NewAccount(privateKey, GetPublicKey(curve, privateKey))
	return account
}

// Generate a ECC private key on ECC P256 curve
func GenerateP256PrivateKey() []byte {
	return GeneratePrivateKey(elliptic.P256())
}

// Get the public key of the given private key on ECC P256 curve
func GetP256PublicKey(privateKey []byte) *PublicKey {
	return GetPublicKey(elliptic.P256(), privateKey)
}

// Get a standard ELA account by the given private key on ECC P256 curve
func GetP256Account(privateKey []byte) *Account {
	return GetAccount(elliptic.P256(), privateKey)
}
