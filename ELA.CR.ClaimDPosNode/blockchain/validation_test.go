package blockchain

import (
	"bytes"
	"crypto/rand"
	"encoding/hex"
	math "math/rand"
	"sort"
	"testing"

	"github.com/elastos/Elastos.ELA/core"

	"github.com/elastos/Elastos.ELA.Utility/common"
	"github.com/elastos/Elastos.ELA.Utility/crypto"
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

func TestRunPrograms(t *testing.T) {
	/*
		1. Transaction with 1 checksig program
		2. Transaction with many checksig program
		3. Transaction with 1 multisig program
		4. Transaction with many multisig program
		5. Transaction with many different sig program
	*/

	commonTest(t)
	checksigTest(t)
	multisigTest(t)
	diffsigTest(t)
}

func commonTest(t *testing.T) {
	var tx *core.Transaction
	var hashes []common.Uint168
	var programs []*core.Program
	var err error
	var errMsg string

	tx = buildTx()
	data := getData(tx)
	// Normal
	num := math.Intn(90) + 10
	init := func() {
		hashes = make([]common.Uint168, 0, num)
		programs = make([]*core.Program, 0, num)
		for i := 0; i < num; i++ {
			act := newAccount(t)
			hashes = append(hashes, *act.programHash)
			signature, err := act.Sign(data)
			if err != nil {
				t.Errorf("Generate signature failed, error %s", err.Error())
			}
			programs = append(programs, &core.Program{Code: act.redeemScript, Parameter: signature})
		}
	}

	// With unmatched hashes
	init()
	for i := 0; i < num; i++ {
		rand.Read(hashes[math.Intn(num)][:])
	}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with unmathed hashes")
	}
	errMsg = "The data hashes is different with corresponding program code."
	if err.Error() != errMsg {
		t.Errorf("Wrong error, expect [%s] got [%s]", errMsg, err.Error())
	}
	t.Logf("[Passed] 0. Common test [Unmatched hashes], %s", err.Error())

	// With disordered hashes
	init()
	common.SortProgramHashes(hashes)
	sort.Sort(sort.Reverse(byHash(programs)))
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with disordered hashes")
	}
	errMsg = "The data hashes is different with corresponding program code."
	if err.Error() != errMsg {
		t.Errorf("Wrong error, expect [%s] got [%s]", errMsg, err.Error())
	}
	t.Logf("[Passed] 0. Common test [Disordered hashes], %s", err.Error())

	// With no programs
	init()
	programs = []*core.Program{}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with no programs")
	}
	errMsg = "The number of data hashes is different with number of programs."
	if err.Error() != errMsg {
		t.Errorf("Wrong error, expect [%s] got [%s]", errMsg, err.Error())
	}
	t.Logf("[Passed] 0. Common test [No programs], %s", err.Error())

	// With random no code
	init()
	for i := 0; i < num; i++ {
		programs[math.Intn(num)].Code = nil
	}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with random no code")
	}
	errMsg = "[ToProgramHash] failed, empty program code"
	if err.Error() != errMsg {
		t.Errorf("Wrong error, expect [%s] got [%s]", errMsg, err.Error())
	}
	t.Logf("[Passed] 0. Common test [Random no code], %s", err.Error())

	// With random no parameter
	init()
	for i := 0; i < num; i++ {
		programs[math.Intn(num)].Parameter = nil
	}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with random no parameter")
	}
	errMsg = "Invalid signature length"
	if err.Error() != errMsg {
		t.Errorf("Wrong error, expect [%s] got [%s]", errMsg, err.Error())
	}
	t.Logf("[Passed] 0. Common test [Random no parameter], %s", err.Error())
}

