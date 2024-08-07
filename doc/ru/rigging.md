# Риггинг

## Скелеты

Скелеты сущностей создаются через json файлы в папке skeletons.

> [!IMPORTANT]
> 
> Скелет является неиндексируемой единицей контента. При его загрузке к имени добавляется префикс пака (пример: *drop* в паке base -> *base:drop*).

Элемент скелета, или кость, состоит из матрицы транформации, определяющей её положение, вращение и масштаб относительно родительского элемента (кости) или сущности, если элемент является корневым, модели и списка под-элементов.

Файл скелета имеет следующую структуру:
```json
{
    "root": {
        "name": "имя",
        "model": "имя_модели",
        "offset": [x, y, z],
        "nodes": [
           ...
        ]
    }
}
```

- root - корневой элемент
- name - имя элемента для получения индекса (поле опционально)
- model - имя модели для отображения элемента (поле опционально)
- offset - смещение элемента относительно родителя (поле опционально)
- nodes - список элементов - потомков, на которые влияет матрица данного элемента (поле опционально)

На данный момент расположение, вращение, масштабирование выполняется через скриптинг, так же как и анимация.

Процесс работы со скелетами будет упрощен в будущем.

Модели загружаются автоматически, добавление их в preload.json не требуется.

## Модели

Модели должны располагаться в папке models. На данный момент поддерживается только OBJ формат.

>[!IMPORTANT]
> При загрузке obj модели игнорируется файл \*.mtl. 

 Текстура определяется именем материала, соответствующем формату имен текстур, используемому в preload.json. 
 
 Текстуры загружаются автоматически, указывать используемые моделью текстуры в preload.json не обязательно.
