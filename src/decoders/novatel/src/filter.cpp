////////////////////////////////////////////////////////////////////////
//
// COPYRIGHT NovAtel Inc, 2022. All rights reserved.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.
//
////////////////////////////////////////////////////////////////////////
//                            DESCRIPTION
//
//! \file filter.cpp
//! \brief Filter messages based on information contained in the
//! MetaDataStruct
////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include "decoders/novatel/api/filter.hpp"

using namespace novatel::edie;
using namespace novatel::edie::oem;

// -------------------------------------------------------------------------------------------------------
Filter::Filter()
{
   pclMyLogger = Logger().RegisterLogger("novatel_filter");

   ClearFilters();

   pclMyLogger->debug("Filter initialized");
}

// -------------------------------------------------------------------------------------------------------
std::shared_ptr<spdlog::logger>
Filter::GetLogger()
{
   return pclMyLogger;
}

// -------------------------------------------------------------------------------------------------------
void
Filter::SetLoggerLevel(spdlog::level::level_enum eLevel_)
{
   pclMyLogger->set_level(eLevel_);
}

// -------------------------------------------------------------------------------------------------------
void
Filter::ShutdownLogger()
{
   Logger::Shutdown();
}

// -------------------------------------------------------------------------------------------------------
void Filter::PushUnique(bool (Filter::* filter)(const MetaDataStruct&))
{
   if (std::find(vMyFilterFunctions.begin(), vMyFilterFunctions.end(), filter) == vMyFilterFunctions.end())
   {
      vMyFilterFunctions.push_back(filter);
   }
}

// -------------------------------------------------------------------------------------------------------
void
Filter::SetIncludeLowerTimeBound(uint32_t uiWeek_, double dSec_)
{
   bMyFilterLowerTime = true;
   uiMyLowerWeek = uiWeek_;
   uiMyLowerMSec = static_cast<uint32_t>(dSec_ * 1000.0);
   PushUnique(&Filter::FilterTime);
}

// -------------------------------------------------------------------------------------------------------
void
Filter::SetIncludeUpperTimeBound(uint32_t uiWeek_, double dSec_)
{
   bMyFilterUpperTime = true;
   uiMyUpperWeek = uiWeek_;
   uiMyUpperMSec = static_cast<uint32_t>(dSec_ * 1000.0);
   PushUnique(&Filter::FilterTime);
}

// -------------------------------------------------------------------------------------------------------
void
Filter::InvertTimeFilter(bool bInvert_)
{
   bMyInvertTimeFilter = bInvert_;
}

// -------------------------------------------------------------------------------------------------------
void
Filter::SetIncludeDecimation(double dPeriodSec_)
{
   bMyDecimate = true;
   uiMyDecimationPeriodMS = static_cast<uint32_t>(dPeriodSec_ * 1000.0); // Convert provided seconds to MSec
   PushUnique(&Filter::FilterDecimation);
}

// -------------------------------------------------------------------------------------------------------
void
Filter::InvertDecimationFilter(bool bInvert_)
{
   bMyInvertDecimation = bInvert_;
}

// -------------------------------------------------------------------------------------------------------
void
Filter::IncludeTimeStatus(TIME_STATUS eTimeStatus_)
{
   vMyTimeStatusFilters.push_back(eTimeStatus_);
   PushUnique(&Filter::FilterTimeStatus);
}

// -------------------------------------------------------------------------------------------------------
void
Filter::IncludeTimeStatus(std::vector<TIME_STATUS> vTimeStatuses_)
{
   vMyTimeStatusFilters.insert(vMyTimeStatusFilters.end(), vTimeStatuses_.begin(), vTimeStatuses_.end());
   PushUnique(&Filter::FilterTimeStatus);
}

// -------------------------------------------------------------------------------------------------------
void
Filter::InvertTimeStatusFilter(bool bInvert_)
{
   bMyInvertTimeStatusFilter = bInvert_;
}

// -------------------------------------------------------------------------------------------------------
void
Filter::IncludeMessageId(uint32_t uiId_, HEADERFORMAT eFormat_, MEASUREMENT_SOURCE eSource_)
{
   vMyMessageIdFilters.emplace_back(uiId_, eFormat_, eSource_);
   PushUnique(&Filter::FilterMessageId);
}

