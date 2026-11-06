# Premises Control

Система контролю доступу до приміщень.

## Залежності

- SQLite3
- OpenSSL

## Збірка

```bash
cmake -B cmake-build-debug -DCMAKE_BUILD_TYPE=Debug
cmake --build cmake-build-debug
```

## Налаштування

```bash
cp .env.sample .env
```

## Запуск

```bash
env $(cat .env | xargs) ./cmake-build-debug/premises_control
```
