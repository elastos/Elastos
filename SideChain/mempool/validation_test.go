package mempool

import (
	"bytes"
	"crypto/rand"
	"encoding/hex"
	math "math/rand"
	"sort"
	"testing"

	"github.com/elastos/Elastos.ELA.SideChain/types"
	"github.com/elastos/Elastos.ELA.SideChain/vm"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/crypto"
	"github.com/stretchr/testify/assert"
)

type act interface {
	RedeemScript() []byte
	ProgramHash() *common.Uint168
	Sign(data []byte) ([]byte, error)
}

type account struct {
	private      []byte
	public       *crypto.PublicKey
	redeemScript []byte
	programHash  *common.Uint168
}

func (a *account) RedeemScript() []byte {
	return a.redeemScript
}

func (a *account) ProgramHash() *common.Uint168 {
	return a.programHash
}

func (a *account) Sign(data []byte) ([]byte, error) {
	return sign(a.private, data)
}

type multiAccount struct {
	accounts     []*account
	redeemScript []byte
	programHash  *common.Uint168
}

func (a *multiAccount) RedeemScript() []byte {
	return a.redeemScript
}

func (a *multiAccount) ProgramHash() *common.Uint168 {
	return a.programHash
}

func (a *multiAccount) Sign(data []byte) ([]byte, error) {
	var signatures []byte
	for _, act := range a.accounts {
		signature, err := sign(act.private, data)
		if err != nil {
			return nil, err
		}
		signatures = append(signatures, signature...)
	}
	return signatures, nil
}

func TestCheckCheckSigSignature(t *testing.T) {
	var tx *types.Transaction

	tx = buildTx()
	act := newAccount(t)
	signature, err := act.Sign(getData(tx))
	if err != nil {
		t.Errorf("Generate signature failed, error %s", err.Error())
	}

	// Normal
	tx.Programs = []*types.Program{{Code: act.redeemScript, Parameter: signature}}
	err = RunPrograms(tx, []common.Uint168{*act.programHash}, tx.Programs)
	assert.NoError(t, err)

	// invalid signature length
	var fakeSignature = make([]byte, crypto.SignatureScriptLength-1)
	rand.Read(fakeSignature)
	tx.Programs = []*types.Program{{Code: act.redeemScript, Parameter: fakeSignature}}
	err = RunPrograms(tx, []common.Uint168{*act.programHash}, tx.Programs)
	assert.EqualError(t, err, "[VM] Finish State not equal to HALT", "Invalid signature length")

	// invalid signature content
	fakeSignature = make([]byte, crypto.SignatureScriptLength)
	tx.Programs = []*types.Program{{Code: act.redeemScript, Parameter: fakeSignature}}
	err = RunPrograms(tx, []common.Uint168{*act.programHash}, tx.Programs)
	assert.EqualError(t, err, "[VM] Execute Engine Stack Count Error", "[Validation], Verify failed.")

	// invalid data content
	tx.Programs = []*types.Program{{Code: act.redeemScript, Parameter: fakeSignature}}
	err = RunPrograms(nil, []common.Uint168{*act.programHash}, tx.Programs)
	assert.EqualError(t, err, "invalid data content nil transaction", "[Validation], Verify failed.")
}

