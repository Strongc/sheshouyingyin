#pragma once

#include <map>
#include <string>
#include "../../src/apps/mplayerc/resource.h"

#define ADD_REMOTECMD(cmd, msgid) \
  remotecmd[cmd] = msgid;

typedef struct {
  std::string cmdstring;
  UINT msgid;
  time_t timestamp;
  WPARAM wParam;
  LPARAM lParam;
} RemoteMsg;

#define MAX_REMOTEMSG_10 10
#define REMOTEMSG_CHANNELNAME "SPlayerMsgChannle"

struct tagRemoteCmdInfo
{
  tagRemoteCmdInfo(UINT id = 0, WPARAM w = 0, LPARAM l = 0) : msgid(id), wParam(w), lParam(l) {}
  UINT msgid;
  WPARAM wParam;
  LPARAM lParam;
};
typedef std::map<std::string, tagRemoteCmdInfo> REMOTECMD;

inline void GetRemoteCmdMap(REMOTECMD& remotecmd)
{
  // system
  ADD_REMOTECMD("exit", tagRemoteCmdInfo(ID_FILE_EXIT, ID_FILE_EXIT, 0));

  // basic function
  ADD_REMOTECMD("play", tagRemoteCmdInfo(ID_PLAY_PLAY, ID_PLAY_PLAY, 0));
  ADD_REMOTECMD("pause", tagRemoteCmdInfo(ID_PLAY_PAUSE, ID_PLAY_PAUSE, 0));
  ADD_REMOTECMD("stop", tagRemoteCmdInfo(ID_FILE_CLOSEPLAYLIST, ID_FILE_CLOSEPLAYLIST, 0));
  ADD_REMOTECMD("prev", tagRemoteCmdInfo(ID_NAVIGATE_SKIPBACKPLITEM, ID_NAVIGATE_SKIPBACKPLITEM, 0));
  ADD_REMOTECMD("next", tagRemoteCmdInfo(ID_NAVIGATE_SKIPFORWARDPLITEM, ID_NAVIGATE_SKIPFORWARDPLITEM, 0));

  // audio
  ADD_REMOTECMD("toggle_mute", tagRemoteCmdInfo(ID_VOLUME_MUTE, ID_VOLUME_MUTE, 0));

  ADD_REMOTECMD("audio_channel_default", tagRemoteCmdInfo(IDS_AUDIOCHANNALMAPNORMAL, IDS_AUDIOCHANNALMAPNORMAL, 0));
  ADD_REMOTECMD("audio_channel_left", tagRemoteCmdInfo(IDS_AUDIOCHANNALMAPLEFT, IDS_AUDIOCHANNALMAPLEFT, 0));
  ADD_REMOTECMD("audio_channel_right", tagRemoteCmdInfo(IDS_AUDIOCHANNALMAPRIGHT, IDS_AUDIOCHANNALMAPRIGHT, 0));
  ADD_REMOTECMD("audio_channel_center", tagRemoteCmdInfo(IDS_AUDIOCHANNALMAPCENTER, IDS_AUDIOCHANNALMAPCENTER, 0));

  // subtitle
  ADD_REMOTECMD("intelligent_subtitle_download", tagRemoteCmdInfo(ID_FILE_ISDB_DOWNLOAD, ID_FILE_ISDB_DOWNLOAD, 0));

  // video
  ADD_REMOTECMD("normal_display", tagRemoteCmdInfo(ID_VIEW_VF_FROMINSIDE, ID_VIEW_VF_FROMINSIDE, 0));
  ADD_REMOTECMD("fullscreen_display", tagRemoteCmdInfo(ID_VIEW_FULLSCREEN, ID_VIEW_FULLSCREEN, 0));
  ADD_REMOTECMD("fromoutside_display", tagRemoteCmdInfo(ID_VIEW_VF_FROMOUTSIDE, ID_VIEW_VF_FROMOUTSIDE, 0));
  ADD_REMOTECMD("stretch_display", tagRemoteCmdInfo(ID_VIEW_VF_STRETCH, ID_VIEW_VF_STRETCH, 0));
  ADD_REMOTECMD("quality_mode_display", tagRemoteCmdInfo(ID_VIEW_MODE_QUALITY_MODE, ID_VIEW_MODE_QUALITY_MODE, 0));
  ADD_REMOTECMD("performance_mode_display", tagRemoteCmdInfo(ID_VIEW_MODE_PERMORMANCE_MODE, ID_VIEW_MODE_PERMORMANCE_MODE, 0));
  ADD_REMOTECMD("dxva_mode_display", tagRemoteCmdInfo(ID_VIEW_MODE_DXVA_MODE, ID_VIEW_MODE_DXVA_MODE, 0));
  ADD_REMOTECMD("gothsync_mode_display", tagRemoteCmdInfo(ID_VIEW_MODE_GOTHSYNC_MODE, ID_VIEW_MODE_GOTHSYNC_MODE, 0));

  ADD_REMOTECMD("default_aspect_ratio", tagRemoteCmdInfo(ID_ASPECTRATIO_SOURCE, ID_ASPECTRATIO_SOURCE, 0));
  ADD_REMOTECMD("4_3_ratio", tagRemoteCmdInfo(ID_ASPECTRATIO_4_3, ID_ASPECTRATIO_4_3, 0));
  ADD_REMOTECMD("5_4_ratio", tagRemoteCmdInfo(ID_ASPECTRATIO_5_4, ID_ASPECTRATIO_5_4, 0));
  ADD_REMOTECMD("16_9_ratio", tagRemoteCmdInfo(ID_ASPECTRATIO_16_9, ID_ASPECTRATIO_16_9, 0));
  ADD_REMOTECMD("235_1_ratio", tagRemoteCmdInfo(ID_ASPECTRATIO_235_1, ID_ASPECTRATIO_235_1, 0));
  ADD_REMOTECMD("compmondeskardiff", tagRemoteCmdInfo(ID_VIEW_VF_COMPMONDESKARDIFF, ID_VIEW_VF_COMPMONDESKARDIFF, 0));

  ADD_REMOTECMD("h_rotate", tagRemoteCmdInfo(ID_ROTATE_H, ID_ROTATE_H, 0));
  ADD_REMOTECMD("v_rotate", tagRemoteCmdInfo(ID_ROTATE_V, ID_ROTATE_V, 0));
  ADD_REMOTECMD("270_rotate", tagRemoteCmdInfo(ID_ROTATE_270, ID_ROTATE_270, 0));
  ADD_REMOTECMD("90_rotate", tagRemoteCmdInfo(ID_ROTATE_90, ID_ROTATE_90, 0));
  ADD_REMOTECMD("180_rotate", tagRemoteCmdInfo(ID_ROTATE_180, ID_ROTATE_180, 0));
  ADD_REMOTECMD("reset_rotate", tagRemoteCmdInfo(ID_ROTATE_RESET, ID_ROTATE_RESET, 0));
  ADD_REMOTECMD("enable_rotate", tagRemoteCmdInfo(ID_ENABLE_ROTATE, ID_ENABLE_ROTATE, 0));

  // playback
  ADD_REMOTECMD("normal_loop", tagRemoteCmdInfo(ID_PLAYBACK_LOOP_NORMAL, ID_PLAYBACK_LOOP_NORMAL, 0));
  ADD_REMOTECMD("playlist_loop", tagRemoteCmdInfo(ID_PLAYBACK_LOOP_PLAYLIST, ID_PLAYBACK_LOOP_PLAYLIST, 0));
  ADD_REMOTECMD("current_loop", tagRemoteCmdInfo(ID_PLAYBACK_LOOP_CURRENT, ID_PLAYBACK_LOOP_CURRENT, 0));
  ADD_REMOTECMD("random_loop", tagRemoteCmdInfo(ID_PLAYBACK_LOOP_RANDOM, ID_PLAYBACK_LOOP_RANDOM, 0));

  ADD_REMOTECMD("inc_speed_play", tagRemoteCmdInfo(ID_PLAY_INCRATE, ID_PLAY_INCRATE, 0));
  ADD_REMOTECMD("dec_speed_play", tagRemoteCmdInfo(ID_PLAY_DECRATE, ID_PLAY_DECRATE, 0));

  ADD_REMOTECMD("close_after_playback", tagRemoteCmdInfo(ID_AFTERPLAYBACK_CLOSE, ID_AFTERPLAYBACK_CLOSE, 0));
  ADD_REMOTECMD("standby_after_playback", tagRemoteCmdInfo(ID_AFTERPLAYBACK_STANDBY, ID_AFTERPLAYBACK_STANDBY, 0));
  ADD_REMOTECMD("hibernate_after_playback", tagRemoteCmdInfo(ID_AFTERPLAYBACK_HIBERNATE, ID_AFTERPLAYBACK_HIBERNATE, 0));
  ADD_REMOTECMD("shutdown_after_playback", tagRemoteCmdInfo(ID_AFTERPLAYBACK_SHUTDOWN, ID_AFTERPLAYBACK_SHUTDOWN, 0));
  ADD_REMOTECMD("logoff_after_playback", tagRemoteCmdInfo(ID_AFTERPLAYBACK_LOGOFF, ID_AFTERPLAYBACK_LOGOFF, 0));

  // screen snapshot
  ADD_REMOTECMD("auto_save_image", tagRemoteCmdInfo(ID_FILE_SAVE_IMAGE_AUTO, ID_FILE_SAVE_IMAGE_AUTO, 0));

  // UI
  ADD_REMOTECMD("minimal_ui", tagRemoteCmdInfo(ID_VIEW_PRESETS_MINIMAL, ID_VIEW_PRESETS_MINIMAL, 0));
  ADD_REMOTECMD("normal_ui", tagRemoteCmdInfo(ID_VIEW_PRESETS_NORMAL, ID_VIEW_PRESETS_NORMAL, 0));
  ADD_REMOTECMD("aeroglass_ui", tagRemoteCmdInfo(ID_THEME_AEROGLASS, ID_THEME_AEROGLASS, 0));

  ADD_REMOTECMD("english_lang", tagRemoteCmdInfo(ID_LANGUAGE_ENGLISH, ID_LANGUAGE_ENGLISH, 0));
  ADD_REMOTECMD("chinese_simplified_lang", tagRemoteCmdInfo(ID_LANGUAGE_CHINESE_SIMPLIFIED, ID_LANGUAGE_CHINESE_SIMPLIFIED, 0));
  ADD_REMOTECMD("chinese_traditional_lang", tagRemoteCmdInfo(ID_LANGUAGE_CHINESE_TRADITIONAL, ID_LANGUAGE_CHINESE_TRADITIONAL, 0));
  ADD_REMOTECMD("french_lang", tagRemoteCmdInfo(ID_LANGUAGE_FRENCH, ID_LANGUAGE_FRENCH, 0));
  ADD_REMOTECMD("russian_lang", tagRemoteCmdInfo(ID_LANGUAGE_RUSSIAN, ID_LANGUAGE_RUSSIAN, 0));
  ADD_REMOTECMD("german_lang", tagRemoteCmdInfo(ID_LANGUAGE_GERMAN, ID_LANGUAGE_GERMAN, 0));
}