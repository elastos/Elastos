Build Wallet and DID Service: 
```bash
$ mvn clean
$ mvn install -Dmaven.test.skip -Dgpg.skip
```

Prepare Staging Environment:
```bash
repo=Elastos.ELA
repo2=ela
docker_file=Dockerfile-ela
binary=ela

repo=Elastos.ELA.Arbiter
repo2=ela-arbiter
docker_file=Dockerfile-ela-arbiter
binary=arbiter

repo=Elastos.ELA.SideChain.ID
repo2=ela-did
docker_file=Dockerfile-did
binary=did

repo=Elastos.ELA.SideChain.Token
repo2=ela-token
docker_file=Dockerfile-token
binary=token

repo=Elastos.ORG.API.Misc
repo2=api-misc
docker_file=Dockerfile-ela-api
binary=misc

rm -rf ~/dev/src/github.com/kpachhai/ela-privnet-staging/$repo2/*; cp -r ~/dev/src/github.com/elastos/$repo/* ~/dev/src/github.com/kpachhai/ela-privnet-staging/$repo2/.
docker build -t test -f $docker_file .
container_id=$(docker run -it -d test)
docker cp $container_id:/go/src/github.com/elastos/$repo/$binary /Users/kpachhai/dev/src/github.com/cyber-republic/elastos-privnet/blockchain/
```