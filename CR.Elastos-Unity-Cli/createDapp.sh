#!/bin/bash
echo "Creating React Native app..."
npx react-native init $1;
cd $1;
echo "Installing packages..."

# install both Carrier and Wallet
if [ $2 -eq 0 ]
then
    echo "Installing Elastos Unity Wallet and Carrier packages..."
    yarn add react-native-elastos-unity-carrier
    yarn add react-native-elastos-unity-wallet
    # update build.gradle for carrier

else 
    if [ $2 -eq 1 ]
    then
        echo "Installing Elastos Unity Wallet package..."
        yarn add react-native-elastos-unity-wallet
    else
        echo "Installing Elastos Unity Carrier package..."
        yarn add react-native-elastos-unity-carrier;
    fi
fi

# install the pods
cd ios;
pod install;
cd ..
wget https://github.com/cyber-republic/elastos-unity/releases/download/1.0.0/ElastosUnityResource.zip
unzip ElastosUnityResource.zip
mv ElastosUnityResource resources
rm ElastosUnityResource.zip

# grab the resources/ folder with example images and icons

# write some last instructions
echo "DONE"
echo "Your project was successfully created"
echo ""
echo "You can now start using the Elastos services in your app"
echo "To see some examples you can check the following repositories"
echo "https://github.com/cyber-republic/elastos-unity-carrier-demo"
echo "https://github.com/cyber-republic/elastos-unity-carrier-carrier"
echo ""
echo "#########################"
echo ""
echo "NEXT STEPS"
echo "If you are planning to run your app for Android please do as follow"
echo ""
echo " 1 - Update the minSdkVersion to 21"
echo ""
echo " 2 - Update the AndroidManifest.xml and change allowBackup=\"false\" to allowBackup=\"true\" "
echo ""
echo " 3 - for Wallet: add the following code into the android/app/build.gradle file anywhere in the android{} block"
echo "packagingOptions {"
echo "	pickFirst '**/libc++_shared.so'"
echo "}"
echo ""
echo " 4 - for Carrier: add the following code into the android/build.gradle file anywhere in the repositories{} block"
echo "flatDir{"
echo "	dirs \"\$rootDir/../node_modules/react-native-elastos-unity-carrier/android/libs\""
echo "}"
echo ""
echo ""
echo "To get started you can see a complete example at https://github.com/cyber-republic/elastos-unity-cli/blob/master/demo.js"
echo ""
