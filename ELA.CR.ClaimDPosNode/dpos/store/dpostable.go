package store

import (
	"bytes"
	"errors"
	"io"

	"github.com/elastos/Elastos.ELA/dpos/log"

	"github.com/elastos/Elastos.ELA.Utility/common"
)

const (
	FieldUint8   = 0x01
	FieldUint16  = 0x02
	FieldUint32  = 0x03
	FieldUint64  = 0x04
	FieldUint168 = 0x05
	FieldUint256 = 0x06
	FieldInt     = 0x07
	FieldInt32   = 0x08
	FieldIn64    = 0x09
	FieldBytes   = 0x0a
	FieldString  = 0x0b
	FieldBool    = 0x0c
)

var (
	tablePrefix    = []byte{'t'}
	indexPrefixSep = []byte("i")
	idPrefix       = []byte("id")
)

type Field struct {
	Name  string
	Value interface{}
}

func (f *Field) Data() []byte {
	buf := new(bytes.Buffer)
	// todo  support string
	err := common.WriteElement(buf, f.Value)
	if err != nil {
		log.Error("value to bytes failed")
		return nil
	}
	return buf.Bytes()
}

type DposTable struct {
	Name       string
	PrimaryKey uint64
	Indexes    []uint64
	Fields     []string
}

func (d *DposTable) Serialize(w io.Writer) error {
	return nil
}

func (d *DposTable) Deserialize(r io.Reader) error {
	return nil
}

func (d *DposTable) Data(fields []*Field) ([]byte, error) {
	// todo check fields in d.table

	// todo return bytes from fields
	// result: [len,value1,value2,value3,...]
	// value1: index_type_data

	buf := new(bytes.Buffer)
	if err := common.WriteVarUint(buf, uint64(len(fields))); err != nil {
		return nil, err
	}

	for index, field := range d.Fields {
		for _, f := range fields {
			if field == f.Name {
				if err := common.WriteVarUint(buf, uint64(index)); err != nil {
					return nil, err
				}
				if err := writeElements(buf, f.Value); err != nil {
					return nil, err
				}
			}
		}
	}
	return nil, nil
}

func (d *DposTable) GetFields(data []byte) ([]*Field, error) {
	r := bytes.NewReader(data)
	len, err := common.ReadVarUint(r, 0)
	if err != nil {
		return nil, err
	}
	var result []*Field
	for i := uint64(0); i < len; i++ {
		index, err := common.ReadVarUint(r, 0)
		if err != nil {
			return nil, err
		}
		value, err := readElements(r)
		if err != nil {
			return nil, err
		}
		result = append(result, &Field{
			Name:  d.Fields[index],
			Value: value,
		})
	}
	return nil, nil
}

func writeElements(buf *bytes.Buffer, element interface{}) (err error) {
	// todo wrie all types of element
	switch e := element.(type) {
	case uint8:
		buf.WriteByte(FieldUint8)
		common.WriteElement(buf, e)
	case uint16:
		buf.WriteByte(FieldUint16)
		common.WriteElement(buf, e)
	case uint32:
	case uint64:
	case common.Uint168:
	case common.Uint256:
	case int:
	case int32:
	case int64:
	case []byte:
	case string:
	case bool:
	}
	return nil
}

func readElements(reader io.Reader) (interface{}, error) {
	// todo read all types of element
	fieldType, err := common.ReadBytes(reader, 1)
	if err != nil {
		return nil, err
	}
	switch fieldType[0] {
	case FieldUint8:
		var result uint8
		common.ReadElement(reader, result)
		return result, nil
	case FieldUint16:
		var result uint16
		common.ReadElement(reader, result)
		return result, nil
	case FieldUint32:
	case FieldUint64:
	case FieldUint168:
	case FieldUint256:
	case FieldInt:
	case FieldInt32:
	case FieldIn64:
	case FieldBytes:
	case FieldString:
	case FieldBool:
	}

	return nil, errors.New("unknown type")
}

func (d *DposTable) Column(fieldName string) uint64 {
	for i, f := range d.Fields {
		if f == fieldName {
			return uint64(i)
		}
	}
	return 0
}

func getTableKey(tableName string) []byte {
	// todo return tablePrefix_indexPrefixSep_tableName
	return nil
}

func getTableIDKey(tableName string) []byte {
	// todo return tablePrefix_indexPrefixSep_idPrefix_tableName
	return nil
}

func getRowKey(tableName string, rowID uint64) []byte {
	// todo return tablePrefix_indexPrefixSep_tableName_rowID
	return nil
}

func getIndexKey(tableName string, index uint64, columnValue []byte) []byte {
	// todo return tablePrefix_indexPrefixSep_tableName_index_columnValue
	return nil
}

func uint64ToBytes(value uint64) []byte {
	return nil
}

func bytesToUint64(bytes []byte) uint64 {
	return 0
}

func bytesToUint64List(bytes []byte) []uint64 {
	return nil
}

func uint64ListToBytes(indexes []uint64) []byte {
	return nil
}