func TestCheckMultiSigSignature(t *testing.T) {
	var tx *types.Transaction

	tx = buildTx()
	data := getData(tx)

	act := newMultiAccount(math.Intn(2)+3, t)
	signature, err := act.Sign(data)
	assert.NoError(t, err, "Generate signature failed, error %v", err)

	// Normal
	tx.Programs = []*types.Program{{Code: act.redeemScript, Parameter: signature}}
	assert.NoError(t, err, "[CheckMultisigSignature] failed, %v", err)

	// invalid redeem script M < 1
	fakeCode := make([]byte, len(act.redeemScript))
	copy(fakeCode, act.redeemScript)
	fakeCode[0] = fakeCode[0] - fakeCode[0] + crypto.PUSH1 - 1
	tx.Programs = []*types.Program{{Code: fakeCode, Parameter: signature}}
	err = RunPrograms(tx, []common.Uint168{*act.programHash}, tx.Programs)
	assert.EqualError(t, err, "data hash is different from corresponding program code", "invalid multi sign script code")

	// invalid redeem script M > N
	copy(fakeCode, act.redeemScript)
	fakeCode[0] = fakeCode[len(fakeCode)-2] - crypto.PUSH1 + 2
	tx.Programs = []*types.Program{{Code: fakeCode, Parameter: signature}}
	err = RunPrograms(tx, []common.Uint168{*act.programHash}, tx.Programs)
	assert.EqualError(t, err, "data hash is different from corresponding program code", "invalid multi sign script code")

	// invalid redeem script length not enough
	copy(fakeCode, act.redeemScript)
	for len(fakeCode) >= crypto.MinMultiSignCodeLength {
		fakeCode = append(fakeCode[:1], fakeCode[crypto.PublicKeyScriptLength:]...)
	}
	tx.Programs = []*types.Program{{Code: fakeCode, Parameter: signature}}
	err = RunPrograms(tx, []common.Uint168{*act.programHash}, tx.Programs)
	assert.EqualError(t, err, "data hash is different from corresponding program code", "not a valid multi sign transaction code, length not enough")

	// invalid redeem script N not equal to public keys count
	fakeCode = make([]byte, len(act.redeemScript))
	copy(fakeCode, act.redeemScript)
	fakeCode[len(fakeCode)-2] = fakeCode[len(fakeCode)-2] + 1
	tx.Programs = []*types.Program{{Code: fakeCode, Parameter: signature}}
	err = RunPrograms(tx, []common.Uint168{*act.programHash}, tx.Programs)
	assert.EqualError(t, err, "data hash is different from corresponding program code", "invalid multi sign public key script count")

	// invalid redeem script wrong public key
	fakeCode = make([]byte, len(act.redeemScript))
	copy(fakeCode, act.redeemScript)
	fakeCode[2] = 0x01
	tx.Programs = []*types.Program{{Code: fakeCode, Parameter: signature}}
	err = RunPrograms(tx, []common.Uint168{*act.programHash}, tx.Programs)
	assert.EqualError(t, err, "data hash is different from corresponding program code", "The encodeData format is error")

	// invalid signature length not match
	tx.Programs = []*types.Program{{Code: fakeCode, Parameter: signature[math.Intn(64):]}}
	err = RunPrograms(tx, []common.Uint168{*act.programHash}, tx.Programs)
	assert.EqualError(t, err, "data hash is different from corresponding program code", "invalid multi sign signatures, length not match")

	// invalid signature not enough
	cut := len(signature)/crypto.SignatureScriptLength - int(act.redeemScript[0]-crypto.PUSH1)
	tx.Programs = []*types.Program{{Code: act.redeemScript, Parameter: signature[65*cut:]}}
	err = RunPrograms(tx, []common.Uint168{*act.programHash}, tx.Programs)
	assert.EqualError(t, err, "[VM] Finish State not equal to HALT", "invalid signatures, not enough signatures")

	// invalid signature too many
	tx.Programs = []*types.Program{{Code: act.redeemScript,
		Parameter: append(signature[:65], signature...)}}
	err = RunPrograms(tx, []common.Uint168{*act.programHash}, tx.Programs)
	assert.EqualError(t, err, "[VM] Finish State not equal to HALT", "invalid signatures, too many signatures")

	// invalid signature duplicate
	tx.Programs = []*types.Program{{Code: act.redeemScript,
		Parameter: append(signature[:65], signature[:len(signature)-65]...)}}
	err = RunPrograms(tx, []common.Uint168{*act.programHash}, tx.Programs)
	assert.EqualError(t, err, "[VM] Check Sig FALSE", "duplicated signatures")

	// invalid signature fake signature
	signature, err = newMultiAccount(math.Intn(2)+3, t).Sign(data)
	assert.NoError(t, err, "Generate signature failed, error %v", err)
	tx.Programs = []*types.Program{{Code: act.redeemScript, Parameter: signature}}
	err = RunPrograms(tx, []common.Uint168{*act.programHash}, tx.Programs)
	assert.EqualError(t, err, "[VM] Check Sig FALSE", "matched signatures not enough")
}

