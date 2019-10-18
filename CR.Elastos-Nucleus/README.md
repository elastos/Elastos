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


