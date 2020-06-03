FROM ubuntu:18.04

RUN apt-get update -y && \
    apt-get install software-properties-common python3.8 python3.8-dev python3-pip -y && \
    add-apt-repository ppa:ethereum/ethereum && \
    apt-get update -y && \
    apt-get install solc -y

RUN pip3 install virtualenv

RUN virtualenv -p /usr/bin/python3.8 /venv
RUN . venv/bin/activate

ENV SRC_DIR /elastos-smartweb-service

# Setting these environment variables are the same as running
# source /env/bin/activate.
ENV VIRTUAL_ENV /venv
ENV PATH /venv/bin:$PATH
ENV PYTHONPATH="${PYTHONPATH}:${SRC_DIR}/grpc_adenine/stubs/"

WORKDIR ${SRC_DIR}

ADD requirements.txt ${SRC_DIR}/requirements.txt
RUN pip install -r requirements.txt

ADD grpc_adenine ${SRC_DIR}/grpc_adenine
ADD .env.example ${SRC_DIR}/.env

EXPOSE 8001

ENTRYPOINT ["python", "./grpc_adenine/server.py"]
