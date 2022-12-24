# Getting Started with docker

## - Requirements
- Docker 19 and docker-compose 1.17
- Execute the script root directory /docker/data/download-myaac.sh
- To use global ip change SERVER_IP in the server-ip block near the top of the
docker compose file
- Client pointing to http://<ip>:8080/login.php

### Default Values (docker-compose.yml)
- Ports: 7171, 7172, 80(web), 3306(db), 8080(login)
- Database Server: mysql
- Database Name/User/Password: otserver

### - Commands
To compile and start database, webserver and otserver just run
```
$ docker-compose up -d
```

This can take a little bit, especially during the first run of compose up. This
is because of the healtcheck test configured on the mysql container. This ensures
that the mysql container is ready to recieve connections prior to the other
containers finishing their startup process. Once the ping returns successful,
the remaining containers finish starting up with confidence the DB is available.


If you need to restart the server execute
```
docker-compose restart server

or

docker-compose stop server
docker-compose start server
```

To compile your changes in otserver, just stop and start
```
$ docker-compose up -d --build server
```

To finish access the web server at http://<ip> and follow the prompts.

If it says your ip does not match the ip.txt, edit the ip.txt file outside
the container and refresh the browser, no restarts required there.

### - Observations
- The data folder persist db and webserver data;
