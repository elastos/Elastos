from setuptools import setup, find_packages

setup(
    name='smartweb_service',
    version='1.0.0',
    description='RESTful API for  the Elastos NET',
    url='https://github.com/cyber-republic/elastos-smartweb-service',

    keywords='rest restful api flask swagger openapi flask-restplus',

    packages=find_packages(),

    install_requires=['flask-restplus==0.12.1', 'Flask-SQLAlchemy==2.4.0'],
)
