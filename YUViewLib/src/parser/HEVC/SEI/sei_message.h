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

#include "../NalUnitHEVC.h"
#include "../commonMaps.h"
#include "parser/common/SubByteReaderLogging.h"

namespace parser::hevc
{

class seq_parameter_set_rbsp;

// Parsing of an SEI message may fail when the required parameter sets are not yet available and
// parsing has to be performed once the required parameter sets are recieved.
enum class SEIParsingResult
{
  OK,                     // Parsing is done
  WAIT_FOR_PARAMETER_SETS // We have to wait for valid parameter sets before we can parse this SEI
};

class sei_payload
{
public:
  sei_payload() = default;
  virtual ~sei_payload() = default;

  virtual SEIParsingResult parse(reader::SubByteReaderLogging &          reader,
                                 bool                                    reparse,
                                 VPSMap &                                vpsMap,
                                 SPSMap &                                spsMap,
                                 std::shared_ptr<seq_parameter_set_rbsp> associatedSPS) = 0;
};

class unknown_sei : public sei_payload
{
public:
  unknown_sei() = default;

  SEIParsingResult parse(reader::SubByteReaderLogging &          reader,
                         bool                                    reparse,
                         VPSMap &                                vpsMap,
                         SPSMap &                                spsMap,
                         std::shared_ptr<seq_parameter_set_rbsp> associatedSPS) override;
};

class sei_message : public NalRBSP
{
public:
  sei_message(NalType nal_unit_type) : seiNalUnitType(nal_unit_type) {}
  ~sei_message() = default;
  SEIParsingResult parse(reader::SubByteReaderLogging &          reader,
                         VPSMap &                                vpsMap,
                         SPSMap &                                spsMap,
                         std::shared_ptr<seq_parameter_set_rbsp> associatedSPS);

  SEIParsingResult
  reparse(VPSMap &vpsMap, SPSMap &spsMap, std::shared_ptr<seq_parameter_set_rbsp> associatedSPS);

  std::string getPayloadTypeName() const;

  unsigned payloadType{};
  unsigned payloadSize{};

  std::shared_ptr<sei_payload> payload;

  NalType seiNalUnitType;

private:
  SEIParsingResult parsePayloadData(bool                                    reparse,
                                    VPSMap &                                vpsMap,
                                    SPSMap &                                spsMap,
                                    std::shared_ptr<seq_parameter_set_rbsp> associatedSPS);

  reader::SubByteReaderLogging payloadReader;
  bool                         parsingDone{false};
};

} // namespace parser::hevc
