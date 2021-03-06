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
#include <deque>
#include <map>
#include <unordered_map>
#include <boost/noncopyable.hpp>
#include <boost/system/error_code.hpp>
#include <boost/thread.hpp>
#include <UsageEnvironment.hh>
#include <LiveMediaExt/LiveRtspServer.h>
#include <Media/AudioChannelDescriptor.h>
#include <Media/ChannelManager.h>
#include <Media/VideoChannelDescriptor.h>

// fwd
class BasicUsageEnvironment;

namespace lme
{

// fwd
class LiveSourceTaskScheduler;
class IRateAdaptationFactory;
class IRateController;

/**
 * @brief The RtspService manages the delivery of media samples to clients via an RTSP server.
 *
 * Media samples can be handed over to the RTSP service for delivery by calling.
 * Each live stream or channel is identified by a channel ID. A channel may consist
 * of multiple media streams e.g. one for audio and one for video.
 */
class RtspService : private boost::noncopyable
{
  static const uint16_t DEFAULT_RTSP_PORT;
public:
  /**
   * @brief Constructor
   */
  RtspService(ChannelManager& channelManager, IRateAdaptationFactory* pFactory = NULL, IRateController* pGlobalRateControl = NULL);
  /**
   * @brief Initialises live555 components
   */
  boost::system::error_code init(uint16_t uiRtspPort = DEFAULT_RTSP_PORT);
  /**
   * @brief starts RTSP service: blocking method! init() must be called prior to this!
   */
  boost::system::error_code start();
  /**
   * @brief stops periodic generation of media samples
   */
  boost::system::error_code stop();
  /**
   * @brief creates a channel for distribution.
   *
   * This results in a ServerMediaSession being added to the RTSP server. This method is thread-safe and can be called by any thread.
   */
  boost::system::error_code createChannel(uint32_t uiChannelId, const std::string& sChannelName, const VideoChannelDescriptor& videoDescriptor, const AudioChannelDescriptor& audioDescriptor);
  /**
   * @brief creates a video-only channel for distribution.
   *
   * This results in a ServerMediaSession being added to the RTSP server.
   */
  boost::system::error_code createChannel(uint32_t uiChannelId, const std::string& sChannelName, const VideoChannelDescriptor& videoDescriptor);
  /**
   * @brief creates a audio-only channel for distribution.
   *
   * This results in a ServerMediaSession being added to the RTSP server.
   */
  boost::system::error_code createChannel(uint32_t uiChannelId, const std::string& sChannelName, const AudioChannelDescriptor& audioDescriptor);
  /**
   * @brief removes a channel for distribution
   *
   * This results in a ServerMediaSession being removed from the RTSP server.
   */
  boost::system::error_code removeChannel(uint32_t uiChannelId);
  /**
   * @brief Setter for notifications when a client session is issues the PLAY command to the RTSP server.
   */
  void setOnClientSessionPlayCallback(OnClientSessionPlayHandler onClientSessionPlay){ m_onClientSessionPlay = onClientSessionPlay; }
  /**
   * @brief Callback to be called by RTSP server on client session PLAY.
   */
  void onRtspClientSessionPlay(unsigned uiClientSessionId);

private:
  /**
   * @brief live555 event loop: blocking method
   */
  void live555EventLoop();
  /**
   * @brief cleans up the live555 environment
   */
  void cleanupLiveMediaEnvironment();
  /**
   * @brief live555 callback mechanism to call checkSessionsTask()
   */
  static void checkSessionsTask(void* clientData);
  /**
   * @brief checks if RTSP sessions for RTCP updates.
   */
  void doCheckSessionsTask();
  /**
   * @brief live555 callback mechanism to call checkChannelsTask()
   */
  static void checkChannelsTask(void* clientData);
  /**
   * @brief checks if channels have been added or removed.
   */
  void doCheckChannelsTask();
private:

  /// Channel manager
  ChannelManager& m_channelManager;
  /// condition variable to control event loop lifetime
  char m_cEventloop;
  /// live555 RTSP server
  LiveRtspServer* m_pRtspServer;
  /// live555 task scheduler
  LiveSourceTaskScheduler* m_pScheduler;
  /// live555 usage environment
  BasicUsageEnvironment* m_pEnv;
  /// live thread
  std::unique_ptr<boost::thread> m_pLive555Thread;
  /// flag for event loop status
  bool m_bEventLoopRunning;
  /// live555 tasks
  TaskToken m_pCheckSessionsTask;
  /// live555 tasks
  TaskToken m_pCheckChannelsTask;
  /// queue to notifiy live555 thread of new channels to be added
  std::deque<Channel> m_qChannelsToBeAdded;
  /// queue to notifiy live555 thread of new channels to be removed
  std::deque<Channel> m_qChannelsToBeRemoved;
  /// mutex to protect channel queues
  boost::mutex m_channelLock;
  typedef std::unordered_map<uint32_t, Channel> ChannelMap_t;
  /// Map that stores channel info per Channel Id
  ChannelMap_t m_mChannels;
  /// Rate adaptation module
  IRateAdaptationFactory* m_pFactory;
  /// Rate control
  IRateController* m_pGlobalRateControl;
  /// On client session play callback
  OnClientSessionPlayHandler m_onClientSessionPlay;
};

} // lme
