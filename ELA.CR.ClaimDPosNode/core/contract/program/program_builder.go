package program

import (
	"bytes"
	"encoding/binary"
	"math/big"

	"github.com/elastos/Elastos.ELA/vm"
)

type Builder struct {
	buffer bytes.Buffer
}

func NewProgramBuilder() *Builder {
	return &Builder{
	//TODO: add sync pool for create Builder
	}
}

func (pb *Builder) AddOp(op vm.OpCode) {
	pb.buffer.WriteByte(byte(op))
}

func (pb *Builder) AddCodes(codes []byte) {
	pb.buffer.Write(codes)
}

func (pb *Builder) PushNumber(number *big.Int) {
	if number.Cmp(big.NewInt(-1)) == 0 {
		pb.AddOp(vm.PUSHM1)
		return
	}
	if number.Cmp(big.NewInt(0)) == 0 {
		pb.AddOp(vm.PUSH0)
		return
	}
	if number.Cmp(big.NewInt(0)) == 1 && number.Cmp(big.NewInt(16)) <= 0 {
		pb.AddOp(vm.OpCode(byte(vm.PUSH1) - 1 + number.Bytes()[0]))
		return
	}
	pb.PushData(number.Bytes())
}

func (pb *Builder) PushData(data []byte) {
	if data == nil {
		return //TODO: add error
	}

	if len(data) <= int(vm.PUSHBYTES75) {
		pb.buffer.WriteByte(byte(len(data)))
		pb.buffer.Write(data[:])
	} else if len(data) < 0x100 {
		pb.AddOp(vm.PUSHDATA1)
		pb.buffer.WriteByte(byte(len(data)))
		pb.buffer.Write(data[:])
	} else if len(data) < 0x10000 {
		pb.AddOp(vm.PUSHDATA2)
		dataByte := IntToBytes(len(data))
		pb.buffer.Write(dataByte[0:2])
		pb.buffer.Write(data[:])
	} else {
		pb.AddOp(vm.PUSHDATA4)
		dataByte := IntToBytes(len(data))
		pb.buffer.Write(dataByte[0:4])
		pb.buffer.Write(data[:])
	}
}

func (pb *Builder) ToArray() []byte {
	return pb.buffer.Bytes()
}

func IntToBytes(n int) []byte {
	tmp := int32(n)
	bytesBuffer := bytes.NewBuffer([]byte{})
	binary.Write(bytesBuffer, binary.LittleEndian, tmp)
	return bytesBuffer.Bytes()
}

func BytesToInt16(b []byte) int16 {
	bytesBuffer := bytes.NewBuffer(b)
	var tmp int16
	binary.Read(bytesBuffer, binary.BigEndian, &tmp)
	return int16(tmp)
}
