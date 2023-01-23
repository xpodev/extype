py = import('python')
py = py.find_installation(pure: false)
python_dep = py.dependency()

py.install_sources([
    '__init__.py',
    ],
    subdir: 'extypes',
)

py.extension_module('core',
    'extypes.cpp',
    install: true,
    subdir: 'extypes',
)