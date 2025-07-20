服务端：GameMode->GameState（Message Queue, broadcastTimer：1.5s）
客户端：HUD->CreateMessageItem(finishTimer, every 1s)
问题：激烈战斗时，可能战斗已经结束，但通知还需要一段时间才能完全播报完
sol:服务端控制广播节奏
 - 服务端：broadcastTimer 默认max 1s, min 0.1s, 1/当前消息堆积数量，可在发送消息时设置
 - 客户端：finishTimer 时间由服务端下发，同时也维护一个消息队列，但是在NativeTick中处理消息淡入淡出，避免消息展示重叠。

连杀
playerState（lastKillTime，consecutiveKillCnt， consecutiveKillWindow），触发击杀时，window+2，未满足连杀条件时，置为0


