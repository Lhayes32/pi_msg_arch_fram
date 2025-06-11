#!/bin/bash

# ====== Logging functions ======
NoticeLog(){
    message_body="$1"
    echo "[NOTICE] $1"
}

WarnLog(){
    message_body="$1"
    echo "[WARN] $1"
}

ErrorLog(){
    message_body="$1"
    echo "[ERROR] $1"
}

SuccessLog(){
    message_body="$1"
    echo "[SUCCESS] $1"
}

# ====== Check Service Statuses ======
check_status_systemctl(){
    service_name="$1"
    if systemctl is-active --quiet "$service_name"; then
        SuccessLog "$service_name is running."
        return 0
    else
        ErrorLog "$service_name is NOT running."
        return 1
    fi
}

check_status_service() {
    service_name="$1"
    if service "$service_name" status 2>&1 | grep -q "running"; then
        SuccessLog "$service_name is running."
        return 0
    else
        ErrorLog "$service_name is NOT running."
        return 1
    fi
}


# ====== Start Service ======
start_and_check(){
    service_name="$1"
    NoticeLog "Starting $service_name ..."
    NoticeLog "Running: service $service_name start"
    service $service_name start

    if [[ $service_name == "rabbitmq-server" ]]; then
        sleep 4
        rabbitmqctl status >/dev/null 2>&1
        if [ $? -eq 0 ]; then
            SuccessLog "RabbitMQ is running."
        else
            ErrorLog "RabbitMQ is NOT running or failed to start."
        fi
    else
        # Try to check with systemctl, else fallback to service
        if command -v systemctl &> /dev/null; then
            check_status_systemctl "$service_name"
        else
            check_status_service "$service_name"
        fi
    fi
} 

start_rabbitmq() {
    start_and_check "rabbitmq-server"
}

start_zookeeper() {
    start_and_check "zookeeper"
}

start_kafka() {
    /opt/kafka/bin/kafka-server-start.sh -daemon /opt/kafka/config/server.properties

    if ss -ltn | grep -q ':9092'; then
        SuccessLog "Kafka appears to be running on port 9092."
    else
        ErrorLog "Kafka is NOT running or failed to start."
    fi
}

start_rmq_managment() {
    rabbitmqctl status >/dev/null 2>&1
    if [ $? -eq 0 ]; then
        NoticeLog "Starting RabbitMQ Managment Capability"
        rabbitmq-plugins enable rabbitmq_management
        service rabbitmq-server restart
    else
        ErrorLog "RabbitMQ is NOT running and cannot start management console without it. Exitting..."
        exit 1 # Error code 1 is RabbitMQ Error
    fi
}

start_all() {
    start_rabbitmq
    start_zookeeper
    start_kafka
}


if [ $# -eq 0 ]; then
    echo "No services specified. Usage: $0 [rabbitmq] [zookeeper] [kafka] [all]"
    exit 1
fi

# TODO: Make these FLAGS, that way when we run we don't have to keep track of the order in which the user puts the arguements. Also add help dialog.
for arg in "$@"
do
    case $arg in
        # Starts RabbitMQ
        "--rabbitmq")
            start_rabbitmq
            ;;
        # Starts Zookeeper
        "--zookeeper")
            start_zookeeper
            ;;
        # Starts Kafka
        "--kafka") 
            start_kafka
            ;;
        # Starts RabbitMQ Management (requires RabbitMQ to be started)
        "--rmq-man")
            start_rmq_managment
            ;;
        "--all")
            start_all
            ;;
        *)
            echo "Unknown service: $arg"
            ;;
    esac
done