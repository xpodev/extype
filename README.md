# ExTypes
Extypes stands for extensible types, a Python package that enables extending types.


## Build & Installation
To build the package, you'll need to install the `build` python package.

Run
```
pip install build
```

You can run this command in a virtual environment.

After you've installed the `build` package, run it through the command line:

**Windows:**
```
py -m build
```

**Linux:**
```
python3 -m build
```

This command will install some tools which are required to build the package from source, and it'll then 
build the package for you.


After the process is complete, it'll result with a `.whl` file, AKA wheel file. 
You can install this wheel file with `pip`:
```
pip install {path_to_wheel_file}
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

**Note:**
This does not yet work for magic methods, and we'll might add that as a feature in the future.
