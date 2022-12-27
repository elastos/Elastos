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
	"io"
	"io/ioutil"
	"log"
	"os"
	"path/filepath"
	"strings"

	"github.com/docker/docker/api/types"
	"github.com/docker/docker/api/types/container"
	"github.com/docker/docker/api/types/mount"
	"github.com/docker/docker/api/types/strslice"
	"github.com/docker/docker/client"
	"github.com/docker/go-connections/nat"
	"github.com/otiai10/copy"
	"github.com/spf13/cobra"
	"golang.org/x/net/context"
)

// RunCmd represents the run command
var RunCmd = &cobra.Command{
	Use:   "run",
	Short: "Run blockchain nodes",
	Long:  `Run blockchain nodes`,
	Run: func(cmd *cobra.Command, args []string) {
		fmt.Printf("blockchain run called with environment: [%s] and nodes: [%s]\n", Env, Nodes)

		if !strings.EqualFold(Env, "mainnet") &&
			!strings.EqualFold(Env, "testnet") &&
			!strings.EqualFold(Env, "localnet") {
			log.Fatalf("%s not recognized as a valid net type\n", Env)
		}

		ctx := context.Background()
		cli, err := client.NewClientWithOpts(client.FromEnv, client.WithAPIVersionNegotiation())
		if err != nil {
			log.Fatal(err)
		}

		nodes := strings.Split(strings.Replace(Nodes, " ", "", -1), ",")
		if strings.EqualFold(Env, "localnet") {
			// Create a new network to be used for the localnet
			networkResp, err := cli.NetworkCreate(ctx, NetworkName, types.NetworkCreate{CheckDuplicate: true})
			if err != nil {
				log.Fatal("localnet could not be setup correctly: ", err)
			}
			setupLocalNetDockerContainers(ctx, cli, networkResp)
			fmt.Printf("\nSet up initial localnet successfully\n")
			for _, node := range nodes {
				if node == "mainchain" || node == "did" {
					if resp, err := setupLocalNetNode(ctx, cli, networkResp, node, "node"); err != nil {
						log.Fatal("localnet could not be setup correctly: ", err)
					} else {
						fmt.Printf("\nNetwork: localnet\nContainer Type: %s\nContainer Name: develap-localnet-%s-node\nContainer ID: %v\n", node, node, resp.ID[:10])
					}
				}
			}
		} else {
			for _, node := range nodes {
				if node == "eth" && strings.EqualFold(Env, "mainnet") {
					log.Fatalf("%s not recognized as a valid net type for %s\n", Env, node)
				}
				if node == "mainchain" || node == "did" || node == "token" || node == "eth" {
					if resp, err := runDockerContainer(ctx, cli, node); err != nil {
						log.Print(err)
					} else {
						fmt.Printf("\nNetwork: %s\nContainer Type: %s\nContainer ID: %v\n", Env, node, resp.ID[:10])
					}
				} else {
					log.Fatalf("%s not recognized as a valid node type\n", node)
				}
			}
		}
	},
}

