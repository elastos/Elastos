package blockchain

import (
	"bytes"
	"crypto/rand"
	math "math/rand"
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

	for i := 0; i < 100; i++ {
		txSigleChecksig(t)
		txManyChecksig(t)
		txSigleMultisig(t)
		txManyMultisig(t)
		txManyDiffsig(t)
	}
}

func txSigleChecksig(t *testing.T) {
	var tx *core.Transaction
	var hashes []common.Uint168
	var programs []*core.Program

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

	// With unmatched hash
	var fakeHash common.Uint168
	rand.Read(fakeHash[:])
	hashes = []common.Uint168{fakeHash}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with unmatched hash")
	}
	t.Logf("[Passed] 1. Transaction with 1 checksig program [Unmatched hash], %s", err.Error())

	// With unmatched signature
	var fakeSignature [64]byte
	rand.Read(fakeSignature[:])
	hashes = []common.Uint168{*act.programHash}
	programs = []*core.Program{{Code: act.redeemScript, Parameter: fakeSignature[:]}}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with unmatched signature")
	}
	t.Logf("[Passed] 1. Transaction with 1 checksig program [Unmatched signature], %s", err.Error())

	// With no program
	programs = []*core.Program{}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with no program")
	}
	t.Logf("[Passed] 1. Transaction with 1 checksig program [No program], %s", err.Error())

	// With no code
	programs = []*core.Program{{Parameter: signature}}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with no code")
	}
	t.Logf("[Passed] 1. Transaction with 1 checksig program [No code], %s", err.Error())

	// With no parameter
	programs = []*core.Program{{Code: act.redeemScript}}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with no parameter")
	}
	t.Logf("[Passed] 1. Transaction with 1 checksig program [No parameter], %s", err.Error())
}

func txManyChecksig(t *testing.T) {
	var tx *core.Transaction
	var hashes []common.Uint168
	var programs []*core.Program

	tx = buildTx()
	data := getData(tx)
	// Normal
	num := math.Intn(99) + 1
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
	err := RunPrograms(data, hashes, programs)
	if err != nil {
		t.Errorf("[RunProgram] with many checksig failed, %s", err.Error())
	}
	t.Logf("[Passed] 2. Transaction with many checksig program [Normal]")

	// With unmatched hash
	fakeIndex := math.Intn(num)
	rand.Read(hashes[fakeIndex][:])
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with unmathed hash")
	}
	t.Logf("[Passed] 2. Transaction with many checksig program [Unmatched hash], %s", err.Error())

	// With unmatched hashes
	init()
	for i := 0; i < num; i++ {
		fakeIndex = math.Intn(num)
		rand.Read(hashes[fakeIndex][:])
	}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with unmathed hashes")
	}
	t.Logf("[Passed] 2. Transaction with many checksig program [Unmatched hashes], %s", err.Error())

	// With no programs
	init()
	programs = []*core.Program{}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with no programs")
	}
	t.Logf("[Passed] 2. Transaction with many checksig program [No programs], %s", err.Error())

	// With random no code
	init()
	fakeIndex = math.Intn(num)
	programs[fakeIndex].Code = nil
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with random no code")
	}
	t.Logf("[Passed] 2. Transaction with many checksig program [Random no code], %s", err.Error())

	// With random no codes
	init()
	for i := 0; i < num; i++ {
		fakeIndex = math.Intn(num)
		programs[fakeIndex].Code = nil
	}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with random no codes")
	}
	t.Logf("[Passed] 2. Transaction with many checksig program [Random no codes], %s", err.Error())

	// With random no parameter
	init()
	fakeIndex = math.Intn(num)
	programs[fakeIndex].Parameter = nil
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with random no parameter")
	}
	t.Logf("[Passed] 2. Transaction with many checksig program [Random no parameter], %s", err.Error())

	// With random no parameters
	init()
	for i := 0; i < num; i++ {
		fakeIndex = math.Intn(num)
		programs[fakeIndex].Parameter = nil
	}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with random no parameters")
	}
	t.Logf("[Passed] 2. Transaction with many checksig program [Random no parameters], %s", err.Error())
}