func TestRunPrograms(t *testing.T) {
	var err error
	var tx *types.Transaction
	var acts []act
	var hashes []common.Uint168
	var programs []*types.Program

	tx = buildTx()
	data := getData(tx)
	// Normal
	num := math.Intn(90) + 10
	acts = make([]act, 0, num)
	init := func() {
		hashes = make([]common.Uint168, 0, num)
		programs = make([]*types.Program, 0, num)
		for i := 0; i < num; i++ {
			if math.Uint32()%2 == 0 {
				act := newAccount(t)
				acts = append(acts, act)
			} else {
				mact := newMultiAccount(math.Intn(2)+3, t)
				acts = append(acts, mact)
			}
			hashes = append(hashes, *acts[i].ProgramHash())
			signature, err := acts[i].Sign(data)
			if err != nil {
				t.Errorf("Generate signature failed, error %s", err.Error())
			}
			programs = append(programs, &types.Program{Code: acts[i].RedeemScript(), Parameter: signature})
		}
	}
	init()

	// 1 loop checksig
	var index int
	for i, act := range acts {
		switch act.(type) {
		case *account:
			index = i
			break
		}
	}
	err = RunPrograms(tx, []common.Uint168{hashes[index]}, []*types.Program{programs[index]})
	assert.NoError(t, err, "[RunProgram] passed with 1 checksig program")

	// 1 loop multisig
	for i, act := range acts {
		switch act.(type) {
		case *multiAccount:
			index = i
			break
		}
	}
	err = RunPrograms(tx, []common.Uint168{hashes[index]}, []*types.Program{programs[index]})
	assert.NoError(t, err, "[RunProgram] passed with 1 multisig program")

	// multiple programs
	err = RunPrograms(tx, hashes, programs)
	assert.NoError(t, err, "[RunProgram] passed with multiple programs")

	// hashes count not equal to programs count
	init()
	removeIndex := math.Intn(num)
	hashes = append(hashes[:removeIndex], hashes[removeIndex+1:]...)
	err = RunPrograms(tx, hashes, programs)
	assert.Equal(t, "number of data hashes is different with number of programs", err.Error())

	// With no programs
	init()
	programs = []*types.Program{}
	err = RunPrograms(tx, hashes, programs)
	assert.Equal(t, "number of data hashes is different with number of programs", err.Error())

	// With unmatched hashes
	init()
	for i := 0; i < num; i++ {
		rand.Read(hashes[math.Intn(num)][:])
	}
	err = RunPrograms(tx, hashes, programs)
	assert.Equal(t, "data hash is different from corresponding program code", err.Error())

	// With disordered hashes
	init()
	common.SortProgramHashByCodeHash(hashes)
	sort.Sort(sort.Reverse(byHash(programs)))
	err = RunPrograms(tx, hashes, programs)
	assert.EqualError(t, err, "data hash is different from corresponding program code")

	// With random no code
	init()
	for i := 0; i < num; i++ {
		programs[math.Intn(num)].Code = nil
	}
	err = RunPrograms(tx, hashes, programs)
	assert.EqualError(t, err, "data hash is different from corresponding program code")

	// With random no parameter
	init()
	for i := 0; i < num; i++ {
		index := math.Intn(num)
		programs[index].Parameter = nil
	}
	err = RunPrograms(tx, hashes, programs)
	assert.Error(t, err, "[RunProgram] passed with random no parameter")
}

func newAccount(t *testing.T) *account {
	a := new(account)
	var err error
	a.private, a.public, err = crypto.GenerateKeyPair()
	if err != nil {
		t.Errorf("Generate key pair failed, error %s", err.Error())
	}

	a.redeemScript, err = contract.CreateStandardRedeemScript(a.public)
	if err != nil {
		t.Errorf("Create standard redeem script failed, error %s", err.Error())
	}

	c, err := contract.CreateStandardContract(a.public)
	if err != nil {
		t.Errorf("Create standard contract failed, error %s", err.Error())
	}

	a.programHash = c.ToProgramHash()

	return a
}

func newMultiAccount(num int, t *testing.T) *multiAccount {
	ma := new(multiAccount)
	publicKeys := make([]*crypto.PublicKey, 0, num)
	for i := 0; i < num; i++ {
		ma.accounts = append(ma.accounts, newAccount(t))
		publicKeys = append(publicKeys, ma.accounts[i].public)
	}

	var err error
	ma.redeemScript, err = contract.CreateMultiSigRedeemScript(num/2+1, publicKeys)
	if err != nil {
		t.Errorf("Create multisig redeem script failed, error %s", err.Error())
	}

	c, err := contract.CreateMultiSigContract(num/2+1, publicKeys)
	if err != nil {
		t.Errorf("Create multi-sign contract failed, error %s", err.Error())
	}

	ma.programHash = c.ToProgramHash()

	return ma
}

