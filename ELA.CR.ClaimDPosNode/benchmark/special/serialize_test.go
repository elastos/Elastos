// Copyright (c) 2017-2020 The Elastos Foundation
// Use of this source code is governed by an MIT
// license that can be found in the LICENSE file.
// 

package special

import (
	"bytes"
	"math/rand"
	"testing"

	"github.com/elastos/Elastos.ELA/common"
)

const (
	largeDataSize = 2048
)

func Benchmark_ReadUint8(b *testing.B) {
	var buf bytes.Buffer
	buf.WriteByte(uint8(rand.Uint32()))

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		common.ReadUint8(&buf)
	}
	b.StopTimer()
}

func Benchmark_ReadUint8_BinarySerializer(b *testing.B) {
	var buf bytes.Buffer
	buf.WriteByte(uint8(rand.Uint32()))

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		common.BinarySerializer.Uint8(&buf)
	}
	b.StopTimer()
}

func Benchmark_ReadUint8_Large(b *testing.B) {
	var buf bytes.Buffer
	for i := 0; i < largeDataSize; i++ {
		buf.WriteByte(uint8(rand.Uint32()))
	}

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		for i := 0; i < largeDataSize; i++ {
			common.ReadUint8(&buf)
		}
	}
	b.StopTimer()
}

func Benchmark_ReadUint8_BinarySerializer_Large(b *testing.B) {
	var buf bytes.Buffer
	for i := 0; i < largeDataSize; i++ {
		buf.WriteByte(uint8(rand.Uint32()))
	}

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		for i := 0; i < largeDataSize; i++ {
			common.BinarySerializer.Uint8(&buf)
		}
	}
	b.StopTimer()
}

func Benchmark_WriteUint8(b *testing.B) {
	var buf bytes.Buffer
	value := uint8(rand.Uint32())

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		common.WriteUint8(&buf, value)
	}
	b.StopTimer()
}

func Benchmark_WriteUint8_BinarySerializer(b *testing.B) {
	var buf bytes.Buffer
	value := uint8(rand.Uint32())

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		common.BinarySerializer.PutUint8(&buf, value)
	}
	b.StopTimer()
}

func Benchmark_WriteUint8_Large(b *testing.B) {
	var buf bytes.Buffer
	value := uint8(rand.Uint32())

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		for j := 0; j < largeDataSize; j++ {
			common.WriteUint8(&buf, value)
		}
	}
	b.StopTimer()
}

func Benchmark_WriteUint8_BinarySerializer_Large(b *testing.B) {
	var buf bytes.Buffer
	value := uint8(rand.Uint32())

	b.ResetTimer()
	for i := 0; i < b.N; i++ {
		for j := 0; j < largeDataSize; j++ {
			common.BinarySerializer.PutUint8(&buf, value)
		}
	}
	b.StopTimer()
}