func txSigleMultisig(t *testing.T) {
	var tx *core.Transaction
	var hashes []common.Uint168
	var programs []*core.Program

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
	t.Logf("[Passed] 3. Transaction with 1 multisig program [Normal]")

	// With unmatched hash
	var fakeHash common.Uint168
	rand.Read(fakeHash[:])
	hashes = []common.Uint168{fakeHash}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with unmatched hash")
	}
	t.Logf("[Passed] 3. Transaction with 1 multisig program [Unmatched hash], %s", err.Error())

	// With unmatched signatures
	var fakeSignatures = make([]byte, crypto.SignatureScriptLength*acts)
	rand.Read(fakeSignatures[:])
	hashes = []common.Uint168{*mact.programHash}
	programs = []*core.Program{{Code: mact.redeemScript, Parameter: fakeSignatures[:]}}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with unmatched signatures")
	}
	t.Logf("[Passed] 3. Transaction with 1 multisig program [Unmatched signatures], %s", err.Error())

	// With not enough signatures
	var signatures []byte
	for i, act := range mact.accounts {
		if i >= len(mact.accounts)/2 {
			break
		}
		signature, err := sign(act.private, data)
		if err != nil {
			t.Errorf("Generate signature failed, %s", err.Error())
		}
		signatures = append(signatures, signature...)
	}
	programs = []*core.Program{{Code: mact.redeemScript, Parameter: signatures}}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with not enough signatures")
	}
	t.Logf("[Passed] 3. Transaction with 1 multisig program [Not enough signatures], %s", err.Error())

	// With too many signatures
	for _, act := range mact.accounts {
		signature, err := sign(act.private, data)
		if err != nil {
			t.Errorf("Generate signature failed, %s", err.Error())
		}
		signatures = append(signatures, signature...)
	}
	programs = []*core.Program{{Code: mact.redeemScript, Parameter: signatures}}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with too many signatures")
	}
	t.Logf("[Passed] 3. Transaction with 1 multisig program [Too many signatures], %s", err.Error())

	// With duplicate signature
	signatures = nil
	for i, act := range mact.accounts {
		if i+1 >= len(mact.accounts) {
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
	programs = []*core.Program{{Code: mact.redeemScript, Parameter: signatures}}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with duplicated signature")
	}
	t.Logf("[Passed] 3. Transaction with 1 multisig program [Duplicated signature], %s", err.Error())

	// With not invalid signatures
	signatures = make([]byte, math.Intn(len(mact.accounts)*crypto.SignatureScriptLength))
	rand.Read(signatures)
	programs = []*core.Program{{Code: mact.redeemScript, Parameter: signatures}}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with invalid signatures")
	}
	t.Logf("[Passed] 3. Transaction with 1 multisig program [Invalid signatures], %s", err.Error())

	// With no program
	programs = []*core.Program{}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with no program")
	}
	t.Logf("[Passed] 3. Transaction with 1 multisig program [No program], %s", err.Error())

	// With no code
	programs = []*core.Program{{Parameter: signatures}}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with no code")
	}
	t.Logf("[Passed] 3. Transaction with 1 multisig program [No code], %s", err.Error())

	// With no parameter
	programs = []*core.Program{{Code: mact.redeemScript}}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with no parameter")
	}
	t.Logf("[Passed] 3. Transaction with 1 multisig program [No parameter], %s", err.Error())
}

