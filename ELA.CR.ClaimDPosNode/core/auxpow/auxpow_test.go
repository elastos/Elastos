package auxpow

import (
	"bytes"
	"crypto/sha256"
	"reflect"
	"testing"

	. "Elastos.ELA/common"
)

func Test_BtcTx(t *testing.T) {
	// coinbase tx
	// 01000000 .............................. Version
	//
	// 01 .................................... Number of inputs
	// | 00000000000000000000000000000000
	// | 00000000000000000000000000000000 ...  Previous outpoint TXID
	// | ffffffff ............................ Previous outpoint index
	// |
	// | 29 .................................. Bytes in coinbase
	// | |
	// | | 03 ................................ Bytes in height
	// | | | 4e0105 .......................... Height: 328014
	// | |
	// | | 062f503253482f0472d35454085fffed
	// | | f2400000f90f54696d65202620486561
	// | | 6c74682021 ........................ Arbitrary data
	// | 00000000 ............................ Sequence
	//
	// 01 .................................... Output count
	// | 2c37449500000000 .................... Satoshis (25.04275756 BTC)
	// | 1976a914a09be8040cbf399926aeb1f4
	// | 70c37d1341f3b46588ac ................ P2PKH script
	// | 00000000 ............................ Locktime

	coinBaseTx := "01000000010000000000000000000000000000000000000000000000000000000000000000ffffffff29034e0105062f503253482f0472d35454085fffedf2400000f90f54696d652026204865616c7468202100000000012c374495000000001976a914a09be8040cbf399926aeb1f470c37d1341f3b46588ac00000000"
	var cbTx BtcTx
	temp, _ := HexToBytes(coinBaseTx)
	buf := bytes.NewBuffer(temp)
	cbTx.Deserialize(buf)

	buf2 := bytes.NewBuffer([]byte{})
	cbTx.Serialize(buf2)
	cbTxStr := ToHexString(buf2.Bytes())
	if cbTxStr != coinBaseTx {
		t.Error("Test_BtcTx error")
	}

	temp, _ = HexToBytes(coinBaseTx)
	temp2 := sha256.Sum256(temp[:])
	temp2 = sha256.Sum256(temp2[:])
	temp3 := cbTx.Hash()
	if reflect.DeepEqual(temp3[:], temp2[:]) != true {
		t.Error("Test_BtcTx fail.")
	}
}
func Test_BtcBlockHeader(t *testing.T) {
	// bitcoin block header
	//02000000 ........................... Block version: 2
	//
	//b6ff0b1b1680a2862a30ca44d346d9e8
	//910d334beb48ca0c0000000000000000 ... Hash of previous block's header
	//9d10aa52ee949386ca9385695f04ede2
	//70dda20810decd12bc9b048aaab31471 ... Merkle root
	//
	//24d95a54 ........................... Unix time: 1415239972
	//30c31b18 ........................... Target: 0x1bc330 * 256**(0x18-3)
	//fe9f0864 ........................... Nonce
	blockheader := "02000000b6ff0b1b1680a2862a30ca44d346d9e8910d334beb48ca0c00000000000000009d10aa52ee949386ca9385695f04ede270dda20810decd12bc9b048aaab3147124d95a5430c31b18fe9f0864"

	var bh BtcBlockHeader
	temp, _ := HexToBytes(blockheader)
	buf := bytes.NewBuffer(temp)
	bh.Deserialize(buf)

	buf2 := bytes.NewBuffer([]byte{})
	bh.Serialize(buf2)
	bhStr := ToHexString(buf2.Bytes())
	if bhStr != blockheader {
		t.Error("Test_BtcBlockHeader error")
	}

	temp, _ = HexToBytes(blockheader)
	temp2 := sha256.Sum256(temp[:])
	temp2 = sha256.Sum256(temp2[:])
	temp3 := bh.Hash()
	if reflect.DeepEqual(temp3[:], temp2[:]) != true {
		t.Error("Test_BtcBlockHeader fail.")
	}

}

