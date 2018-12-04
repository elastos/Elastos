package base58

import (
	"fmt"
	"math/big"
)

// An Encoding is a radix 58 encoding/decoding scheme.
type Encoding struct {
	alphabet  [58]byte
	decodeMap [256]int64
}

func encoding(alphabet []byte) *Encoding {
	enc := &Encoding{}
	copy(enc.alphabet[:], alphabet[:])
	for i := range enc.decodeMap {
		enc.decodeMap[i] = -1
	}
	for i, b := range enc.alphabet {
		enc.decodeMap[b] = int64(i)
	}
	return enc
}

// FlickrEncoding is the encoding scheme used for Flickr's short URLs.
var FlickrEncoding = encoding([]byte("123456789abcdefghijkmnopqrstuvwxyzABCDEFGHJKLMNPQRSTUVWXYZ"))

// RippleEncoding is the encoding scheme used for Ripple addresses.
var RippleEncoding = encoding([]byte("rpshnaf39wBUDNEGHJKLM4PQRST7VWXYZ2bcdeCg65jkm8oFqi1tuvAxyz"))

// BitcoinEncoding is the encoding scheme used for Bitcoin addresses.
var BitcoinEncoding = encoding([]byte("123456789ABCDEFGHJKLMNPQRSTUVWXYZabcdefghijkmnopqrstuvwxyz"))

var radix = big.NewInt(58)

func reverse(data []byte) {
	for i, j := 0, len(data)-1; i < j; i, j = i+1, j-1 {
		data[i], data[j] = data[j], data[i]
	}
}

// Encode encodes the number represented in the byte array base 10.
func (encoding *Encoding) Encode(src []byte) ([]byte, error) {
	if len(src) == 0 {
		return []byte{}, nil
	}
	n, ok := new(big.Int).SetString(string(src), 10)
	if !ok {
		return nil, fmt.Errorf("expecting a number but got %q", src)
	}
	bytes := make([]byte, 0, len(src))
	for _, c := range src {
		if c == '0' {
			bytes = append(bytes, encoding.alphabet[0])
		} else {
			break
		}
	}
	zerocnt := len(bytes)
	mod := new(big.Int)
	zero := big.NewInt(0)
	for {
		switch n.Cmp(zero) {
		case 1:
			n.DivMod(n, radix, mod)
			bytes = append(bytes, encoding.alphabet[mod.Int64()])
		case 0:
			reverse(bytes[zerocnt:])
			return bytes, nil
		default:
			return nil, fmt.Errorf("expecting a positive number in base58 encoding but got %q", n)
		}
	}
}

// Decode decodes the base58 encoded bytes.
func (encoding *Encoding) Decode(src []byte) ([]byte, error) {
	if len(src) == 0 {
		return []byte{}, nil
	}
	var zeros []byte
	for i, c := range src {
		if c == encoding.alphabet[0] && i < len(src)-1 {
			zeros = append(zeros, '0')
		} else {
			break
		}
	}
	n := new(big.Int)
	var i int64
	for _, c := range src {
		if i = encoding.decodeMap[c]; i < 0 {
			return nil, fmt.Errorf("invalid character '%c' in decoding a base58 string \"%s\"", c, src)
		}
		n.Add(n.Mul(n, radix), big.NewInt(i))
	}
	return n.Append(zeros, 10), nil
}
