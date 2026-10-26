import sys
import types

from extype.core import implement_protocol_on_type
from extype.extension_utils import extend_type_with, extension, Protocol


# Each test extends a builtin type that the rest of the test suite (and pytest) doesn't use,
# since extensions on builtin types can't be undone.


def test_mapping_slots_pass_the_key():
    implement_protocol_on_type(types.SimpleNamespace, Protocol.Mapping)

    class MappingExtension:
        @extension
        def __getitem__(self, key):
            return getattr(self, key)

        @extension
        def __setitem__(self, key, value):
            setattr(self, key, value)

        @extension
        def __delitem__(self, key):
            delattr(self, key)

    extend_type_with(types.SimpleNamespace, MappingExtension)

    ns = types.SimpleNamespace()
    ns["a"] = 1
    assert ns.a == 1
    assert ns["a"] == 1
    del ns["a"]
    assert not hasattr(ns, "a")


def make_cell(value):
    # types.CellType only exists since Python 3.8
    return (lambda: value).__closure__[0]


def test_sequence_slots_with_index_arguments():
    cell_type = type(make_cell(None))
    implement_protocol_on_type(cell_type, Protocol.Sequence)

    class SequenceExtension:
        @extension
        def __len__(self):
            return len(self.cell_contents)

        @extension
        def __getitem__(self, index):
            return self.cell_contents[index]

        @extension
        def __setitem__(self, index, value):
            self.cell_contents[index] = value

        @extension
        def __delitem__(self, index):
            del self.cell_contents[index]

        @extension
        def __contains__(self, value):
            # Any truthy value counts, like in Python
            return "yes" if value in self.cell_contents else ""

        @extension
        def __mul__(self, count):
            return self.cell_contents * count

        @extension
        def __imul__(self, count):
            self.cell_contents *= count
            return self

        @extension
        def __iadd__(self, other):
            self.cell_contents += other
            return self

    extend_type_with(cell_type, SequenceExtension)

    cell = make_cell([1, 2, 3])
    assert len(cell) == 3
    assert cell[1] == 2
    cell[1] = 20
    assert cell.cell_contents == [1, 20, 3]
    del cell[1]
    assert cell.cell_contents == [1, 3]
    assert 3 in cell
    assert 2 not in cell
    assert cell * 2 == [1, 3, 1, 3]

    original = cell
    cell *= 2
    assert cell is original
    assert cell.cell_contents == [1, 3, 1, 3]
    cell += [5]
    assert cell is original
    assert cell.cell_contents == [1, 3, 1, 3, 5]


def test_index_slot_is_unary():
    ellipsis_type = type(...)
    implement_protocol_on_type(ellipsis_type, Protocol.Number)

    class IndexExtension:
        @extension
        def __index__(self):
            return 2

    extend_type_with(ellipsis_type, IndexExtension)

    index = ...
    assert [10, 20, 30][index] == 30


def test_hash_of_minus_one_becomes_minus_two():
    class Target:
        pass

    class HashExtension:
        @extension
        def __hash__(self):
            return -1

    extend_type_with(Target, HashExtension)

    assert hash(Target()) == -2


def test_reflected_operator_after_not_implemented():
    class Left:
        pass

    class Right:
        def __radd__(self, other):
            return "radd"

    class AddExtension:
        @extension
        def __add__(self, other):
            return NotImplemented

    extend_type_with(Left, AddExtension)

    assert Left() + Right() == "radd"


def test_slots_do_not_leak_references():
    class Target:
        pass

    class ReprExtension:
        @extension
        def __repr__(self):
            return "target"

    extend_type_with(Target, ReprExtension)

    target = Target()
    before = sys.getrefcount(target)
    for _ in range(1000):
        repr(target)
    assert sys.getrefcount(target) == before


def test_extending_invalidates_attribute_cache():
    class Target:
        def value(self):
            return 1

    target = Target()
    for _ in range(100):
        assert target.value() == 1

    class ValueExtension:
        @extension
        def value(self):
            return 2

    extend_type_with(Target, ValueExtension)

    assert target.value() == 2
