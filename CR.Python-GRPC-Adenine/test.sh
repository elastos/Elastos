#!/bin/bash

# Run pytest
source venv/bin/activate
pip install -r requirements.txt
py.test -v -rxs elastos_adenine/common_test.py
py.test -v -rxs elastos_adenine/hive_test.py
py.test -v -rxs elastos_adenine/wallet_test.py
py.test -v -rxs elastos_adenine/sidechain_eth_test.py
py.test -v -rxs elastos_adenine/node_rpc_test.py