# NFT
### Стек: C/C++, eosio.cdt 1.7.0

# Введение
Контракт предоставляет сервис выпуска, покупки и продажи NFT с возможностью физической поставки. 

# Компиляция
Заменить ABSOLUTE_PATH_TO_CONTRACT на абсолютный путь к директории контракта NFT. 
```
docker run --rm --name eosio.cdt_v1.7.0 --volume /ABSOLUTE_PATH_TO_CONTRACT:/project -w /project eostudio/eosio.cdt:v1.7.0 /bin/bash -c "eosio-cpp -abigen -I include -R include -contract nft -o nft.wasm nft.cpp" &
```


# Генерация документации
Требуется Doxygen версии от 1.9.3
```
git submodule update --init --recursive
doxygen
```


# Установка
```
cleos set contract nft ABSOLUTE_PATH_TO_CONTRACT -p nft
```


# Действующие аккаунты
me - собственное имя контракта;


# Роли

# Сценарии

### Выпуск и продажа NFT

### Покупка NFT

### Сообщения в NFT

### Сообщения в заявке на покупку

### Редактирование NFT
Отредактировать возможно только те NFT, которые ещё небыли проданы. 

### Удаление NFT
Удалить возможно только те NFT, которые ещё небыли проданы. 

### Доставка NFT

### Торги за NFT

### Аукцион NFT

### Перепродажа NFT

## Документация к методам и таблицам контракта
Документация к методам и таблицам контракта доступна в папке docs/html/index.html


# TODO


