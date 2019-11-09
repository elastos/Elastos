# GRPC_python
GRPC python implementation with SQL Alchemy

For macOS, to install python using homebrew:
```
$brew install python3
```

For Linux based OS, to install python:
```
$apt-get install python3
```

Install virtualenv:
```
$pip3 install virtualenv
```

Clone the repository
```
git clone https://github.com/cyber-republic/elastos-smartweb-service.git
cd elastos-smartweb-service
```

Export Path:
```
export PYTHONPATH="$PYTHONPATH:/Users/rahulguna/GIT/elastos-smartweb-service/"
```

To get the API service running, run the following terminal commands:
```
$virtualenv -p `which python3` venv
```
```
$source venv/bin/activate
```
```
(venv) $ pip install -r requirements.txt
```
```
$brew install openssl
```
```
$env LDFLAGS="-I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib" pip install psycopg2
```

Start the server:
```
(venv) $python server.py
```

# Additional Info:
Command to build protocol buffer files:
```
python -m grpc_tools.protoc -I definitions --python_out=stubs --grpc_python_out=stubs definitions/common.proto
```

