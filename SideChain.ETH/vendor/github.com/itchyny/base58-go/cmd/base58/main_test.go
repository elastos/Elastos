package main

import (
	"bytes"
	"testing"

	"github.com/itchyny/base58-go"
)

type testcase struct {
	encoding  *base58.Encoding
	testpairs []testpair
}

type testpair struct {
	decoded string
	encoded string
}

var testcases = []testcase{
	{base58.FlickrEncoding, []testpair{
		{"", ""},
		{"0", "1"},
		{"32", "y"},
		{"64", "27"},
		{"000", "111"},
		{"512", "9Q"},
		{"1024", "iE"},
		{"16777216", "2tZhm"},
		{"00068719476736", "1112NGvhhq"},
		{"18446744073709551616", "JPwcyDCgEuq"},
		{"79228162514264337593543950336", "5QchsBFApWPVxyp9C"},
	}},
	{base58.RippleEncoding, []testpair{
		{"", ""},
		{"0", "r"},
		{"32", "Z"},
		{"64", "pf"},
		{"000", "rrr"},
		{"512", "9q"},
		{"1024", "JC"},
		{"16777216", "p7zHM"},
		{"00068719476736", "rrrpo6WHHR"},
		{"18446744073709551616", "jFXUZedGCVR"},
		{"79228162514264337593543950336", "nqUHTcgbQAFvYZQ9d"},
	}},
	{base58.BitcoinEncoding, []testpair{
		{"", ""},
		{"0", "1"},
		{"32", "Z"},
		{"64", "27"},
		{"000", "111"},
		{"512", "9q"},
		{"1024", "Jf"},
		{"16777216", "2UzHM"},
		{"00068719476736", "1112ohWHHR"},
		{"18446744073709551616", "jpXCZedGfVR"},
		{"79228162514264337593543950336", "5qCHTcgbQwpvYZQ9d"},
	}},
}

func Test_run_encode(t *testing.T) {
	for _, testcase := range testcases {
		opt := &option{decode: false, encoding: testcase.encoding}
		var decoded string
		var encoded string
		for _, pair := range testcase.testpairs {
			decoded += pair.decoded + "\n"
			encoded += pair.encoded + "\n"
		}
		var inbuf bytes.Buffer
		var outbuf bytes.Buffer
		var outerrbuf bytes.Buffer
		_, _ = inbuf.WriteString(decoded)
		run(opt, &inbuf, &outbuf, &outerrbuf)
		want := encoded
		got := outbuf.String()
		if got != encoded {
			t.Errorf("run() outputs %s, want %s", got, want)
		}
	}
}

func Test_run_encode_error(t *testing.T) {
	for _, testcase := range testcases {
		opt := &option{decode: false, encoding: testcase.encoding}
		decodedInvalid := "foo\nbar\n"
		var inbuf bytes.Buffer
		var outbuf bytes.Buffer
		var outerrbuf bytes.Buffer
		_, _ = inbuf.WriteString(decodedInvalid)
		run(opt, &inbuf, &outbuf, &outerrbuf)
		want := "expecting a number but got \"foo\"\n"
		want += "expecting a number but got \"bar\"\n"
		got := outerrbuf.String()
		if got != want {
			t.Errorf("run() outputs error %s, want %s", got, want)
		}
	}
}

func Test_run_decode(t *testing.T) {
	for _, testcase := range testcases {
		opt := &option{decode: true, encoding: testcase.encoding}
		var decoded string
		var encoded string
		for _, pair := range testcase.testpairs {
			decoded += pair.decoded + "\n"
			encoded += pair.encoded + "\n"
		}
		var inbuf bytes.Buffer
		var outbuf bytes.Buffer
		var outerrbuf bytes.Buffer
		_, _ = inbuf.WriteString(encoded)
		run(opt, &inbuf, &outbuf, &outerrbuf)
		want := decoded
		got := outbuf.String()
		if got != want {
			t.Errorf("run() outputs %s, want %s", got, want)
		}
	}
}

func Test_run_decode_error(t *testing.T) {
	for _, testcase := range testcases {
		opt := &option{decode: true, encoding: testcase.encoding}
		encodedInvalid := "FOO\nFal\n"
		var inbuf bytes.Buffer
		var outbuf bytes.Buffer
		var outerrbuf bytes.Buffer
		_, _ = inbuf.WriteString(encodedInvalid)
		run(opt, &inbuf, &outbuf, &outerrbuf)
		want := "invalid character 'O' in decoding a base58 string \"FOO\"\n"
		want += "invalid character 'l' in decoding a base58 string \"Fal\"\n"
		got := outerrbuf.String()
		if got != want {
			t.Errorf("run() outputs error %s, want %s", got, want)
		}
	}
}
