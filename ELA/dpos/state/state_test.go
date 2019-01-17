package state

import (
	"crypto/rand"
	"fmt"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/core/types"
	"github.com/elastos/Elastos.ELA/core/types/outputpayload"
	"github.com/elastos/Elastos.ELA/core/types/payload"

	"github.com/stretchr/testify/assert"
)

// mockRegisterProducerTx creates a register producer transaction with the given
// ProducerInfo.
func mockRegisterProducerTx(info *payload.ProducerInfo) *types.Transaction {
	return &types.Transaction{
		TxType:  types.RegisterProducer,
		Payload: info,
	}
}

// mockUpdateProducerTx creates a update producer transaction with the given
// ProducerInfo.
func mockUpdateProducerTx(info *payload.ProducerInfo) *types.Transaction {
	return &types.Transaction{
		TxType:  types.UpdateProducer,
		Payload: info,
	}
}

// mockCancelProducerTx creates a cancel producer transaction by the producer
// public key.
func mockCancelProducerTx(publicKey []byte) *types.Transaction {
	return &types.Transaction{
		TxType: types.CancelProducer,
		Payload: &payload.CancelProducer{
			PublicKey: publicKey,
		},
	}
}

// mockVoteTx creates a vote transaction with the producers public keys.
func mockVoteTx(publicKeys [][]byte) *types.Transaction {
	output := &types.Output{
		Value: 100,
		Type:  types.OTVote,
		Payload: &outputpayload.VoteOutput{
			Version: 0,
			Contents: []outputpayload.VoteContent{
				{outputpayload.Delegate, publicKeys},
			},
		},
	}

	return &types.Transaction{
		Version: types.TxVersion09,
		TxType:  types.TransferAsset,
		Outputs: []*types.Output{output},
	}
}

// mockVoteTx creates a cancel vote transaction with the previous vote
// transaction.
func mockCancelVoteTx(tx *types.Transaction) *types.Transaction {
	inputs := make([]*types.Input, len(tx.Outputs))
	for i := range tx.Outputs {
		inputs[i] = &types.Input{
			Previous: *types.NewOutPoint(tx.Hash(), uint16(i)),
		}
	}

	return &types.Transaction{
		Version: types.TxVersion09,
		TxType:  types.TransferAsset,
		Inputs:  inputs,
	}
}

// mockIllegalBlockTx creates a illegal block transaction with the producer
// public key.
func mockIllegalBlockTx(publicKey []byte) *types.Transaction {
	return &types.Transaction{
		TxType: types.IllegalBlockEvidence,
		Payload: &types.PayloadIllegalBlock{
			DposIllegalBlocks: types.DposIllegalBlocks{
				Evidence: types.BlockEvidence{
					Signers: [][]byte{publicKey},
				},
				CompareEvidence: types.BlockEvidence{
					Signers: [][]byte{publicKey},
				},
			},
		},
	}
}

