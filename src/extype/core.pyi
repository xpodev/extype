from typing import Type, Dict, Any
from . import Protocol


def get_builtin_type_dict(typ: Type) -> Dict[str, Any]: ...


def implement_protocol_on_type(typ: Type, protocols: Protocol): ...


def enable_magic_method(typ: Type, method: str): ...