func txManyMultisig(t *testing.T) {
	var tx *core.Transaction
	var hashes []common.Uint168
	var programs []*core.Program

	tx = buildTx()
	data := getData(tx)
	// Normal
	num := math.Intn(99) + 1
	macts := make([]*multiAccount, 0, num)
	init := func() {
		hashes = make([]common.Uint168, 0, num)
		programs = make([]*core.Program, 0, num)
		for i := 0; i < num; i++ {
			acts := math.Intn(2) + 3
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
	err := RunPrograms(data, hashes, programs)
	if err != nil {
		t.Errorf("[RunProgram] with many multisig failed, %s", err.Error())
	}
	t.Logf("[Passed] 4. Transaction with many multisig program [Normal]")

	// With unmatched hash
	fakeIndex := math.Intn(num)
	rand.Read(hashes[fakeIndex][:])
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with unmathed hash")
	}
	t.Logf("[Passed] 4. Transaction with many multisig program [Unmatched hash], %s", err.Error())

	// With unmatched hashes
	init()
	for i := 0; i < num; i++ {
		fakeIndex = math.Intn(num)
		rand.Read(hashes[fakeIndex][:])
	}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with unmathed hashes")
	}
	t.Logf("[Passed] 4. Transaction with many multisig program [Unmatched hashes], %s", err.Error())

	// With random not enough signature
	init()
	for i := 0; i < num; i++ {
		fakeIndex = math.Intn(num)
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
	t.Logf("[Passed] 4. Transaction with many multisig program [Random not enough signature], %s", err.Error())

	// With random too many signature
	init()
	for i := 0; i < num; i++ {
		fakeIndex = math.Intn(num)
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
	t.Logf("[Passed] 4. Transaction with many multisig program [Random too many signature], %s", err.Error())

	// With random duplicate signature
	init()
	for i := 0; i < num; i++ {
		fakeIndex = math.Intn(num)
		var signatures []byte
		for i, act := range macts[fakeIndex].accounts {
			if i+1 >= len(macts[fakeIndex].accounts) {
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
	t.Logf("[Passed] 4. Transaction with many multisig program [Random duplicated signature], %s", err.Error())

	// With random invalid signatures
	init()
	for i := 0; i < num; i++ {
		fakeIndex = math.Intn(num)
		var signatures = make([]byte, math.Intn(len(macts[fakeIndex].accounts)*crypto.SignatureScriptLength))
		rand.Read(signatures)
		programs[fakeIndex].Parameter = signatures
	}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with invalid signatures")
	}
	t.Logf("[Passed] 4. Transaction with many multisig program [Invalid signatures], %s", err.Error())

	// With no programs
	init()
	for i := 0; i < num; i++ {
		fakeIndex = math.Intn(num)
		rand.Read(hashes[fakeIndex][:])
	}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with no programs")
	}
	t.Logf("[Passed] 4. Transaction with many multisig program [No programs], %s", err.Error())

	// With random no code
	init()
	fakeIndex = math.Intn(num)
	programs[fakeIndex].Code = nil
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with random no code")
	}
	t.Logf("[Passed] 4. Transaction with many multisig program [Random no code], %s", err.Error())

	// With random no codes
	init()
	for i := 0; i < num; i++ {
		fakeIndex = math.Intn(num)
		programs[fakeIndex].Code = nil
	}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with random no codes")
	}
	t.Logf("[Passed] 4. Transaction with many multisig program [Random no codes], %s", err.Error())

	// With random no parameter
	init()
	fakeIndex = math.Intn(num)
	programs[fakeIndex].Parameter = nil
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with random no parameter")
	}
	t.Logf("[Passed] 4. Transaction with many multisig program [Random no parameter], %s", err.Error())

	// With random no parameters
	init()
	for i := 0; i < num; i++ {
		fakeIndex = math.Intn(num)
		programs[fakeIndex].Parameter = nil
	}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with random no parameters")
	}
	t.Logf("[Passed] 4. Transaction with many multisig program [Random no parameters], %s", err.Error())
}

func txManyDiffsig(t *testing.T) {
	var tx *core.Transaction
	var hashes []common.Uint168
	var programs []*core.Program

	tx = buildTx()
	data := getData(tx)
	// Normal
	num := math.Intn(99) + 1
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
	t.Logf("[Passed] 5. Transaction with many diffsig program [Normal]")

	// With unmatched hash
	fakeIndex := math.Intn(num)
	rand.Read(hashes[fakeIndex][:])
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with unmathed hash")
	}
	t.Logf("[Passed] 5. Transaction with many diffsig program [Unmatched hash], %s", err.Error())

	// With unmatched hashes
	init()
	for i := 0; i < num; i++ {
		fakeIndex = math.Intn(num)
		rand.Read(hashes[fakeIndex][:])
	}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with unmathed hashes")
	}
	t.Logf("[Passed] 5. Transaction with many diffsig program [Unmatched hashes], %s", err.Error())

	// With random invalid signature
	init()
	for i := 0; i < num; i++ {
		fakeIndex = math.Intn(num)
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
	t.Logf("[Passed] 5. Transaction with many diffsig program [Invalid signature], %s", err.Error())

	// With no programs
	init()
	for i := 0; i < num; i++ {
		fakeIndex = math.Intn(num)
		rand.Read(hashes[fakeIndex][:])
	}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with no programs")
	}
	t.Logf("[Passed] 5. Transaction with many diffsig program [No programs], %s", err.Error())

	// With random no code
	init()
	fakeIndex = math.Intn(num)
	programs[fakeIndex].Code = nil
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with random no code")
	}
	t.Logf("[Passed] 5. Transaction with many diffsig program [Random no code], %s", err.Error())

	// With random no codes
	init()
	for i := 0; i < num; i++ {
		fakeIndex = math.Intn(num)
		programs[fakeIndex].Code = nil
	}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with random no codes")
	}
	t.Logf("[Passed] 5. Transaction with many diffsig program [Random no codes], %s", err.Error())

	// With random no parameter
	init()
	fakeIndex = math.Intn(num)
	programs[fakeIndex].Parameter = nil
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with random no parameter")
	}
	t.Logf("[Passed] 5. Transaction with many diffsig program [Random no parameter], %s", err.Error())

	// With random no parameters
	init()
	for i := 0; i < num; i++ {
		fakeIndex = math.Intn(num)
		programs[fakeIndex].Parameter = nil
	}
	err = RunPrograms(data, hashes, programs)
	if err == nil {
		t.Errorf("[RunProgram] passed with random no parameters")
	}
	t.Logf("[Passed] 5. Transaction with many diffsig program [Random no parameters], %s", err.Error())
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
