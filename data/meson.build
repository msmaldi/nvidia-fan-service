desktop_config = configuration_data()
desktop_config.set('EXEC_PATH', join_paths(install_path, 'nvidia-fan-service'))
configure_file (
    input: meson.project_name() + '.desktop.in',
    output: meson.project_name() + '.desktop',
    configuration: desktop_config,
    install_dir: join_paths(get_option('sysconfdir'), 'xdg', 'autostart')
)
