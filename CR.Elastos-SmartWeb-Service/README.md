README for MasterAPIService

$ virtualenv -p `which python3` venv
$ source venv/bin/activate
(venv) $ pip install -r requirements.txt
Now let’s set up the app for development and start it:

(venv) $ python setup.py develop
(venv) $ python master_api_service/app.py


env LDFLAGS="-I/usr/local/opt/openssl/include -L/usr/local/opt/openssl/lib" pip install psycopg2