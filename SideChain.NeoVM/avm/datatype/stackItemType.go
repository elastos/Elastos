package datatype

type StackItemType byte

const (
	TYPE_ByteArray        StackItemType = iota
	TYPE_Boolean          StackItemType = 0x01
	TYPE_Integer          StackItemType = 0x02
	TYPE_InteropInterface StackItemType = 0x40
	TYPE_Array            StackItemType = 0x80
	TYPE_Struct           StackItemType = 0x81
	TYPE_Map              StackItemType = 0x82
)
