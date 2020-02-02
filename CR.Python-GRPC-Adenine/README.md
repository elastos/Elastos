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
Setup environment variables and update variables if needed:
```
cp .env.example .env;
```
Install elastos-adenine via pip:
```
pip3 install elastos-adenine==0.1.9
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
Setup virtualenv:
```
virtualenv -p `which python3` venv;
source venv/bin/activate;
```
Install dependencies:
```
pip3 install -r requirements.txt;
```
Setup environment variables and update variables if needed:
```
cp .env.example .env;
```
Run sample.py:
```
python3 sample.py
```

## How to package up the client library
Setup virtualenv:
```
virtualenv -p `which python3` venv;
source venv/bin/activate;
```
Install dependencies:
```
pip3 install -r requirements.txt;
```
Update setup.py if needed(eg. version number should be modified each time it's pushed to the pypi repo) and get the package ready 
```
rm -rf dist/*;
python3 setup.py sdist bdist_wheel
```
Push to pypi repo:
- For testing purposes, do the following: 
  ```
  python3 -m twine upload --repository-url https://test.pypi.org/legacy/ dist/*
  ```
- For production, do the following:
  ```
  python3 -m twine upload dist/*
  ```

## Unit Testing

Requires pytest version 5.3.5. Included as part of requirements.txt

If pytest is not installed. Run the following command:
```
$pip install pytest==5.3.5
```

To run the pytest:
```
$py.test test/py_test.py
```