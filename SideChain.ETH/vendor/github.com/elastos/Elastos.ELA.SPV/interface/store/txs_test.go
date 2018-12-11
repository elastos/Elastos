package store

import (
	"crypto/rand"
	"encoding/binary"
	"fmt"
	math "math/rand"
	"testing"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

func TestTxIds(t *testing.T) {

	var data []byte

	var txIds []*common.Uint256
	for i := 0; i < 10; i++ {
		var txId common.Uint256
		rand.Read(txId[:])
		txIds = append(txIds, &txId)
		fmt.Println(txId)
	}

	data = putTxId(data, txIds[0])
	fmt.Println(data)

	fmt.Println(getTxIds(data))

	data = delTxId(data, txIds[0])
	fmt.Println(data)

	for _, txId := range txIds {
		data = putTxId(data, txId)
		fmt.Println(getTxIds(data))
		fmt.Println()
	}

	for txIds := getTxIds(data); len(txIds) > 0; txIds = getTxIds(data) {
		count := binary.BigEndian.Uint16(data[:2])
		txId := txIds[math.Intn(int(count))]
		data = delTxId(data, txId)
		fmt.Println(txIds)
		fmt.Println()
	}
}
