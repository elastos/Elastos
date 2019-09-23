#### Usage ####
# Build image for master branch:
# docker build -t ela:master .
# Build image for other branch:
# docker build --build-arg CODE_VERSION=release_v0.4.0 \
#   --build-arg MAKE_VERSION=dev -t ela:0.4.0 .
##############
FROM golang:alpine as builder
# Packaging version
ARG MAKE_VERSION=all
ARG CODE_VERSION=master
ARG GIT_URL=https://github.com/elastos/Elastos.ELA.git
# Install system tools
RUN apk update && apk -U upgrade && apk add --no-cache git curl make
# Set environment
RUN mkdir -p $GOPATH/src/github.com/elastos
WORKDIR $GOPATH/src/github.com/elastos
# Clone elastos source code
RUN git clone -b ${CODE_VERSION} --depth 10 ${GIT_URL}
# Build Elastos.ELA Node
RUN cd $GOPATH/src/github.com/elastos/Elastos.ELA && \
 cp -r test/transaction / && make $MAKE_VERSION && \
 mv ela ela-cli /
# Building ela image
FROM alpine:3.10.2
ENV DATADIR=/data CONFFILE=/data/config.json \
  PASSWORD="" WALLETFILE=/data/keystore.dat
# Copy ela binary
COPY --from=builder /ela /ela-cli /
COPY --from=builder /transaction ./
# Expose p2p rpc http port
EXPOSE 20334 20335 20336 20338 20339
VOLUME [${DATADIR}]
# Entrypoint for ela node
CMD [ "/bin/sh", "-c", "/ela --conf=${CONFFILE} --datadir=${DATADIR} -p=${PASSWORD} -w=${WALLETFILE}" ]
