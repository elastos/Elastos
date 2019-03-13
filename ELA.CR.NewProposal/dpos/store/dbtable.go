package store

import (
	"bytes"
	"encoding/binary"
	"errors"
	"fmt"
	"io"
	"math"

	"github.com/elastos/Elastos.ELA/common"
	"github.com/elastos/Elastos.ELA/dpos/log"
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

	DefaultMaxDataSize = 1024 * 1024
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
	err := writeElements(buf, f.Value)
	if err != nil {
		log.Error("value to bytes failed")
		return nil
	}
	return buf.Bytes()
}

// todo change fields to map[string]FieldType, change indexes to ma[uint64]struct{}
type DBTable struct {
	// name of table
	Name string

	// primary key range from 1 to len(table.Fields)
	// if give other value, will use default rowID as primary key only
	PrimaryKey uint64

	// database index range from 1 to len(table.Fields)
	// if give values not in scope, the database index will not be effective
	Indexes []uint64

	// field name of table
	Fields []string
}

func (d *DBTable) Serialize(w io.Writer) error {
	if err := common.WriteVarString(w, d.Name); err != nil {
		return err
	}

	if err := common.WriteUint64(w, d.PrimaryKey); err != nil {
		return err
	}

	if err := common.WriteVarUint(w, uint64(len(d.Indexes))); err != nil {
		return err
	}

	for _, index := range d.Indexes {
		if err := common.WriteUint64(w, index); err != nil {
			return err
		}
	}

	if err := common.WriteVarUint(w, uint64(len(d.Fields))); err != nil {
		return err
	}

	for _, field := range d.Fields {
		if err := common.WriteVarString(w, field); err != nil {
			return err
		}
	}

	return nil
}

func (d *DBTable) Deserialize(r io.Reader) error {
	var err error
	d.Name, err = common.ReadVarString(r)
	if err != nil {
		return err
	}

	d.PrimaryKey, err = common.ReadUint64(r)
	if err != nil {
		return err
	}

	count, err := common.ReadVarUint(r, 0)
	if err != nil {
		return err
	}

	for i := uint64(0); i < count; i++ {
		index, err := common.ReadUint64(r)
		if err != nil {
			return err
		}
		d.Indexes = append(d.Indexes, index)
	}

	count, err = common.ReadVarUint(r, 0)
	if err != nil {
		return err
	}

	for i := uint64(0); i < count; i++ {
		field, err := common.ReadVarString(r)
		if err != nil {
			return err
		}
		d.Fields = append(d.Fields, field)
	}

	return nil
}

func (d *DBTable) Data(fields []*Field) ([]byte, error) {
	fieldsMap := make(map[string]struct{})
	for _, f := range d.Fields {
		fieldsMap[f] = struct{}{}
	}
	for _, f := range fields {
		if _, ok := fieldsMap[f.Name]; !ok {
			return nil, fmt.Errorf("not found field %s in table %s", f.Name, d.Name)
		}
	}

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
	return buf.Bytes(), nil
}

func (d *DBTable) GetFields(data []byte) ([]*Field, error) {
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
	return result, nil
}

// column range from 1 to len(table.Fields), if a field name not found in table will return 0
func (d *DBTable) Column(fieldName string) uint64 {
	for i, f := range d.Fields {
		if f == fieldName {
			return uint64(i) + 1
		}
	}
	return 0
}

func writeElements(buf *bytes.Buffer, element interface{}) error {
	switch e := element.(type) {
	case uint8:
		if err := buf.WriteByte(FieldUint8); err != nil {
			return err
		}
		common.WriteElement(buf, e)
	case uint16:
		if err := buf.WriteByte(FieldUint16); err != nil {
			return err
		}
		common.WriteElement(buf, e)
	case uint32:
		if err := buf.WriteByte(FieldUint32); err != nil {
			return err
		}
		common.WriteElement(buf, e)
	case uint64:
		if err := buf.WriteByte(FieldUint64); err != nil {
			return err
		}
		common.WriteElement(buf, e)
	case common.Uint168:
		if err := buf.WriteByte(FieldUint168); err != nil {
			return err
		}
		if err := e.Serialize(buf); err != nil {
			return err
		}
	case common.Uint256:
		if err := buf.WriteByte(FieldUint256); err != nil {
			return err
		}
		if err := e.Serialize(buf); err != nil {
			return err
		}
	case int:
		if err := buf.WriteByte(FieldInt); err != nil {
			return err
		}
		if err := writeVarInt(buf, e); err != nil {
			return err
		}
	case int32:
		if err := buf.WriteByte(FieldInt32); err != nil {
			return err
		}
		if err := writeVarInt(buf, e); err != nil {
			return err
		}
	case int64:
		if err := buf.WriteByte(FieldIn64); err != nil {
			return err
		}
		if err := writeVarInt(buf, e); err != nil {
			return err
		}
	case []byte:
		if err := buf.WriteByte(FieldBytes); err != nil {
			return err
		}
		common.WriteVarBytes(buf, e)
	case string:
		if err := buf.WriteByte(FieldString); err != nil {
			return err
		}
		common.WriteVarString(buf, e)

	case bool:
		if err := buf.WriteByte(FieldBool); err != nil {
			return err
		}

		if e {
			common.WriteUint8(buf, uint8(1))
		} else {
			common.WriteUint8(buf, uint8(0))
		}
	}
	return nil
}

