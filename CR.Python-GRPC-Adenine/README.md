# Python-gRPC-client
This repository contains the python package for Elastos Smart Web Service.

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
## Instructions
Clone the repository
```
git clone https://github.com/cyber-republic/python-grpc-adenine.git
cd python-grpc-adenine
```

To get the API service running, run the following terminal commands:
```
virtualenv -p `which python3` venv
```
```
source venv/bin/activate
```
```
pip3 install -r requirements.txt
```
```
cp .env.example .env
```

Export Path:
```
export PYTHONPATH="$PYTHONPATH:$PWD/adenine/stubs/"
```

Run the client:
```
python3 sample.py
```
