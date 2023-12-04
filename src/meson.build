retrospriteeditor_sources = [
  'main.c',
  'retrospriteeditor-application.c',
  'retrospriteeditor-window.c',
  'retrospriteeditor-canvas.c',
  'retrospriteeditor-frame-palette.c',
  'retrospriteeditor-nes-palette.c',
  'custom_math.c',
  'retrospriteeditor-tool-pencil.c',
  'retrospriteeditor-tool-button.c',
  'retrospriteeditor-nes-list-palettes.c',
  'retrospriteeditor-nes-item-palette.c',
  'type-palette.c',
  'retrospriteeditor-nes-current-palette.c',
  'retrospriteeditor-setup-palette-nes.c',
  'retrospriteeditor-new-project-nes.c',
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
