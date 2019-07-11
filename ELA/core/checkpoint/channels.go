package checkpoint

type channels struct {
	save chan int
	clean chan int
	replace chan int
}

func (c *channels) messageLoop() {

}
