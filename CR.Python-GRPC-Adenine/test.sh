#!/bin/bash

# Run pytest
source venv/bin/activate
pip install -r requirements.txt
py.test elastos_adenine/common_test.py
py.test elastos_adenine/hive_test.py
py.test elastos_adenine/wallet_test.py
py.test elastos_adenine/sidechain_eth_test.py
py.test elastos_adenine/node_rpc_test.py