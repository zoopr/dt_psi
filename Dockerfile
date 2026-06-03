FROM ubuntu:24.04

RUN apt-get update 
RUN apt-get install build-essential=12.10ubuntu1 cmake=3.28.3-1build7 ninja-build=1.11.1-2 -y
RUN mkdir /home/ubuntu/dt_psi

ADD ./src /home/ubuntu/dt_psi/src
ADD ./vendor /home/ubuntu/dt_psi/vendor
ADD ./CMakeLists.txt /home/ubuntu/dt_psi/CMakeLists.txt

RUN mkdir /home/ubuntu/dt_psi/build
WORKDIR /home/ubuntu/dt_psi/build

RUN cmake -G Ninja -DCMAKE_BUILD_TYPE=Release ..
RUN ninja

CMD [ "./app" ]