func Test_reverse(t *testing.T) {
	testIn := []byte{0, 1, 2, 3, 4, 5, 6, 7, 8, 9}
	target := []byte{9, 8, 7, 6, 5, 4, 3, 2, 1, 0}
	testOut := reverse(testIn)
	if reflect.DeepEqual(testOut, target) != true {
		t.Error("test reverse fail.")
	}
}

func Test_GetExpectedIndex(t *testing.T) {
	nonce := uint32(12345)
	chainId := 1
	height := 30
	target := 684357196
	index := GetExpectedIndex(nonce, chainId, height)
	if index != target {
		t.Error("1: index need %d, but get %d", target, index)
	}

	height = 1
	target = 0
	index = GetExpectedIndex(nonce, chainId, height)
	if index != target {
		t.Error("2: index need %d, but get %d", target, index)
	}
}
func Test_CheckMerkleBranch(t *testing.T) {
	auxHashStr := "02F7C9EDFC32335AA1956D16842407AA355A172D5BDF66E3CC15B134EF857467"
	auxRootStr := "40f1cdf9af288ec07fa6b06b549522b2de5046cb9eace325d0b180f2b2959d00"
	auxPathStr := []string{
		"7C51BC6A17950584FB3B2D1D97BDD0F50D5FB7F8263ACDF05E5D112156B2980F",
		"18c838597ae96ff927123000611e696902035ae6fb97ffe3446d07a936288b19",
	}
	auxIdx := 1

	var auxHash, auxRoot Uint256
	var auxPath []Uint256
	temp, _ := HexToBytes(auxHashStr)
	copy(auxHash[:], temp)

	temp, _ = HexToBytes(auxRootStr)
	copy(auxRoot[:], temp)
	for _, ip := range auxPathStr {
		var temp2 Uint256
		temp, _ := HexToBytes(ip)
		copy(temp2[:], temp)
		auxPath = append(auxPath, temp2)
	}

	res := CheckMerkleBranch(auxHash, auxPath, auxIdx)
	if reflect.DeepEqual(res[:], auxRoot[:]) != true {
		t.Error("Test_CheckMerkleBranch fail.")
	}

	res = CheckMerkleBranch(auxRoot, []Uint256{}, 0)
	if reflect.DeepEqual(res[:], auxRoot[:]) != true {
		t.Error("Test_CheckMerkleBranch fail.")
	}
}

