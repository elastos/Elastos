package blockchain

import (
	"bytes"
	"crypto/rand"
	math "math/rand"
	"sort"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/contract"
	"github.com/elastos/Elastos.ELA/core/contract/program"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/payload"
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
	data := getData(tx)
	act := newAccount(t)
	signature, err := act.Sign(data)
	if err != nil {
		t.Errorf("Generate signature failed, error %s", err.Error())
	}

	// Normal
	err = checkStandardSignature(program.Program{Code: act.redeemScript, Parameter: signature}, data)
	assert.NoError(t, err, "[CheckChecksigSignature] failed, %v", err)

	// invalid signature length
	var fakeSignature = make([]byte, crypto.SignatureScriptLength-math.Intn(64)-1)
	rand.Read(fakeSignature)
	err = checkStandardSignature(program.Program{Code: act.redeemScript, Parameter: fakeSignature}, data)
	assert.Error(t, err, "[CheckChecksigSignature] with invalid signature length")
	assert.Equal(t, "invalid signature length", err.Error())

	// invalid signature content
	fakeSignature = make([]byte, crypto.SignatureScriptLength)
	err = checkStandardSignature(program.Program{Code: act.redeemScript, Parameter: fakeSignature}, data)
	assert.Error(t, err, "[CheckChecksigSignature] with invalid signature content")
	assert.Equal(t, "[Validation], Verify failed.", err.Error())

	// invalid data content
	err = checkStandardSignature(program.Program{Code: act.redeemScript, Parameter: fakeSignature}, nil)
	assert.Error(t, err, "[CheckChecksigSignature] with invalid data content")
	assert.Equal(t, "[Validation], Verify failed.", err.Error())
}

func TestCheckMultiSigSignature(t *testing.T) {
	var tx *types.Transaction

	tx = buildTx()
	data := getData(tx)

	act := newMultiAccount(math.Intn(2)+3, t)
	signature, err := act.Sign(data)
	assert.NoError(t, err, "Generate signature failed, error %v", err)

	// Normal
	err = checkMultiSigSignatures(program.Program{Code: act.redeemScript, Parameter: signature}, data)
	assert.NoError(t, err, "[CheckMultisigSignature] failed, %v", err)

	// invalid redeem script M < 1
	fakeCode := make([]byte, len(act.redeemScript))
	copy(fakeCode, act.redeemScript)
	fakeCode[0] = fakeCode[0] - fakeCode[0] + crypto.PUSH1 - 1
	err = checkMultiSigSignatures(program.Program{Code: fakeCode, Parameter: signature}, data)
	assert.Error(t, err, "[CheckMultisigSignature] code with M < 1 passed")
	assert.Equal(t, "invalid multi sign script code", err.Error())

	// invalid redeem script M > N
	copy(fakeCode, act.redeemScript)
	fakeCode[0] = fakeCode[len(fakeCode)-2] - crypto.PUSH1 + 2
	err = checkMultiSigSignatures(program.Program{Code: fakeCode, Parameter: signature}, data)
	assert.Error(t, err, "[CheckMultisigSignature] code with M > N passed")
	assert.Equal(t, "invalid multi sign script code", err.Error())

	// invalid redeem script length not enough
	copy(fakeCode, act.redeemScript)
	for len(fakeCode) >= crypto.MinMultiSignCodeLength {
		fakeCode = append(fakeCode[:1], fakeCode[crypto.PublicKeyScriptLength:]...)
	}
	err = checkMultiSigSignatures(program.Program{Code: fakeCode, Parameter: signature}, data)
	assert.Error(t, err, "[CheckMultisigSignature] invalid length code passed")
	assert.Equal(t, "not a valid multi sign transaction code, length not enough", err.Error())

	// invalid redeem script N not equal to public keys count
	fakeCode = make([]byte, len(act.redeemScript))
	copy(fakeCode, act.redeemScript)
	fakeCode[len(fakeCode)-2] = fakeCode[len(fakeCode)-2] + 1
	err = checkMultiSigSignatures(program.Program{Code: fakeCode, Parameter: signature}, data)
	assert.Error(t, err, "[CheckMultisigSignature] invalid redeem script N not equal to public keys count")
	assert.Equal(t, "invalid multi sign public key script count", err.Error())

	// invalid redeem script wrong public key
	fakeCode = make([]byte, len(act.redeemScript))
	copy(fakeCode, act.redeemScript)
	fakeCode[2] = 0x01
	err = checkMultiSigSignatures(program.Program{Code: fakeCode, Parameter: signature}, data)
	assert.Error(t, err, "[CheckMultisigSignature] invalid redeem script wrong public key")
	assert.Equal(t, "the encodeData format is error", err.Error())

	// invalid signature length not match
	err = checkMultiSigSignatures(program.Program{Code: fakeCode, Parameter: signature[1+math.Intn(64):]}, data)
	assert.Error(t, err, "[CheckMultisigSignature] invalid signature length not match")
	assert.Equal(t, "invalid multi sign signatures, length not match", err.Error())

	// invalid signature not enough
	cut := len(signature)/crypto.SignatureScriptLength - int(act.redeemScript[0]-crypto.PUSH1)
	err = checkMultiSigSignatures(program.Program{Code: act.redeemScript, Parameter: signature[65*cut:]}, data)
	assert.Error(t, err, "[CheckMultisigSignature] invalid signature not enough")
	assert.Equal(t, "invalid signatures, not enough signatures", err.Error())

	// invalid signature too many
	err = checkMultiSigSignatures(program.Program{Code: act.redeemScript,
		Parameter: append(signature[:65], signature...)}, data)
	assert.Error(t, err, "[CheckMultisigSignature] invalid signature too many")
	assert.Equal(t, "invalid signatures, too many signatures", err.Error())

	// invalid signature duplicate
	err = checkMultiSigSignatures(program.Program{Code: act.redeemScript,
		Parameter: append(signature[:65], signature[:len(signature)-65]...)}, data)
	assert.Error(t, err, "[CheckMultisigSignature] invalid signature duplicate")
	assert.Equal(t, "duplicated signatures", err.Error())

	// invalid signature fake signature
	signature, err = newMultiAccount(math.Intn(2)+3, t).Sign(data)
	assert.NoError(t, err, "Generate signature failed, error %v", err)
	err = checkMultiSigSignatures(program.Program{Code: act.redeemScript, Parameter: signature}, data)
	assert.Error(t, err, "[CheckMultisigSignature] invalid signature fake signature")
}

