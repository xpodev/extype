from skbuild import setup


setup(
    name="extypes",
    version="0.1.0",
    packages=['extypes'],
    package_dir={'': 'src'},
    cmake_install_dir="src/extypes",
)
