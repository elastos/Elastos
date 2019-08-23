#!/usr/bin/env node

// the command line should ask the following questions to the user
// 1. name of the repo
// 2. option to choose the different npm packages to install (wallet, carrier, DID)

//execute the code of cli here e.g
console.log('Welcome to Elastos Unity cli tool');
var unityLogo =`
                                             
`;
console.log(unityLogo)

// create a new folder with the name proviced by the user
var inquirer = require('inquirer');
inquirer
  .prompt([
        {
            type:"input",
            name:"projectname",
            message:"What is the name of your project?"
        },
        {
            type: 'checkbox',
            message: 'Select Elastos Unity components your project needs',
            name: 'components',
            choices: [
                {
                name: 'Wallet'
                },
                {
                name: 'Carrier'
                },
                {
                name: 'DID'
                }
            ],
        }
    ])
  .then(answers => {console.log("Thanks")});

// create a new react-native app

// install the elastos-unity modules

