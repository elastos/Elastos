#!/bin/bash

# Create a Python virtual environment
virtualenv -p `which python3` venv

# Activate the virtual environment
source venv/bin/activate

# Install all the dependencies
pip3 install -r requirements.txt

# Setup all environment variables
cp .env.example .env

# Run postgres server as a docker container
cd tools
./postgres.sh
cd ..
sleep 5

# Migrate the database
python3 manage.py makemigrations
python3 manage.py migrate
# Collect all static content
echo "yes" | python3 manage.py collectstatic
# Finally, run the Django server
python3 manage.py runserver