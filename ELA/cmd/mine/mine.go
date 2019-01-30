package mine

import "github.com/urfave/cli"

func NewCommand() *cli.Command {
	return &cli.Command{
		Name:      "mine",
		Usage:     "Control the solo mining or the discrete mining",
		ArgsUsage: "[args]",
	}
}
