# all source files
set(ROOT ${PROJECT_SOURCE_DIR}/src/Segues)
set (SRC
  ${ROOT}/BlackWashFade.h
  ${ROOT}/BlendFadeIn.h
  ${ROOT}/BlurFadeIn.h
  ${ROOT}/Checkerboard.h
  ${ROOT}/CircleClose.h
  ${ROOT}/CircleOpen.h
  ${ROOT}/CrossZoom.h
  ${ROOT}/Cube3D.h
  ${ROOT}/DiamondTileCircle.h
  ${ROOT}/DiamondTileSwipe.h
  ${ROOT}/Dream.h
  ${ROOT}/HorizontalOpen.h
  ${ROOT}/HorizontalSlice.h
  ${ROOT}/Morph.h
  ${ROOT}/PageTurn.h
  ${ROOT}/PixelateBlackWashFade.h
  ${ROOT}/PushIn.h
  ${ROOT}/RadialCCW.h
  ${ROOT}/RetroBlit.h
  ${ROOT}/SlideIn.h
  ${ROOT}/SwipeIn.h
  ${ROOT}/VerticalOpen.h
  ${ROOT}/VerticalSlice.h
  ${ROOT}/WhiteWashFade.h
  ${ROOT}/ZoomFadeIn.h
  ${ROOT}/ZoomFadeInBounce.h
  ${ROOT}/ZoomIn.h
  ${ROOT}/ZoomOut.h
)
source_group ("" FILES ${SRC})

target_sources (${PROJECT_NAME} PUBLIC ${SRC})
