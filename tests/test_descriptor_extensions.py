from extypes import extension, extend_type_with


def test_extends_descriptors():
    class ListExtensions(list):
        @extension
        @property
        def length(self):
            return len(self)

    extend_type_with(list, ListExtensions)

    test_list =  [1, 2, 3]
    assert test_list.length == 3
    test_list.append(4)
    assert test_list.length == 4
