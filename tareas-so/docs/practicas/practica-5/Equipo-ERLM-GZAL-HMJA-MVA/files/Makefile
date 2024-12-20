SHELL := /bin/bash
CFLAGS := -g -fcommon
CC := gcc

# Archivos fuente y binario objetivo
SOURCES := daemon.c log.c util.c lockfile.c socket.c confdata.c daemon-child-func.c
TARGET := daemon

# Archivos de configuración y scripts
CONFIG_FILES := memcached memcached.conf ports.conf make.conf
DEPLOY_FILES := api.json memcached-stats.sh
TEST_SCRIPT := pruebas.sh

# Dependencias
DEPENDENCIES := memcached apache2

.PHONY: all setup dependencies build deploy test clean

all: setup dependencies build deploy test

setup:
	@echo "Preparando entorno..."

dependencies:
	@echo "Instalando dependencias necesarias..."
	sudo apt-get update && sudo apt-get install -y $(DEPENDENCIES)
	sudo install -v -o root -g root -m 0644 memcached /etc/default/memcached
	sudo install -v -o root -g root -m 0644 memcached.conf /etc/memcached.conf
	sudo install -v -o root -g root -m 0644 ports.conf /etc/apache2/ports.conf
	sudo install -v -o root -g root -m 0644 make.conf /etc/apache2/sites-available/make.conf
	sudo mkdir -p /var/run/memcached && sudo chmod 777 /var/run/memcached
	sudo service memcached start
	@echo "Verificando estado de memcached..."
	ps aux | grep memcached || echo "Error: memcached no está ejecutándose."

build: $(SOURCES)
	@echo "Compilando $(TARGET)..."
	$(CC) $(CFLAGS) -o $(TARGET) $(SOURCES)
	@echo "Configurando permisos para $(TARGET)..."
	chmod +x $(TARGET)

deploy:
	@echo "Moviendo archivos de la API..."
	@for file in $(DEPLOY_FILES); do \
		if [ -f $$file ]; then \
			sudo cp $$file /var/www/html/; \
		else \
			echo "Advertencia: $$file no encontrado."; \
		fi; \
	done
	@echo "Configurando permisos para $(TEST_SCRIPT)..."
	chmod +x $(TEST_SCRIPT)

test:
	@echo "Ejecutando pruebas..."
	if [ -f ./$(TEST_SCRIPT) ]; then \
		sudo ./$(TEST_SCRIPT); \
	else \
		echo "Error: El archivo $(TEST_SCRIPT) no existe."; \
	fi

clean:
	@echo "Limpiando archivos generados..."
	rm -fv $(TARGET) *~ logfile