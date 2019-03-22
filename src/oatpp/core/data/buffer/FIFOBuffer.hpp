/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#ifndef oatpp_data_buffer_FIFOBuffer_hpp
#define oatpp_data_buffer_FIFOBuffer_hpp

#include "oatpp/core/data/stream/Stream.hpp"
#include "oatpp/core/data/IODefinitions.hpp"
#include "oatpp/core/async/Coroutine.hpp"
#include "oatpp/core/concurrency/SpinLock.hpp"

namespace oatpp { namespace data { namespace buffer {

/**
 * FIFO operations over the buffer
 * !FIFOBuffer is NOT an IOStream despite having similar APIs!
 */
class FIFOBuffer {
private:
  p_char8 m_buffer;
  v_io_size m_bufferSize;
  data::v_io_size m_readPosition;
  data::v_io_size m_writePosition;
  bool m_canRead;
public:

  /**
   * Constructor.
   * @param buffer - pointer to buffer used for reads/writes.
   * @param bufferSize - buffer size.
   * @param readPosition - initial read position in buffer.
   * @param writePosition - initial write position in buffer.
   * @param canRead - flag to resolve ambiguity when readPosition == writePosition. If(readPosition == writePosition && canRead) then
   * &l:FIFOBuffer::availableToRead (); returns buffer size, and &l:FIFOBuffer::availableToWrite (); returns 0.
   */
  FIFOBuffer(void* buffer, v_io_size bufferSize,
             data::v_io_size readPosition = 0, data::v_io_size writePosition = 0,
             bool canRead = false);

  /**
   * Set read and write positions in buffer.
   * @param readPosition - read position in buffer.
   * @param writePosition - write position in buffer.
   * @param canRead - flag to resolve ambiguity when readPosition == writePosition. If(readPosition == writePosition && canRead) then
   * &l:FIFOBuffer::availableToRead (); returns buffer size, and &l:FIFOBuffer::availableToWrite (); returns 0.
   */
  void setBufferPosition(data::v_io_size readPosition, data::v_io_size writePosition, bool canRead);

  /**
   * Amount of bytes currently available to read from buffer.
   * @return &id:oatpp::data::v_io_size;.
   */
  data::v_io_size availableToRead() const;

  /**
   * Amount of buffer space currently available for data writes.
   * @return &id:oatpp::data::v_io_size;.
   */
  data::v_io_size availableToWrite() const;

  /**
   * Get FIFOBuffer size.
   * @return - FIFOBuffer size.
   */
  data::v_io_size getBufferSize() const;

  /**
   * read up to count bytes from the buffer to data
   * @param data
   * @param count
   * @return [1..count], IOErrors.
   */
  data::v_io_size read(void *data, data::v_io_size count);

  /**
   * write up to count bytes from data to buffer
   * @param data
   * @param count
   * @return [1..count], IOErrors.
   */
  data::v_io_size write(const void *data, data::v_io_size count);

  /**
   * call read and then write bytes read to output stream
   * @param stream
   * @param count
   * @return [1..count], IOErrors.
   */
  data::v_io_size readAndWriteToStream(data::stream::OutputStream& stream, data::v_io_size count);

  /**
   * call stream.read() and then write bytes read to buffer
   * @param stream
   * @param count
   * @return
   */
  data::v_io_size readFromStreamAndWrite(data::stream::InputStream& stream, data::v_io_size count);

  /**
   * flush all availableToRead bytes to stream
   * @param stream
   * @return
   */
  data::v_io_size flushToStream(data::stream::OutputStream& stream);

  /**
   * flush all availableToRead bytes to stream in asynchronous manner
   * @param parentCoroutine
   * @param actionOnFinish
   * @return
   */
  oatpp::async::Action flushToStreamAsync(oatpp::async::AbstractCoroutine* parentCoroutine,
                                          const oatpp::async::Action& actionOnFinish,
                                          data::stream::OutputStream& stream);

  
};

/**
 * Same as FIFOBuffer + synchronization with SpinLock
 */
class SynchronizedFIFOBuffer {
private:
  FIFOBuffer m_fifo;
  oatpp::concurrency::SpinLock::Atom m_atom;
public:

  /**
   * Constructor.
   * @param buffer - pointer to buffer used for reads/writes.
   * @param bufferSize - buffer size.
   * @param readPosition - initial read position in buffer.
   * @param writePosition - initial write position in buffer.
   * @param canRead - flag to resolve ambiguity when readPosition == writePosition. If(readPosition == writePosition && canRead) then
   * &l:SynchronizedFIFOBuffer::availableToRead (); returns buffer size, and &l:SynchronizedFIFOBuffer::availableToWrite (); returns 0.
   */
  SynchronizedFIFOBuffer(void* buffer, v_io_size bufferSize,
                         data::v_io_size readPosition = 0, data::v_io_size writePosition = 0,
                         bool canRead = false);

  /**
   * Set read and write positions in buffer.
   * @param readPosition - read position in buffer.
   * @param writePosition - write position in buffer.
   * @param canRead - flag to resolve ambiguity when readPosition == writePosition. If(readPosition == writePosition && canRead) then
   * &l:SynchronizedFIFOBuffer::availableToRead (); returns buffer size, and &l:SynchronizedFIFOBuffer::availableToWrite (); returns 0.
   */
  void setBufferPosition(data::v_io_size readPosition, data::v_io_size writePosition, bool canRead);

  /**
   * Amount of bytes currently available to read from buffer.
   * @return &id:oatpp::data::v_io_size;.
   */
  data::v_io_size availableToRead();

  /**
   * Amount of buffer space currently available for data writes.
   * @return &id:oatpp::data::v_io_size;.
   */
  data::v_io_size availableToWrite();

  /**
   * read up to count bytes from the buffer to data
   * @param data
   * @param count
   * @return [1..count], IOErrors.
   */
  data::v_io_size read(void *data, data::v_io_size count);

  /**
   * write up to count bytes from data to buffer
   * @param data
   * @param count
   * @return [1..count], IOErrors.
   */
  data::v_io_size write(const void *data, data::v_io_size count);

  /* No implementation of other methods */
  /* User should implement his own synchronization for other methods */

};
  
}}}

#endif /* FIFOBuffer_hpp */
