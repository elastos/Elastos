package utils

import "crypto/rand"

func RandomBytes(len int) []byte {
	a := make([]byte, len)
	rand.Read(a)
	return a
}
