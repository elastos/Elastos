FROM ubuntu:xenial

ENV PATH=/usr/lib/go-1.9/bin:$PATH

RUN \
  apt-get update && apt-get upgrade -q -y && \
  apt-get install -y --no-install-recommends golang-1.9 git make gcc libc-dev ca-certificates && \
  git clone --depth 1 https://github.com/elastos/Elastos.ELA.SideChain.ETH && \
  (cd Elastos.ELA.SideChain.ETH && make geth) && \
  cp Elastos.ELA.SideChain.ETH/build/bin/geth /geth && \
  apt-get remove -y golang-1.9 git make gcc libc-dev && apt autoremove -y && apt-get clean && \
  rm -rf /Elastos.ELA.SideChain.ETH

EXPOSE 20636
EXPOSE 20638

ENTRYPOINT ["/geth"]
