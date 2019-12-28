# The Google Cloud Platform Python runtime is based on Debian Jessie
# You can read more about the runtime at:
#   https://github.com/GoogleCloudPlatform/python-runtime
FROM gcr.io/google_appengine/python

RUN apt-get update -y && \
    apt-get install software-properties-common -y && \
    add-apt-repository ppa:ethereum/ethereum && \
    apt-get update -y && \
    apt-get install solc -y

# Create a virtualenv for dependencies. This isolates these packages from
# system-level packages.
RUN virtualenv -p python3.6 /env

ENV SRC_DIR /elastos-smartweb-service

# Setting these environment variables are the same as running
# source /env/bin/activate.
ENV VIRTUAL_ENV /env
ENV PATH /env/bin:$PATH
ENV PYTHONPATH="${PYTHONPATH}:${SRC_DIR}/grpc_adenine/stubs/"

WORKDIR ${SRC_DIR}

ADD requirements.txt ${SRC_DIR}/requirements.txt
RUN pip install -r requirements.txt

ADD grpc_adenine ${SRC_DIR}/grpc_adenine
ADD .env.example ${SRC_DIR}/.env

EXPOSE 8001

ENTRYPOINT ["python", "./grpc_adenine/server.py"]