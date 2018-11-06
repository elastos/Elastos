package oppayload

import (
	"io"

	"github.com/elastos/Elastos.ELA/core"
)

type DefaultOutput struct {
}

func (o *DefaultOutput) Data() []byte {
	return nil
}

func (o *DefaultOutput) Serialize(w io.Writer) error {
	return nil
}

func (o *DefaultOutput) Deserialize(r io.Reader) error {
	return nil
}

func (o *DefaultOutput) GetType() (core.OutputPayloadType, error) {
	return core.DefaultOutput, nil
}

func (o *DefaultOutput) GetVersion() (byte, error) {
	return 0, nil
}

func (o *DefaultOutput) String() string {
	return ""
}