func checksigTest(t *testing.T) {
	var tx *core.Transaction
	var hashes []common.Uint168
	var programs []*core.Program
	var errMsg string

	tx = buildTx()
	data := getData(tx)
	// Normal
	act := newAccount(t)
	signature, err := act.Sign(data)
	if err != nil {
		t.Errorf("Generate signature failed, error %s", err.Error())
	}

	hashes = []common.Uint168{*act.programHash}
	programs = []*core.Program{{Code: act.redeemScript, Parameter: signature}}
	err = RunPrograms(data, hashes, programs)
	if err != nil {
		t.Errorf("[RunProgram] with 1 checksig failed, %s", err.Error())
	}
	t.Logf("[Passed] 1. Transaction with 1 checksig program [Normal]")

	// With fake signature
	var fakeSignature [crypto.SignatureScriptLength]byte
	rand.Read(fakeSignature[:])
	hashes = []common.Uint168{*act.programHash}
	programs = []*core.Program{{Code: act.redeemScript, Parameter: fakeSignature[:]}}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with fake signature")
	}
	errMsg = "[Validation], Verify failed."
	if err.Error() != errMsg {
		t.Errorf("Wrong error, expect [%s] got [%s]", errMsg, err.Error())
	}
	t.Logf("[Passed] 1. Transaction with 1 checksig program [Fake signature], %s", err.Error())

	// Many checksig
	num := math.Intn(90) + 10
	init := func() {
		hashes = make([]common.Uint168, 0, num)
		programs = make([]*core.Program, 0, num)
		for i := 0; i < num; i++ {
			act := newAccount(t)
			hashes = append(hashes, *act.programHash)
			signature, err := act.Sign(data)
			if err != nil {
				t.Errorf("Generate signature failed, error %s", err.Error())
			}
			programs = append(programs, &core.Program{Code: act.redeemScript, Parameter: signature})
		}
	}
	init()
	err = RunPrograms(data, hashes, programs)
	if err != nil {
		t.Errorf("[RunProgram] with many checksig failed, %s", err.Error())
	}
	t.Logf("[Passed] 1. Transaction with many checksig program [Normal]")
}

func multisigTest(t *testing.T) {
	var tx *core.Transaction
	var macts []*multiAccount
	var hashes []common.Uint168
	var programs []*core.Program
	var errMsg string

	tx = buildTx()
	data := getData(tx)

	// Normal
	acts := math.Intn(2) + 3
	mact := newMultiAccount(acts, t)
	signature, err := mact.Sign(data)
	if err != nil {
		t.Errorf("Generate signature failed, error %s", err.Error())
	}
	hashes = []common.Uint168{*mact.programHash}
	programs = []*core.Program{{Code: mact.redeemScript, Parameter: signature}}
	err = RunPrograms(data, hashes, programs)
	if err != nil {
		t.Errorf("[RunProgram] with 1 multisig failed, %s", err.Error())
	}
	t.Logf("[Passed] 2. Transaction with 1 multisig program [Normal]")

	// With unmatched signatures
	var fakeSignatures = make([]byte, crypto.SignatureScriptLength*acts)
	rand.Read(fakeSignatures[:])
	hashes = []common.Uint168{*mact.programHash}
	programs = []*core.Program{{Code: mact.redeemScript, Parameter: fakeSignatures[:]}}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with unmatched signatures")
	}
	errMsg = "matched signatures not enough"
	if err.Error() != errMsg {
		t.Errorf("Wrong error, expect [%s] got [%s]", errMsg, err.Error())
	}
	t.Logf("[Passed] 2. Transaction with 1 multisig program [Unmatched signatures], %s", err.Error())

	// Many multisig
	num := math.Intn(90) + 10
	init := func() {
		macts = make([]*multiAccount, 0, num)
		hashes = make([]common.Uint168, 0, num)
		programs = make([]*core.Program, 0, num)
		for i := 0; i < num; i++ {
			acts := math.Intn(3) + 3
			mact := newMultiAccount(acts, t)
			macts = append(macts, mact)
			hashes = append(hashes, *mact.programHash)
			var signatures []byte
			for _, act := range mact.accounts {
				signature, err := sign(act.private, data)
				if err != nil {
					t.Errorf("Generate signature failed, %s", err.Error())
				}
				signatures = append(signatures, signature...)
			}
			programs = append(programs, &core.Program{Code: mact.redeemScript, Parameter: signatures})
		}
	}
	init()
	err = RunPrograms(data, hashes, programs)
	if err != nil {
		t.Errorf("[RunProgram] with many multisig failed, %s", err.Error())
	}
	t.Logf("[Passed] 2. Transaction with many multisig program [Normal]")

	// With random not enough signature
	init()
	for i := 0; i < num; i++ {
		fakeIndex := math.Intn(num)
		var signatures []byte
		for i, act := range macts[fakeIndex].accounts {
			if i >= len(macts[fakeIndex].accounts)/2 {
				break
			}
			signature, err := sign(act.private, data)
			if err != nil {
				t.Errorf("Generate signature failed, %s", err.Error())
			}
			signatures = append(signatures, signature...)
		}
		programs[fakeIndex].Parameter = signatures
	}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with not enough signature")
	}
	errMsg = "invalid signatures, not enough signatures"
	if err.Error() != errMsg {
		t.Errorf("Wrong error, expect [%s] got [%s]", errMsg, err.Error())
	}
	t.Logf("[Passed] 2. Transaction with many multisig program [Not enough signature], %s", err.Error())

	// With random too many signature
	init()
	for i := 0; i < num; i++ {
		fakeIndex := math.Intn(num)
		var signatures []byte
		for _, act := range macts[fakeIndex].accounts {
			signature, err := sign(act.private, data)
			if err != nil {
				t.Errorf("Generate signature failed, %s", err.Error())
			}
			signatures = append(signatures, signature...)
			signatures = append(signatures, signature...)
		}
		programs[fakeIndex].Parameter = signatures
	}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with too many signature")
	}
	errMsg = "invalid signatures, too many signatures"
	if err.Error() != errMsg {
		t.Errorf("Wrong error, expect [%s] got [%s]", errMsg, err.Error())
	}
	t.Logf("[Passed] 2. Transaction with many multisig program [Too many signature], %s", err.Error())

	// With random duplicate signature
	init()
	for i := 0; i < num; i++ {
		fakeIndex := math.Intn(num)
		var signatures []byte
		for i, act := range macts[fakeIndex].accounts {
			if i+2 >= len(macts[fakeIndex].accounts) {
				break
			}
			signature, err := sign(act.private, data)
			if err != nil {
				t.Errorf("Generate signature failed, %s", err.Error())
			}
			if i == 0 {
				signatures = append(signatures, signature...)
			}
			signatures = append(signatures, signature...)
		}
		programs[fakeIndex].Parameter = signatures
	}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with duplicated signature")
	}
	errMsg = "duplicated signatures"
	if err.Error() != errMsg {
		t.Errorf("Wrong error, expect [%s] got [%s]", errMsg, err.Error())
	}
	t.Logf("[Passed] 2. Transaction with many multisig program [Duplicated signature], %s", err.Error())

	// With random invalid signatures
	init()
	for i := 0; i < num; i++ {
		fakeIndex := math.Intn(num)
		signatures := make([]byte, len(mact.accounts)*crypto.SignatureScriptLength-math.Intn(64))
		rand.Read(signatures)
		programs[fakeIndex].Parameter = signatures
	}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with invalid signatures")
	}
	errMsg = "invalid multi sign signatures, length not match"
	if err.Error() != errMsg {
		t.Errorf("Wrong error, expect [%s] got [%s]", errMsg, err.Error())
	}
	t.Logf("[Passed] 2. Transaction with many multisig program [Invalid signatures], %s", err.Error())
}

