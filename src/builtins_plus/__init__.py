from .core import get_builtin_type_dict


def extend_list():
    tpd = get_builtin_type_dict(list)

    def _map(self, fn):
        """
        Returns a new list transformed by the given function

        :fn: The transformation function
        """

        return list(map(fn, self))

    tpd["map"] = _map


def extend_builtin_types():
    extend_list()
