# Wallet Service - elastos.org
# This is an official but unsupported docker image

FROM maven:3.6.0-jdk-8-alpine AS builder

LABEL maintainer="kpachhai"

RUN apk update

# copy folders
COPY service.wallet /restful-services/wallet-service

RUN cd /restful-services/wallet-service \
    && mvn clean                        \
    && mvn install -Dmaven.test.skip -Dgpg.skip

# Maven 3.6.0
FROM maven:3.6.0-jdk-8-alpine

ENV SRC_DIR="/home/elauser"

WORKDIR $SRC_DIR

COPY --from=builder /restful-services/wallet-service/base.api/target/base.api-0.1.0.jar ${SRC_DIR}/base.api-0.1.0.jar

RUN apk update \
    && apk add --no-cache curl ca-certificates \
    && addgroup -g 1000 -S elauser \
    && adduser -h $SRC_DIR -u 1000 -S elauser -G elauser \
    && chown -R elauser:elauser $SRC_DIR

USER elauser

EXPOSE 8090

ENTRYPOINT ["java", "-Dspring.config.location=./application.properties", "-jar", "./base.api-0.1.0.jar"]