func diffsigTest(t *testing.T) {
	var tx *core.Transaction
	var hashes []common.Uint168
	var programs []*core.Program

	tx = buildTx()
	data := getData(tx)
	// Normal
	num := math.Intn(90) + 10
	acts := make([]act, 0, num)
	init := func() {
		hashes = make([]common.Uint168, 0, num)
		programs = make([]*core.Program, 0, num)
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
			programs = append(programs, &core.Program{Code: acts[i].RedeemScript(), Parameter: signature})
		}
	}
	init()
	err := RunPrograms(data, hashes, programs)
	if err != nil {
		t.Errorf("[RunProgram] with many diffsig failed, %s", err.Error())
	}
	t.Logf("[Passed] 3. Transaction with many diffsig program [Normal]")

	// With random invalid signature
	init()
	for i := 0; i < num; i++ {
		fakeIndex := math.Intn(num)
		switch act := acts[fakeIndex].(type) {
		case *account:
			var signature = make([]byte, math.Intn(crypto.SignatureScriptLength))
			rand.Read(signature)
			programs[fakeIndex].Parameter = signature
		case *multiAccount:
			if i%2 == 0 {
				var signatures = make([]byte, math.Intn(len(act.accounts)*crypto.SignatureScriptLength))
				rand.Read(signatures)
				programs[fakeIndex].Parameter = signatures
			} else {
				var signatures []byte
				for i, ac := range act.accounts {
					if i >= len(act.accounts)/2 {
						break
					}
					signature, err := sign(ac.private, data)
					if err != nil {
						t.Errorf("Generate signature failed, %s", err.Error())
					}
					signatures = append(signatures, signature...)
				}
				programs[fakeIndex].Parameter = signatures
			}
		}
	}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with invalid signature")
	}
	t.Logf("[Passed] 3. Transaction with many diffsig program [Invalid signature], %s", err.Error())
}