// Setup required nodes for Local Net
func setupLocalNetDockerContainers(ctx context.Context, cli *client.Client, networkResp types.NetworkCreateResponse) {
	var (
		resp container.ContainerCreateCreatedBody
		err  error
	)

	// Setup mainchain nodes
	if resp, err = setupLocalNetNode(ctx, cli, networkResp, "mainchain", "crc-1"); err != nil {
		log.Fatal("localnet could not be setup correctly: ", err)
	} else {
		fmt.Printf("\nNetwork: localnet\nContainer Type: infrastructure-mainchain\nContainer Name: develap-localnet-mainchain-%s\nContainer ID: %v\n", "crc-1", resp.ID[:10])
	}
	if resp, err = setupLocalNetNode(ctx, cli, networkResp, "mainchain", "crc-2"); err != nil {
		log.Fatal("localnet could not be setup correctly: ", err)
	} else {
		fmt.Printf("\nNetwork: localnet\nContainer Type: infrastructure-mainchain\nContainer Name: develap-localnet-mainchain-%s\nContainer ID: %v\n", "crc-2", resp.ID[:10])
	}
	if resp, err = setupLocalNetNode(ctx, cli, networkResp, "mainchain", "origin-1"); err != nil {
		log.Fatal("localnet could not be setup correctly: ", err)
	} else {
		fmt.Printf("\nNetwork: localnet\nContainer Type: infrastructure-mainchain\nContainer Name: develap-localnet-mainchain-%s\nContainer ID: %v\n", "origin-1", resp.ID[:10])
	}
	if resp, err = setupLocalNetNode(ctx, cli, networkResp, "mainchain", "origin-2"); err != nil {
		log.Fatal("localnet could not be setup correctly: ", err)
	} else {
		fmt.Printf("\nNetwork: localnet\nContainer Type: infrastructure-mainchain\nContainer Name: develap-localnet-mainchain-%s\nContainer ID: %v\n", "origin-2", resp.ID[:10])
	}

	// Setup arbitrator nodes
	if resp, err = setupLocalNetNode(ctx, cli, networkResp, "arbitrator", "crc-1"); err != nil {
		log.Fatal("localnet could not be setup correctly: ", err)
	} else {
		fmt.Printf("\nNetwork: localnet\nContainer Type: infrastructure-arbitrator\nContainer Name: develap-localnet-arbitrator-%s\nContainer ID: %v\n", "crc-1", resp.ID[:10])
	}
	if resp, err = setupLocalNetNode(ctx, cli, networkResp, "arbitrator", "crc-2"); err != nil {
		log.Fatal("localnet could not be setup correctly: ", err)
	} else {
		fmt.Printf("\nNetwork: localnet\nContainer Type: infrastructure-arbitrator\nContainer Name: develap-localnet-arbitrator-%s\nContainer ID: %v\n", "crc-2", resp.ID[:10])
	}
	if resp, err = setupLocalNetNode(ctx, cli, networkResp, "arbitrator", "origin-1"); err != nil {
		log.Fatal("localnet could not be setup correctly: ", err)
	} else {
		fmt.Printf("\nNetwork: localnet\nContainer Type: infrastructure-arbitrator\nContainer Name: develap-localnet-arbitrator-%s\nContainer ID: %v\n", "origin-1", resp.ID[:10])
	}
	if resp, err = setupLocalNetNode(ctx, cli, networkResp, "arbitrator", "origin-2"); err != nil {
		log.Fatal("localnet could not be setup correctly: ", err)
	} else {
		fmt.Printf("\nNetwork: localnet\nContainer Type: infrastructure-arbitrator\nContainer Name: develap-localnet-arbitrator-%s\nContainer ID: %v\n", "origin-2", resp.ID[:10])
	}
}

func setupLocalNetNode(ctx context.Context, cli *client.Client, networkResp types.NetworkCreateResponse, chainType, name string) (container.ContainerCreateCreatedBody, error) {
	var (
		resp container.ContainerCreateCreatedBody
		err  error
	)

	dockerContainer, err := getDockerContainer(name, chainType)
	if err != nil {
		return resp, err
	}

	// Pull the image from dockerhub
	out, err := cli.ImagePull(ctx, dockerContainer.ImageName, types.ImagePullOptions{})
	if err != nil {
		return resp, err
	}
	io.Copy(ioutil.Discard, out)
	// Create appropriate data directory that will be mounted between host and container
	// Also, create the mount points in the process
	var mounts = []mount.Mount{}
	for hostPath, volume := range dockerContainer.Volumes {
		// Create mountpoints
		mounts = append(mounts, mount.Mount{
			Type:   mount.TypeBind,
			Source: hostPath,
			Target: volume.ContainerPath,
		})
	}

	// Create the container
	resp, err = cli.ContainerCreate(
		ctx,
		&container.Config{
			Hostname:     dockerContainer.ContainerName,
			Domainname:   dockerContainer.ContainerName,
			Image:        dockerContainer.ImageName,
			ExposedPorts: dockerContainer.ContainerExposedPorts,
			Tty:          true,
			Entrypoint:   dockerContainer.EntryPoint,
			Cmd:          dockerContainer.Cmd,
		},
		&container.HostConfig{
			PortBindings: dockerContainer.HostPortMappings,
			Mounts:       mounts,
		},
		nil,
		dockerContainer.ContainerName,
	)
	if err != nil {
		return resp, err
	}
	// Start the container
	if err = cli.ContainerStart(ctx, resp.ID, types.ContainerStartOptions{}); err != nil {
		return resp, err
	}
	// Connect to the "blockchain" network
	if err = cli.NetworkConnect(ctx, networkResp.ID, resp.ID, nil); err != nil {
		return resp, err
	}

	return resp, nil
}