func Test_Check(t *testing.T) {
	// 01000000 .............................. Version
	//
	// 01 .................................... Number of inputs
	// | 00000000000000000000000000000000
	// | 00000000000000000000000000000000 ...  Previous outpoint TXID
	// | ffffffff ............................ Previous outpoint index
	// |
	// | 30 .................................. Bytes in coinbase
	// | |
	// | | 03 ................................ Bytes in height
	// | | | 4e0105 .......................... Height: 328014
	// | |
	// | | fabe6d6d
	// | | 40f1cdf9af288ec07fa6b06b549522b2
	// | | de5046cb9eace325d0b180f2b2959d00
	// | | 04000000
	// | | 06000000  ........................ Aux data
	// | 00000000 ............................ Sequence
	//
	// 01 .................................... Output count
	// | 2c37449500000000 .................... Satoshis (25.04275756 BTC)
	// | 1976a914a09be8040cbf399926aeb1f4
	// | 70c37d1341f3b46588ac ................ P2PKH script
	// | 00000000 ............................ Locktime

	AuxCoinBase := "01000000010000000000000000000000000000000000000000000000000000000000000000ffffffff30034e0105fabe6d6d40f1cdf9af288ec07fa6b06b549522b2de5046cb9eace325d0b180f2b2959d00040000000600000000000000012c374495000000001976a914a09be8040cbf399926aeb1f470c37d1341f3b46588ac00000000"

	// bitcoin block header
	//02000000 ........................... Block version: 2
	//
	//b6ff0b1b1680a2862a30ca44d346d9e8
	//910d334beb48ca0c0000000000000000 ... Hash of previous block's header
	//d347c0cff3968faf33d63eed22168962
	//261b6d3ee261a9badcbefc92e3efc55d ... Merkle root
	//
	//24d95a54 ........................... Unix time: 1415239972
	//30c31b18 ........................... Target: 0x1bc330 * 256**(0x18-3)
	//fe9f0864 ........................... Nonce

	blockheader := "02000000b6ff0b1b1680a2862a30ca44d346d9e8910d334beb48ca0c0000000000000000d347c0cff3968faf33d63eed22168962261b6d3ee261a9badcbefc92e3efc55d24d95a5430c31b18fe9f0864"

	auxHashStr := "02F7C9EDFC32335AA1956D16842407AA355A172D5BDF66E3CC15B134EF857467"
	auxPathStr := []string{
		"7C51BC6A17950584FB3B2D1D97BDD0F50D5FB7F8263ACDF05E5D112156B2980F",
		"18c838597ae96ff927123000611e696902035ae6fb97ffe3446d07a936288b19",
	}
	auxIdx := 1
	chainId := 1

	var auxHash Uint256
	temp, _ := HexToBytes(auxHashStr)
	copy(auxHash[:], temp)

	var auxPath []Uint256
	for _, ip := range auxPathStr {
		var temp2 Uint256
		temp, _ := HexToBytes(ip)
		copy(temp2[:], temp)
		auxPath = append(auxPath, temp2)
	}

	var cbTx BtcTx
	temp, _ = HexToBytes(AuxCoinBase)
	buf := bytes.NewBuffer(temp)
	cbTx.Deserialize(buf)

	var bh BtcBlockHeader
	temp, _ = HexToBytes(blockheader)
	buf = bytes.NewBuffer(temp)
	bh.Deserialize(buf)

	ap := NewAuxPow(auxPath, auxIdx, cbTx, []Uint256{}, 0, bh)
	if ap.Check(auxHash, chainId) != true {
		t.Error("Test_Check fail")
	}

}

func Test_Check2(t *testing.T) {
	auxHashStr := "02F7C9EDFC32335AA1956D16842407AA355A172D5BDF66E3CC15B134EF857467"
	chainId := 1
	auxDataStr := "01000000010000000000000000000000000000000000000000000000000000000000000000ffffffff4902e174044c1dcb592f4254432e434f4d2ffabe6d6d02f7c9edfc32335aa1956d16842407aa355a172d5bdf66e3cc15b134ef8574670100000000000000010000001017000000000000ffffffff0200000000000000001976a91489893957178347e87e2bb3850e6f6937de7372b288ac0000000000000000266a24aa21a9ede2f61c3f71d1defd3fa999dfa36953755c690689799962b48bebd836974e8cf900000000262b25ee945edb5655e17484431aaa81688950aa5381009ee1e5e775b5d6e3960000000000000000000000000020f9e2a4a5f4cd21fd1b126170bc12a90acfddb7fea5820e41d59ace0569affd35aef1152384b83c5b6f670cb43a597666803de4d0ad3faf423124553cbe68a454561dcb59ffff7f20561dcb59"

	var auxHash Uint256
	temp, _ := HexToBytes(auxHashStr)
	copy(auxHash[:], temp)
	auxData, _ := HexToBytes(auxDataStr)

	var ap AuxPow
	buf := bytes.NewBuffer(auxData)
	err := ap.Deserialize(buf)
	if err != nil {
		t.Error("AuxPow Deserialize error!")
	}

	buf2 := bytes.NewBuffer([]byte{})
	err = ap.Serialize(buf2)
	if err != nil {
		t.Error("AuxPow Serialize error!")
	}

	if reflect.DeepEqual(buf2.Bytes(), auxData) != true {
		t.Error("AuxPow Serialize error!")
	}

	if ap.Check(auxHash, chainId) != true {
		t.Error("Test_Check fail")
	}

}
