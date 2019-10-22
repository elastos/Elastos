# Elastos Console

The front-end web console that runs on ReactJS and Django.

## Installation

Phython3 and Django are required for this build. 

### If using a Mac

First we clone the repository:

```
$ git clone https://github.com/carlosverastegui/elastos-console.git
```

Next, we create a Python Virtual Environment:

```
cd elastos-console
=======
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
python get-pip.py
```
Now install virtualenv by running the following:
```
$ pip install virtualenv
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
$ pip install Django
```

```
$ virtualenv -p `which python3` venv
```

```
$ source venv/bin/activate
```

```
$ pip install -r requirements.txt
```

Finally, run the server:

```
runserver with Python3 manage.py runserver
```

The console can be viewed on the following IP Address [here]

[here]: http://127.0.0.1:8000/hello

### If using a Windows 

First we clone the repository:

```
$ git clone https://github.com/carlosverastegui/elastos-console.git
```

Next, we create a Python Virtual Environment:

```
cd elastos-console
```
```
$ pip install virtualenv
```
```
$ virtualenv -p C:\ProgramData\Anaconda3/python.exe
 ```
 (Or wherever the directory the python.exe file exists)

 ```
$ C:\ProgramData\Anaconda3\Lib\venv\scripts\nt\activate.bat 
 ```
 (Or wherever the directory containing the activate.bat file exists)

 ```
$ python -m pip install --user --upgrade pip 
```
```
$ pip install --user -r requirements.txt
 ```
 (Warnings may appear here to add sqlformat.exe to path, you can either add it or ignore warnings and proceed)


Finally, run the server:

```
$ python manage.py runserver
```

The console can be viewed on the following IP Address [here]

[here]: http://127.0.0.1:8000/hello


=======
$ pip install -r requirements.txt
```
Finally, run the server:
```
runserver Python3 manage.py runserver
```
The console can be viewed on the following IP Address [here]

[here]: http://127.0.0.1:8000/hello