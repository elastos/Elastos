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
## How to Run
First, clone the repository into the current directory:
```
git clone https://github.com/cyber-republic/elastos-console.git
cd elastos-console
```
If this is your first time running the django, do the following as the admin user needs to be created. 
If not, you can skip to the next step of executing the automated bash script:
- Create a Python virtual environment:
    ```
    virtualenv -p `which python3` venv
    source venv/bin/activate
    pip3 install -r requirements.txt
    cp .env.example .env
    ```
- Start postgres server
    ``` 
    cd tools
    ./postgres.sh
    cd ..
    ```
    If you're on a MAC and encounter any errors, be sure to use the homebrew version of openssl by doing the following:
    ```
    brew install openssl
    export PATH="/usr/local/opt/openssl/bin:$PATH" # This is a temporary solution and will not work if you switch to another terminal window
    # If you want to make the changes permanent, do the following:
    echo 'export PATH="/usr/local/opt/openssl/bin:$PATH"' >> ~/.bash_profile
    . ~/.bash_profile
    ```
- Create an admin user for django admin interface if not already:
    ```
    // Use "admin" & "admin" as both username/password for ease of use while developing
    python3 manage.py createsuperuser
    python3 manage.py makemigrations
    python3 manage.py migrate
    ```
- Deactivate virtualenv
    ```
    deactivate
    ```
Run the automated script to set up everything and start Django server:
```
./run.sh
```
The console can be viewed on at [http://127.0.0.1:8000](http://127.0.0.1:8000)

You can also use the admin interface at [http://127.0.0.1:8000/admin](http://127.0.0.1:8000/admin)

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