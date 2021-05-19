# ETH Sidechain Oracle - elastos.org
# This is an official but unsupported docker image

FROM node:11.15.0-alpine

ENV SRC_DIR="/oracle"

WORKDIR $SRC_DIR

COPY sidechain.eth.oracle ${SRC_DIR}

RUN apk update \
    && apk add --no-cache curl ca-certificates git python make g++ \
    && npm install web3 express \
    && addgroup -g 1001 -S elauser \
    && adduser -h $SRC_DIR -u 1001 -S elauser -G elauser \
    && chown -R elauser:elauser $SRC_DIR
    
USER elauser

EXPOSE 20632

ENTRYPOINT ["/bin/sh", "-c", "node crosschain_oracle.js"]