## HOW TO USE Dockerfile

1. cd to directory with Dockerfile
2. docker build .
3. docker tag IMAGE-ID elastos-dev
4. docker run -tiv YOUR-PROJECT-DIR:/home/elastos/Projects --tmpfs=/tmp elastos-dev /bin/bash

