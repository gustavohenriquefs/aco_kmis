@echo off
cd /d "%~dp0\code"

echo ============================================
echo   Compilando projeto
echo ============================================
echo.

echo Compilando projeto...
g++ -std=c++17 -O3 -fno-inline -o main.exe main.cpp bibliotecas/roaring.c -lpsapi
if %ERRORLEVEL% equ 0 (
    echo [OK] Compilação bem-sucedida!
    echo.
    echo ============================================
    echo   Executando programa
    echo ============================================
    echo.
    main.exe
    echo.
    echo ============================================
    echo   Programa finalizado
    echo ============================================
) else (
    echo [ERRO] Falha na compilação do projeto!
    echo.
    pause
)