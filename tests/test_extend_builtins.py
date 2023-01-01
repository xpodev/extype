import extypes

# `list` is actually the builtin type, but we import it so we get
# proper typing. Note that the static type checker doesn't recognize
# `[]` as an object of type list so it'll not add the extension methods.
from extypes import list


def test_extend_builtins():
    extypes.extend_builtin_types()

    assert hasattr(list, "map")

    assert [1, 2, 3].map((1).__add__) == [2, 3, 4]

    assert [1, 2, 3].reduce(int.__add__) == 6