// -------------------------------------------------------------------------------------------------------
void
Filter::IncludeMessageId(std::vector<std::tuple<uint32_t, HEADERFORMAT, MEASUREMENT_SOURCE>> &vIds_)
{
   vMyMessageIdFilters.insert(vMyMessageIdFilters.end(), vIds_.begin(), vIds_.end());
   PushUnique(&Filter::FilterMessageId);
}

// -------------------------------------------------------------------------------------------------------
void
Filter::InvertMessageIdFilter(bool bInvert_)
{
   bMyInvertMessageIdFilter = bInvert_;
}

// -------------------------------------------------------------------------------------------------------
void
Filter::IncludeMessageName(std::string szMsgName_, HEADERFORMAT eFormat_, MEASUREMENT_SOURCE eSource_)
{
   vMyMessageNameFilters.emplace_back(szMsgName_, eFormat_, eSource_);
   PushUnique(&Filter::FilterMessage);
}

// -------------------------------------------------------------------------------------------------------
void
Filter::IncludeMessageName(std::vector<std::tuple<std::string, HEADERFORMAT, MEASUREMENT_SOURCE>> &vNames_)
{
   vMyMessageNameFilters.insert(vMyMessageNameFilters.end(), vNames_.begin(), vNames_.end());
   PushUnique(&Filter::FilterMessage);
}

// -------------------------------------------------------------------------------------------------------
void
Filter::InvertMessageNameFilter(bool bInvert_)
{
   bMyInvertMessageNameFilter = bInvert_;
}

// -------------------------------------------------------------------------------------------------------
void
Filter::IncludeNMEAMessages(bool bIncludeNMEA_)
{
   bMyIncludeNMEA_ = bIncludeNMEA_;
}

// -------------------------------------------------------------------------------------------------------
void
Filter::ClearFilters()
{
   vMyTimeStatusFilters.clear();
   bMyInvertTimeStatusFilter = false;

   vMyMessageIdFilters.clear();
   bMyInvertMessageIdFilter = false;

   vMyMessageNameFilters.clear();
   bMyInvertMessageNameFilter = false;

   uiMyLowerWeek = 0;
   uiMyLowerMSec = 0;
   uiMyUpperWeek = 0;
   uiMyUpperMSec = 0;
   bMyFilterLowerTime = false;
   bMyFilterUpperTime = false;
   bMyInvertTimeFilter = false;

   uiMyDecimationPeriodMS = 0;
   bMyDecimate = false;
   bMyInvertDecimation = false;

   bMyIncludeNMEA_ = false;
   vMyFilterFunctions.clear();
}

// -------------------------------------------------------------------------------------------------------
bool
Filter::FilterTime(const MetaDataStruct& stMetaData_)
{
   const auto usMetaDataWeek = static_cast<uint32_t>(stMetaData_.usWeek);
   const auto usMetaDataMilliseconds = static_cast<uint32_t>(stMetaData_.dMilliseconds);

   if (bMyInvertTimeFilter)
   {
      const bool bAboveLowerTime = usMetaDataWeek > uiMyLowerWeek
         || (usMetaDataWeek == uiMyLowerWeek && usMetaDataMilliseconds >= uiMyLowerMSec);
      const bool bBelowUpperTime = usMetaDataWeek < uiMyUpperWeek
         || (usMetaDataWeek == uiMyUpperWeek && usMetaDataMilliseconds <= uiMyUpperMSec);

      return bMyFilterLowerTime && bMyFilterUpperTime
         ? !(bAboveLowerTime && bBelowUpperTime)
         : !((bAboveLowerTime && bMyFilterLowerTime) || (bBelowUpperTime && bMyFilterUpperTime));
   }

   const bool bBelowLowerTime = usMetaDataWeek < uiMyLowerWeek
      || (usMetaDataWeek == uiMyLowerWeek && usMetaDataMilliseconds < uiMyLowerMSec);
   const bool bAboveUpperTime = usMetaDataWeek > uiMyUpperWeek
      || (usMetaDataWeek == uiMyUpperWeek && usMetaDataMilliseconds > uiMyUpperMSec);

   return !((bMyFilterLowerTime && bBelowLowerTime) || (bMyFilterUpperTime && bAboveUpperTime));
}

