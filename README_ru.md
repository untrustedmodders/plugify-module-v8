[![English](https://img.shields.io/badge/English-%F0%9F%87%AC%F0%9F%87%A7-blue?style=for-the-badge)](README.md)

# Модуль языка JavaScript (V8) для Plugify

Модуль языка JavaScript для Plugify позволяет разработчикам писать плагины на современном JavaScript (ECMAScript 2023) и запускать их с помощью высокопроизводительного [движка V8](https://v8.dev/). С этим модулем вы получаете гибкость JavaScript при полной интеграции в экосистему плагинов Plugify.

## Возможности

- **Плагины на JavaScript**: Пишите плагины полностью на JavaScript, используя привычный синтаксис и инструменты.
- **Бесшовная интеграция**: Плавно интегрируйте JavaScript-плагины в систему Plugify вместе с плагинами на других поддерживаемых языках.
- **Межъязыковое взаимодействие**: Легко взаимодействуйте с плагинами, написанными на C++, Python, Lua и других языках.
- **Изолированное выполнение**: Безопасно выполняйте JavaScript-код в изолированной (sandbox) среде V8 с контролируемым доступом к нативным API.

## Начало работы

### Требования

- Рекомендуется JavaScript `ES6`.
- Установлен фреймворк Plugify.

### Установка

#### Вариант 1: Установка через менеджер плагинов Plugify

```bash
mamba install -n your_env_name -c https://untrustedmodders.github.io/plugify-module-v8/ plugify-module-v8
```

#### Вариант 2: Ручная установка

1. Установите зависимости:  

   a. Windows  
   > Настройка [инструментов CMake через Visual Studio Installer](https://learn.microsoft.com/en-us/cpp/build/cmake-projects-in-visual-studio#installation)

   b. Linux:  
   ```sh
   sudo apt-get install -y build-essential cmake ninja-build
   ```
   
   c. Mac:  
   ```sh
   brew install cmake ninja
   ```

2. Клонируйте репозиторий:

   ```bash
   git clone https://github.com/untrustedmodders/plugify-module-v8.git --recursive
   ```

3. Соберите модуль языка JavaScript:

   ```bash
   mkdir build && cd build
   cmake ..
   cmake --build .
   ```

### Использование

1. **Интеграция с Plugify**

   Убедитесь, что модуль языка JavaScript доступен в той же директории, что и ваша установка Plugify.

2. **Пишите плагины на JavaScript**

   Разрабатывайте плагины на JavaScript, используя API Plugify V8. Подробные инструкции смотрите в [Plugify JavaScript Plugin Guide](https://untrustedmodders.github.io/languages/javascript/first-plugin).

3. **Сборка и установка плагинов**

   Поместите ваши JavaScript-файлы в директорию, доступную для ядра Plugify.

4. **Запуск Plugify**

   Запустите фреймворк Plugify — он динамически загрузит ваши JavaScript-плагины.

## Пример

```javascript
import { Plugin } from 'plugify';

export class ExamplePlugin extends Plugin {
    pluginStart() {
        console.log('ExamplePlugin::pluginStart');
    }
    pluginUpdate(dt) {
        console.log('ExamplePlugin::pluginUpdate');
    }
    pluginEnd() {
        console.log('ExamplePlugin::pluginEnd');
    }
}
```

## Справочник API

- [`assert`](src/builtin/assert.rs) Предоставляет набор функций для проверок (assertions)
    - `assert.ok()` Проверяет, что значение истинно.
    - `assert.equal()` Проверяет, что два значения равны, используя оператор равенства (==).
    - `assert.notEqual()` Проверяет, что два значения не равны, используя оператор неравенства (!=).
- [`buffer`](src/builtin/buffer.rs) Работа с бинарными данными
  - TODO
- [`console`](src/builtin/buffer.rs) Объект консоли.
    - `console.log()` Выводит сообщение в консоль.
    - `console.info()` Выводит информационное сообщение в консоль.
    - `console.warn()` Выводит предупреждение в консоль.
    - `console.error()` Выводит ошибку в консоль.
    - `console.debug()` Выводит отладочное сообщение в консоль.
- [`child_process`](src/builtin/child_process.rs) Для запуска дочерних процессов
  - TODO
- [`cluster`](src/builtin/cluster.rs) Для разбиения одного процесса Node на несколько процессов
  - TODO
- [`crypto`](src/builtin/crypto.rs) Для работы с криптографическими функциями OpenSSL
  - TODO
- [`dgram`](src/builtin/dgram.rs) Реализация UDP-датаграммных сокетов
  - TODO
- [`dns`](src/builtin/dns.rs) Для DNS-запросов и функций разрешения имён
  - TODO
- [`events`](src/builtin/events.rs) Для обработки событий
  - TODO
- [`fs`](src/builtin/fs.rs) Для работы с файловой системой
    - `fs.rename()` переименовать файл.
    - `fs.exists()` проверить существование.
    - `fs.rmdir()` удалить директорию.
    - `fs.mkdir()` создать директорию.
    - `fs.stat()` получить статус по указанному пути.
    - `fs.readdir()` прочитать содержимое директории.
    - `fs.readFile()` прочитать файл по указанному пути.
    - `fs.writeFile()` записать в файл по указанному пути (байты/строку).
    - `fs.appendFile()` добавить в конец файла (байты/строку).
- [`http`](src/builtin/http.rs) Для работы Node.js в качестве HTTP-сервера
  - TODO
- [`https`](src/builtin/https.rs) Для работы Node.js в качестве HTTPS-сервера
  - TODO
- [`net`](src/builtin/net.rs) Для создания серверов и клиентов
  - TODO
- [`os`](src/builtin/os.rs) Предоставляет информацию об операционной системе
    - `os.tmpdir()` Возвращает директорию по умолчанию для временных файлов.
    - `os.homedir()` Возвращает домашнюю директорию пользователя.
    - `os.endianness()` Возвращает порядок байтов (endianness) CPU.
    - `os.type()` Возвращает имя операционной системы.
    - `os.platform()` Возвращает информацию о платформе ОС.
    - `os.arch()` Возвращает архитектуру CPU.
- [`path`](src/builtin/path.rs) Для работы с файловыми путями
    - `path.normalize()` нормализует строку пути.
    - `path.join()` Объединяет указанные пути в один.
    - `path.resolve()` Преобразует указанные пути в абсолютный путь.
    - `path.isAbsolute()` Проверяет, является ли путь абсолютным.
    - `path.relative()` Возвращает относительный путь между двумя путями.
    - `path.dirname()` Возвращает имя директории пути.
    - `path.basename()` Возвращает базовое имя пути.
    - `path.extname()` Возвращает расширение файла.
- [`querystring`](src/builtin/querystring.rs) Для обработки строк запросов URL
  - TODO
- [`readlin`](src/builtin/readlin.rs) Для обработки читаемых потоков по одной строке
  - TODO
- [`stream`](src/builtin/stream.rs) Для работы со стримами данных
  - TODO
- [`string_decoder`](src/builtin/string_decoder.rs) Для декодирования объектов Buffer в строки
  - TODO
- [`timers`](src/builtin/timers.rs) Для выполнения функции через заданное количество миллисекунд
    - `timers.setTimeout()` Выполняет функцию через указанное время (в миллисекундах).
    - `timers.clearTimeout()` Отменяет объект таймаута.
- [`tls`](src/builtin/tls.rs) Для реализации протоколов TLS и SSL
  - TODO
- [`tty`](src/builtin/tty.rs) Классы, используемые текстовым терминалом
  - TODO
- [`url`](src/builtin/url.rs) Для парсинга URL-строк
  - TODO
- [`util`](src/builtin/util.rs) Утилитные функции
  - TODO
- [`v8`](src/builtin/v8.rs) Для доступа к информации о V8 (движке JavaScript)
  - TODO
- [`vm`](src/builtin/vm.rs) Для компиляции JavaScript-кода в виртуальной машине
  - TODO
- [`zlib`](src/builtin/zlib.rs) Для сжатия или распаковки файлов
  - TODO

## Документация

Для подробной документации по написанию плагинов на JavaScript с использованием фреймворка Plugify см. [Plugify Documentation](https://untrustedmodders.github.io).

## Как внести вклад

Не стесняйтесь вносить вклад, открывая issues или отправляя pull request'ы. Мы приветствуем ваши отзывы и идеи!

## Лицензия

Этот модуль языка JavaScript (V8) для Plugify распространяется под лицензией [MIT License](LICENSE).
