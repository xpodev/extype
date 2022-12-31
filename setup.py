from distutils.core import setup, Extension

builtins_plus_module = Extension('builtins_plus.core',
                    sources = ['builtins_plus.cpp'])

setup (name = 'BuiltinsPlus',
       version = '1.0.0',
       description = 'BuiltinsPlus is a module that allows you to extend builtins types.',
       author="Xpo Development",
       author_email="biny.xpo.dev",
       packages=["builtins_plus"],
       package_dir={
              "builtins_plus": "src/builtins_plus"
       },
       ext_modules = [builtins_plus_module])