func getDockerContainer(name, chainType string) (DockerContainer, error) {
	var (
		containerRESTPort, containerRPCPort, hostRESTPort, hostRPCPort nat.Port
		dockerContainer                                                DockerContainer
		err                                                            error
	)

	dockerContainer = DockerContainer{
		ContainerName: fmt.Sprintf("develap-localnet-%s-%s", chainType, name),
		ImageName:     NodeDockerPath[chainType].ImageName,
		Volumes: map[string]DockerContainerDataDir{
			filepath.FromSlash(fmt.Sprintf("%s/data/localnet/%s/%s/%s", CurrentDir, chainType, name, filepath.Base(NodeDockerPath[chainType].DataPath))): DockerContainerDataDir{true, NodeDockerPath[chainType].DataPath},
		},
	}
	containerRESTPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPath[chainType].PortMapping[Env].ContainerRESTPort))
	containerRPCPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPath[chainType].PortMapping[Env].ContainerRPCPort))
	hostRESTPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPath[chainType].PortMapping[Env].HostRESTPort))
	hostRPCPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPath[chainType].PortMapping[Env].HostRPCPort))
	if chainType == "mainchain" {
		dockerContainer.Volumes[filepath.FromSlash(fmt.Sprintf("%s/localnet/%s/%s/config.json", CurrentDir, chainType, name))] = DockerContainerDataDir{false, NodeDockerPath[chainType].ConfigPath}
		dockerContainer.Volumes[filepath.FromSlash(fmt.Sprintf("%s/localnet/%s/%s/keystore.dat", CurrentDir, chainType, name))] = DockerContainerDataDir{false, NodeDockerPath[chainType].KeystorePath}
		if name == "node" {
			dockerContainer.HostPortMappings = nat.PortMap{
				containerRESTPort: []nat.PortBinding{{HostIP: "0.0.0.0", HostPort: hostRESTPort.Port()}},
				containerRPCPort:  []nat.PortBinding{{HostIP: "0.0.0.0", HostPort: hostRPCPort.Port()}},
			}
		}
	} else if chainType == "arbitrator" {
		dockerContainer.Volumes[filepath.FromSlash(fmt.Sprintf("%s/localnet/%s/%s/config.json", CurrentDir, chainType, name))] = DockerContainerDataDir{false, NodeDockerPath[chainType].ConfigPath}
		dockerContainer.Volumes[filepath.FromSlash(fmt.Sprintf("%s/localnet/%s/%s/keystore.dat", CurrentDir, chainType, name))] = DockerContainerDataDir{false, NodeDockerPath[chainType].KeystorePath}
		dockerContainer.Volumes[filepath.FromSlash(fmt.Sprintf("%s/localnet/%s/wait_for_mainchain.sh", CurrentDir, chainType))] = DockerContainerDataDir{false, "/arbiter/wait_for_mainchain.sh"}
		dockerContainer.EntryPoint = strslice.StrSlice{"/bin/sh", "-c", fmt.Sprintf("./wait_for_mainchain.sh develap-localnet-mainchain-%s:%s -- ./arbiter -p 123", name, NodeDockerPath["mainchain"].PortMapping[Env].ContainerRPCPort)}
	} else if chainType == "did" || chainType == "token" {
		dockerContainer.Volumes[filepath.FromSlash(fmt.Sprintf("%s/localnet/%s/%s/config.json", CurrentDir, chainType, name))] = DockerContainerDataDir{false, NodeDockerPath[chainType].ConfigPath}
	}

	// Copy pre-existing blockchain data to the host path that will be mounted to the container
	for hostPath, volume := range dockerContainer.Volumes {
		if volume.HostCreate {
			if err = copy.Copy(fmt.Sprintf("localnet/%s/%s/%s", chainType, name, filepath.Base(NodeDockerPath[chainType].DataPath)), hostPath); err != nil {
				return dockerContainer, err
			}
		}
	}

	return dockerContainer, nil
}

