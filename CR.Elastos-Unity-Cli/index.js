#!/usr/bin/env node

// the command line should ask the following questions to the user
// 1. name of the repo
// 2. option to choose the different npm packages to install (wallet, carrier, DID)

//execute the code of cli here e.g
console.log('Welcome to Elastos Unity cli tool');
var unityLogo =``;
console.log(unityLogo)
const { exec } = require('child_process');

// create a new folder with the name proviced by the user
var inquirer = require('inquirer');
inquirer
  .prompt([
        {
            type:"input",
            name:"projectname",
            message:"How would you like to name your project?"
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
                }
            ],
        }
    ])
    .then(answers => {
        console.log("Creating Elastos Dapp...")
        // console.log(answers)

        // console.log(answers.components.length)
        // install wallet and carrier packages
        if (answers.components.length == 2){
            const myShellScript = exec('sh '+__dirname+'/createDapp.sh '+answers.projectname+" 0");
            myShellScript.stdout.on('data', (data)=>{
                console.log(data); 
                // do whatever you want here with data
            });
            myShellScript.stderr.on('data', (data)=>{
                console.error(data);
            });
        }
        else {
            // only install wallet package
            if (answers.components.indexOf("Wallet") > -1){
                const myShellScript = exec('sh '+__dirname+'/createDapp.sh '+answers.projectname+" 1");
                myShellScript.stdout.on('data', (data)=>{
                    console.log(data); 
                    // do whatever you want here with data
                });
                myShellScript.stderr.on('data', (data)=>{
                    console.error(data);
                });
            }
            // only install carrier package
            else {
                const myShellScript = exec('sh '+__dirname+'/createDapp.sh '+answers.projectname+" 2");
                myShellScript.stdout.on('data', (data)=>{
                    console.log(data); 
                    // do whatever you want here with data
                });
                myShellScript.stderr.on('data', (data)=>{
                    console.error(data);
                });
            }
        }        
    });

// install the elastos-unity modules

