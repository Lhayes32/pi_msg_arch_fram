FROM ubuntu:22.04

ENV DEBIAN_FRONTEND=noninteractive

# Install basic build tools and dependencies (but NOT Mongo yet)
RUN apt-get update && apt-get install -y \
    wget git build-essential cmake pkg-config \
    libssl-dev libsasl2-dev \
    libprotobuf-dev protobuf-compiler \
    openmpi-bin libopenmpi-dev \
    libmongoc-1.0-0 libmongoc-dev \
    libbson-1.0-0 libbson-dev \
    libgconf-2-4 libgtk-3-0 libnss3 libxss1 libasound2 libxtst6 libatk-bridge2.0-0 libatspi2.0-0 \
    curl gnupg lsb-release python3 python3-pip libboost-all-dev

# Add the MongoDB 6.0 GPG key and repository **before any apt-get update for Mongo**
RUN curl -fsSL https://pgp.mongodb.com/server-6.0.asc | gpg -o /usr/share/keyrings/mongodb-server-6.0.gpg --dearmor


RUN echo "deb [ arch=amd64,arm64 signed-by=/usr/share/keyrings/mongodb-server-6.0.gpg] https://repo.mongodb.org/apt/ubuntu jammy/mongodb-org/6.0 multiverse" | tee /etc/apt/sources.list.d/mongodb-org-6.0.list

RUN apt-get update && apt-get install -y mongodb-org

# C++ driver version
ENV MONGO_CXX_DRIVER_VERSION=3.7.0

WORKDIR /tmp

# Build and install mongo-cxx-driver (bsoncxx and mongocxx)
RUN wget https://github.com/mongodb/mongo-cxx-driver/releases/download/r${MONGO_CXX_DRIVER_VERSION}/mongo-cxx-driver-r${MONGO_CXX_DRIVER_VERSION}.tar.gz && \
    tar -xzf mongo-cxx-driver-r${MONGO_CXX_DRIVER_VERSION}.tar.gz && \
    cd mongo-cxx-driver-r${MONGO_CXX_DRIVER_VERSION} && \
    cd build && \
    cmake .. -DCMAKE_BUILD_TYPE=Release -DCMAKE_INSTALL_PREFIX=/usr/local -DBUILD_SHARED_LIBS=ON && \
    make -j$(nproc) && \
    make install

RUN wget https://downloads.mongodb.com/compass/mongodb-compass_1.40.4_amd64.deb -O /tmp/compass.deb && \
    dpkg -i /tmp/compass.deb || apt-get install -fy && rm /tmp/compass.deb

RUN rm -rf /tmp/*

WORKDIR /app

EXPOSE 27017

CMD ["mongod", "--bind_ip_all"]
