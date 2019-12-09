# Python-gRPC-client
This repository contains the python client library to interact with [Elastos Smart Web Service](https://github.com/cyber-republic/elastos-smartweb-service).

## Prerequisites
First, install Python3:

```
brew install python3 // On Mac
sudo apt-get install python3 // On Ubuntu
```

Normally, pip comes with python3 if you're downloading the latest version (or any version above 3.4). If that is not the case, install pip by running the following:

```
curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py
python3 get-pip.py
```

Install virtualenv:
```
pip3 install virtualenv
```

## Instructions on how to run from official pypi
Setup virtualenv:
```
virtualenv -p `which python3` venv;
source venv/bin/activate;
```
Install dependencies:
```
pip3 install -r requirements.txt;
```
Setup environment variables:
```
cp .env.example .env;
```
Install elastos-adenine via pip:
```
pip3 install elastos-adenine==0.1.0
```
Run sample.py
```
python3 sample.py
```

## Instructions on how to build and run locally
Clone the repository
```
git clone https://github.com/cyber-republic/python-grpc-adenine.git
cd python-grpc-adenine
```
etup virtualenv:
```
virtualenv -p `which python3` venv;
source venv/bin/activate;
```
Install dependencies:
```
pip3 install -r requirements.txt;
```
Setup environment variables:
```
cp .env.example .env;
```
Run sample.py:
```
python3 sample.py
```
