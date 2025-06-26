from collections.abc import Iterable, Generator


def flatten(iterable: Iterable) -> Generator:
    """make
    Генератор flatten принимает итерируемый объект iterable и с помощью обхода в глубину отдает все вложенные объекты.
    Для любых итерируемых вложенных объектов, не являющихся строками, нужно делать рекурсивный заход.
    В результате генератор должен пробегать по всем вложенным объектам на любом уровне вложенности.
    """
    for value in iterable:
        if isinstance(value, Iterable) and not isinstance(value, str):
            yield from flatten(value)
        else:
            yield value
    pass
