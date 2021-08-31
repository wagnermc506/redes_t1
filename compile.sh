#!/usr/bin/bash

echo "Compiling link_layer_server.c"
gcc utils.c link_layer_server.c -o server -lrt

echo "Compiling link_layer_client.c"
gcc utils.c link_layer_client.c -o client -lrt

echo "Compiling server_app.c"
gcc utils.c server_app.c -o server_app -lrt

echo "Compiling client_app.c"
gcc utils.c client_app.c -o client_app -lrt

