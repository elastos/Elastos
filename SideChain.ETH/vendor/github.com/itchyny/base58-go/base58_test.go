package base58

import (
	"testing"
)

type testcase struct {
	encoding  *Encoding
	testpairs []testpair
}

type testpair struct {
	decoded string
	encoded string
}

var testcases = []testcase{
	{FlickrEncoding, []testpair{
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
	{RippleEncoding, []testpair{
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
	{BitcoinEncoding, []testpair{
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

func TestEncode(t *testing.T) {
	for _, testcase := range testcases {
		for _, pair := range testcase.testpairs {
			got, err := testcase.encoding.Encode([]byte(pair.decoded))
			if err != nil {
				t.Errorf("Error occurred while encoding %s.", pair.decoded)
			}
			if string(got) != pair.encoded {
				t.Errorf("Encode(%s) = %s, want %s", pair.decoded, string(got), pair.encoded)
			}
		}
	}
}

func TestDecode(t *testing.T) {
	for _, testcase := range testcases {
		for _, pair := range testcase.testpairs {
			got, err := testcase.encoding.Decode([]byte(pair.encoded))
			if err != nil {
				t.Errorf("Error occurred while decoding %s.", pair.encoded)
			}
			if string(got) != pair.decoded {
				t.Errorf("Decode(%s) = %s, want %s", pair.encoded, string(got), pair.decoded)
			}
		}
	}
}

func BenchmarkEncode(b *testing.B) {
	for i := 0; i < b.N; i++ {
		for _, testcase := range testcases {
			for _, pair := range testcase.testpairs {
				_, _ = testcase.encoding.Encode([]byte(pair.decoded))
			}
		}
	}
}

func BenchmarkDecode(b *testing.B) {
	for i := 0; i < b.N; i++ {
		for _, testcase := range testcases {
			for _, pair := range testcase.testpairs {
				_, _ = testcase.encoding.Decode([]byte(pair.encoded))
			}
		}
	}
}
