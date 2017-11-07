add_executable(UnitTest1 UnitTest1.cpp)

target_link_libraries(UnitTest1
    OrbitPTM
    OrbitBase
    ${GTEST_BOTH_LIBRARIES}
    procps
    stdc++fs
)