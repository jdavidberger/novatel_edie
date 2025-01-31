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
//! \file encoder.hpp
//! \brief Encode OEM messages.
////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------
// Recursive Inclusion
//-----------------------------------------------------------------------
#ifndef NOVATEL_ENCODER_HPP
#define NOVATEL_ENCODER_HPP

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include "decoders/common/api/common.hpp"
#include "decoders/common/api/jsonreader.hpp"
#include "decoders/novatel/api/common.hpp"
#include "decoders/novatel/api/message_decoder.hpp"

#include <nlohmann/json.hpp>
#include <logger/logger.hpp>

#include <cassert>

using nlohmann::json;

namespace novatel::edie::oem {

//============================================================================
//! \class Encoder
//! \brief Class to encode OEM messages.
//============================================================================
class Encoder
{
private:
   std::shared_ptr<spdlog::logger> pclMyLogger;
   uint32_t uiMyAbbrevAsciiIndentationLevel;
   JsonReader* pclMyMsgDb{ nullptr };
   MessageDefinition stMyRespDef;
   EnumDefinition* vMyRespDefns{ nullptr };
   EnumDefinition* vMyCommandDefns{ nullptr };
   EnumDefinition* vMyPortAddrDefns{ nullptr };
   EnumDefinition* vMyGPSTimeStatusDefns{ nullptr };

   // Inline buffer functions
   [[nodiscard]] bool PrintToBuffer(char** ppcBuffer_, uint32_t& uiBufferBytesRemaining_, const char* szFormat_, ...)
   {
      va_list args;
      va_start(args, szFormat_);
      const uint32_t uiBytesBuffered = vsnprintf(*ppcBuffer_, uiBufferBytesRemaining_, szFormat_, args);
      va_end(args);
      if (uiBufferBytesRemaining_ < uiBytesBuffered)
      {
         return false;
      }
      *ppcBuffer_ += uiBytesBuffered;
      uiBufferBytesRemaining_ -= uiBytesBuffered;
      return true;
   }

   template<typename T>
   void MakeConversionString(const FieldContainer& fc_, char* acNewConvertString)
   {
      const int32_t iBeforePoint = fc_.field_def->conversionBeforePoint;
      const int32_t iAfterPoint = fc_.field_def->conversionAfterPoint;
      const auto floatVal = std::get<T>(fc_.field_value);

      [[maybe_unused]] int result;

      if (floatVal == 0.0)
         result = snprintf(acNewConvertString, 7, "%%0.%df", iAfterPoint);
      else if ((iAfterPoint == 0) && (iBeforePoint == 0))
         result = snprintf(acNewConvertString, 7, "%%0.1f");
      else if (fabs(floatVal) > pow(10.0, iBeforePoint))
         result = snprintf(acNewConvertString, 7, "%%0.%de", iBeforePoint + iAfterPoint - 1);
      else if (fabs(floatVal) < pow(10.0, -iBeforePoint))
         result = snprintf(acNewConvertString, 7, "%%0.%de", iAfterPoint);
      else
         result = snprintf(acNewConvertString, 7, "%%0.%df", iAfterPoint);

      assert(0 <= result && result < 7);
   }

   template <typename T>
   [[nodiscard]] bool CopyToBuffer(unsigned char** ppucBuffer_, uint32_t& uiBufferBytesRemaining_, T* ptItem_)
   {
      uint32_t uiItemSize_;

      if constexpr (std::is_same<T, const char>())
         uiItemSize_ = static_cast<uint32_t>(strlen(ptItem_));
      else
         uiItemSize_ = sizeof(*ptItem_);

      if (uiBufferBytesRemaining_ < uiItemSize_)
      {
         return false;
      }
      memcpy(*ppucBuffer_, ptItem_, uiItemSize_);
      *ppucBuffer_ += uiItemSize_;
      uiBufferBytesRemaining_ -= uiItemSize_;
      return true;
   }

   [[nodiscard]] bool SetInBuffer(unsigned char** ppucBuffer_, uint32_t& uiBufferBytesRemaining_, int32_t iItem_, uint32_t uiItemSize_)
   {
      if (uiBufferBytesRemaining_ < uiItemSize_)
      {
         return false;
      }
      memset(*ppucBuffer_, iItem_, uiItemSize_);
      *ppucBuffer_ += uiItemSize_;
      uiBufferBytesRemaining_ -= uiItemSize_;
      return true;
   }

   // Enum util functions
   void InitEnumDefns();
   void CreateResponseMsgDefns();
   uint32_t MsgNameToMsgId(std::string sMsgName_) const;
   std::string MsgIdToMsgName(uint32_t uiMessageID_) const;
   std::string JsonHeaderToMsgName(const IntermediateHeader& stIntermediateHeader_) const;

