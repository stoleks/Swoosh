set(ROOT ${PROJECT_SOURCE_DIR}/src/Swoosh/Renderers)
set (SRC
  ${ROOT}/Renderer.h
  ${ROOT}/SimpleRenderer.h
)
source_group ("" FILES ${SRC})

target_sources (${PROJECT_NAME} PRIVATE ${SRC})
