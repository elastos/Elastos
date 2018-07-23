# ELA Locking transaction recognition

## UTXO Lock

The UTXO lock is similar to CLTV of Bitcoin (Check lock time Verify), which is a more flexible timing lock that locks one or more output during a transaction.
UTXO Lock adds a field to the output structure of the transaction. The OutputLock and LockTime are used in combination to only lock UTXO, not the transaction.
1. After the sender creates a transaction, UTXO will be locked by setting OutputLock to a certain height in the future. This means that UTXO can be spent only after the height of the block reaches the OutputLock limit.\
2. When the recipient creates a transaction which costs UTXO, which includes OutputLock. The LockTime will need to be set to a higher value than OutputLock to spend the UTXO. This forces the recipient to create a transaction with a transaction lock in order to spend the UTXO locked by the UTXO lock, which limits the time spending the UTXO.

## Verification of the UTXO Lock
1. To determine whether all input referred to UTXO which includes the UTXO LOCK (OutputLock>0). If there are no references, return true;
2. Judge whether the Sequence which referred to the UTXO Lock's input is equal to 0xfffffffe, if not equal, return to false;
3. Determine whether the TimeLock of the transaction is greater than the value of all UTXO OutputLock, if not greater than it, return false;
4. Return true after passing validation.
