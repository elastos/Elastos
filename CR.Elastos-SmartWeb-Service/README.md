# GMU Master API Service

An API service that handles API calls between front-end and back-end operations using Flask. Allows both the CLI tool and the Elastos Console to use the API to retreive node properties and other information stored in the database.

## Installation

To get the API service running, run the following terminal commands:

```
$ virtualenv -p `which python3` venv
```

```
$ source venv/bin/activate
```

```
(venv) $ pip install -r requirements.txt
```

While running the requirements.txt, if you are get a error "Library LSSL not found" while installing psycopg2, then run the following command or else skip this.

```
env LDFLAGS="-I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib" pip install psycopg2
```

Now let’s set up the app for development and start it:

```
(venv) $ python setup.py develop
```

```
(venv) $ python master_api_service/app.py
```


