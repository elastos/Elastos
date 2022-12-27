# ELA privnet - elastos.org
# This is an unofficial docker image


######################################
# 1. BUILD THE COMPILE ENV           #
######################################
FROM ubuntu:16.04 as builder

LABEL maintainer="kpachhai"

RUN apt-get clean
RUN apt-get update 
RUN apt-get install -y apt-utils
RUN apt-get install -y build-essential autoconf automake autopoint libtool bison texinfo pkg-config cmake 
RUN apt-get install -y python3.5 python3-sphinx python3-pip 												
RUN apt-get install -y net-tools fakeroot tofrodos flex bison texinfo openssl git gperf make bc zip 		
RUN apt-get install -y gdbserver doxygen graphviz libssl-dev libncurses5-dev 								
RUN apt-get install -y wget curl	

RUN pip3 install breathe
RUN update-alternatives --install /usr/bin/python python /usr/bin/python3 10
RUN update-alternatives --set python /usr/bin/python3													
RUN curl -L -o /tmp/flatcc-0.5.0.tar.gz https://github.com/dvidelabs/flatcc/archive/v0.5.0.tar.gz
RUN cd /tmp && tar xzvf flatcc-0.5.0.tar.gz                                
RUN mkdir -p /tmp/flatcc-0.5.0/build/install                                       
RUN cd /tmp/flatcc-0.5.0/build/install && cmake ../.. -DFLATCC_INSTALL=on          
RUN cd /tmp/flatcc-0.5.0/build/install && make install                             
RUN rm -rf /tmp/*                                                                  
RUN rm -rf /var/lib/apt/list/*

COPY bootstrapd ./ela-carrier/bootstrap

WORKDIR /ela-carrier/bootstrap

RUN rm -rf build/*
RUN mkdir -p build/linux && \
	cd build/linux && \
	cmake -DCMAKE_INSTALL_PREFIX=outputs ../.. && \
	make install 

RUN ls -alh build/linux/outputs/etc/elastos/bootstrapd.conf
RUN ls -alh build/linux/outputs/usr/bin/ela-bootstrapd

######################################
# 2. START FRESH - for smaller image #
######################################
FROM ubuntu:16.04

ENV SRC_DIR="/home/elauser"

WORKDIR $SRC_DIR

COPY --from=builder /ela-carrier/bootstrap/build/linux/outputs/usr/bin/ela-bootstrapd $SRC_DIR/ela-bootstrapd
COPY --from=builder /ela-carrier/bootstrap/build/linux/outputs/etc/elastos/bootstrapd.conf $SRC_DIR/bootstrapd.conf

RUN mkdir -p $SRC_DIR/db

RUN useradd -d $SRC_DIR elauser && \
    chown -R elauser:elauser $SRC_DIR

USER elauser

EXPOSE 33445

ENTRYPOINT ["/bin/sh", "-c", "./ela-bootstrapd --config=./bootstrapd.conf --foreground"]