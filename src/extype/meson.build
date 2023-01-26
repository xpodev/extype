# https://mesonbuild.com/Python-module.html
py_mod = import('python')
python = py_mod.find_installation(pure: false)
python_dep = python.dependency()


python_sources = [
   '__init__.py',
   'extension_utils.py'
]

python.install_sources(
  python_sources,
  subdir: 'extype'
)

python.extension_module('core',
    'extype.cpp',
    install: true,
    subdir: 'extype',
)

subdir('builtin_extensions')
