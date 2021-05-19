package base64url

import (
	"bytes"
	"encoding/base64"
	"strings"
)

// EncodeToString returns the base64URL encoding of src.
func EncodeToString(src []byte) string {
	s := base64.StdEncoding.EncodeToString(src)
	s = strings.Replace(s, "=", "", -1)
	s = strings.Replace(s, "+", "-", -1)
	s = strings.Replace(s, "/", "_", -1)
	return s
}

// DecodeString returns the bytes represented by the base64URL string s.
func DecodeString(s string) ([]byte, error) {
	s = strings.Replace(s, "-", "+", -1)
	s = strings.Replace(s, "_", "/", -1)

	append := (4 - len(s)%4) % 4

	var buffer bytes.Buffer
	buffer.WriteString(s)
	for i := 0; i < append; i++ {
		buffer.WriteString("=")
	}
	return base64.StdEncoding.DecodeString(buffer.String())
}
