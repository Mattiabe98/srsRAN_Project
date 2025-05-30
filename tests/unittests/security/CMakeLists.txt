#
# Copyright 2021-2025 Software Radio Systems Limited
#
# This file is part of srsRAN
#
# srsRAN is free software: you can redistribute it and/or modify
# it under the terms of the GNU Affero General Public License as
# published by the Free Software Foundation, either version 3 of
# the License, or (at your option) any later version.
#
# srsRAN is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
# GNU Affero General Public License for more details.
#
# A copy of the GNU Affero General Public License can be found in
# the LICENSE file in the top-level directory of this distribution
# and at http://www.gnu.org/licenses/.
#

set_directory_properties(PROPERTIES LABELS "security")

add_executable(security_test security_test.cpp)
target_link_libraries(security_test srsran_security srsran_support srslog gtest gtest_main)
gtest_discover_tests(security_test)

add_executable(integrity_engine_test integrity_engine_test.cpp)
target_include_directories(integrity_engine_test PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(integrity_engine_test srsran_security srsran_support srslog gtest gtest_main)
gtest_discover_tests(integrity_engine_test)

add_executable(ciphering_engine_test ciphering_engine_test.cpp)
target_include_directories(ciphering_engine_test PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(ciphering_engine_test srsran_security srsran_support srslog gtest gtest_main)
gtest_discover_tests(ciphering_engine_test)

add_executable(k_gnb_star_gen_helper k_gnb_star_gen_helper.cpp)
target_include_directories(k_gnb_star_gen_helper PRIVATE ${CMAKE_SOURCE_DIR})
target_link_libraries(k_gnb_star_gen_helper srsran_security srsran_support srslog)