func newAccount(t *testing.T) *account {
	a := new(account)
	var err error
	a.private, a.public, err = crypto.GenerateKeyPair()
	if err != nil {
		t.Errorf("Generate key pair failed, error %s", err.Error())
	}

	a.redeemScript, err = crypto.CreateStandardRedeemScript(a.public)
	if err != nil {
		t.Errorf("Create standard redeem script failed, error %s", err.Error())
	}

	a.programHash, err = crypto.ToProgramHash(a.redeemScript)
	if err != nil {
		t.Errorf("To program hash failed, error %s", err.Error())
	}

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
	ma.redeemScript, err = crypto.CreateMultiSignRedeemScript(uint(num/2+1), publicKeys)
	if err != nil {
		t.Errorf("Create multisig redeem script failed, error %s", err.Error())
	}

	ma.programHash, err = crypto.ToProgramHash(ma.redeemScript)
	if err != nil {
		t.Errorf("To program hash failed, error %s", err.Error())
	}

	return ma
}

func buildTx() *core.Transaction {
	tx := new(core.Transaction)
	tx.TxType = core.TransferAsset
	tx.Inputs = randomInputs()
	tx.Outputs = randomOutputs()
	return tx
}

func randomInputs() []*core.Input {
	num := math.Intn(1000)
	inputs := make([]*core.Input, 0, num)
	for i := 0; i < num; i++ {
		var txId common.Uint256
		rand.Read(txId[:])
		index := math.Intn(1000)
		inputs = append(inputs, &core.Input{
			Previous: *core.NewOutPoint(txId, uint16(index)),
		})
	}
	return inputs
}

func randomOutputs() []*core.Output {
	num := math.Intn(1000)
	outputs := make([]*core.Output, 0, num)
	var asset common.Uint256
	rand.Read(asset[:])
	for i := 0; i < num; i++ {
		var addr common.Uint168
		rand.Read(addr[:])
		outputs = append(outputs, &core.Output{
			AssetID:     asset,
			Value:       common.Fixed64(math.Int63()),
			OutputLock:  0,
			ProgramHash: addr,
		})
	}
	return outputs
}

func getData(tx *core.Transaction) []byte {
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
	count := 100
	hashes := make([]common.Uint168, 0, count)
	programs := make([]*core.Program, 0, count)
	for i := 0; i < count; i++ {
		program := new(core.Program)
		randType := math.Uint32()
		switch randType % 3 {
		case 0: // CHECKSIG
			program.Code = make([]byte, crypto.PublicKeyScriptLength)
			rand.Read(program.Code)
			program.Code[len(program.Code)-1] = common.STANDARD
		case 1: // MULTISIG
			num := math.Intn(5) + 3
			program.Code = make([]byte, (crypto.PublicKeyScriptLength-1)*num+3)
			rand.Read(program.Code)
			program.Code[len(program.Code)-1] = common.MULTISIG
		case 2: // CROSSCHAIN
			num := math.Intn(5) + 3
			program.Code = make([]byte, (crypto.PublicKeyScriptLength-1)*num+3)
			rand.Read(program.Code)
			program.Code[len(program.Code)-1] = common.CROSSCHAIN
		}
		hash, err := crypto.ToProgramHash(program.Code)
		if err != nil {
			t.Errorf("ToProgramHash failed, %s", err.Error())
		}
		hashes = append(hashes, *hash)
		programs = append(programs, program)
	}

	common.SortProgramHashes(hashes)
	SortPrograms(programs)

	for i, hash := range hashes {
		programsHash, err := crypto.ToProgramHash(programs[i].Code)
		if err != nil {
			t.Errorf("ToProgramHash failed, %s", err.Error())
		}
		if !hash.IsEqual(*programsHash) {
			t.Errorf("Hash %s not match with ProgramHash %s", hex.EncodeToString(hash[:]), hex.EncodeToString(programsHash[:]))
		}

		t.Logf("Hash[%02d] %s match with ProgramHash[%02d] %s", i, hex.EncodeToString(hash[:]), i, hex.EncodeToString(programsHash[:]))
	}
}
