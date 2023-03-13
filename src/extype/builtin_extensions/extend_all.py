from . import (
    list_ext,
    int_ext,
    float_ext,
    seq_ext,
    function_ext,
    dict_ext,
)

for ext in [
    list_ext,
    int_ext,
    float_ext,
    seq_ext,
    function_ext,
    dict_ext,
]:
    ext.extend()
