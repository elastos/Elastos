# Elastos SmartWeb Service

An API service that handles API calls between front-end and back-end operations using Flask. Allows both the CLI tool and the Elastos Console to use the API to retreive node properties and other information stored in the database.

## Installation

Tested in Python version 3.7.4

For macOS, to install python using homebrew:
```
$brew install python3
```

For Linux based OS, to install python:
```
$apt-get install python3
```

Install virtualenv:
```
$pip3 install virtualenv
```

To get the API service running, run the following terminal commands:

```
$virtualenv -p `which python3` venv
```

```
$source venv/bin/activate
```

```
(venv) $ pip install -r requirements.txt
```

While running the requirements.txt, if you are get a error "Library LSSL not found" while installing psycopg2, then run the following command or else skip this.

```
$brew install openssl
$env LDFLAGS="-I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib" pip install psycopg2
```

Now let’s set up the app for development and start it:

```
(venv) $python setup.py develop
```

```
(venv) $python smartweb_service/app.py
```

## PostgreSQL Setup
Step 1: Install PostgreSQL server 11.3

Step 2: Create a database name 'elastos_console'

Step 3: Start the PostgreSQL Server

```
$pg_ctl -D /usr/local/var/postgres start
```

Step 4: Execute the create table scripts at smartweb_service/scripts/elastos_console_create_scripts.sql

Step 5: Execute the insert scripts at smartweb_service/scripts/elastos_insert_script.sql


## Unit Testing

Requires pytest version 2.9.1. Included as part of requirements.txt

If pytest is not installed. Run the following command:
```
$pip install pytest==2.9.1
```

Step 1:
```
$cd _pytest
```

Step 2:
```
$py.test test_api1.py
```
