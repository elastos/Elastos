import setuptools

with open("README.md", "r") as fh:
    long_description = fh.read()

setuptools.setup(
    name="elastos_adenine",
    version="1.1.0",
    license='MIT',
    author="Kiran Pachhai",
    author_email="kiran@tuum.tech",
    description="gRPC Client library to interact with Elastos Smartweb Service",
    long_description=long_description,
    long_description_content_type="text/markdown",
    url="https://github.com/cyber-republic/python-grpc-adenine",
    keywords=['elastos', 'smartweb-service', 'adenine'],   # Keywords that define your package best
    packages=setuptools.find_packages(),
    classifiers=[
        "Programming Language :: Python :: 3",
        "License :: OSI Approved :: MIT License",
        "Operating System :: OS Independent",
    ],
    python_requires='>=3.6',
)
