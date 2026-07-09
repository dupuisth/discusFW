#!/bin/bash

# ESP-MATTER setup
# cd /opt/esp-matter
# ./install.sh

cd /workspaces/discus
idf.py fullclean
idf.py set-target esp32h2
idf.py reconfigure
idf.py build