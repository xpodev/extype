# https://mesonbuild.com/Python-module.html
py_mod = import('python')
python = py_mod.find_installation(pure: false)
python_dep = python.dependency()


python_sources = [
   '__init__.py',
]

python.install_sources(
  python_sources,
  subdir: 'extypes'
)

python.extension_module('core',
    'extypes.cpp',
    install: true,
    subdir: 'extypes',
)
