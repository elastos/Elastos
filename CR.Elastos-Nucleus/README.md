# GMU Web Console
The front-end web console for the GMUNet that allows the developer to manage docker containers running sample applications and creates a DID for the developer using a call access key tool.

## Prerequisites
Python3 and Django are required to run this build. First, to install Python3 on MacOS, run the following command line:
```
$ brew install python3
```
In order to install Django you also need pip and virtualenv installed. Normally, pip comes with python3 if you're downloading the latest version (or any version above 3.4). If that is not the case, install pip by running the following:
```
$ curl https://bootstrap.pypa.io/get-pip.py -o get-pip.py
```
```
python3 get-pip.py
```
Now install virtualenv by running the following:
```
$ pip3 install virtualenv
```
## Instructions
First, clone the repository into the current directory:
```
$ git clone https://github.com/cyber-republic/elastos-console.git
```
Next create a Python virtual environment:
```
$ cd elastos-console
```
```
$ virtualenv -p `which python3` venv
```
```
$ source venv/bin/activate
```
Now install Django and other dependencies:
```
$ pip3 install Django
```
```
$ pip3 install -r requirements.txt
```
Finally, run the server:
```
python3 manage.py runserver
```
The console can be viewed on the following IP Address [here]

[here]: http://127.0.0.1:8000
