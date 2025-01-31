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
//! \file fileparser.hpp
//! \brief FileParser class that manages parsing an input file stream.
////////////////////////////////////////////////////////////////////////

//-----------------------------------------------------------------------
// Recursive Inclusion
//-----------------------------------------------------------------------
#ifndef NOVATEL_FILEPARSER_HPP
#define NOVATEL_FILEPARSER_HPP

//-----------------------------------------------------------------------
// Includes
//-----------------------------------------------------------------------
#include <unordered_map>
#include "decoders/common/api/common.hpp"
#include "decoders/novatel/api/parser.hpp"
#include "hw_interface/stream_interface/api/inputfilestream.hpp"
#include "hw_interface/stream_interface/api/outputfilestream.hpp"

namespace novatel::edie::oem {

//============================================================================
//! \class FileParser
//! \brief Frame, decode and re-encode OEM logs from an InputFileStream.
//============================================================================
class FileParser
{
   //! TODO: Manage copy/move/assignment constructors better.
   //! NOTE: The following constructors prevent this class from ever being
   //! constructed from a copy, move or assignment.
   FileParser(const FileParser&) = delete;
   FileParser(const FileParser&&) = delete;
   FileParser& operator=(const FileParser&) = delete;

private:
   std::shared_ptr<spdlog::logger> pclMyLogger;

   Parser clMyParser;
   InputFileStream* pclMyInputStream;
   StreamReadStatus stMyStreamReadStatus;
   ReadDataStructure stMyReadData;
   unsigned char* const pcMyStreamReadBuffer;

   [[nodiscard]] bool ReadStream();

public:
   //----------------------------------------------------------------------------
   //! \brief A constructor for the FileParser class.
   //
   //! \param[in] sDbPath_ Filepath to a JSON message DB.
   //----------------------------------------------------------------------------
   FileParser(const std::string sDbPath_);

   //----------------------------------------------------------------------------
   //! \brief A constructor for the FileParser class.
   //
   //! \param[in] sDbPath_ Filepath to a JSON message DB.
   //----------------------------------------------------------------------------
   FileParser(const std::u32string sDbPath_);

   //----------------------------------------------------------------------------
   //! \brief A constructor for the FileParser class.
   //
   //! \param[in] pclJsonDb_ A pointer to a JsonReader object.  Defaults to nullptr.
   //----------------------------------------------------------------------------
   FileParser(JsonReader* pclJsonDb_ = nullptr);

   //----------------------------------------------------------------------------
   //! \brief A destructor for the FileParser class.
   //----------------------------------------------------------------------------
   ~FileParser();

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
   GetLogger();

   //----------------------------------------------------------------------------
   //! \brief Set the level of detail produced by the internal components'
   //! internal loggers
   //
   //! \param [in] eLevel_ The logging level to enable.
   //! \param [in] sFileName_ The logging level to enable.
   //----------------------------------------------------------------------------
   void
   EnableFramerDecoderLogging(spdlog::level::level_enum eLevel_ = spdlog::level::debug, std::string sFileName_ = "edie.log");

   //----------------------------------------------------------------------------
   //! \brief Set the level of detail produced by the internal logger.
   //
   //! \param[in] eLevel_ The logging level to enable.
   //----------------------------------------------------------------------------
   void
   SetLoggerLevel(spdlog::level::level_enum eLevel_);

   //----------------------------------------------------------------------------
   //! \brief Shutdown the internal logger.
   //----------------------------------------------------------------------------
   void
   ShutdownLogger();

   //----------------------------------------------------------------------------
   //! \brief Set the abbreviated ASCII response option.
   //
   //! \param [in] bIgnoreAbbreivatedAsciiResponses_ true to ignore abbreivated
   //! ASCII responses.
   //----------------------------------------------------------------------------
   void
   SetIgnoreAbbreviatedAsciiResponses(bool bIgnoreAbbreivatedAsciiResponses_);

   //----------------------------------------------------------------------------
   //! \brief Get the abbreviated ASCII response option.
   //
   //! \return The current option for ignoring abbreviated ASCII responses.
   //----------------------------------------------------------------------------
   bool
   GetIgnoreAbbreviatedAsciiResponses(void);

   //----------------------------------------------------------------------------
   //! \brief Get the percent of the InputFileStream that has been parsed.
   //
   //! \return An integer percentage.
   //----------------------------------------------------------------------------
   uint32_t
   GetPercentRead();

