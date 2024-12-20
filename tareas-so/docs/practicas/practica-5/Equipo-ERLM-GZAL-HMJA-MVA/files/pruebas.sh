#!/bin/bash

echo "Iniciando el daemon"
./daemon -p 80

DAEMON_ID=$(pidof daemon)

echo "PRUEBA /"
curl -v http://localhost/

echo -e "\n\n"

echo "PRUEBA /version"
curl -v http://localhost/version

echo -e "\n\n"

echo "PRUEBA /estado"
curl -v http://localhost/estado

echo -e "\n\n"

echo "PRUEBA /dia"
curl -v POST http://localhost/dia      -H "Content-Type: application/json"      -d '{ "dia": { "fecha": "1999-12-31", "valores": [ 10, 20, 30, 40, 50, 60, 70, 80, 90, 100 ] } }'

echo "Matando al daemon con id: $DAEMON_ID"
kill -HUP $DAEMON_ID    