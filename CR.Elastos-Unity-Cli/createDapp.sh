#!/bin/bash
echo "Creating React Native app..."
react-native init $1;
cd $1;
echo "Installing packages..."
#if both
if [ $2 -eq 0 ]
then
    echo "Installing Elastos Unity Wallet and Carrier packages..."
    yarn add github:cyber-republic/elastos-unity-wallet;
    yarn add elastos-unity-carrier;
    # npm install --save elastos-unity-carrier;
    # react-native link elastos-unity-carrier;
    # npm install github:cyber-republic/elastos-unity-wallet --save;
    # react-native link elastos-unity-wallet;
else 
    if [ $2 -eq 1 ]
    then
        echo "Installing Elastos Unity Wallet package..."
        yarn add github:cyber-republic/elastos-unity-wallet;
        # react-native link elastos-unity-wallet;
    else
        echo "Installing Elastos Unity Carrier package..."
        yarn add elastos-unity-carrier;
        # react-native link elastos-unity-carrier;
    fi
fi


