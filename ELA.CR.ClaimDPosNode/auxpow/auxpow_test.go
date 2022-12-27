// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package auxpow

import (
	"bytes"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
)

func TestAuxPow_Check(t *testing.T) {

	// 1. ela auxpow data. chainid = 6. AuxMerkleBranch = 1
	const hashHex1 = "7926398947f332fe534b15c628ff0cd9dc6f7d3ea59c74801dc758ac65428e64"
	const auxPowHex1 = "02000000010000000000000000000000000000000000000000000000000000000000000000ffffffff4b0313ee0904a880495b742f4254432e434f4d2ffabe6d6d9581ba0156314f1e92fd03430c6e4428a32bb3f1b9dc627102498e5cfbf26261020000004204cb9a010f32a00601000000000000ffffffff0200000000000000001976a914c0174e89bd93eacd1d5a1af4ba1802d412afc08688ac0000000000000000266a24aa21a9ede2f61c3f71d1defd3fa999dfa36953755c690689799962b48bebd836974e8cf90000000014acac4ee8fdd8ca7e0b587b35fce8c996c70aefdf24c333038bdba7af531266000000000001ccc205f0e1cb435f50cc2f63edd53186b414fcb22b719da8c59eab066cf30bdb0000000000000020d1061d1e456cae488c063838b64c4911ce256549afadfc6a4736643359141b01551e4d94f9e8b6b03eec92bb6de1e478a0e913e5f733f5884857a7c2b965f53ca880495bffff7f20a880495b"

	var auxpow1 AuxPow

	buf1, _ := common.HexStringToBytes(auxPowHex1)
	if err := auxpow1.Deserialize(bytes.NewReader(buf1)); err != nil {
		t.Error("auxpow1 deserialization failed", auxPowHex1)
	}

	hash1, err := common.Uint256FromHexString(hashHex1)
	if err != nil {
		t.Error(err)
	}
	if ok := auxpow1.Check(hash1, 6); !ok {
		t.Error("auxpow1 checking failed")
	}

	// 2. namecoin auxpow data. chainid = 6. AuxMerkleBranch = 1
	const hashHex2 = "21187623de86cd62b4ce211cd8a74e88f80eda6cc12f279bf3cdb5c0d9539a9d"
	const auxPowHex2 = "02000000010000000000000000000000000000000000000000000000000000000000000000ffffffff4b039aff0904db044a5b742f4254432e434f4d2ffabe6d6d35ecfc5f5ca2971449ee78b7d810f280de7e3e7c407e3c0162ef8692df350ef8020000004204cb9a011fde202e00000000000000ffffffff0200000000000000001976a914c0174e89bd93eacd1d5a1af4ba1802d412afc08688ac0000000000000000266a24aa21a9ede2f61c3f71d1defd3fa999dfa36953755c690689799962b48bebd836974e8cf9000000001d1879510258c5186e39cfcde4539c88686854b1ca640681dd38ed9527e635600000000000015f2f03802d61504f12e25d4b679b881ddb374cc04f240b6eb765d887679fb6360000000000000020a9f32bdb09d7777f3fa308fcd221e531393441f50e7f8b2d4ef63b2c3440940ec866338e7674b07d6a92269317f09f6c0fdb60ce7052e0211133e0015727ebb2db044a5bffff7f20db044a5b"

	var auxpow2 AuxPow

	buf2, _ := common.HexStringToBytes(auxPowHex2)
	if err := auxpow2.Deserialize(bytes.NewReader(buf2)); err != nil {
		t.Error("auxpow2 deserialization failed", auxPowHex2)
	}

	hash2, err := common.Uint256FromHexString(hashHex2)
	if err != nil {
		t.Error(err)
	}
	if ok := auxpow2.Check(hash2, 6); !ok {
		t.Error("auxpow2 checking failed")
	}

	// 3. ela auxpow data. chainid = 6. AuxMerkleBranch = 0
	const hashHex3 = "a4c78cf0c73256f8607e85baaa72874408525d7c5488a4cc69ad6930d1186d2c"
	const auxPowHex3 = "02000000010000000000000000000000000000000000000000000000000000000000000000ffffffff4a02050e04a4e2515b742f4254432e434f4d2ffabe6d6da4c78cf0c73256f8607e85baaa72874408525d7c5488a4cc69ad6930d1186d2c01000000000000000108d7517400000000000000ffffffff0300e1f505000000001976a914c0174e89bd93eacd1d5a1af4ba1802d412afc08688ac0000000000000000266a24aa21a9ede2f61c3f71d1defd3fa999dfa36953755c690689799962b48bebd836974e8cf90000000000000000424063643337386238613335653764623466356636343562303833396130373635613661326637613064343338663565626432653638663036323633313832333034f90000000042cbe48afcac502073e24700fcb536d52737c1d7938ff859685e31558df685f800000000000000000000000000207f9ebb83cd305988685bbc7c8ee006ba6934f791708f37c1e4d913fd8b0c000070833a09a50ea430f421b89292925ca8499f0bb3c2a6f7bcc804eb8105ea4bbca7e2515b7182281ea7e2515b"

	var auxpow3 AuxPow

	buf3, _ := common.HexStringToBytes(auxPowHex3)
	if err := auxpow3.Deserialize(bytes.NewReader(buf3)); err != nil {
		t.Error("auxpow3 deserialization failed", auxPowHex3)
	}

	hash3, err := common.Uint256FromHexString(hashHex3)
	if err != nil {
		t.Error(err)
	}
	if ok := auxpow3.Check(hash3, 6); !ok {
		t.Error("auxpow3 checking failed")
	}
}
