include_directories(../) # so we can access the headers normally (e.g. #include "Patch.h") from the tests
include_directories(../Testing ../Helpers)

add_executable(TestStatistics TestStatistics.cpp)
target_link_libraries(TestStatistics ${VTK_LIBRARIES} ${ITK_LIBRARIES} Helpers libTesting)
add_test(TestStatistics TestStatistics)

add_executable(TestParallelSort TestParallelSort.cpp)
target_link_libraries(TestParallelSort ${VTK_LIBRARIES} ${ITK_LIBRARIES} Helpers libTesting)
add_test(TestParallelSort TestParallelSort)

add_executable(TestIntroducedEnergy TestIntroducedEnergy.cpp)
target_link_libraries(TestIntroducedEnergy ${VTK_LIBRARIES} ${ITK_LIBRARIES} Mask libTesting)
add_test(TestIntroducedEnergy TestIntroducedEnergy)
