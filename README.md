# ExType
Extype stands for extensible types, a Python package that enables extending types.

[![PyPI version](https://badge.fury.io/py/extype.svg?)](https://badge.fury.io/py/extype)

![Windows](https://github.com/xpodev/extypes/actions/workflows/python-test-windows.yml/badge.svg)
![MacOS](https://github.com/xpodev/extypes/actions/workflows/python-test-macos.yml/badge.svg)
![Linux](https://github.com/xpodev/extypes/actions/workflows/python-test-linux.yml/badge.svg)

## Installation

```sh
pip install extype
```

Alternatively, you can install through git (make sure to have pip 22.0 or higher):
```sh
python -m pip install --upgrade pip
pip install git+https://github.com/xpodev/extype/
```

## Usage
First, in your Python code, import the package:
```py
import extype
```

Then, you can use the built-in extensions for the builtin types. In order to apply these extensions, 
import the `extend_all` module from the `extype.builtin_extensions` package:
```py
from extype.builtin_extensions import extend_all
```
This will apply the extensions to all builtins we support, *as a side-effect of the import* (to extend only some builtins, you can use the dedicated modules, see below).

Sometimes you don't want to apply all the extensions that we provide, but only for some specific type(s).

Say, for example, we only want to apply the provided extensions for `list`. We'll need to manually
apply them like so:
```py
from extype.builtin_extensions import list_ext

list_ext.extend()
```

> **Note:** All built-in extension modules have an `extend` function which will apply the extensions in the module to the relevant type.

Currently, we provide the following extensions:


|       file      |           extended types           |
|:---------------:|:----------------------------------:|
|   dict_ext.py   | dict_keys, dict_values, dict_items |
|   float_ext.py  | float |
| function_ext.py | FunctionType, LambdaType  |
|    int_ext.py   | int |
|   list_ext.py   | list |
|    seq_ext.py   | map, filter, range, zip |
|    str_ext.py   | str |



Then you can use these extensions. Here's an example of using the `list.map` extension:
```py
print([1, 2, 3].map(lambda x: x + 1))  # [2, 3, 4]
```

There's a list of all the built-in extensions [here](#built-in-extensions)


### Creating your own extensions
You can create your own extension methods, with which you can extend any type you want! (not only builtins)
For example, let's make our own `tofloat` function in the `int` type.
What we want to have at the end is:
```py
x = 10
print(isinstance(x.tofloat(), float))  # True
```

First, we'll need some tools:
```py
from extype import extension, extend_type_with
```

Next, we'll define our class which will hold the extension method. Note that this class will not get instantiated.
It is also recommended to make this class inherit the type you want to extend, so you get better typing support.
```py
class IntExtension(int):  # inheriting `int` for typing
  @extension  # marks this method to be added as an extension
  def tofloat(self):  # self will be of the same type we extend, which, in this case, is `int`
    return float(self)  # convert the int to float and return the result
```

After we create the class which will contain the extension methods, we need to apply them to the types we want to extend:
```py
extend_type_with(int, IntExtension)
```

Now, we can run the code from above:
```py
x = 10
print(isinstance(x.tofloat(), float))  # True
```

We can also apply multiple extensions to the same type or even the same extension to multiple types.

Only methods marked with `@extension` will be added as extension methods.

**Note:**
Extending a type will extend it in all modules, not just the one that called the `extend_type_with`, 
so make sure you don't override an existing function, unless, of course, it is what you want.


## Features
- [x] Exteranlly extend type via another type
- [x] Basic support for magic method extensions
  - [x] Number protocol
  - [x] Mapping protocol
  - [x] Sequence protocol
- [x] Add support for reverse methods (e.g. `__radd__`)
- [x] Make this features/todo list look nicer
- [ ] Add support for the rich comparison function


## Maintainers

### Build & Installation

We use [Hatch](https://hatch.pypa.io/latest/) to manage the build environment,
 and [mesonpy]( https://github.com/mesonbuild/meson-python) to build the package.
> **Note:** Currently, we use unreleased mesonpy features, so we install it from git. 

First, install Hatch: https://hatch.pypa.io/latest/install/. We recommend using [pipx](https://hatch.pypa.io/latest/install/#pipx).

After you've installed Hatch, you can build the package with the following command:
```sh
hatch run install_editable
```
With this, you can start using the package in your code. 
Spawn shell within the build environment:
```sh
hatch shell
```
It'll rebuild the package every time you import it, so you can test your changes.
If you don't want to rebuild the package every time you import it, you can install it with:
```sh
hatch run install
```
But note that any changes you make won't be reflected in the installed package.

To build the wheel, you can use:
```sh
hatch run dist:build
```
This will build the wheel for all python versions, and put it in the `dist` folder.

### Testing

To run tests for all python versions, run:
```sh
hatch run dist:test
```
To run tests for a specific python version, run:
```sh
hatch run +py=39 dist:test
```

Both commands will build, install the package into an isolated environment,
 and run the tests in it.


### Built-in Extensions

> **Note:** All of the following `list` extensions also exist on `dict_keys`, `dict_values` and `dict_items`.

```py
list.all(self: List[T], fn: Callable[[T], bool] = bool) -> bool
```
Returns true if all elements, mapped through the given `fn`, are `True`.

```py
list.any(self: List[T], fn: Callable[[T], bool] = bool) -> bool
```
Returns true if any of the elements, mapped through the given `fn`, is `True`.

```py
list.map(self: List[T], fn: Callable[[T], U]) -> List[U]
```
Returns a new list whose elements are the result of applying the given function on each element in the original list.

```py
list.reduce(self: List[T], fn: Callable[[T, T], T]) -> T
```
Reduces the list to a single value, using the given function as the reduction (combination) function.

Raises `TypeError` if the list is empty.

```py
list.reduce(self: List[T], fn: Callable[[U, T], U], initial_value: U) -> U
```
Reduces the list to a single value, using the given function as the reduction (combination) function and the initial value.

```py
list.filter(self: List[T], fn: Callable[[T], bool]) -> List[T]
```
Returns a new list containing all the elements that match the given predicate `fn`.

```py
list.first(self: List[T]) -> T, raise IndexError
```
Returns the first element in the list, or raises an `IndexError` if the list is empty.

```py
list.last(self: List[T]) -> T, raise IndexError
```
Returns the last element in the list, or raises `IndexError` if the list is empty.

```py
float.round(self: float) -> int
```
Rounds the floating point number to the nearest integer.

```py
float.round(self: float, ndigits: int) -> int | float
```
Round the floating point number to the nearest float with `ndigits` fraction digits.

```py
# function @ functioin
function.__matmul__(self: Callable[[T], U], other: Callable[..., T]) -> Callable[..., U]
```
Compose 2 functions such that doing `(foo @ bar)(*args, **kwargs)` will have the same result as calling `foo(bar(*args, **kwargs))`.

```py
int.hex(self: int) -> str
```
Returns the hexadecimal representation of the integer.

```py
int.oct(self: int) -> str
```
Returns the octal representation of the integer.

```py
int.bin(self: int) -> str
```
Returns the binary representation of the integer.

```py
str.to_int(self: str, base: int = 10, default: T = ...) -> int | T
```
Converts the given string to an int with the given base. If it can't be
converted and `default` is given, it is returned. Otherwise, a `ValueError`
is thrown.

```py
str.to_float(self: str, default: T = ...) -> float | T
```
Converts the given string to a float. If it can't be
converted and `default` is given, it is returned. Otherwise, a `ValueError`
is thrown.

* The following extensions are valid for `map`, `filter`, `range` and `zip`
```py
.tolist(self: Iterable[T]) -> List[T]
```
Exhausts the iterble and creates a list from it.

```py
.map(self: Iterable[T], fn: Callable[[T], U]) -> Iterable[U]
```
Maps the iterable with the given function to create a new iterable.

This does not iterates through the original iterable.

```py
.filter(self: Iterable[T], fn: Callable[[T], bool]) -> Iterable[T]
```
Filters the iterable with the given function as the predicate function.

This does not iterates through the original iterable.
