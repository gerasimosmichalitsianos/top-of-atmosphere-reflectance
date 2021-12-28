# use the Ubuntu (debian) operating system
FROM ubuntu:latest

# update software
RUN apt-get update
RUN apt-get install -y apt-utils
RUN apt-get -y install make

# Ensure locales configured correctly
ENV LC_ALL='en_US.utf8'

# install GCC/g++ compilers for C++
RUN apt install g++ -y

# install GDAL
RUN apt-get install gdal-bin -y
RUN apt-get install libgdal-dev -y

# Update C env vars so compiler can find gdal
ENV CPLUS_INCLUDE_PATH=/usr/include/gdal
ENV C_INCLUDE_PATH=/usr/include/gdal

# Add source files root path of docker container
RUN mkdir src/
RUN mkdir libs/
ADD src/ImageUtil.cpp src/
ADD src/ImageUtil.h src/
ADD src/Main.cpp src/
ADD src/Misc.cpp src/
ADD src/Misc.h src/
ADD src/TOAUtil.cpp src/
ADD src/TOAUtil.h src/
ADD libs libs/
ADD makefile /

# compile the code
RUN make
ENTRYPOINT [ "bin/toa" ]
