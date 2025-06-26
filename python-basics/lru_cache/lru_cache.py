from collections.abc import Callable
from collections import OrderedDict
from functools import wraps

class Stats:
    def __init__(self, cache_hits: int, cache_misses: int) -> int:
        self.cache_misses = cache_misses
        self.cache_hits = cache_hits

def lru_cache(max_items: int) -> Callable:
    """
    Функция создает декоратор, позволяющий кэшировать результаты выполнения обернутой функции по принципу LRU-кэша.
    Размер LRU кэша ограничен количеством max_items. При попытке сохранить новый результат в кэш, в том случае, когда
    размер кэша уже равен max_size, происходит удаление одного из старых элементов, сохраненных в кэше.
    Удаляется тот элемент, к которому обращались давнее всего.
    Также у обернутой функции должен появиться атрибут stats, в котором лежит объект с атрибутами cache_hits и
    cache_misses, подсчитывающие количество успешных и неуспешных использований кэша.
    :param max_items: максимальный размер кэша.
    :return: декоратор, добавляющий LRU-кэширование для обернутой функции.
    """
    def decorator(func: Callable) -> Callable:
        func.stats = Stats(0, 0)
        cache = OrderedDict()
        @wraps(func)
        def wrapper(*args, **kwargs):
            sub_list = sorted(kwargs.items())
            key = str((tuple(args), tuple(sub_list)))
            if key not in cache:
                func.stats.cache_misses = func.stats.cache_misses + 1
                cache[key] = func(*args, **kwargs)
                if len(cache) > max_items:
                    cache.popitem(last = 0)
            else:
                val = cache[key]
                del cache[key]
                cache[key] = val;
                func.stats.cache_hits = func.stats.cache_hits + 1
            return cache[key]
        return wrapper
    return decorator