package msg

import "io"

// empty represent a message without payload.
type empty struct{}

func (msg *empty) MaxLength() uint32           { return 0 }
func (msg *empty) Serialize(io.Writer) error   { return nil }
func (msg *empty) Deserialize(io.Reader) error { return nil }
