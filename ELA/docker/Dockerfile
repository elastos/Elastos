FROM golang:alpine as builder
# Packaging version
ARG CODE_VERSION=release_v0.4.0
# Install system tools
RUN apk update && apk -U upgrade && apk add --no-cache git curl make
# Set environment
RUN mkdir -p $GOPATH/src/github.com/elastos
WORKDIR $GOPATH/src/github.com/elastos
# Clone elastos source code
RUN git clone -b ${CODE_VERSION} --depth 10 https://github.com/elastos/Elastos.ELA.git
# Install glide and build Elastos.ELA Node
RUN curl https://glide.sh/get | sh && cd $GOPATH/src/github.com/elastos/Elastos.ELA && \
  glide update && glide install && make all && mv ela ela-cli /
# Building ela image
FROM alpine:3.10.2
# Copy ela binary
COPY --from=builder /ela /ela-cli /
# Expose p2p rpc http port
EXPOSE 20334 20335 20336 20338
# Entrypoint for ela node
ENTRYPOINT ["/ela"]
