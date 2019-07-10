# GMU Master API Service

An API service that handles API calls between front-end and back-end operations using Flask. Allows both the CLI tool and the Elastos Console to use the API to retreive node properties and other information stored in the database.

``$ virtualenv -p `which python3` venv``

`$ source venv/bin/activate`

`(venv) $ pip install -r requirements.txt`

Now let’s set up the app for development and start it:

``(venv) $ python setup.py develop``

`(venv) $ python master_api_service/app.py`

`env LDFLAGS="-I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib" pip install psycopg2`