func TestRunPrograms(t *testing.T) {
	var err error
	var tx *types.Transaction
	var acts []act
	var hashes []common.Uint168
	var programs []*program.Program

	tx = buildTx()
	data := getData(tx)
	// Normal
	num := math.Intn(90) + 10
	acts = make([]act, 0, num)
	init := func() {
		hashes = make([]common.Uint168, 0, num)
		programs = make([]*program.Program, 0, num)
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
			programs = append(programs, &program.Program{Code: acts[i].RedeemScript(), Parameter: signature})
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
	err = RunPrograms(data, hashes[index:index+1], programs[index:index+1])
	assert.NoError(t, err, "[RunProgram] passed with 1 checksig program")

	// 1 loop multisig
	for i, act := range acts {
		switch act.(type) {
		case *multiAccount:
			index = i
			break
		}
	}
	err = RunPrograms(data, hashes[index:index+1], programs[index:index+1])
	assert.NoError(t, err, "[RunProgram] passed with 1 multisig program")

	// multiple programs
	err = RunPrograms(data, hashes, programs)
	assert.NoError(t, err, "[RunProgram] passed with multiple programs")

	// hashes count not equal to programs count
	init()
	removeIndex := math.Intn(num)
	hashes = append(hashes[:removeIndex], hashes[removeIndex+1:]...)
	err = RunPrograms(data, hashes, programs)
	assert.Error(t, err, "[RunProgram] passed with unmathed hashes")
	assert.Equal(t, "the number of data hashes is different with number of programs", err.Error())

	// With no programs
	init()
	programs = []*program.Program{}
	err = RunPrograms(data, hashes, programs)
	assert.Error(t, err, "[RunProgram] passed with no programs")
	assert.Equal(t, "the number of data hashes is different with number of programs", err.Error())

	// With unmatched hashes
	init()
	for i := 0; i < num; i++ {
		rand.Read(hashes[math.Intn(num)][:])
	}
	err = RunPrograms(data, hashes, programs)
	assert.Error(t, err, "[RunProgram] passed with unmathed hashes")
	assert.Equal(t, "the data hashes is different with corresponding program code", err.Error())

	// With disordered hashes
	init()
	common.SortProgramHashByCodeHash(hashes)
	sort.Sort(sort.Reverse(byHash(programs)))
	err = RunPrograms(data, hashes, programs)
	assert.Error(t, err, "[RunProgram] passed with disordered hashes")
	assert.Equal(t, "the data hashes is different with corresponding program code", err.Error())

	// With random no code
	init()
	for i := 0; i < num; i++ {
		programs[math.Intn(num)].Code = nil
	}
	err = RunPrograms(data, hashes, programs)
	assert.Error(t, err, "[RunProgram] passed with random no code")
	assert.Equal(t, "the data hashes is different with corresponding program code", err.Error())

	// With random no parameter
	init()
	for i := 0; i < num; i++ {
		index := math.Intn(num)
		programs[index].Parameter = nil
	}
	err = RunPrograms(data, hashes, programs)
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
	tx.Payload = new(payload.TransferAsset)
	tx.Inputs = randomInputs()
	tx.Outputs = randomOutputs()
	return tx
}

func randomInputs() []*types.Input {
	num := math.Intn(100) + 1
	inputs := make([]*types.Input, 0, num)
	for i := 0; i < num; i++ {
		var txID common.Uint256
		rand.Read(txID[:])
		index := math.Intn(100)
		inputs = append(inputs, &types.Input{
			Previous: *types.NewOutPoint(txID, uint16(index)),
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
	programs := make([]*program.Program, 0, 10)
	for i := 0; i < 2; i++ {
		p := new(program.Program)
		p.Code = getInvalidCode()
		programs = append(programs, p)
	}
	SortPrograms(programs)

	count := 100
	hashes := make([]common.Uint168, 0, count)
	programs = make([]*program.Program, 0, count)
	for i := 0; i < count; i++ {
		p := new(program.Program)
		randType := math.Uint32()
		switch randType % 3 {
		case 0: // CHECKSIG
			p.Code = make([]byte, crypto.PublicKeyScriptLength)
			rand.Read(p.Code)
			p.Code[len(p.Code)-1] = common.STANDARD
		case 1: // MULTISIG
			num := math.Intn(5) + 3
			p.Code = make([]byte, (crypto.PublicKeyScriptLength-1)*num+3)
			rand.Read(p.Code)
			p.Code[len(p.Code)-1] = common.MULTISIG
			//case 2: // CROSSCHAIN
			//	num := math.Intn(5) + 3
			//	p.Code = make([]byte, (crypto.PublicKeyScriptLength-1)*num+3)
			//	rand.Read(p.Code)
			//	p.Code[len(p.Code)-1] = common.CROSSCHAIN
			//}
			//c := contract.Contract{}
			//hash, err := crypto.ToProgramHash(p.Code)
			//if err != nil {
			//	t.Errorf("ToProgramHash failed, %s", err)
			//}
			//hashes = append(hashes, *hash)
			//programs = append(programs, p)
		}

		common.SortProgramHashByCodeHash(hashes)
		SortPrograms(programs)

		//fixme
		//for i, hash := range hashes {
		//	programsHash, err := crypto.ToProgramHash(programs[i].Code)
		//	if err != nil {
		//		t.Errorf("ToProgramHash failed, %s", err)
		//	}
		//	if !hash.IsEqual(*programsHash) {
		//		t.Errorf("Hash %s not match with ProgramHash %s", hex.EncodeToString(hash[:]), hex.EncodeToString(programsHash[:]))
		//	}
		//
		//	t.Logf("Hash[%02d] %s match with ProgramHash[%02d] %s", i, hex.EncodeToString(hash[:]), i, hex.EncodeToString(programsHash[:]))
		//}
	}
}
