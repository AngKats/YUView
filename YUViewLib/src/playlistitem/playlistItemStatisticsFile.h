/*  This file is part of YUView - The YUV player with advanced analytics toolset
*   <https://github.com/IENT/YUView>
*   Copyright (C) 2015  Institut für Nachrichtentechnik, RWTH Aachen University, GERMANY
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

#include <QBasicTimer>
#include <QFuture>
#include "filesource/FileSource.h"
#include "playlistItem.h"
#include "statistics/statisticHandler.h"

class playlistItemStatisticsFile : public playlistItem
{
  Q_OBJECT

public:

  playlistItemStatisticsFile(const QString &itemNameOrFileName);
  virtual ~playlistItemStatisticsFile();

  virtual void savePlaylist(QDomElement &root, const QDir &playlistDir) const override;

  virtual QSize getSize() const override { return statSource.getFrameSize(); }
  
  // Return the info title and info list to be shown in the fileInfo groupBox.
  virtual infoData getInfo() const override;

  virtual void drawItem(QPainter *painter, int frameIdx, double zoomFactor, bool drawRawData) override;

  // ------ Statistics ----

  // Do we need to load the statistics first?
  virtual itemLoadingState needsLoading(int frameIdx, bool loadRawdata) override { Q_UNUSED(loadRawdata); return statSource.needsLoading(frameIdx); }
  // Load the statistics for the given frame. Emit signalItemChanged(true,false) when done. Always called from a thread.
  virtual void loadFrame(int frameIdx, bool playback, bool loadRawdata, bool emitSignals=true) override;
  // Are statistics currently being loaded?
  virtual bool isLoading() const override { return isStatisticsLoading; }

  // Override from playlistItem. Return the statistics values under the given pixel position.
  virtual ValuePairListSets getPixelValues(const QPoint &pixelPos, int frameIdx) override { Q_UNUSED(frameIdx); return ValuePairListSets("Stats",statSource.getValuesAt(pixelPos)); }

  // A statistics file source of course provides statistics
  virtual statisticHandler *getStatisticsHandler() override { return &statSource; }

  // ----- Detection of source/file change events -----
  virtual bool isSourceChanged()  override { return file.isFileChanged(); }
  virtual void updateSettings()   override { file.updateFileWatchSetting(); statSource.updateSettings(); }

protected:

  // Overload from playlistItem. Create a properties widget custom to the statistics item
  // and set propertiesWidget to point to it.
  virtual void createPropertiesWidget() override;

  // Get the tag/name which is used when saving the item to a playlist
  virtual QString getPlaylistTag() const = 0;

  // The statistics source
  statisticHandler statSource;

  // Is the loadFrame function currently loading?
  bool isStatisticsLoading;

  QFuture<void> backgroundParserFuture;
  double backgroundParserProgress;
  bool cancelBackgroundParser;
  // A timer is used to frequently update the status of the background process (every second)
  QBasicTimer timer;
  virtual void timerEvent(QTimerEvent *event) override; // Overloaded from QObject. Called when the timer fires.

  // Set if the file is sorted by POC and the types are 'random' within this POC (true)
  // or if the file is sorted by typeID and the POC is 'random'
  bool fileSortedByPOC;
  // If not -1, this gives the POC in which the parser noticed a block that was outside of the "frame"
  int blockOutsideOfFrame_idx;
  // The maximum POC number in the file (as far as we know)
  int maxPOC;

  // If an error occurred while parsing, this error text will be set and can be shown
  QString parsingError;

  FileSource file;

  int currentDrawnFrameIdx;
};
