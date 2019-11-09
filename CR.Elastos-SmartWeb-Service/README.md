# GRPC_python
GRPC python implementation with SQL Alchemy

Command to build protocol buffer files:
```
python -m grpc_tools.protoc -I definitions --python_out=stubs --grpc_python_out=stubs definitions/common.proto
```
