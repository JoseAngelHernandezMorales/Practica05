SHELL := /bin/bash
CFLAGS := -g -fcommon
CC := gcc

SOURCES := daemon.c log.c util.c lockfile.c socket.c confdata.c daemon-child-func.c
TARGET := daemon

.PHONY: all
all: setup dependencies build deploy test

setup:
	@echo "Preparando entorno..."

dependencies:
	@echo "Instalando dependencias necesarias..."
	sudo apt-get update && sudo apt-get install -y \
		memcached apache2
	sudo install -v -o root -g root -m 0644 memcached /etc/default/memcached
	sudo install -v -o root -g root -m 0644 memcached.conf /etc/memcached.conf
	sudo install -v -o root -g root -m 0644 ports.conf /etc/apache2/ports.conf
	sudo install -v -o root -g root -m 0644 make.conf /etc/apache2/sites-available/make.conf
	sudo mkdir -p /var/run/memcached && sudo chmod 777 /var/run/memcached
	sudo service memcached start
	ps aux | grep memcached || true

build: $(SOURCES)
	@echo "Compilando $(TARGET)..."
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)

deploy:
	@echo "Moviendo archivos de la API..."
	sudo cp api.json /var/www/html/
	sudo cp memcached-stats.sh /var/www/html/

test: deploy
	@echo "Ejecutando pruebas..."
	sudo ./pruebas.sh

.PHONY: clean
clean:
	@echo "Limpiando archivos generados..."
	rm -fv *.o $(TARGET) *~ logfile