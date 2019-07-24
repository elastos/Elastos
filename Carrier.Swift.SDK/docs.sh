#!/bin/sh

FIRST_ARGUMENT="$1"
if [[ $FIRST_ARGUMENT == "objc" ]]
then
appledoc --project-name ElastosCarrier --project-company "elastos.org" --company-id org.elastos --no-create-docset --output ./oc-docs/ .
else
sudo gem install jazzy
jazzy --min-acl internal -o swift-docs
fi

