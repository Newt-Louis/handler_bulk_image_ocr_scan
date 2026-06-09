file(REMOVE_RECURSE
  "AutoPhoto/CoverFlowView.qml"
  "AutoPhoto/Main.qml"
  "AutoPhoto/ToolPanel.qml"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/autophoto_tooling.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
