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

set(SOURCES
        ngap_asn1_packer.cpp
        ngap_asn1_utils.cpp
        log_helpers.cpp
        ngap_factory.cpp
        ngap_impl.cpp
        ngap_connection_handler.cpp
        ngap_validators/ngap_validators.cpp
        gateways/n2_connection_client_factory.cpp
        metrics/ngap_metrics_aggregator.cpp
        procedures/ng_setup_procedure.cpp
        procedures/ng_reset_procedure.cpp
        procedures/ngap_initial_context_setup_procedure.cpp
        procedures/ngap_dl_nas_message_transfer_procedure.cpp
        procedures/ngap_pdu_session_resource_setup_procedure.cpp
        procedures/ngap_pdu_session_resource_modify_procedure.cpp
        procedures/ngap_pdu_session_resource_release_procedure.cpp
        procedures/ngap_handover_preparation_procedure.cpp
        procedures/ngap_handover_resource_allocation_procedure.cpp
        procedures/ngap_ue_context_release_procedure.cpp
    )


add_library(srsran_ngap STATIC ${SOURCES})

target_link_libraries(srsran_ngap
	srsran_cu_cp
	ngap_asn1
	srsran_support)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
