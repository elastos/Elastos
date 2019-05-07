# Elastos privnet - DID Sidechain - elastos.org
# This is an official but unsupported docker image

# alpine3.8 - use image digest for security
FROM alpine@sha256:dad671370a148e9d9573e3e10a9f8cc26ce937bea78f3da80b570c2442364406

ENV SRC_DIR="/root/did"

WORKDIR $SRC_DIR

COPY . .

RUN apk update \
    && apk add --no-cache curl 

EXPOSE 10231-10235

ENTRYPOINT ["/bin/sh", "-c", "./did"]