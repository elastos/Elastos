package payload

import "io"

const ActivateArbitratorsVersion byte = 0x00

type UpdateVersion struct {
}

func (a *UpdateVersion) Data(version byte) []byte {
	return nil
}

func (a *UpdateVersion) Serialize(w io.Writer, version byte) error {
	return nil
}

func (a *UpdateVersion) Deserialize(r io.Reader, version byte) error {
	return nil
}
