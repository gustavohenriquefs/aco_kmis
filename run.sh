#!/bin/bash

cd "$(dirname "$0")/code"

echo "============================================"
echo "  Compilando projeto"
echo "============================================"
echo ""

echo "Compilando projeto..."
g++ -std=c++17 -O3 -fno-inline -o main main.cpp bibliotecas/roaring.c
if [ $? -eq 0 ]; then
    echo "[OK] Compilação bem-sucedida!"
    echo ""
    echo "============================================"
    echo "  Executando programa"
    echo "============================================"
    echo ""
    ./main
    echo ""
    echo "============================================"
    echo "  Programa finalizado"
    echo "============================================"
else
    echo "[ERRO] Falha na compilação do projeto!"
    echo ""
    read -p "Pressione ENTER para continuar..."
fi
