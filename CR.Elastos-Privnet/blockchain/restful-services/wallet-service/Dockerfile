# Elastos privnet - Wallet Service - elastos.org
# This is an official but unsupported docker image

FROM maven:3.6.0-jdk-8-alpine AS builder

LABEL maintainer="kpachhai"

RUN apk update

# copy folders
COPY service /restful-services/wallet-service

RUN sed -i "s#http://localhost:20334#http://mainchain-node-1:20334#g" /restful-services/wallet-service/base.api/src/main/resources/application.properties
RUN cd /restful-services/wallet-service \
    && mvn clean                        \
    && mvn install -Dmaven.test.skip -Dgpg.skip

FROM maven:3.6.0-jdk-8-alpine

ENV SRC_DIR="/home/elauser"

WORKDIR $SRC_DIR

COPY --from=builder /restful-services/wallet-service/base.api/target/base.api-0.1.0.jar ${SRC_DIR}/base.api-0.1.0.jar

RUN apk update \
    && apk add --no-cache curl \
    && adduser -h $SRC_DIR -D -g '' elauser \
    && chown -R elauser:elauser $SRC_DIR

USER elauser

EXPOSE 8090

ENTRYPOINT ["java", "-jar", "./base.api-0.1.0.jar"]