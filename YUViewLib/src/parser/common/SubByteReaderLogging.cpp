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

#include "SubByteReaderLogging.h"

#include <iomanip>
#include <sstream>

namespace parser::reader
{

namespace
{

template <typename T> std::string formatCoding(const std::string formatName, T value)
{
  std::ostringstream stringStream;
  stringStream << formatName;
  if (formatName.find("(v)") != std::string::npos && formatName != "Calc")
    stringStream << " -> u(" << value << ")";
  return stringStream.str();
}

void checkAndLog(TreeItem *         item,
                 const std::string &formatName,
                 const std::string &symbolName,
                 const Options &    options,
                 int64_t            value,
                 const std::string &code)
{
  CheckResult checkResult;
  for (auto &check : options.checkList)
  {
    checkResult = check->checkValue(value);
    if (!checkResult)
      break;
  }

  if (item && !options.loggingDisabled)
  {
    std::string meaning = options.meaningString;
    if (options.meaningMap.count(int64_t(value)) > 0)
      meaning = options.meaningMap.at(value);

    if (!checkResult)
      meaning += " " + checkResult.errorMessage;
    auto newItem = new TreeItem(item,
                                symbolName,
                                std::to_string(value),
                                formatCoding(formatName, code.size()),
                                code,
                                meaning);
    if (!checkResult)
      newItem->setError();
  }
  if (!checkResult)
    throw std::logic_error(checkResult.errorMessage);
}

void checkAndLog(TreeItem *         item,
                 const std::string &formatName,
                 const std::string &symbolName,
                 const Options &    options,
                 ByteVector         value,
                 const std::string &code)
{
  // There are no range checks for ByteVectors. Also the meaningMap does nothing.
  if (item && !options.loggingDisabled)
  {
    if (code.size() != value.size() * 8)
      throw std::logic_error("Nr bytes and size of code does not match.");

    auto byteVectorItem = new TreeItem(item, symbolName);
    for (size_t i = 0; i < value.size(); i++)
    {
      auto              c = value.at(i);
      std::stringstream valueStream;
      valueStream << "0x" << std::setfill('0') << std::setw(2) << std::hex << unsigned(c) << " ("
                  << c << ")";
      auto byteCode = code.substr(i * 8, 8);
      new TreeItem(byteVectorItem,
                   "Byte " + std::to_string(i),
                   valueStream.str(),
                   formatCoding(formatName, code.size()),
                   byteCode,
                   options.meaningString);
    }
  }
}

} // namespace

ByteVector SubByteReaderLogging::convertToByteVector(QByteArray data)
{
  ByteVector ret;
  for (auto i = 0u; i < unsigned(data.size()); i++)
  {
    ret.push_back(data.at(i));
  }
  return ret;
}

QByteArray SubByteReaderLogging::convertToQByteArray(ByteVector data)
{
  QByteArray ret;
  for (auto i = 0u; i < unsigned(data.size()); i++)
  {
    ret.append(data.at(i));
  }
  return ret;
}

SubByteReaderLogging::SubByteReaderLogging(SubByteReader &reader,
                                           TreeItem *        item,
                                           std::string       new_sub_item_name)
    : SubByteReader(reader)
{
  if (item)
  {
    if (new_sub_item_name.empty())
      this->currentTreeLevel = item;
    else
      this->currentTreeLevel = new TreeItem(item, new_sub_item_name);
  }
  this->itemHierarchy.push(this->currentTreeLevel);
}

SubByteReaderLogging::SubByteReaderLogging(const ByteVector &inArr,
                                           TreeItem *        item,
                                           std::string       new_sub_item_name,
                                           size_t            inOffset)
    : SubByteReader(inArr, inOffset)
{
  if (item)
  {
    if (new_sub_item_name.empty())
      this->currentTreeLevel = item;
    else
      this->currentTreeLevel = new TreeItem(item, new_sub_item_name);
  }
  this->itemHierarchy.push(this->currentTreeLevel);
}

void SubByteReaderLogging::addLogSubLevel(const std::string name)
{
  assert(!name.empty());
  if (itemHierarchy.top() == nullptr)
    return;
  this->currentTreeLevel = new TreeItem(this->itemHierarchy.top(), name);
  this->itemHierarchy.push(this->currentTreeLevel);
}

void SubByteReaderLogging::removeLogSubLevel()
{
  if (itemHierarchy.size() <= 1)
    // Don't remove the root
    return;
  this->itemHierarchy.pop();
  this->currentTreeLevel = this->itemHierarchy.top();
}

uint64_t
SubByteReaderLogging::readBits(const std::string &symbolName, int numBits, const Options &options)
{
  try
  {
    auto [value, code] = SubByteReader::readBits(numBits);
    checkAndLog(this->currentTreeLevel, "u(v)", symbolName, options, value, code);
    return value;
  }
  catch (const std::exception &ex)
  {
    this->logExceptionAndThrowError(ex, std::to_string(numBits) + " bit symbol " + symbolName);
  }
}

bool SubByteReaderLogging::readFlag(const std::string &symbolName, const Options &options)
{
  try
  {
    auto [value, code] = SubByteReader::readBits(1);
    checkAndLog(this->currentTreeLevel, "u(1)", symbolName, options, value, code);
    return (value != 0);
  }
  catch (const std::exception &ex)
  {
    this->logExceptionAndThrowError(ex, "flag " + symbolName);
  }
}

uint64_t SubByteReaderLogging::readUEV(const std::string &symbolName, const Options &options)
{
  try
  {
    auto [value, code] = SubByteReader::readUE_V();
    checkAndLog(this->currentTreeLevel, "ue(v)", symbolName, options, value, code);
    return value;
  }
  catch (const std::exception &ex)
  {
    this->logExceptionAndThrowError(ex, "UEV symbol " + symbolName);
  }
}

int64_t SubByteReaderLogging::readSEV(const std::string &symbolName, const Options &options)
{
  try
  {
    auto [value, code] = SubByteReader::readSE_V();
    checkAndLog(this->currentTreeLevel, "se(v)", symbolName, options, value, code);
    return value;
  }
  catch (const std::exception &ex)
  {
    this->logExceptionAndThrowError(ex, "SEV symbol " + symbolName);
  }
}

uint64_t SubByteReaderLogging::readLEB128(const std::string &symbolName, const Options &options)
{
  try
  {
    auto [value, code] = SubByteReader::readLEB128();
    checkAndLog(this->currentTreeLevel, "leb128(v)", symbolName, options, value, code);
    return value;
  }
  catch (const std::exception &ex)
  {
    this->logExceptionAndThrowError(ex, "LEB128 symbol " + symbolName);
  }
}

uint64_t
SubByteReaderLogging::readNS(const std::string &symbolName, uint64_t maxVal, const Options &options)
{
  try
  {
    auto [value, code] = SubByteReader::readNS(maxVal);
    checkAndLog(this->currentTreeLevel, "ns(n)", symbolName, options, value, code);
    return value;
  }
  catch (const std::exception &ex)
  {
    this->logExceptionAndThrowError(ex, "ns symbol " + symbolName);
  }
}

int64_t
SubByteReaderLogging::readSU(const std::string &symbolName, unsigned nrBits, const Options &options)
{
  try
  {
    auto [value, code] = SubByteReader::readSU(nrBits);
    checkAndLog(this->currentTreeLevel, "su(n)", symbolName, options, value, code);
    return value;
  }
  catch (const std::exception &ex)
  {
    this->logExceptionAndThrowError(ex, "su symbol " + symbolName);
  }
}

ByteVector SubByteReaderLogging::readBytes(const std::string &symbolName,
                                           size_t             nrBytes,
                                           const Options &    options)
{
  try
  {
    if (!this->byte_aligned())
      throw std::logic_error("Trying to ready bytes while not byte aligned.");

    auto [value, code] = SubByteReader::readBytes(nrBytes);
    checkAndLog(this->currentTreeLevel, "u(8)", symbolName, options, value, code);
    return value;
  }
  catch (const std::exception &ex)
  {
    this->logExceptionAndThrowError(ex, " " + std::to_string(nrBytes) + " bytes.");
  }
}

void SubByteReaderLogging::logCalculatedValue(const std::string &symbolName,
                                              int64_t            value,
                                              const Options &    options)
{
  checkAndLog(this->currentTreeLevel, "Calc", symbolName, options, value, "");
}

void SubByteReaderLogging::logArbitrary(const std::string &symbolName,
                                        const std::string &value,
                                        const std::string &coding,
                                        const std::string &code,
                                        const std::string &meaning)
{
  new TreeItem(this->currentTreeLevel, symbolName, value, coding, code, meaning);
}

void SubByteReaderLogging::logExceptionAndThrowError(const std::exception &ex,
                                                     const std::string &   when)
{
  if (this->currentTreeLevel)
  {
    auto errorMessage = "Reading error " + std::string(ex.what());
    auto item         = new TreeItem(this->currentTreeLevel, "Error", "", "", "", errorMessage);
    item->setError();
  }
  throw std::logic_error("Error reading " + when);
}

} // namespace parser::reader