func TestState_ProcessTransaction(t *testing.T) {
	state := NewState()

	// Create 10 producers info.
	producers := make([]*payload.ProducerInfo, 10)
	for i, p := range producers {
		p = &payload.ProducerInfo{
			PublicKey: make([]byte, 33),
		}
		for j := range p.PublicKey {
			p.PublicKey[j] = byte(i)
		}
		p.NickName = fmt.Sprintf("Producer-%d", i+1)
		producers[i] = p
	}

	// Register each producer on one height.
	for i, p := range producers {
		tx := mockRegisterProducerTx(p)
		state.ProcessTransactions([]*types.Transaction{tx}, uint32(i+1))
	}

	// At this point, we have 6 pending, 4 active and 10 in total producers.
	if !assert.Equal(t, 6, len(state.GetPendingProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 4, len(state.GetActiveProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 10, len(state.GetProducers())) {
		t.FailNow()
	}

	// Test update producer.
	producers[0].NickName = "Updated"
	tx := mockUpdateProducerTx(producers[0])
	state.ProcessTransactions([]*types.Transaction{tx}, 11)
	p := state.getProducer(producers[0].PublicKey)
	if !assert.Equal(t, "Updated", p.info.NickName) {
		t.FailNow()
	}

	// Test cancel producer.
	tx = mockCancelProducerTx(producers[0].PublicKey)
	state.ProcessTransactions([]*types.Transaction{tx}, 12)
	// at this point, we have 1 canceled, 4 pending, 5 active and 9 in total producers.
	if !assert.Equal(t, 1, len(state.GetCanceledProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 4, len(state.GetPendingProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 5, len(state.GetActiveProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 9, len(state.GetProducers())) {
		t.FailNow()
	}

	// Test vote producer.
	publicKeys := make([][]byte, 5)
	for i, p := range producers[1:6] {
		publicKeys[i] = p.PublicKey
	}
	tx = mockVoteTx(publicKeys)
	state.ProcessTransactions([]*types.Transaction{tx}, 13)

	for _, pk := range publicKeys {
		p := state.getProducer(pk)
		if !assert.Equal(t, common.Fixed64(100), p.votes) {
			t.FailNow()
		}
	}

	// Test illegal producer.
	tx = mockIllegalBlockTx(producers[1].PublicKey)
	state.ProcessTransactions([]*types.Transaction{tx}, 14)
	// at this point, we have 1 canceled, 2 pending, 6 active, 1 illegal and 8 in total producers.
	if !assert.Equal(t, 1, len(state.GetCanceledProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 2, len(state.GetPendingProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 6, len(state.GetActiveProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 1, len(state.GetIllegalProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 8, len(state.GetProducers())) {
		t.FailNow()
	}
}

func TestState_ProcessTransactions(t *testing.T) {
	state := NewState()

	// Create 100 producers info.
	producers := make([]*payload.ProducerInfo, 100)
	for i, p := range producers {
		p = &payload.ProducerInfo{
			PublicKey: make([]byte, 33),
		}
		for j := range p.PublicKey {
			p.PublicKey[j] = byte(i)
		}
		p.NickName = fmt.Sprintf("Producer-%d", i+1)
		producers[i] = p
	}

	// Register 10 producers on one height.
	for i := 0; i < 10; i++ {
		txs := make([]*types.Transaction, 10)
		for i, p := range producers[i*10 : (i+1)*10] {
			txs[i] = mockRegisterProducerTx(p)
		}
		state.ProcessTransactions(txs, uint32(i+1))
	}
	// at this point, we have 60 pending, 44 active and 100 in total producers.
	if !assert.Equal(t, 60, len(state.GetPendingProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 40, len(state.GetActiveProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 100, len(state.GetProducers())) {
		t.FailNow()
	}

	// Update 10 producers.
	txs := make([]*types.Transaction, 10)
	for i := range txs {
		producers[i].NickName = fmt.Sprintf("Updated-%d", i)
		txs[i] = mockUpdateProducerTx(producers[i])
	}
	state.ProcessTransactions(txs, 11)
	for i := range txs {
		p := state.getProducer(producers[i].PublicKey)
		if !assert.Equal(t, fmt.Sprintf("Updated-%d", i), p.info.NickName) {
			t.FailNow()
		}
	}

	// Cancel 10 producers.
	txs = make([]*types.Transaction, 10)
	for i := range txs {
		txs[i] = mockCancelProducerTx(producers[i].PublicKey)
	}
	state.ProcessTransactions(txs, 12)
	// at this point, we have 10 canceled, 40 pending, 50 active and 90 in total producers.
	if !assert.Equal(t, 10, len(state.GetCanceledProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 40, len(state.GetPendingProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 50, len(state.GetActiveProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 90, len(state.GetProducers())) {
		t.FailNow()
	}

	// Vote 10 producers for 10 times.
	publicKeys := make([][]byte, 10)
	for i, p := range producers[10:20] {
		publicKeys[i] = p.PublicKey
	}
	txs = make([]*types.Transaction, 10)
	for i := range txs {
		txs[i] = mockVoteTx(publicKeys)
	}
	state.ProcessTransactions(txs, 13)
	for _, pk := range publicKeys {
		p := state.getProducer(pk)
		if !assert.Equal(t, common.Fixed64(1000), p.votes) {
			t.FailNow()
		}
	}

	// Illegal 10 producers.
	txs = make([]*types.Transaction, 10)
	for i := range txs {
		txs[i] = mockIllegalBlockTx(producers[10+i].PublicKey)
	}
	state.ProcessTransactions(txs, 14)
	// at this point, we have 10 canceled, 20 pending, 60 active, 10 illegal and 80 in total producers.
	if !assert.Equal(t, 10, len(state.GetCanceledProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 20, len(state.GetPendingProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 60, len(state.GetActiveProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 10, len(state.GetIllegalProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 80, len(state.GetProducers())) {
		t.FailNow()
	}

	// Mixed transactions 1 register, 2 cancel, 3 updates, 4 votes, 5 illegals.
	txs = make([]*types.Transaction, 15)
	info := &payload.ProducerInfo{
		PublicKey: make([]byte, 33),
	}
	for i := range info.PublicKey {
		info.PublicKey[i] = byte(101)
	}
	info.NickName = "Producer-101"
	txs[0] = mockRegisterProducerTx(info)

	for i := 0; i < 2; i++ {
		txs[1+i] = mockCancelProducerTx(producers[20+i].PublicKey)
	}

	for i := 0; i < 3; i++ {
		txs[3+i] = mockUpdateProducerTx(producers[30+i])
	}

	publicKeys = make([][]byte, 4)
	for i, p := range producers[40:44] {
		publicKeys[i] = p.PublicKey
	}
	for i := 0; i < 4; i++ {
		txs[6+i] = mockVoteTx(publicKeys)
	}

	for i := 0; i < 5; i++ {
		txs[10+i] = mockIllegalBlockTx(producers[50+i].PublicKey)
	}
	state.ProcessTransactions(txs, 15)
	// at this point, we have 12 canceled, 11 pending, 63 active, 15 illegal and 74 in total producers.
	// 10+2
	if !assert.Equal(t, 12, len(state.GetCanceledProducers())) {
		t.FailNow()
	}
	// 20-10+1
	if !assert.Equal(t, 11, len(state.GetPendingProducers())) {
		t.FailNow()
	}
	// 60+10-2-5
	if !assert.Equal(t, 63, len(state.GetActiveProducers())) {
		t.FailNow()
	}
	// 10+5
	if !assert.Equal(t, 15, len(state.GetIllegalProducers())) {
		t.FailNow()
	}
	// 101-12-15
	if !assert.Equal(t, 74, len(state.GetProducers())) {
		t.FailNow()
	}
	for _, pk := range publicKeys {
		p := state.getProducer(pk)
		if !assert.Equal(t, common.Fixed64(400), p.votes) {
			t.FailNow()
		}
	}
}

func TestState_ProcessIllegalBlockEvidence(t *testing.T) {
	state := NewState()

	// Create 10 producers info.
	producers := make([]*payload.ProducerInfo, 10)
	for i, p := range producers {
		p = &payload.ProducerInfo{
			PublicKey: make([]byte, 33),
		}
		for j := range p.PublicKey {
			p.PublicKey[j] = byte(i)
		}
		p.NickName = fmt.Sprintf("Producer-%d", i+1)
		producers[i] = p
	}

	// Register each producer on one height.
	for i, p := range producers {
		tx := mockRegisterProducerTx(p)
		state.ProcessTransactions([]*types.Transaction{tx}, uint32(i+1))
	}
	// At this point, we have 6 pending, 4 active and 10 in total producers.

	// Make producer 0 illegal.
	tx := mockIllegalBlockTx(producers[0].PublicKey)
	state.ProcessIllegalBlockEvidence(tx.Payload)
	// At this point, we have 6 pending, 3 active 1 illegal and 9 in total producers.
	if !assert.Equal(t, 6, len(state.GetPendingProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 3, len(state.GetActiveProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 1, len(state.GetIllegalProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 9, len(state.GetProducers())) {
		t.FailNow()
	}

	// Process next height, state will rollback illegal producer.
	state.ProcessTransactions(nil, 11)
	// At this point, we have 5 pending, 5 active and 10 in total producers.
	if !assert.Equal(t, 5, len(state.GetPendingProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 5, len(state.GetActiveProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 10, len(state.GetProducers())) {
		t.FailNow()
	}
}

func TestState_Rollback(t *testing.T) {
	state := NewState()

	// Create 10 producers info.
	producers := make([]*payload.ProducerInfo, 10)
	for i, p := range producers {
		p = &payload.ProducerInfo{
			PublicKey: make([]byte, 33),
		}
		for j := range p.PublicKey {
			p.PublicKey[j] = byte(i)
		}
		p.NickName = fmt.Sprintf("Producer-%d", i+1)
		producers[i] = p
	}

	// Register each producer on one height.
	for i, p := range producers {
		tx := mockRegisterProducerTx(p)
		state.ProcessTransactions([]*types.Transaction{tx}, uint32(i+1))
	}
	// At this point, we have 6 pending, 4 active and 10 in total producers.
	if !assert.Equal(t, 6, len(state.GetPendingProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 4, len(state.GetActiveProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 10, len(state.GetProducers())) {
		t.FailNow()
	}

	err := state.RollbackTo(9)
	if !assert.NoError(t, err) {
		t.FailNow()
	}

	// At this point, we have 6 pending, 3 active and 9 in total producers.
	if !assert.Equal(t, 6, len(state.GetPendingProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 3, len(state.GetActiveProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 9, len(state.GetProducers())) {
		t.FailNow()
	}
}

func TestState_GetHistory(t *testing.T) {
	state := NewState()

	// Create 10 producers info.
	producers := make([]*payload.ProducerInfo, 10)
	for i, p := range producers {
		p = &payload.ProducerInfo{
			PublicKey: make([]byte, 33),
		}
		for j := range p.PublicKey {
			p.PublicKey[j] = byte(i)
		}
		p.NickName = fmt.Sprintf("Producer-%d", i+1)
		producers[i] = p
	}

	// Register each producer on one height.
	for i, p := range producers {
		tx := mockRegisterProducerTx(p)
		state.ProcessTransactions([]*types.Transaction{tx}, uint32(i+1))
	}
	// At this point, we have 6 pending, 4 active and 10 in total producers.

	// Test update producer.
	producers[0].NickName = "Updated"
	tx := mockUpdateProducerTx(producers[0])
	state.ProcessTransactions([]*types.Transaction{tx}, 11)
	p := state.getProducer(producers[0].PublicKey)
	if !assert.Equal(t, "Updated", p.info.NickName) {
		t.FailNow()
	}

	// Test cancel producer.
	tx = mockCancelProducerTx(producers[0].PublicKey)
	state.ProcessTransactions([]*types.Transaction{tx}, 12)
	// At this point, we have 1 canceled, 4 pending, 5 active and 9 in total producers.

	// Test vote producer.
	publicKeys := make([][]byte, 5)
	for i, p := range producers[1:6] {
		publicKeys[i] = p.PublicKey
	}
	tx = mockVoteTx(publicKeys)
	state.ProcessTransactions([]*types.Transaction{tx}, 13)
	for _, pk := range publicKeys {
		p := state.getProducer(pk)
		if !assert.Equal(t, common.Fixed64(100), p.votes) {
			t.FailNow()
		}
	}

	// Test illegal producer.
	tx = mockIllegalBlockTx(producers[1].PublicKey)
	state.ProcessTransactions([]*types.Transaction{tx}, 14)
	// At this point, we have 1 canceled, 2 pending, 6 active, 1 illegal and 8 in total producers.

	_, err := state.GetHistory(0)
	limitHeight := state.history.height - uint32(len(state.history.changes))
	if !assert.EqualError(t, err, fmt.Sprintf("seek to %d overflow"+
		" history capacity, at most seek to %d", 0, limitHeight)) {
		t.FailNow()
	}

	s, err := state.GetHistory(10)
	if !assert.NoError(t, err) {
		t.FailNow()
	}
	// At this point, we have 4 pending and 4 in total producers.
	if !assert.Equal(t, 6, len(s.GetPendingProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 4, len(s.GetActiveProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 10, len(s.GetProducers())) {
		t.FailNow()
	}

	s, err = state.GetHistory(14)
	if !assert.NoError(t, err) {
		t.FailNow()
	}
	// At this point, we have 1 canceled, 2 pending, 6 active, 1 illegal and 8 in total producers.
	if !assert.Equal(t, 1, len(state.GetCanceledProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 2, len(state.GetPendingProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 6, len(state.GetActiveProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 1, len(state.GetIllegalProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 8, len(state.GetProducers())) {
		t.FailNow()
	}

	s, err = state.GetHistory(12)
	if !assert.NoError(t, err) {
		t.FailNow()
	}
	// At this point, we have 1 canceled, 4 pending, 5 active and 9 in total producers.
	if !assert.Equal(t, 1, len(s.GetCanceledProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 4, len(s.GetPendingProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 5, len(s.GetActiveProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 9, len(s.GetProducers())) {
		t.FailNow()
	}

	// Process a new height see if state go to best height.
	state.ProcessTransactions(nil, 15)
	// At this point, we have 1 canceled, 1 pending, 7 active, 1 illegal and 8 in total producers.
	if !assert.Equal(t, 1, len(state.GetCanceledProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 1, len(state.GetPendingProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 7, len(state.GetActiveProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 1, len(state.GetIllegalProducers())) {
		t.FailNow()
	}
	if !assert.Equal(t, 8, len(state.GetProducers())) {
		t.FailNow()
	}

}

func TestState_IsUnusedNickname(t *testing.T) {
	state := NewState()

	// Create 10 producers info.
	producers := make([]*payload.ProducerInfo, 10)
	for i, p := range producers {
		p = &payload.ProducerInfo{
			PublicKey: make([]byte, 33),
		}
		for j := range p.PublicKey {
			p.PublicKey[j] = byte(i)
		}
		p.NickName = fmt.Sprintf("Producer-%d", i+1)
		producers[i] = p
	}

	// Register each producer on one height.
	for i, p := range producers {
		tx := mockRegisterProducerTx(p)
		state.ProcessTransactions([]*types.Transaction{tx}, uint32(i+1))
	}

	for i := range producers {
		if !assert.Equal(t, false, state.IsUnusedNickname(
			fmt.Sprintf("Producer-%d", i+1))) {
			t.FailNow()
		}
	}

	// Change producer-1 nickname to Updated.
	producers[0].NickName = "Updated"
	tx := mockUpdateProducerTx(producers[0])
	state.ProcessTransactions([]*types.Transaction{tx}, 11)
	p := state.getProducer(producers[0].PublicKey)
	if !assert.Equal(t, "Updated", p.info.NickName) {
		t.FailNow()
	}

	if !assert.Equal(t, true, state.IsUnusedNickname("Producer-1")) {
		t.FailNow()
	}

	// Cancel producer-2, see if nickname change to unused.
	tx = mockCancelProducerTx(producers[1].PublicKey)
	state.ProcessTransactions([]*types.Transaction{tx}, 12)
	if !assert.Equal(t, true, state.IsUnusedNickname("Producer-2")) {
		t.FailNow()
	}

	// Make producer-3 illegal, see if nickname change to unused.
	tx = mockIllegalBlockTx(producers[2].PublicKey)
	state.ProcessIllegalBlockEvidence(tx.Payload)
	if !assert.Equal(t, true, state.IsUnusedNickname("Producer-3")) {
		t.FailNow()
	}
}

func TestState_IsDPOSTransaction(t *testing.T) {
	state := NewState()

	producer := &payload.ProducerInfo{
		PublicKey: make([]byte, 33),
		NickName:  "Producer",
	}
	rand.Read(producer.PublicKey)

	tx := mockRegisterProducerTx(producer)
	if !assert.Equal(t, true, state.IsDPOSTransaction(tx)) {
		t.FailNow()
	}
	state.ProcessTransactions([]*types.Transaction{tx}, 1)
	for i := uint32(1); i < 10; i++ {
		state.ProcessTransactions(nil, i)
	}

	tx = mockUpdateProducerTx(producer)
	if !assert.Equal(t, true, state.IsDPOSTransaction(tx)) {
		t.FailNow()
	}

	tx = mockCancelProducerTx(producer.PublicKey)
	if !assert.Equal(t, true, state.IsDPOSTransaction(tx)) {
		t.FailNow()
	}

	tx = mockVoteTx([][]byte{producer.PublicKey})
	if !assert.Equal(t, true, state.IsDPOSTransaction(tx)) {
		t.FailNow()
	}
	state.ProcessTransactions([]*types.Transaction{tx}, 10)
	p := state.getProducer(producer.PublicKey)
	if !assert.Equal(t, common.Fixed64(100), p.votes) {
		t.FailNow()
	}

	tx = mockCancelVoteTx(tx)
	if !assert.Equal(t, true, state.IsDPOSTransaction(tx)) {
		t.FailNow()
	}
	state.ProcessTransactions([]*types.Transaction{tx}, 11)
	p = state.getProducer(producer.PublicKey)
	if !assert.Equal(t, common.Fixed64(0), p.votes) {
		t.FailNow()
	}

	tx = mockIllegalBlockTx(producer.PublicKey)
	if !assert.Equal(t, true, state.IsDPOSTransaction(tx)) {
		t.FailNow()
	}
}
