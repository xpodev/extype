# ExType
Extype stands for extensible types, a Python package that enables extending types.

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
call the `extend_builtin_types` function:
```py
extype.extend_builtin_types()
```

The built-in extensions include a map function for the `list` type:
```py
print([1, 2, 3].map(lambda x: x + 1))  # [2, 3, 4]
```

### Creating your own extensions
You can create your own extension methods, with which you can extend any type you want!
For example, let's make our own hex function in the `int` type.
What we want to have at the end is:
```py
x = 0xFA
print(x.hex())  # 0xfa
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
  def hex(self):  # self will be of the same type we extend, which, in this case, is `int`
    return hex(self)
```

After we create the class which will contain the extension methods, we need to apply to to the types we want to extend:
```py
extend_type_with(int, IntExtension)
```

Now, we can run the code from above:
```py
x = 0xFA
print(x.hex())  # 0xfa
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