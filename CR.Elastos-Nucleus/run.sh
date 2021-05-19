#!/bin/bash

# Create a Python virtual environment
virtualenv -p `which python3` venv

# Activate the virtual environment
source venv/bin/activate

# Install all the dependencies
pip install -r requirements.txt

# Run postgres server as a docker container
cd tools
./postgres.sh
cd ..
sleep 7

# Migrate the database
# Collect all static content
python manage.py collectstatic --no-input --clear

python manage.py makemigrations
python manage.py migrate

# Finally, run the Django server
python manage.py runserver 0.0.0.0:8000
