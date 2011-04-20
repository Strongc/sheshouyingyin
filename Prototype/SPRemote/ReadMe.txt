========================================================================
    SPRemote 命令说明
========================================================================
调用方法：
    http://127.0.0.1:8080/splayer?id=3972&cmd=play&p=
    id是指射手影音的进程ID，cmd就是要发送的命令名称，p为附加参数（暂时未用）

命令列表：
分类                  名称                                    说明
系统                  exit                                    退出播放器

基本功能              play                                    开始播放
                      pause                                   暂停
                      stop                                    停止
                      prev                                    前一个媒体文件
                      next                                    后一个媒体文件

声音                  toggle_mute                             设置/取消静音

                      audio_channel_default                   设置为默认声道
                      audio_channel_left                      设置为左声道
                      audio_channel_right                     设置为右声道
                      audio_channel_center                    设置为中置声道

字幕                  intelligent_subtitle_download           智能字幕匹配下载

视频                  normal_display                          标准画面
                      fullscreen_display                      全屏
                      fromoutside_display                     智能去黑边
                      stretch_display                         铺满播放窗口
                      quality_mode_display                    画质优先模式
                      performance_mode_display                性能优先模式
                      dxva_mode_display                       启用硬件加速
                      gothsync_mode_display                   智能防撕裂

                      default_aspect_ratio                    默认宽高比
                      4_3_ratio                               4:3
                      5_4_ratio                               5:4
                      16_9_ratio                              16:9
                      235_1_ratio                             2.35:1
                      compmondeskardiff                       智能显示器校正

                      h_rotate                                水平旋转
                      v_rotate                                垂直旋转
                      270_rotate                              向右90度
                      90_rotate                               向左90度
                      180_rotate                              180度旋转
                      reset_rotate                            恢复默认角度
                      enable_rotate                           启用画面翻转

播放                  normal_loop                             不循环播放
                      playlist_loop                           列表循环播放
                      current_loop                            当前文件循环
                      random_loop                             随机播放

                      inc_speed_play                          加快播放速度
                      dec_speed_play                          减慢播放速度

                      close_after_playback                    结束后退出
                      standby_after_playback                  结束后待机
                      hibernate_after_playback                结束后休眠
                      shutdown_after_playback                 结束后关机
                      logoff_after_playback                   结束后注销

截图                  auto_save_image                         快速截图到默认目录

界面                  minimal_ui                              小巧界面
                      normal_ui                               标准界面
                      aeroglass_ui                            玻璃效果

                      english_lang                            英语
                      chinese_simplified_lang                 简体中文
                      chinese_traditional_lang                繁体中文
                      french_lang                             法语
                      russian_lang                            俄语
                      german_lang                             德语
