FROM alpine:3.7

RUN \
  apk add --update go git make gcc musl-dev linux-headers ca-certificates && \
  git clone --depth 1 --branch release/1.8 https://github.com/elastos/Elastos.ELA.SideChain.ETH && \
  (cd Elastos.ELA.SideChain.ETH && make geth) && \
  cp Elastos.ELA.SideChain.ETH/build/bin/geth /geth && \
  apk del go git make gcc musl-dev linux-headers && \
  rm -rf /Elastos.ELA.SideChain.ETH && rm -rf /var/cache/apk/*

EXPOSE 20636
EXPOSE 20638

ENTRYPOINT ["/geth"]
