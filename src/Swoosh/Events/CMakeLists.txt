set(ROOT ${PROJECT_SOURCE_DIR}/src/Swoosh/Events)
set (SRC
  ${ROOT}/Events.h
)
source_group ("" FILES ${SRC})

target_sources (${PROJECT_NAME} PRIVATE ${SRC})
