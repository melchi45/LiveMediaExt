/**********
This library is free software; you can redistribute it and/or modify it under
the terms of the GNU Lesser General Public License as published by the
Free Software Foundation; either version 2.1 of the License, or (at your
option) any later version. (See <http://www.gnu.org/copyleft/lesser.html>.)

This library is distributed in the hope that it will be useful, but WITHOUT
ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
more details.

You should have received a copy of the GNU Lesser General Public License
along with this library; if not, write to the Free Software Foundation, Inc.,
51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
**********/
// "CSIR"
// Copyright (c) 2014 CSIR.  All rights reserved.
#pragma once
#define LOCK_FREE
#define SINGLE_QUEUE
#ifdef LOCK_FREE
#include <boost/lockfree/spsc_queue.hpp>
#else
#include <deque>
#include <boost/thread/mutex.hpp>
#endif
#include <boost/optional.hpp>
#include <Media/MediaChannel.h>

namespace lme
{

  typedef std::pair<int, MediaSample> MediaSample_t;

  /**
  * @brief Implementation of packet manager-based media channel
  *
  * This implementation will test using the lock-free queue implementations
  * of the boost libraries.
  */
  class PacketManagerMediaChannel : public MediaChannel
  {
  public:
    /**
    * @brief Constructor
    */
    PacketManagerMediaChannel(uint32_t uiChannelId);
    /**
    * @brief The subclass must implement delivery of video media samples to the media sink
    */
    virtual boost::system::error_code deliverVideo(uint32_t uiChannelId, const std::vector<MediaSample>& mediaSamples);
    /**
    * @brief The subclass must implement delivery of audio media samples to the media sink
    */
    virtual boost::system::error_code deliverAudio(uint32_t uiChannelId, const std::vector<MediaSample>& mediaSamples);
    /**
    * @brief This method returns a video media sample if available, and a null pointer otherwise
    */
    boost::optional<MediaSample> getVideo();
    /**
    * @brief This method returns a video media sample if available, and a null pointer otherwise
    */
    boost::optional<MediaSample> getAudio();

  private:
#ifdef LOCK_FREE
#define LF_CAPACITY 10240
    /// lock free queue to store video media samples in
    boost::lockfree::spsc_queue<MediaSample, boost::lockfree::capacity<LF_CAPACITY> > m_videoSamples;
    /// lock free queue to store audio media samples in
    boost::lockfree::spsc_queue<MediaSample, boost::lockfree::capacity<LF_CAPACITY> > m_audioSamples;
#else
#ifdef SINGLE_QUEUE
    /// @typedef media type( 0 = audio, 1= video), media sample
    std::deque<MediaSample_t> m_mediaSamples;
    boost::mutex m_mediaLock;
    double m_dQueueDuration;
#else
    std::deque<MediaSample> m_videoSamples;
    std::deque<MediaSample> m_audioSamples;
    boost::mutex m_videoLock;
    boost::mutex m_audioLock;
#endif
#endif
  };

} //lme
