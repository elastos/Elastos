package state

// ICRRecord defines necessary operations about CR checkpoint
type ICRRecord interface {
	GetHeightsDesc() ([]uint32, error)
	GetCheckpoint(height uint32) (*CheckPoint, error)
	SaveCheckpoint(point *CheckPoint) error
}
