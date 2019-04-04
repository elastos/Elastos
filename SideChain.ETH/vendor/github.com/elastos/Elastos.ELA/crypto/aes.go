package crypto

import (
	"crypto/aes"
	"crypto/cipher"
	"crypto/sha256"
	"errors"
)

func ToAesKey(pwd []byte) []byte {
	hash := sha256.Sum256(pwd)
	double := sha256.Sum256(hash[:])
	return double[:]
}

func AesEncrypt(plaintext []byte, key []byte, iv []byte) ([]byte, error) {
	block, err := aes.NewCipher(key)
	if err != nil {
		return nil, errors.New("invalid decrypt key")
	}
	blockMode := cipher.NewCBCEncrypter(block, iv)

	cipherText := make([]byte, len(plaintext))
	blockMode.CryptBlocks(cipherText, plaintext)

	return cipherText, nil
}

func AesDecrypt(cipherText []byte, key []byte, iv []byte) ([]byte, error) {

	block, err := aes.NewCipher(key)
	if err != nil {
		return nil, errors.New("invalid decrypt key")
	}

	blockSize := block.BlockSize()

	if len(cipherText) < blockSize {
		return nil, errors.New("cipherText too short")
	}

	if len(cipherText)%blockSize != 0 {
		return nil, errors.New("cipherText is not a multiple of the block size")
	}

	blockModel := cipher.NewCBCDecrypter(block, iv)

	plaintext := make([]byte, len(cipherText))
	blockModel.CryptBlocks(plaintext, cipherText)

	return plaintext, nil
}
