from typing import Type, Dict, Any, TYPE_CHECKING

if TYPE_CHECKING:
    from .extension_utils import Protocol

def get_type_dict(typ: Type) -> Dict[str, Any]: ...


def implement_protocol_on_type(typ: Type, protocols: "Protocol"): ...


def enable_magic_method(typ: Type, method: str): ...


if TYPE_CHECKING:
    del Protocol
