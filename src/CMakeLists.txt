add_executable(dsa main.cpp
    dsa_framework/framework.h
    dsa_framework/framework.cpp


    algo/sorting.h
    algo/searching.h
    algo/graph.h
    algo/dp.h
    algo/heap.h
    algo/trie.h
    algo/linked_list.h
    algo/tree.h
    algo/hash_table.h
    algo/bit_manip.h
    algo/string_algo.h
    algo/segment_tree.h
    algo/monotonic_queue.h
)

target_link_libraries(
  dsa
  PRIVATE myproject::myproject_options
          myproject::myproject_warnings
          CLI11::CLI11
          fmt::fmt
          spdlog::spdlog)


myproject_configure_linker(dsa)

target_include_directories(dsa PRIVATE "${CMAKE_BINARY_DIR}/configured_files/include"
                                    "${CMAKE_SOURCE_DIR}/src" )

# Apply WASM-specific configuration
if(EMSCRIPTEN)
  myproject_configure_wasm_target(dsa
    TITLE "DSA Framework"
    DESCRIPTION "DSA Framework"
  )
endif()
