import pytest
from extype.builtin_extensions import extend_all  # noqa: F401


# dict keys extension tests


def test_dict_keys_all():
    assert {True: None, "String": None, 10: None}.keys().all()
    assert not {True: None, False: None, 3: None}.keys().all()
    assert not {True: None, 10: None, "": None}.keys().all()

    assert {}.keys().all()
    assert not {False: None}.keys().all()


def test_dict_keys_any():
    assert {True: None, False: None, 0: None}.keys().any()
    assert {False: None, 0: None, True: None}.keys().any()

    assert not {"": None, 0: None, False: None, (): None}.keys().any()

    assert not {}.keys().any()


def test_dict_keys_map():
    assert {1: None, 2: None, 3: None}.keys().map((3).__add__) == [4, 5, 6]

    assert {1: None, 2: None, 3: None}.keys().map(str) == ["1", "2", "3"]


def test_dict_keys_reduce():
    assert {1: None, 2: None, 3: None}.keys().reduce(int.__add__) == 6

    assert {1: None, 2: None, 3: None}.keys().reduce(int.__add__, 5) == 6 + 5


def test_dict_keys_filter():
    assert {1: None, 2: None, 3: None, 4: None, 5: None,
            6: None}.keys().filter(lambda x: x % 2 == 0) == [2, 4, 6]


def test_dict_keys_first():
    with pytest.raises(IndexError):
        {}.keys().first()

    assert {1: None}.keys().first() == 1
    assert {1: None, 2: None, 3: None}.keys().first() == 1


def test_dict_keys_last():
    with pytest.raises(IndexError):
        {}.keys().last()

    assert {1: None}.keys().last() == 1
    assert {1: None, 2: None, 3: None}.keys().last() == 3


###################################################


# dict values extension tests


def test_dict_values_all():
    assert {0: True, 1: "String", 2: 10}.values().all()
    assert not {0: True, 1: False, 2: 3}.values().all()
    assert not {0: True, 1: 10, 2: ""}.values().all()

    assert {}.values().all()
    assert not {0: False}.values().all()


def test_dict_values_any():
    assert {0: True, 1: False, 2: 0}.values().any()
    assert {0: False, 1: 0, 2: True}.values().any()

    assert not {0: "", 1: 0, 2: False, 3: ()}.values().any()

    assert not {}.values().any()


def test_dict_values_map():
    assert {0: 1, 1: 2, 2: 3}.values().map((3).__add__) == [4, 5, 6]

    assert {0: 1, 1: 2, 2: 3}.values().map(str) == ["1", "2", "3"]


def test_dict_values_reduce():
    assert {0: 1, 1: 2, 2: 3}.values().reduce(int.__add__) == 6

    assert {0: 1, 1: 2, 2: 3}.values().reduce(int.__add__, 5) == 6 + 5


def test_dict_values_filter():
    assert {0: 1, 1: 2, 3: 4, 4: 5, 5: 6}.values().filter(lambda x: x %
                                                          2 == 0) == [2, 4, 6]


def test_dict_values_first():
    with pytest.raises(IndexError):
        {}.values().first()

    assert {0: 1}.values().first() == 1
    assert {0: 1, 1: 2, 2: 3}.values().first() == 1


def test_dict_values_last():
    with pytest.raises(IndexError):
        {}.values().last()

    assert {0: 1}.values().last() == 1
    assert {0: 1, 1: 2, 2: 3}.values().last() == 3


###################################################


# dict items extension tests


def test_dict_items_all():
    assert {None: None}.items().all()
    assert {}.items().all()


def test_dict_items_any():
    assert {None: None}.items().any()
    assert not {}.items().any()


def test_dict_items_map():
    assert {1: None, 2: None, 3: None}.items().map(lambda x: x[0] + 3) == [4, 5, 6]

    assert {1: None, 2: None, 3: None}.items().map(lambda x: str(x[0])) == ["1", "2", "3"]


def test_dict_items_reduce():
    assert {1: None, 2: None, 3: None}.items().reduce(lambda x, y: (x[0] + y[0],))[0] == 6

    assert {1: None, 2: None, 3: None}.items().reduce(lambda x, y: x + y[0], 5) == 6 + 5


def test_dict_items_filter():
    assert {1: None, 2: None, 3: None, 4: None, 5: None,
            6: None}.items().filter(lambda x: x[0] % 2 == 0) == [(2, None), (4, None), (6, None)]


def test_dict_items_first():
    with pytest.raises(IndexError):
        {}.items().first()

    assert {1: None}.items().first() == (1, None)
    assert {1: None, 2: None, 3: None}.items().first() == (1, None)


def test_dict_items_last():
    with pytest.raises(IndexError):
        {}.items().last()

    assert {1: None}.items().last() == (1, None)
    assert {1: None, 2: None, 3: None}.items().last() == (3, None)


###################################################


# list extension tests


def test_list_all():
    assert [True, "String", 10].all()
    assert not [True, False, True].all()
    assert not [True, 10, ""].all()

    assert [].all()
    assert not [False].all()


def test_list_any():
    assert [True, False, False].any()
    assert [False, False, True].any()

    assert not ["", 0, False, []].any()

    assert not [].any()


def test_list_map():
    assert [1, 2, 3].map((3).__add__) == [4, 5, 6]

    assert [1, 2, 3].map(str) == ["1", "2", "3"]


def test_list_reduce():
    with pytest.raises(TypeError):
        [].reduce(int.__add__)

    assert [1, 2, 3].reduce(int.__add__) == 6

    assert [1, 2, 3].reduce(int.__add__, 5) == 6 + 5


