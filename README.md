# Premises Control

Термінал контролю доступу до приміщення (картка + PIN).

## Залежності

- SQLite3 (FetchContent, amalgamation)
- FTXUI (FetchContent)
- C++20 / CMake ≥ 3.20

## Збірка та запуск

```bash
cmake -B cmake-build-debug
cmake --build cmake-build-debug --config Debug

copy .env.sample .env
cmake-build-debug\Debug\premises_control.exe
```

Автоперевірка сценаріїв плану T1:

```bash
cmake --build cmake-build-debug --config Debug --target verify_plan
cmake-build-debug\Debug\verify_plan.exe
```

## Тестові дані (перший запуск)

| Роль  | Картка             | PIN  | Примітка |
|-------|--------------------|------|----------|
| ADMIN | `0000000000000001` | 0000 | вхід у режим адміністратора |
| USER  | `1234567890123456` | 1111 | Пн–Пт 09:00–18:00 |
| USER  | `9876543210987654` | 2222 | без розкладу (доступ заборонено) |

Стан зберігається в `app.db`. Щоб пересоздати seed — видаліть `app.db`.
