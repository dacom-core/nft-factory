# PART
### Стек: C/C++, eosio.cdt 1.7.0

# Введение
Контракт предоставляет сервис выпуска, покупки и продажи NFT с возможностью физической поставки. 

# Компиляция
Заменить ABSOLUTE_PATH_TO_CONTRACT на абсолютный путь к директории контракта PART. 
```
docker run --rm --name eosio.cdt_v1.7.0 --volume /ABSOLUTE_PATH_TO_CONTRACT:/project -w /project eostudio/eosio.cdt:v1.7.0 /bin/bash -c "eosio-cpp -abigen -I include -R include -contract part -o part.wasm part.cpp" &
```


# Генерация документации
Требуется Doxygen версии от 1.9.3
```
git submodule update --init --recursive
doxygen
```


# Установка
```
cleos set contract part ABSOLUTE_PATH_TO_CONTRACT -p part
```


# Действующие аккаунты
me - собственное имя контракта;


# Роли

# Сценарии

### Выпуск и продажа NFT

### Покупка NFT

### Доставка NFT

### Перепродажа NFT

### Личные сообщения к заявке

## Документация к методам и таблицам контракта
Документация к методам и таблицам контракта доступна в папке docs/html/index.html


# TODO


