# GMU Web Console

The front-end web console for the GMUNet that allows the developer to manage docker containers running sample applications and creates a DID for the developer using a call access key tool.

## Prerequisites

Python3 and Django are required to run this build. First, install Python3:

```
brew install python3 // On Mac
sudo apt-get install python3 // On Ubuntu
```

In order to install Django you also need pip and virtualenv installed. Normally, pip comes with python3 if you're downloading the latest version (or any version above 3.4). If that is not the case, install pip by running the following:

```
curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py
```

```
python3 get-pip.py
```

Now install virtualenv by running the following:

```
pip3 install virtualenv
```

If on a Mac, make sure to install openssl
```
brew install openssl
```

## How to Run

First, clone the repository into the current directory:

```
git clone https://github.com/cyber-republic/elastos-nucleus.git
cd elastos-nucleus
```

Copy the environment example file
```
yes | cp -rf .env.example .env
```

Set the following only if you want to test signup/login using Elephant Wallet. Otherwise, it's not needed:
```
# Replace '192.168.1.23' with your own IP address(cannot be localhost)
ALLOWED_HOSTS=.127.0.0.1, .localhost, .www.nucleusconsole.com, .nucleusconsole.com, .192.168.1.23
# Replace '192.168.1.23' with your own IP address(cannot be localhost)
APP_URL=http://192.168.1.23:8000
DEVELOPMENT=False
# Replace the following values with your own SMTP server settings
EMAIL_HOST='smtp.example.com'
EMAIL_HOST_USER='support@example.com'
EMAIL_HOST_PASSWORD='password'
EMAIL_PORT=587
```
Run the automated script to set up everything and start Django server:
```
./run.sh
```

The console can be viewed on at [http://127.0.0.1:8000](http://127.0.0.1:8000)

You can also use the admin interface at [http://127.0.0.1:8000/admin](http://127.0.0.1:8000/admin)

For testing purposes, you can use the following to log in to the admin interface. Note that this only works if you run the server at least once using DEVELOPMENT=True on your .env file:
- email: test@nucleusconsole.com
- password: admin

## Debugging

Interact with your database:

```
# Connect to postgresql database
docker container exec -it nucleus-postgres psql -h localhost -U gmu -d nucleus
# Look at the tables
\dt
# Get all items from the table 'login_didrequest'
select * from login_didrequest;
```

Check all the available url routing currently available by the project

```
python3 manage.py show_urls
```
