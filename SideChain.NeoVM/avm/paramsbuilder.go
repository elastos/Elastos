package avm

import (
	"bytes"
	"encoding/binary"

	"github.com/elastos/Elastos.ELA/common"
)

type ParamsBuilder struct {
	buffer *bytes.Buffer
}

func NewParamsBuider(buffer *bytes.Buffer) *ParamsBuilder {
	return &ParamsBuilder{buffer}
}

func (p *ParamsBuilder) Emit(op OpCode) {
	p.buffer.WriteByte(byte(op))
}

func (p *ParamsBuilder) EmitPushBool(data bool) {
	if (data) {
		p.Emit(PUSHT)
		return
	}
	p.Emit(PUSHF)
}

func (p *ParamsBuilder) EmitPushInteger(data int64) {
	if data == -1 {
		p.Emit(PUSHM1)
		return
	}
	if data == 0 {
		p.Emit(PUSH0)
		return
	}
	if data > 0 && data < 16 {
		p.Emit(OpCode(int(PUSH1) - 1 + int(data)))
		return
	}
	buf := bytes.NewBuffer([]byte{})
	binary.Write(buf, binary.BigEndian, data)
	p.EmitPushByteArray(common.BytesReverse(buf.Bytes()))
}

func (p *ParamsBuilder) EmitPushByteArray(data []byte) {
	l := len(data)
	if l < int(PUSHBYTES75) {
		p.buffer.WriteByte(byte(l))
	} else if l < 0x100 {
		p.Emit(PUSHDATA1)
		p.buffer.WriteByte(byte(l))
	} else if l < 0x10000 {
		p.Emit(PUSHDATA2)
		b := make([]byte, 2)
		binary.LittleEndian.PutUint16(b, uint16(l))
		p.buffer.Write(b)
	} else {
		p.Emit(PUSHDATA4)
		b := make([]byte, 4)
		binary.LittleEndian.PutUint32(b, uint32(l))
		p.buffer.Write(b)
	}
	p.buffer.Write(data)
}

func (p *ParamsBuilder) EmitPushCall(codeHash []byte) {
	p.Emit(TAILCALL)
	p.buffer.Write(codeHash)
}

func (p *ParamsBuilder) EmitSysCall(api string, args...interface{}) {
	for i := len(args) - 1; i >= 0; i-- {
		switch v := args[i].(type) {
		case int:
			p.EmitPushInteger(int64(v))
		case int32:
			p.EmitPushInteger(int64(v))
		case int64:
			p.EmitPushInteger(int64(v))
		case int16:
			p.EmitPushInteger(int64(v))
		case int8:
			p.EmitPushInteger(int64(v))
		case uint8:
			p.EmitPushInteger(int64(v))
		case []byte:
			p.EmitPushByteArray(v)
		case string:
			p.EmitPushByteArray([]byte(v))
		case *common.Uint168:
			p.EmitPushByteArray(v.Bytes())
		default:
			log.Error("EmitSysCall error:", i, v)
			continue
		}
	}
	p.Emit(SYSCALL)
	p.EmitPushByteArray([]byte(api))
}

func (p *ParamsBuilder) Bytes() []byte {
	return p.buffer.Bytes()
}