func writeVarInt(w io.Writer, value interface{}) error {
	var v int64
	switch e := value.(type) {
	case int:
		v = int64(e)
	case int32:
		v = int64(e)
	case int64:
		v = int64(e)
	default:
		return errors.New("invlaid int value")
	}
	if v < 0 {
		if err := common.WriteUint8(w, uint8(0)); err != nil {
			return err
		}
		if err := common.WriteVarUint(w, uint64(-v)); err != nil {
			return err
		}
	} else {
		if err := common.WriteUint8(w, uint8(1)); err != nil {
			return err
		}
		if err := common.WriteVarUint(w, uint64(v)); err != nil {
			return err
		}
	}
	return nil
}

func readElements(r io.Reader) (interface{}, error) {
	fieldType, err := common.ReadBytes(r, 1)
	if err != nil {
		return nil, err
	}
	switch fieldType[0] {
	case FieldUint8:
		var result uint8
		if err := common.ReadElement(r, result); err != nil {
			return nil, errors.New("[readElements] read uint8 failed")
		}
		return result, nil
	case FieldUint16:
		var result uint16
		if err := common.ReadElement(r, result); err != nil {
			return nil, err
		}
		return result, nil
	case FieldUint32:
		var result uint32
		if err := common.ReadElement(r, result); err != nil {
			return nil, err
		}
		return result, nil
	case FieldUint64:
		var result uint64
		if err := common.ReadElement(r, result); err != nil {
			return nil, err
		}
		return result, nil
	case FieldUint168:
		var result common.Uint168
		if err := result.Deserialize(r); err != nil {
			return nil, err
		}
		return result, nil
	case FieldUint256:
		var result common.Uint256
		if err := result.Deserialize(r); err != nil {
			return nil, err
		}
		return result, nil
	case FieldInt:
		result, err := readVarInt(r)
		if err != nil {
			return nil, err
		}
		return int(result), nil
	case FieldInt32:
		result, err := readVarInt(r)
		if err != nil {
			return nil, err
		}
		return int32(result), nil
	case FieldIn64:
		result, err := readVarInt(r)
		if err != nil {
			return nil, err
		}
		return int64(result), nil
	case FieldBytes:
		result, err := common.ReadVarBytes(r, DefaultMaxDataSize, "data")
		if err != nil {
			return nil, err
		}
		return result, nil
	case FieldString:
		result, err := common.ReadVarString(r)
		if err != nil {
			return nil, err
		}
		return result, nil
	case FieldBool:
		value, err := common.ReadUint8(r)
		if err != nil {
			return nil, err
		}
		if value == uint8(1) {
			return true, nil
		}
		return false, nil
	}

	return nil, errors.New("unknown type")
}

func readVarInt(r io.Reader) (int64, error) {
	flag, err := common.ReadUint8(r)
	if err != nil {
		return 0, err
	}
	value, err := common.ReadVarUint(r, 0)
	if err != nil {
		return 0, err
	}
	var neg int64
	if flag == uint8(1) {
		neg = 1
	} else {
		neg = -1
	}
	return int64(value) * neg, nil
}

func GetTableKey(tableName string) []byte {
	// tablePrefix_indexPrefixSep_tableName
	buf := new(bytes.Buffer)
	buf.Write(tablePrefix)
	buf.WriteString("_")
	buf.Write(indexPrefixSep)
	buf.WriteString("_")
	buf.WriteString(tableName)
	return buf.Bytes()
}

func GetTableIDKey(tableName string) []byte {
	// return tablePrefix_indexPrefixSep_idPrefix_tableName
	buf := new(bytes.Buffer)
	buf.Write(tablePrefix)
	buf.WriteString("_")
	buf.Write(indexPrefixSep)
	buf.WriteString("_")
	buf.Write(idPrefix)
	buf.WriteString("_")
	buf.WriteString(tableName)
	return buf.Bytes()
}

func GetRowKey(tableName string, rowID uint64) []byte {
	// return tablePrefix_indexPrefixSep_tableName_rowID
	buf := new(bytes.Buffer)
	buf.Write(tablePrefix)
	buf.WriteString("_")
	buf.Write(indexPrefixSep)
	buf.WriteString("_")
	buf.WriteString(tableName)
	buf.WriteString("_")
	binary.Write(buf, binary.LittleEndian, rowID)
	return buf.Bytes()
}

func GetIndexKey(tableName string, index uint64, columnValue []byte) []byte {
	// return tablePrefix_indexPrefixSep_tableName_index_columnValue
	buf := new(bytes.Buffer)
	buf.Write(tablePrefix)
	buf.WriteString("_")
	buf.Write(indexPrefixSep)
	buf.WriteString("_")
	buf.WriteString(tableName)
	buf.WriteString("_")
	binary.Write(buf, binary.LittleEndian, index)
	buf.WriteString("_")
	buf.Write(columnValue)
	return buf.Bytes()
}

func Uint64ToBytes(value uint64) []byte {
	buf := new(bytes.Buffer)
	common.WriteUint64(buf, value)
	return buf.Bytes()
}

func BytesToUint64(b []byte) uint64 {
	reader := bytes.NewReader(b)
	value, err := common.ReadUint64(reader)
	if err != nil {
		return math.MaxUint64
	}
	return value
}

func BytesToUint64List(b []byte) ([]uint64, error) {
	r := bytes.NewReader(b)
	count, err := common.ReadVarUint(r, 0)
	if err != nil {
		return nil, err
	}

	var result []uint64
	for i := uint64(0); i < count; i++ {
		value, err := common.ReadUint64(r)
		if err != nil {
			return nil, err
		}
		result = append(result, value)
	}
	return result, nil
}

func Uint64ListToBytes(indexes []uint64) ([]byte, error) {
	buf := new(bytes.Buffer)
	if err := common.WriteVarUint(buf, uint64(len(indexes))); err != nil {
		return nil, err
	}

	for _, index := range indexes {
		if err := common.WriteUint64(buf, index); err != nil {
			return nil, err
		}
	}
	return buf.Bytes(), nil
}
