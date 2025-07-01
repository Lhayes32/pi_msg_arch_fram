FROM ubuntu:22.04

# --- Installs System Dependencies---
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    wget \
    curl \
    python3 python3-pip python3-venv \
    openmpi-bin openmpi-common libopenmpi-dev \
    libzmq3-dev \
    rabbitmq-server \
    default-jdk \
    net-tools iputils-ping \
    # Boost for SimpleAmqpClient
    librabbitmq-dev \
    libboost-all-dev \
    # Json testing
    nlohmann-json3-dev
    # Kafka prequisites
    openjdk-11-jre-headless \
    zookeeperd \
    # For gRPC C++ build
    autoconf automake libtool pkg-config \
    # Editors (because vi is dumb idc what you say)
    vim nano \
    # Google Protobuf
    protobuf-compiler libprotobuf-dev libprotoc-dev \
    # Clean up
    && rm -rf /var/lib/apt/lists/*

# --- Python Messaging & Serialization Packages ---
RUN python3 -m pip install --upgrade pip && \
    pip install \
    grpcio grpcio-tools \
    protobuf \
    pyzmq \
    pika \
    fastavro \
    flatbuffers \
    pycapnp
    
# --- Build and install gRPC C++ ---


# --- Build and install SimpleAmqpClient
RUN git clone https://github.com/alanxz/SimpleAmqpClient.git \
    cd SimpleAmqpClient && mkdir build && cd build \
    cmake .. && make && make install \
    cd ../.. && rm -rf SimpleAmqpClient

# --- Add your workspace ---
WORKDIR /workspace
COPY . /workspace

# --- Expose common ports ---
# gRPC default
EXPOSE 50051  
# RabbitMQ
EXPOSE 5672    
# RabbitMQ mgmt
EXPOSE 15672   
# Kafka 
EXPOSE 9092    
# Zookeeper
EXPOSE 2181    