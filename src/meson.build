retrospriteeditor_sources = [
  'main.c',
  'retro-app.c',
  'main-window.c',
  'retro-canvas.c',
  'frame-platform.c',
  'nes-palette.c',
  'custom-math.c',
  'tool-pencil.c',
  'tool-button.c',
  'nes-list-palettes.c',
  'nes-item-palette.c',
  'global-functions.c',
  'nes-current-palette.c',
  'nes-setup-palette.c',
  'nes-new-project.c',
  'project.c',
]

retrospriteeditor_deps = [
  dependency('gtk4'),
  dependency('libadwaita-1', version: '>= 1.4'),
  dependency('libxml2'),
]

retrospriteeditor_sources += gnome.compile_resources('retrospriteeditor-resources',
  'retrospriteeditor.gresource.xml',
  c_name: 'retrospriteeditor'
)

executable('retrospriteeditor', retrospriteeditor_sources,
  dependencies: retrospriteeditor_deps,
       install: true,
)
