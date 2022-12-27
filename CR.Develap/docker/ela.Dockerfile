# Mainchain - elastos.org
# This is an official but unsupported docker image

FROM golang:1.13-alpine3.10 AS builder

LABEL maintainer="kpachhai"

RUN apk update
RUN apk add --no-cache make 
RUN apk add --no-cache git

ENV SRC_DIR /elastos
ENV RELEASE 0.5.0

ADD https://github.com/elastos/Elastos.ELA/archive/v${RELEASE}.tar.gz ${SRC_DIR}/

WORKDIR ${SRC_DIR}

RUN tar -xzvf v${RELEASE}.tar.gz && \
	mv Elastos.ELA-${RELEASE} Elastos.ELA

RUN cd ${SRC_DIR}/Elastos.ELA && \
		make

# alpine3.10
FROM alpine:3.10

ENV SRC_DIR="/ela"

WORKDIR $SRC_DIR

COPY --from=builder /elastos/Elastos.ELA/ela ${SRC_DIR}/ela

RUN apk update \
    && apk add --no-cache curl ca-certificates \
    && addgroup -g 1000 -S elauser \
    && adduser -h $SRC_DIR -u 1000 -S elauser -G elauser \
    && chown -R elauser:elauser $SRC_DIR
    
USER elauser

EXPOSE 20333-20339

ENTRYPOINT ["/bin/sh", "-c", "./ela -p 123"]
