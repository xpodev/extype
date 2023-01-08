from extypes import *


def test_length_function():
    class Extensible:
        ...

    class Length:
        @extension
        def __len__(self):
            return 5

    extend_type_with(Extensible, Length)

    assert len(Length()) == 5
