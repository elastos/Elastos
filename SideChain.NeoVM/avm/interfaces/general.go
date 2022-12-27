package interfaces

import "io"

type IGeneralInterface interface {
	Serialize(w io.Writer) error
	Deserialize(r io.Reader) error
}