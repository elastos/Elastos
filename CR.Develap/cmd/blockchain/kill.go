/*
Copyright © 2019 Cyber Republic

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/
package blockchain

import (
	"fmt"
	"log"
	"strings"

	"github.com/docker/docker/api/types"
	"github.com/docker/docker/client"
	"github.com/spf13/cobra"
	"golang.org/x/net/context"
)

// KillCmd represents the kill command
var KillCmd = &cobra.Command{
	Use:   "kill",
	Short: "Kill blockchain nodes",
	Long:  `Kill blockchain nodes`,
	Run: func(cmd *cobra.Command, args []string) {
		fmt.Printf("blockchain kill called with environment: [%s] and nodes: [%s]\n", Env, Nodes)

		ctx := context.Background()
		cli, err := client.NewClientWithOpts(client.FromEnv, client.WithAPIVersionNegotiation())
		if err != nil {
			log.Fatal(err)
		}

		containers, err := cli.ContainerList(ctx, types.ContainerListOptions{All: true})
		if err != nil {
			log.Fatal(err)
		}

		nodes := strings.Split(strings.Replace(Nodes, " ", "", -1), ",")
		for _, container := range containers {
			for _, containerName := range container.Names {
				if strings.Contains(containerName, "develap") && strings.Contains(containerName, Env) {
					if len(nodes) == 0 {
						fmt.Printf("Stopping container '%v' with ID '%v'...\n", containerName[1:], container.ID[:10])
						if err := cli.ContainerStop(ctx, container.ID, nil); err != nil {
							log.Fatal(err)
						}
						if err := cli.ContainerRemove(ctx, container.ID, types.ContainerRemoveOptions{Force: true}); err != nil {
							log.Fatal(err)
						}
					} else {
						for _, node := range nodes {
							if strings.Contains(containerName, node) {
								fmt.Printf("Stopping container '%v' with ID '%v'...\n", containerName[1:], container.ID[:10])
								if err := cli.ContainerStop(ctx, container.ID, nil); err != nil {
									log.Fatal(err)
								}
								if err := cli.ContainerRemove(ctx, container.ID, types.ContainerRemoveOptions{Force: true}); err != nil {
									log.Fatal(err)
								}
							}
						}
					}
					break
				}
			}
		}

		networks, err := cli.NetworkList(ctx, types.NetworkListOptions{})
		if err != nil {
			log.Fatal(err)
		}
		for _, network := range networks {
			if network.Name == NetworkName {
				fmt.Printf("\nRemoving network '%v' with ID '%v'...\n", network.Name, network.ID)
				_ = cli.NetworkRemove(ctx, network.ID)
			}
		}
	},
}

func init() {
	KillCmd.Flags().StringVarP(&Nodes, "nodes", "n", "", "Nodes to use [mainchain,did,token,eth]")
}
