from ..extension_utils import extend_type_with, extension
from .list_ext import ListExtension
from _collections_abc import dict_items, dict_keys, dict_values


__all__ = [
    "extend",
    "DictExtension"
]


class DictExtension(ListExtension):
    @extension
    def first(self):
        try:
            return next(iter(self))
        except StopIteration:
            raise IndexError

    @extension
    def last(self):
        return list(self).last()


def extend():
    extend_type_with(dict_items, DictExtension)
    extend_type_with(dict_keys, DictExtension)
    extend_type_with(dict_values, DictExtension)