// -------------------------------------------------------------------------------------------------------
bool
Filter::FilterTimeStatus(const MetaDataStruct& stMetaData_)
{
   return vMyTimeStatusFilters.empty()
      || bMyInvertTimeStatusFilter == (vMyTimeStatusFilters.end() == std::find(vMyTimeStatusFilters.begin(), vMyTimeStatusFilters.end(), stMetaData_.eTimeStatus));
}

// -------------------------------------------------------------------------------------------------------
bool
Filter::FilterMessageId(const MetaDataStruct& stMetaData_)
{
   if (vMyMessageIdFilters.empty())
   {
      return true;
   }

   uint32_t uiMessageId = static_cast<uint32_t>(stMetaData_.usMessageID);
   HEADERFORMAT eFormat = stMetaData_.eFormat;
   MEASUREMENT_SOURCE eSource = stMetaData_.eMeasurementSource;

   const auto isMessageIDFilterMatch = [&uiMessageId, eFormat, eSource](const std::tuple<uint32_t, HEADERFORMAT, MEASUREMENT_SOURCE>& elem) {
      return uiMessageId == std::get<0>(elem) && HEADERFORMAT::ALL == std::get<1>(elem) && eSource == std::get<2>(elem);
   };

   return bMyInvertMessageIdFilter == (vMyMessageIdFilters.end() == std::find_if(vMyMessageIdFilters.begin(), vMyMessageIdFilters.end(), isMessageIDFilterMatch)
                                    && vMyMessageIdFilters.end() == std::find   (vMyMessageIdFilters.begin(), vMyMessageIdFilters.end(), std::make_tuple(uiMessageId, eFormat, eSource)));
}

// -------------------------------------------------------------------------------------------------------
bool
Filter::FilterMessage(const MetaDataStruct& stMetaData_)
{
   if (vMyMessageNameFilters.empty())
   {
      return true;
   }

   std::string szMessageName = stMetaData_.MessageName();
   HEADERFORMAT eFormat = stMetaData_.eFormat;
   MEASUREMENT_SOURCE eSource = stMetaData_.eMeasurementSource;

   const auto isMessageNameFilterMatch = [&szMessageName, eFormat, eSource](const std::tuple<std::string, HEADERFORMAT, MEASUREMENT_SOURCE>& elem) {
      return szMessageName == std::get<0>(elem) && HEADERFORMAT::ALL == std::get<1>(elem) && eSource == std::get<2>(elem);
   };

   return bMyInvertMessageNameFilter == (vMyMessageNameFilters.end() == std::find_if(vMyMessageNameFilters.begin(), vMyMessageNameFilters.end(), isMessageNameFilterMatch)
                                      && vMyMessageNameFilters.end() == std::find   (vMyMessageNameFilters.begin(), vMyMessageNameFilters.end(), std::make_tuple(szMessageName, eFormat, eSource)));
}

// -------------------------------------------------------------------------------------------------------
bool
Filter::FilterDecimation(const MetaDataStruct& stMetaData_)
{
   return !bMyDecimate || bMyInvertDecimation == static_cast<bool>(static_cast<uint32_t>(stMetaData_.dMilliseconds) % uiMyDecimationPeriodMS);
}

// -------------------------------------------------------------------------------------------------------
bool
Filter::DoFiltering(MetaDataStruct& stMetaData_)
{
   if (stMetaData_.eFormat == HEADERFORMAT::UNKNOWN)
   {
      return false;
   }
   if (stMetaData_.eFormat == HEADERFORMAT::NMEA)
   {
      return bMyIncludeNMEA_;
   }

   for (uint64_t ullIndex = 0; ullIndex < vMyFilterFunctions.size(); ullIndex++)
   {
      if (!(this->*vMyFilterFunctions.at(ullIndex))(stMetaData_))
      {
         return false;
      }
   }

   return true;
}
