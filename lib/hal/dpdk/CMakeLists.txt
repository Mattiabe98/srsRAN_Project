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

add_subdirectory(bbdev)

set(SOURCES
  dpdk_eal_factory.cpp
  dpdk.cpp
  mbuf_pool_factory.cpp
)

add_library(hal_dpdk STATIC ${SOURCES})
target_compile_options(hal_dpdk PRIVATE ${DPDK_CFLAGS})
target_link_libraries(hal_dpdk srslog ${DPDK_LIBRARIES})

add_to_exported_libs(hal_dpdk)
