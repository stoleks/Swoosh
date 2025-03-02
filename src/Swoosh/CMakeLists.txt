add_subdirectory (Events)
add_subdirectory (Renderers)

# all source files
set(ROOT ${PROJECT_SOURCE_DIR}/src/Swoosh)
set (SRC
  ${ROOT}/ActionList.h
  ${ROOT}/Activity.h
  ${ROOT}/ActivityController.h
  ${ROOT}/Ease.h
  ${ROOT}/EmbedGLSL.h
  ${ROOT}/Segue.h
  ${ROOT}/Shaders.h
  ${ROOT}/Timer.h
  ${ROOT}/Utils.h
)
source_group ("" FILES ${SRC})

target_sources (${PROJECT_NAME} PRIVATE ${SRC})