   MsgFieldsVector* GetMsgDefFromCRC(const MessageDefinition* pclMessageDef_, uint32_t& uiMsgDefCRC_) const;

protected:
   // Encode binary
   [[nodiscard]] bool EncodeBinaryHeader     (const IntermediateHeader& stIntermediateHeader_,   unsigned char** ppcOutBuf_, uint32_t& uiMyBufferBytesRemaining_);
   [[nodiscard]] bool EncodeBinaryShortHeader(const IntermediateHeader& stIntermediateHeader_,   unsigned char** ppcOutBuf_, uint32_t& uiMyBufferBytesRemaining_);
   [[nodiscard]] bool EncodeBinaryBody       (const IntermediateMessage& stIntermediateMessage_, unsigned char** ppcOutBuf_, uint32_t& uiMyBufferBytesRemaining_, bool bFlatten_);
   [[nodiscard]] bool FieldToBinary          (const FieldContainer& fc_,                         unsigned char** ppcOutBuf_, uint32_t& uiMyBufferBytesRemaining_);

   // Encode ascii
   [[nodiscard]] bool EncodeAsciiHeader           (const IntermediateHeader& stIntermediateHeader_,         char** ppcOutBuf_, uint32_t& uiMyBufferBytesRemaining_);
   [[nodiscard]] bool EncodeAsciiShortHeader      (const IntermediateHeader& stIntermediateHeader_,         char** ppcOutBuf_, uint32_t& uiMyBufferBytesRemaining_);
   [[nodiscard]] bool EncodeAbbrevAsciiHeader     (const IntermediateHeader& stIntermediateHeader_,         char** ppcOutBuf_, uint32_t& uiMyBufferBytesRemaining_, bool bIsEmbeddedHeader_ = false);
   [[nodiscard]] bool EncodeAbbrevAsciiShortHeader(const IntermediateHeader& stIntermediateHeader_,         char** ppcOutBuf_, uint32_t& uiMyBufferBytesRemaining_);
   [[nodiscard]] bool EncodeAsciiBody             (const std::vector<FieldContainer>& vIntermediateFormat_, char** ppcOutBuf_, uint32_t& uiMyBufferBytesRemaining_);
   [[nodiscard]] bool EncodeAbbrevAsciiBody       (const std::vector<FieldContainer>& vIntermediateFormat_, char** ppcOutBuf_, uint32_t& uiMyBufferBytesRemaining_);
   [[nodiscard]] bool FieldToAscii                (const FieldContainer& fc_,                               char** ppcOutBuf_, uint32_t& uiMyBufferBytesRemaining_);

   // Encode JSON
   [[nodiscard]] bool EncodeJsonHeader     (const IntermediateHeader& stIntermediateHeader_,         char** ppcOutBuf_, uint32_t& uiMyBufferBytesRemaining_);
   [[nodiscard]] bool EncodeJsonShortHeader(const IntermediateHeader& stIntermediateHeader_,         char** ppcOutBuf_, uint32_t& uiMyBufferBytesRemaining_);
   [[nodiscard]] bool EncodeJsonBody       (const std::vector<FieldContainer>& vIntermediateFormat_, char** ppcOutBuf_, uint32_t& uiMyBufferBytesRemaining_);
   [[nodiscard]] bool FieldToJson          (const FieldContainer& fc_,                               char** ppcOutBuf_, uint32_t& uiMyBufferBytesRemaining_);

public:
   //----------------------------------------------------------------------------
   //! \brief A constructor for the Encoder class.
   //
   //! \param[in] pclJsonDb_ A pointer to a JsonReader object.  Defaults to nullptr.
   //----------------------------------------------------------------------------
   Encoder(JsonReader* pclJsonDb_ = nullptr);

   //----------------------------------------------------------------------------
   //! \brief Load a JsonReader object.
   //
   //! \param[in] pclJsonDb_ A pointer to a JsonReader object.
   //----------------------------------------------------------------------------
   void
   LoadJsonDb(JsonReader* pclJsonDb_);

   //----------------------------------------------------------------------------
   //! \brief Get the internal logger.
   //
   //! \return A shared_ptr to the spdlog::logger.
   //----------------------------------------------------------------------------
   std::shared_ptr<spdlog::logger>
   GetLogger() const;

   //----------------------------------------------------------------------------
   //! \brief Set the level of detail produced by the internal logger.
   //
   //! \param[in] eLevel_  The logging level to enable.
   //----------------------------------------------------------------------------
   void
   SetLoggerLevel(spdlog::level::level_enum eLevel_);

   //----------------------------------------------------------------------------
   //! \brief Shutdown the internal logger.
   //----------------------------------------------------------------------------
   static void
   ShutdownLogger();

