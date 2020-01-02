project('com.github.msmaldi.nvidia-fan-service', [ 'c' ],
    version: '0.1.0',
    meson_version: '>= 0.40.0',
)

libXNVCtrl_proj = subproject ('libXNVCtrl')
libXNCtrl_dep = libXNVCtrl_proj.get_variable('libXNCtrl_dependency')

install_path = join_paths(get_option('prefix'), 'bin')

subdir('data')
subdir('src')

nvidia_fan_service_deps = [
    dependency('xext'),
    dependency('x11'),
    libXNCtrl_dep
]

executable('nvidia-fan-service', nvidia_fan_service_sources,
    dependencies: nvidia_fan_service_deps,
    install: true
)
