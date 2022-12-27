# CyberRepublic

Home of cyberrepublic.org code

## How to run
### Frontend
```shell
npm i
npm start
```

### Backend
```shell
npm i
npm start
```

## Frontend Eslint
We are using airbnb's elint rules plus little customizations which can be found in `.eslintrc` file

To fix the eslint error for legacy code, you can use the command:
```shell
node_modules/.bin/eslint --fix PATH_OF_FOLDER_OR_FILE
```

## Workflow

https://docs.google.com/document/d/12W_iTCFrlxq2THHYRvAJD6S0XZq1CoewtBbv5wiD9G0/edit?usp=sharing

The workflow is made based on the situation that the team is globally distributed around the world and part of the team are working during their spare time.

### Agile
* Use Pivotal Tracker as the collaboration tool
* Every week is an iteration period.
* The estimation of story points is from 0 - 3, simple and everyone can do an easy estimation

### Git branches
Simplified git branches workflow:
* staging branch -> dev/staging server
* master branch -> production server
* staging branch -> New feature branch out from 
* staging branch <- New feature pull request
* master branch <- staging branch

We do not have testing team, Developer and Production Manager will do the testing, so `staging server` is used for both `dev` and `staging`, which will save our man power and maintenance cost.