   //----------------------------------------------------------------------------
   //! \brief Encode an OEM message from the provided intermediate structures.
   //
   //! \param[out] ppucEncodeBuffer_ A pointer to the buffer to return the encoded
   //! message to.
   //! \param[in] uiEncodeBufferSize_ The length of ppcEncodeBuffer_.
   //! \param[in] stHeader_ A reference to the decoded header intermediate.
   //! This must be populated by the HeaderDecoder.
   //! \param[in] stMessage_ A reference to the decoded message intermediate.
   //! This must be populated by the MessageDecoder.
   //! \param[out] stMessageData_ A reference to a MessageDataStruct to be
   //! populated by the encoder.
   //! \param[in] stMetaData_ A reference to a populated MetaDataStruct
   //! containing relevant information about the decoded log.
   //! This must be populated by the Framer and HeaderDecoder.
   //! \param[in] eEncodeFormat_ The format to encode the message to.
   //
   //! \return An error code describing the result of encoding.
   //!   SUCCESS: The operation was successful.
   //!   NULL_PROVIDED: ppucEncodeBuffer_ either points to a null pointer or is
   //! a null pointer itself.
   //!   NO_DATABASE: No database was ever loaded into this component.
   //!   BUFFER_FULL: An attempt was made to write bytes to the provided buffer,
   //! but the buffer is already full or could not write the bytes without
   //! over-running.
   //!   FAILURE: stMessageData_.pucMessageHeader was not correctly set inside
   //! this function.  This should not happen.
   //!   UNSUPPORTED: eEncodeFormat_ contains a format that is not supported for
   //! encoding.
   //----------------------------------------------------------------------------
   [[nodiscard]] STATUS
   Encode(unsigned char** ppucEncodeBuffer_, uint32_t uiEncodeBufferSize_, IntermediateHeader& stHeader_, IntermediateMessage& stMessage_, MessageDataStruct& stMessageData_, MetaDataStruct& stMetaData_, ENCODEFORMAT eEncodeFormat_);

   //----------------------------------------------------------------------------
   //! \brief Encode an OEM message header from the provided intermediate header.
   //
   //! \param[out] ppucEncodeBuffer_ A pointer to the buffer to return the encoded
   //! message to.
   //! \param[in] uiEncodeBufferSize_ The length of ppcEncodeBuffer_.
   //! \param[in] stHeader_ A reference to the decoded header intermediate.
   //! This must be populated by the HeaderDecoder.
   //! \param[out] stMessageData_ A reference to a MessageDataStruct to be
   //! populated by the encoder.
   //! \param[in] stMetaData_ A reference to a populated MetaDataStruct
   //! containing relevant information about the decoded log.
   //! This must be populated by the Framer and HeaderDecoder.
   //! \param[in] eEncodeFormat_ The format to encode the message to.
   //! \param[in] bIsEmbeddedHeader_ This header is embedded in an RXCONFIG
   //! message, and should be treated as such.  Is a default argument and is
   //! defaulted as false.
   //
   //! \return An error code describing the result of encoding.
   //!   SUCCESS: The operation was successful.
   //!   NULL_PROVIDED: ppucEncodeBuffer_ either points to a null pointer or is
   //! a null pointer itself.
   //!   NO_DATABASE: No database was ever loaded into this component.
   //!   BUFFER_FULL: An attempt was made to write bytes to the provided buffer,
   //! but the buffer is already full or could not write the bytes without
   //! over-running.
   //!   UNSUPPORTED: eEncodeFormat_ contains a format that is not supported for
   //! encoding.
   //----------------------------------------------------------------------------
   [[nodiscard]] STATUS
   EncodeHeader(unsigned char** ppucEncodeBuffer_, uint32_t uiEncodeBufferSize_, IntermediateHeader& stHeader_, MessageDataStruct& stMessageData_, MetaDataStruct& stMetaData_, ENCODEFORMAT eEncodeFormat_, bool bIsEmbeddedHeader_ = false);

   //----------------------------------------------------------------------------
   //! \brief Encode an OEM message body from the provided intermediate message.
   //
   //! \param[out] ppucEncodeBuffer_ A pointer to the buffer to return the encoded
   //! message to.
   //! \param[in] uiEncodeBufferSize_ The length of ppcEncodeBuffer_.
   //! \param[in] stMessage_ A reference to the decoded message intermediate.
   //! This must be populated by the MessageDecoder.
   //! \param[out] stMessageData_ A reference to a MessageDataStruct to be
   //! populated by the encoder.
   //! \param[in] stMetaData_ A reference to a populated MetaDataStruct
   //! containing relevant information about the decoded log.
   //! This must be populated by the Framer and HeaderDecoder.
   //! \param[in] eEncodeFormat_ The format to encode the message to.
   //
   //! \return An error code describing the result of encoding.
   //!   SUCCESS: The operation was successful.
   //!   NULL_PROVIDED: ppucEncodeBuffer_ either points to a null pointer or is
   //! a null pointer itself.
   //!   NO_DATABASE: No database was ever loaded into this component.
   //!   BUFFER_FULL: An attempt was made to write bytes to the provided buffer,
   //! but the buffer is already full or could not write the bytes without
   //! over-running.
   //!   FAILURE: stMessageData_.pucMessageHeader is a null pointer.
   //!   UNSUPPORTED: eEncodeFormat_ contains a format that is not supported for
   //! encoding.
   //----------------------------------------------------------------------------
   [[nodiscard]] STATUS
   EncodeBody(unsigned char** ppucEncodeBuffer_, uint32_t uiEncodeBufferSize_, IntermediateMessage& stMessage_, MessageDataStruct& stMessageData_, MetaDataStruct& stMetaData_, ENCODEFORMAT eEncodeFormat_);
};
}
#endif // NOVATEL_ENCODER_HPP
