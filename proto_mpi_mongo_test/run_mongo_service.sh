mkdir -p /data/db

mongod --dbpath /data/db --bind_ip_all > mongo_log.txt