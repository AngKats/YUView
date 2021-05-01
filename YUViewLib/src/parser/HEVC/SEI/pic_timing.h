/*  This file is part of YUView - The YUV player with advanced analytics toolset
 *   <https://github.com/IENT/YUView>
 *   Copyright (C) 2015  Institut f�r Nachrichtentechnik, RWTH Aachen University, GERMANY
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   In addition, as a special exception, the copyright holders give
 *   permission to link the code of portions of this program with the
 *   OpenSSL library under certain conditions as described in each
 *   individual source file, and distribute linked combinations including
 *   the two.
 *
 *   You must obey the GNU General Public License in all respects for all
 *   of the code used other than OpenSSL. If you modify file(s) with this
 *   exception, you may extend this exception to your version of the
 *   file(s), but you are not obligated to do so. If you do not wish to do
 *   so, delete this exception statement from your version. If you delete
 *   this exception statement from all source files in the program, then
 *   also delete it here.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program. If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "parser/common/SubByteReaderLogging.h"
#include "sei_message.h"

namespace parser::hevc
{

class pic_timing : public sei_payload
{
public:
  pic_timing() = default;

  SEIParsingResult parse(reader::SubByteReaderLogging &          reader,
                         bool                                    reparse,
                         VPSMap &                                vpsMap,
                         SPSMap &                                spsMap,
                         std::shared_ptr<seq_parameter_set_rbsp> associatedSPS) override;

  unsigned pic_struct{};
  unsigned source_scan_type{};
  bool     duplicate_flag{};

  unsigned         au_cpb_removal_delay_minus1{};
  unsigned         pic_dpb_output_delay{};
  unsigned         pic_dpb_output_du_delay{};
  unsigned         num_decoding_units_minus1;
  bool             du_common_cpb_removal_delay_flag{};
  unsigned         du_common_cpb_removal_delay_increment_minus1{};
  vector<unsigned> num_nalus_in_du_minus1;
  vector<unsigned> du_cpb_removal_delay_increment_minus1;

private:
  TreeItem *reparseTreeItem{};
};

} // namespace parser::hevc