def test_list_filter():
    assert [1, 2, 3, 4, 5, 6].filter(lambda x: x % 2 == 0) == [2, 4, 6]


def test_list_first():
    with pytest.raises(IndexError):
        [].first()

    assert [1].first() == 1
    assert [1, 2, 3].first() == 1


def test_list_last():
    with pytest.raises(IndexError):
        [].last()

    assert [1].last() == 1
    assert [1, 2, 3].last() == 3


###################################################


# float extensions tests


def test_float_round():
    assert (3.3).round() == 3

    assert (3.14).round(1) == 3.1


###################################################


# function extensions tests


def test_function_composition():
    def foo(x):
        return x + 1

    def bar(x, y):
        return x + y

    assert (foo @ bar)(3, 5) == 9


###################################################


# int extensions tests


def test_int_hex():
    assert (0xCAFE).hex() == "0xcafe"


def test_int_oct():
    assert (0o474).oct() == "0o474"


def test_int_bin():
    assert (0b1001_0110).bin() == "0b10010110"


###################################################


# seq extensions tests


def test_seq_tolist():
    assert map(lambda x: x + 1, [2, 3, 4]).tolist() == [3, 4, 5]
    assert filter(lambda x: x % 2 == 0, [2, 3, 4]).tolist() == [2, 4]
    assert range(6).tolist() == [0, 1, 2, 3, 4, 5]
    assert zip([1, 2, 3], ['a', 'b', 'c']).tolist() == [(1, 'a'), (2, 'b'), (3, 'c')]


def test_seq_map():
    assert map(lambda x: x + 1, [2, 3, 4]).map(lambda x: x + 1).tolist() == [4, 5, 6]
    assert filter(lambda x: x % 2 == 0, [2, 3, 4]).map(lambda x: x + 1).tolist() == [3, 5]
    assert range(6).map(lambda x: x + 1).tolist() == [1, 2, 3, 4, 5, 6]
    assert zip([1, 2, 3], ['a', 'b', 'c']).map(lambda x: (x[0] + 1, x[1])).tolist() == [(2, 'a'), (3, 'b'), (4, 'c')]


def test_seq_filter():
    assert map(lambda x: x + 1, [2, 3, 4]).filter(lambda x: x <= 4).tolist() == [3, 4]
    assert filter(lambda x: x % 2 == 0, [2, 3, 4, 5, 6]).filter(lambda x: x <= 4).tolist() == [2, 4]
    assert range(6).filter(lambda x: x % 2 == 0).tolist() == [0, 2, 4]
    assert zip([1, 2, 3], ['a', 'b', 'c']).filter(lambda x: x[0] >= 2).tolist() == [(2, 'b'), (3, 'c')]


###################################################


# str extensions tests


def test_str_to_int():
    assert "1".to_int() == 1
    assert "-1".to_int() == -1
    assert isinstance("15".to_int(), int)
    with pytest.raises(ValueError):
        "asd".to_int()
    assert "asd".to_int(default=None) is None
    assert "10".to_int(base=2) == 2


def test_str_to_float():
    assert "1".to_float() == 1
    assert "-1".to_float() == -1
    assert "1.5".to_float() == 1.5
    assert isinstance("15".to_float(), float)
    with pytest.raises(ValueError):
        "asd".to_float()
    assert "asd".to_float(default=None) is None


###################################################


# coroutine extensions tests


@pytest.mark.asyncio
async def test_coroutine_then_sync():
    async def foo():
        return 10

    result = await foo().then(lambda x: x + 5)
    assert result == 15


@pytest.mark.asyncio
async def test_coroutine_then_async():
    async def foo():
        return 10

    async def add_five(x):
        return x + 5

    result = await foo().then(add_five)
    assert result == 15


@pytest.mark.asyncio
async def test_coroutine_then_chaining():
    async def foo():
        return 10

    async def add_five(x):
        return x + 5

    result = await foo().then(lambda x: x * 2).then(add_five).then(lambda x: x - 3)
    assert result == 22  # (10 * 2) + 5 - 3 = 22


@pytest.mark.asyncio
async def test_coroutine_catch_no_exception():
    async def foo():
        return 42

    result = await foo().catch(lambda e: 0)
    assert result == 42


@pytest.mark.asyncio
async def test_coroutine_catch_with_exception():
    async def foo():
        raise ValueError("test error")

    result = await foo().catch(lambda e: 100, exception=ValueError)
    assert result == 100


@pytest.mark.asyncio
async def test_coroutine_catch_async_handler():
    async def foo():
        raise ValueError("test error")

    async def handle_error(e):
        return 200

    result = await foo().catch(handle_error, exception=ValueError)
    assert result == 200


@pytest.mark.asyncio
async def test_coroutine_catch_wrong_exception_type():
    async def foo():
        raise ValueError("test error")

    with pytest.raises(ValueError):
        await foo().catch(lambda e: 0, exception=TypeError)


@pytest.mark.asyncio
async def test_coroutine_catch_default_exception():
    async def foo():
        raise RuntimeError("test error")

    result = await foo().catch(lambda e: 300)
    assert result == 300


@pytest.mark.asyncio
async def test_coroutine_then_and_catch_combined():
    async def foo():
        return 10

    result = await foo().then(lambda x: x * 2).catch(lambda e: 0)
    assert result == 20


@pytest.mark.asyncio
async def test_coroutine_catch_and_then_combined():
    async def foo():
        raise ValueError("error")

    result = await foo().catch(lambda e: 50, exception=ValueError).then(lambda x: x + 10)
    assert result == 60


###################################################
