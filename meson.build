project('extype',
    'cpp',
    # The version comes from the latest `vX.Y.Z` git tag, see tools/version.py
    version: run_command('tools/version.py', check: true).stdout().strip(),
    default_options: [
        'buildtype=release',
        'c_std=c17',
        'cpp_std=c++17'
    ]
)

# sdists have no git history, so the version is stored in them when they're made
meson.add_dist_script('tools/version.py', '--write', meson.project_version())

subdir('src/extype')
