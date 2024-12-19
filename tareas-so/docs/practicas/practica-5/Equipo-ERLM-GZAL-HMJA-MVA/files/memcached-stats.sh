#!/usr/bin/env bash

# Path to your Unix socket file
MEMCACHED_SOCKET="/var/run/memcached/memcached.sock"

# Check if the socket file exists and is accessible
if [[ ! -S "$MEMCACHED_SOCKET" ]]; then
  echo "ERROR: Socket file not found or not accessible" >&2
  exit 1
fi

# Retrieve the server stats over the Unix socket
SERVER_STATS=$(
  printf "stats\r\nquit\r\n" | \
  nc -U "$MEMCACHED_SOCKET" | \
  tr -d '\r' | \
  head
) 2>/dev/null

SERVER_STATS_JSON=$(
  echo "${SERVER_STATS}" | \
  awk -v q='"' -v c=',' '{if($1=="STAT"){print "  " q $2 q ": " q $3 q c}}' | \
  sed -e '$ s/.$//'
) 2>/dev/null

SERVER_STATS_JSON=$(printf "{\n%s\n}\n" "${SERVER_STATS_JSON}") 2>/dev/null

echo "${SERVER_STATS_JSON}"