   //----------------------------------------------------------------------------
   //! \brief Set the decompression option for RANGECMP messages.
   //
   //! \param [in] bDecompressRangeCmp_ true to decompress RANGECMP messages.
   //----------------------------------------------------------------------------
   void
   SetDecompressRangeCmp(bool bDecompressRangeCmp_);

   //----------------------------------------------------------------------------
   //! \brief Get the decompression option for RANGECMP messages.
   //
   //! \return The current option for decompressing RANGECMP messages.
   //----------------------------------------------------------------------------
   bool
   GetDecompressRangeCmp();

   //----------------------------------------------------------------------------
   //! \brief Set the return option for unknown bytes.
   //
   //! \param [in] bReturnUnknownBytes_ true to return unknown bytes.
   //----------------------------------------------------------------------------
   void
   SetReturnUnknownBytes(bool bReturnUnknownBytes_);

   //----------------------------------------------------------------------------
   //! \brief Get the return option for unknown bytes.
   //
   //! \return The current option for returning unknown bytes.
   //----------------------------------------------------------------------------
   bool
   GetReturnUnknownBytes();

   //----------------------------------------------------------------------------
   //! \brief Set the encode format for messages.
   //
   //! \param [in] eFormat_ the encode format for future messages.
   //----------------------------------------------------------------------------
   void
   SetEncodeFormat(ENCODEFORMAT eFormat_);

   //----------------------------------------------------------------------------
   //! \brief Get the encode format for messages.
   //
   //! \return The current encode format for messages.
   //----------------------------------------------------------------------------
   ENCODEFORMAT
   GetEncodeFormat();

   //----------------------------------------------------------------------------
   //! \brief Set the Filter for the FileParser.
   //
   //! \param [in] pclFilter_ A pointer to an OEM message Filter object.
   //----------------------------------------------------------------------------
   void
   SetFilter(Filter* pclFilter_);

   //----------------------------------------------------------------------------
   //! \brief Get the config for the FileParser.
   //
   //! \return A pointer to the FileParser's OEM message Filter object.
   //----------------------------------------------------------------------------
   Filter*
   GetFilter();

   //----------------------------------------------------------------------------
   //! \brief Set the InputFileStream for the FileParser.
   //
   //! \param [in] pclInputStream_ A pointer to the input stream.
   //
   //! \return A boolean describing if the operation was successful
   //----------------------------------------------------------------------------
   [[nodiscard]] bool
   SetStream(InputFileStream* pclInputStream_);

   //----------------------------------------------------------------------------
   //! \brief Read a log from the FileParser.
   //
   //! \param [out] stMessageData_ A reference to a MessageDataStruct to be
   //! populated by the FileParser.
   //! \param [out] stMetaData_ A reference to a MetaDataStruct to be populated
   //! by the FileParser.
   //
   //! \return An error code describing the result of parsing.
   //!   SUCCESS: A message was framed, decoded, filtered, encoded and stored
   //! internally, pointed to by stMessageData_.
   //!   UNKNOWN: A message could not be found and unknown bytes were returned
   //! if requested in the ParserConfigStruct given to SetConfig().
   //!   FILE_EMPTY: There are no more bytes to parse in the file provided.
   //----------------------------------------------------------------------------
   [[nodiscard]] STATUS
   Read(MessageDataStruct& stMessageData_, MetaDataStruct& stMetaData_);

   //----------------------------------------------------------------------------
   //! \brief Reset the InputFileStream, and flush all bytes from the internal
   //! FileParser.
   //
   //! \return A boolean describing if the operation was successful.
   //----------------------------------------------------------------------------
   bool
   Reset();

   //----------------------------------------------------------------------------
   //! \brief Flush all bytes from the internal Parser.
   //
   //! \param [in] pcBuffer_ A buffer to contain flushed bytes, if desired.
   //! Defaults to nullptr.
   //! \param [in] ulBufferSize_ The length of ulBufferSize_, if provided.
   //
   //! \return The number of bytes flushed from the internal Parser.
   //----------------------------------------------------------------------------
   uint32_t
   Flush(unsigned char* pucBuffer_ = nullptr, uint32_t uiBufferSize_ = Parser::uiPARSER_INTERNAL_BUFFER_SIZE);

   //----------------------------------------------------------------------------
   //! \brief Get a pointer to the current framed log raw data.
   //
   //! \return A pointer to the internal Parser's internal encode buffer.
   //----------------------------------------------------------------------------
   unsigned char*
   GetInternalBuffer();
};

}
#endif // NOVATEL_FILEPARSER_HPP
