## How to deploy

### front-end

* make sure server_url is correct in webpack/webpack.prod.js
* run "npm run build" 
* push dist dir to repo
* login to server
* pull code from git
* run "docker-compose down && docker-compose up -d"

### back-end

* login to server
* be sure env file is correct
* run "docker-compose down && docker-compose up -d"
