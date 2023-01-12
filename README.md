# ExTypes
Extypes stands for extensible types, a Python package that enables extending types.


## Features
- [x] Exteranlly extend type via another type
- [x] Basic support for magic method extensions
  - [x] Number protocol
  - [x] Mapping protocol
  - [x] Sequence protocol
- [ ] Add support for reverse methods (e.g. `__radd__`)
- [x] Make this features/todo list look nicer


## Build & Installation
We will use [Hatch](https://hatch.pypa.io/latest/) to build the package.
First, install Hatch: https://hatch.pypa.io/latest/install/. We recommend using [pipx](https://hatch.pypa.io/latest/install/#pipx).

After you've installed Hatch, you can build the package with the following command:
```sh
hatch run build
```
**Note:** This is not the normal `hatch build` command, because we need to build an extension module, which is not supported by Hatch yet.

This will build the package and create a wheel file in the `dist` folder.

You can install this wheel file with `pip`:
```sh
pip install {path_to_wheel_file}
```

### Contributing

Run tests:
```sh
hatch run test
```
This will install the wheel file in a virtual environment and run the tests.

Build & test for all python versions:
```sh
hatch run build:full-build
```

## Usage
First, in your Python code, import the package:
```py
import extypes
```

Then, you can use the built-in extensions for the builtin types. In order to apply these extensions, 
call the `extend_builtin_types` function:
```py
extypes.extend_builtin_types()
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
from extypes import extension, extend_type_with
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

