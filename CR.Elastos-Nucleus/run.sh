#!/bin/bash

# Create a Python virtual environment
virtualenv -p `which python3` venv

# Activate the virtual environment
source venv/bin/activate

# Install all the dependencies
pip3 install -r requirements.txt

# Run postgres server as a docker container
cd tools
./postgres.sh
cd ..
sleep 7

# Migrate the database
# Collect all static content
sudo rm -rf www/
python3 manage.py collectstatic --no-input

python3 manage.py makemigrations
python3 manage.py migrate

# Finally, run the Django server

python3 manage.py runserver 0.0.0.0:8000
