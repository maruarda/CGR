#!/bin/bash
# ============================================================
# build.sh
#
# Compila e roda o trabalho final, no mesmo estilo do comando
# usado nos trabalhos anteriores (gcc direto, sem CMake).
#
# Uso:
#   ./build.sh          -> compila e roda
#   ./build.sh build    -> só compila
# ============================================================

set -e  # para o script se algum comando falhar

SOURCES="src/main.c src/shader.c src/camera.c src/profile.c src/mesh.c src/sweep.c external/glad/src/glad.c"
INCLUDES="-Iinclude -Iexternal/glad/include -I$HOME/local/include"
LIBS_PATH="-L$HOME/local/lib"
LIBS="-lglfw -lGL -lGLU -lm -ldl"
OUTPUT="trabalhofinal"

echo "==> Compilando..."
gcc $SOURCES $INCLUDES $LIBS_PATH $LIBS -o $OUTPUT

echo "==> Build OK: ./$OUTPUT"

if [ "$1" != "build" ]; then
    echo "==> Executando..."
    ./$OUTPUT
fi
