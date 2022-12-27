package enumerators

import (
	"testing"
	"fmt"

	"github.com/stretchr/testify/assert"

	"github.com/elastos/Elastos.ELA.SideChain/database"
)

func TestNewListIterator(t *testing.T) {
	iteratorList := NewListIterator()
	iteratorList.Add("111", "1111")
	iteratorList.Add("2222", "22222")
	iteratorList.Add("333", "3333")
	iteratorList.Add("4444", 44444)
	iteratorList.Add([]byte{5,5,5,5}, []byte{5,5,5,5,5})
	iteratorList.Add(66667, 66666)

	var iter database.Iterator
	iter = iteratorList

	for re := iter.First() ; re ; re = iter.Next(){
		fmt.Println(iter.Key())
		fmt.Println(iter.Value())
	}

	fmt.Println("\ntest last")
	for re := iter.Last() ; re ; re = iter.Prev(){
		fmt.Println(iter.Key())
		fmt.Println(iter.Value())
	}


	fmt.Println("\n test Seek")
	//interger := datatype.NewInteger(big.NewInt(66667))
	//data := interger.GetByteArray()
	//re := iter.Seek(data)

	re := iter.Seek([]byte{5,5,5,5})

	for  ; re ; re = iter.Next(){
		fmt.Println(iter.Key())
		fmt.Println(iter.Value())
	}

	fmt.Println("\nrelease")

	iteratorList.Release()

	assert.True(t, iteratorList.Key() == nil)
	assert.True(t, iteratorList.keylist.Len() == 0)
	assert.True(t, iteratorList.valueList.Len() == 0)
	assert.True(t, iteratorList.Value() == nil)

	for re := iter.First() ; re ; re = iter.Next(){
		fmt.Println(iter.Key())
		fmt.Println(iter.Value())
	}

}