func buildTx() *types.Transaction {
	tx := new(types.Transaction)
	tx.TxType = types.TransferAsset
	tx.Payload = new(types.PayloadTransferAsset)
	tx.Inputs = randomInputs()
	tx.Outputs = randomOutputs()
	return tx
}

func randomInputs() []*types.Input {
	num := math.Intn(100) + 1
	inputs := make([]*types.Input, 0, num)
	for i := 0; i < num; i++ {
		var txId common.Uint256
		rand.Read(txId[:])
		index := math.Intn(100)
		inputs = append(inputs, &types.Input{
			Previous: *types.NewOutPoint(txId, uint16(index)),
		})
	}
	return inputs
}

func randomOutputs() []*types.Output {
	num := math.Intn(100) + 1
	outputs := make([]*types.Output, 0, num)
	var asset common.Uint256
	rand.Read(asset[:])
	for i := 0; i < num; i++ {
		var addr common.Uint168
		rand.Read(addr[:])
		outputs = append(outputs, &types.Output{
			AssetID:     asset,
			Value:       common.Fixed64(math.Int63()),
			OutputLock:  0,
			ProgramHash: addr,
		})
	}
	return outputs
}

func getData(tx *types.Transaction) []byte {
	buf := new(bytes.Buffer)
	tx.SerializeUnsigned(buf)
	return buf.Bytes()
}

func sign(private []byte, data []byte) (signature []byte, err error) {
	signature, err = crypto.Sign(private, data)
	if err != nil {
		return signature, err
	}

	buf := new(bytes.Buffer)
	buf.WriteByte(byte(len(signature)))
	buf.Write(signature)
	return buf.Bytes(), err
}

func TestSortPrograms(t *testing.T) {
	// invalid program code
	getInvalidCode := func() []byte {
		var code = make([]byte, 21)
	NEXT:
		rand.Read(code)
		switch code[len(code)-1] {
		case common.STANDARD, common.MULTISIG, common.CROSSCHAIN:
			goto NEXT
		}
		return code
	}
	programs := make([]*types.Program, 0, 10)
	for i := 0; i < 2; i++ {
		program := new(types.Program)
		program.Code = getInvalidCode()
		programs = append(programs, program)
	}
	err := SortPrograms(programs)
	assert.NoError(t, err)

	count := 100
	hashes := make([]common.Uint168, 0, count)
	programs = make([]*types.Program, 0, count)
	for i := 0; i < count; i++ {
		_, pubKey, _ := crypto.GenerateKeyPair()
		program := new(types.Program)
		randType := math.Uint32()
		var hash common.Uint168
		switch randType % 2 {
		case 0: // CHECKSIG
			c, err := contract.CreateStandardContract(pubKey)
			if err != nil {
				t.Fatal(err)
			}
			hash = *c.ToProgramHash()
			program.Code = c.Code

		case 1: // MULTISIG
			num := math.Intn(5) + 3
			pubKeys := make([]*crypto.PublicKey, 0, num)
			for i := 0; i < num; i++ {
				_, pubKey, _ := crypto.GenerateKeyPair()
				pubKeys = append(pubKeys, pubKey)
			}
			c, err := contract.CreateMultiSigContract(num, pubKeys)
			if err != nil {
				t.Fatal(err)
			}
			hash = *c.ToProgramHash()
			program.Code = c.Code
		}

		hashes = append(hashes, hash)
		programs = append(programs, program)
	}

	common.SortProgramHashByCodeHash(hashes)
	SortPrograms(programs)

	for i, hash := range hashes {
		programsHash := toProgramHash(programs[i].Code)
		if !hash.IsEqual(*programsHash) {
			t.Errorf("Hash %s not match with ProgramHash %s", hex.EncodeToString(hash[:]), hex.EncodeToString(programsHash[:]))
		}
	}
}

func toProgramHash(code []byte) *common.Uint168 {
	switch code[len(code)-1] {
	case vm.CHECKSIG:
		return common.ToProgramHash(byte(contract.PrefixStandard), code)
	case vm.CHECKMULTISIG:
		return common.ToProgramHash(byte(contract.PrefixMultiSig), code)
	}
	return nil
}
