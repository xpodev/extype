import sys
import sysconfig
import threading

import pytest

from extype.extension_utils import extend_type_with, extension


free_threaded = bool(sysconfig.get_config_var("Py_GIL_DISABLED"))

THREAD_COUNT = 8


def run_in_threads(*targets):
    """Runs all targets at the same time and re-raises the first exception any of them raised."""
    barrier = threading.Barrier(len(targets))
    errors = []

    def run(target):
        barrier.wait()
        try:
            target()
        except BaseException as e:
            errors.append(e)

    threads = [threading.Thread(target=run, args=(target,)) for target in targets]
    for thread in threads:
        thread.start()
    for thread in threads:
        thread.join()

    if errors:
        raise errors[0]


@pytest.mark.skipif(not free_threaded, reason="requires a free-threaded build")
def test_import_keeps_gil_disabled():
    import extype.core  # noqa: F401

    assert not sys._is_gil_enabled()


def test_extending_while_other_threads_use_the_type():
    class Target:
        pass

    class Extension:
        @extension
        def __add__(self, other):
            return 42

        @extension
        def value(self):
            return 42

    extend_type_with(Target, Extension)

    def writer():
        for _ in range(200):
            extend_type_with(Target, Extension)

    def reader():
        target = Target()
        for _ in range(2000):
            assert target + 1 == 42
            assert target.value() == 42

    run_in_threads(*[writer if i % 2 else reader for i in range(THREAD_COUNT)])


def test_concurrent_extensions_are_all_applied():
    class Target:
        pass

    def make_extension(i):
        def method(self):
            return i

        return type(f"Extension{i}", (), {f"method_{i}": extension(method)})

    extensions = [make_extension(i) for i in range(THREAD_COUNT)]

    run_in_threads(*[lambda e=e: extend_type_with(Target, e) for e in extensions])

    target = Target()
    for i in range(THREAD_COUNT):
        assert getattr(target, f"method_{i}")() == i
