# Premises Control

Система контролю доступу до приміщень.

## Залежності

- SQLite3
- OpenSSL
- FTXUI

## Збірка та запуск

```bash
# Зконфігурувати та зібрати проєкт
cmake -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug

# Зв'язати compile_commands.json для підтримки LSP (Zed / Clangd)
ln -sf cmake-build-debug/compile_commands.json compile_commands.json

# Налаштувати оточення та запустити
cp .env.sample .env
env $(cat .env | xargs) ./cmake-build-debug/premises_control
