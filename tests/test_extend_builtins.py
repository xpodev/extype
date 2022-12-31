import builtins_plus


def test_extend_builtins():
    builtins_plus.extend_builtin_types()

    assert hasattr(list, "map")
