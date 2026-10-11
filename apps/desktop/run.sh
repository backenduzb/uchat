#!bin/bash

set -e

GREEN='\033[1;32m'
BLUE='\033[1;34m'
YELLOW='\033[1;33m'
RED='\033[1;31m'
NC='\033[0m'

echo -e "${BLUE}==> [1/3] CMake konfiguratsiyasi tekshirilmoqda...${NC}"
cmake -S . -B build -G Ninja -DCMAKE_CXX_COMPILER=clang++ -DCMAKE_BUILD_TYPE=Release

echo -e "\n${YELLOW}==> [2/3] Loyoya kompilyatsiya qilinmoqda (Build)...${NC}"
cmake --build build/ -j$(nproc)

echo -e "\n${GREEN}==> [3/3] Dastur ishga tushirilmoqda...${NC}"
echo -e "${NC}----------------------------------------"

./build/uchat

echo -e "\n${GREEN}✔ Muvaffaqiyatli yakunlandi!${NC}"