#!/bin/bash
# Descobre a pasta onde o script está guardado
SCRIPTPATH="$( cd -- "$(dirname "$0")" >/dev/null 2>&1 ; pwd -P )"
cd "$SCRIPTPATH"

# Executa o Argos
./argos
