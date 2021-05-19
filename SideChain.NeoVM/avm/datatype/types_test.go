package datatype

import (
	"testing"
	"math/big"
	"github.com/stretchr/testify/assert"
)

func TestTypes(t *testing.T) {
	i := NewInteger(big.NewInt(1))
	ba := NewByteArray([]byte{1})
	b := NewBoolean(false)
	a1 := NewArray([]StackItem{i})
	//a2 := NewArray([]StackItem{ba})
	t.Log(i.GetByteArray())
	t.Log(ba.GetBoolean())
	t.Log(b.Equals(NewBoolean(false)))
	t.Log(a1.Equals(NewArray([]StackItem{NewInteger(big.NewInt(1))})))
}

func TestDictionary_Remove(t *testing.T) {
	dictionary := NewDictionary()
	key := NewByteArray([]byte{1,2,3})
	value := NewInteger(big.NewInt(100))
	dictionary.PutStackItem(key, value)

	assert.True(t, dictionary.GetValue(key).Equals(value))

	dictionary.Remove(key)

	assert.True(t, dictionary.GetValue(key) == nil)
}