func runDockerContainer(ctx context.Context, cli *client.Client, node string) (container.ContainerCreateCreatedBody, error) {
	var (
		resp container.ContainerCreateCreatedBody
		err  error
	)
	imageName := NodeDockerPath[node].ImageName
	_, err = cli.ImagePull(ctx, imageName, types.ImagePullOptions{})
	if err != nil {
		return resp, err
	}

	var containerRESTPort, containerRPCPort, hostRESTPort, hostRPCPort nat.Port
	if node == "mainchain" {
		containerRESTPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPath[node].PortMapping[Env].ContainerRESTPort))
		containerRPCPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPath[node].PortMapping[Env].ContainerRPCPort))
		hostRESTPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPath[node].PortMapping[Env].HostRESTPort))
		hostRPCPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPath[node].PortMapping[Env].HostRPCPort))
	} else if node == "did" {
		containerRESTPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPath[node].PortMapping[Env].ContainerRESTPort))
		containerRPCPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPath[node].PortMapping[Env].ContainerRPCPort))
		hostRESTPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPath[node].PortMapping[Env].HostRESTPort))
		hostRPCPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPath[node].PortMapping[Env].HostRPCPort))
	} else if node == "token" {
		containerRESTPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPath[node].PortMapping[Env].ContainerRESTPort))
		containerRPCPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPath[node].PortMapping[Env].ContainerRPCPort))
		hostRESTPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPath[node].PortMapping[Env].HostRESTPort))
		hostRPCPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPath[node].PortMapping[Env].HostRPCPort))
	} else if node == "eth" {
		containerRPCPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPath[node].PortMapping[Env].ContainerRPCPort))
		hostRPCPort = nat.Port(fmt.Sprintf("%s/tcp", NodeDockerPath[node].PortMapping[Env].HostRPCPort))
	}

	currentDir, err := os.Getwd()
	if err != nil {
		return resp, err
	}
	volumeData := filepath.FromSlash(fmt.Sprintf("%s/data/%s/%s", currentDir, Env, node))
	os.MkdirAll(volumeData, os.ModePerm)
	mounts := []mount.Mount{
		{
			Type:   mount.TypeBind,
			Source: volumeData,
			Target: NodeDockerPath[node].DataPath,
		},
	}
	containerConfig := &container.Config{
		Image:        imageName,
		ExposedPorts: nat.PortSet{},
	}
	hostConfig := &container.HostConfig{
		PortBindings: nat.PortMap{
			containerRESTPort: []nat.PortBinding{{HostIP: "0.0.0.0", HostPort: hostRESTPort.Port()}},
			containerRPCPort:  []nat.PortBinding{{HostIP: "0.0.0.0", HostPort: hostRPCPort.Port()}},
		},
		Mounts: mounts,
	}
	if node == "eth" {
		if Env == "testnet" {
			containerConfig.Entrypoint = strslice.StrSlice{"/bin/sh"}
			containerConfig.Cmd = strslice.StrSlice{
				"-c", "./geth --testnet --datadir elastos_eth --gcmode 'archive' --rpc --rpcaddr 0.0.0.0 --rpccorsdomain '*' --rpcvhosts '*' --rpcport 20636 --rpcapi 'eth,net,web3' --ws --wsaddr 0.0.0.0 --wsorigins '*' --wsport 20635 --wsapi 'eth,net,web3'",
			}
		}
		containerConfig.ExposedPorts = nat.PortSet{
			containerRPCPort: struct{}{},
		}
		hostConfig.PortBindings = nat.PortMap{
			containerRPCPort: []nat.PortBinding{{HostIP: "0.0.0.0", HostPort: hostRPCPort.Port()}},
		}
	} else {
		if node == "mainchain" {
			mounts = append(mounts,
				mount.Mount{
					Type:   mount.TypeBind,
					Source: filepath.FromSlash(fmt.Sprintf("%s/%s/%s/config.json", currentDir, Env, node)),
					Target: NodeDockerPath[node].ConfigPath,
				},
				mount.Mount{
					Type:   mount.TypeBind,
					Source: filepath.FromSlash(fmt.Sprintf("%s/%s/%s/keystore.dat", currentDir, Env, node)),
					Target: "/ela/keystore.dat",
				},
			)
		} else {
			mounts = append(mounts, mount.Mount{
				Type:   mount.TypeBind,
				Source: filepath.FromSlash(fmt.Sprintf("%s/%s/%s/config.json", currentDir, Env, node)),
				Target: NodeDockerPath[node].ConfigPath,
			})
		}
		containerConfig.ExposedPorts = nat.PortSet{
			containerRESTPort: struct{}{},
			containerRPCPort:  struct{}{},
		}
		hostConfig.PortBindings = nat.PortMap{
			containerRESTPort: []nat.PortBinding{{HostIP: "0.0.0.0", HostPort: hostRESTPort.Port()}},
			containerRPCPort:  []nat.PortBinding{{HostIP: "0.0.0.0", HostPort: hostRPCPort.Port()}},
		}
	}

	containerName := fmt.Sprintf("develap-%s-%s-node", Env, node)

	resp, err = cli.ContainerCreate(
		ctx,
		containerConfig,
		hostConfig,
		nil,
		containerName,
	)
	if err != nil {
		return resp, err
	}
	if err := cli.ContainerStart(ctx, resp.ID, types.ContainerStartOptions{}); err != nil {
		return resp, err
	}
	return resp, nil
}

func init() {
	RunCmd.Flags().StringVarP(&Nodes, "nodes", "n", "", "Nodes to use [mainchain,did,token,eth]")
	RunCmd.MarkFlagRequired("nodes")
}
