```cpp
PlayerController 创建
↓
GameMode::PostLogin 调用
↓
GameMode::HandleStartingNewPlayer 调用
↓
SpawnDefaultPawnFor → 生成 Character
↓
PlayerController->Possess(Character)
```
✅ 客户端流程：
客户端是从服务端同步状态的，不会主动创建 PlayerController 或 Character，而是接收服务端的复制结果。

客户端关键步骤（通常由引擎自动完成）：

- PlayerController 在连接后由服务端分配并复制到客户端。  
- Character 创建并 Possess 后会通过网络同步，客户端也会看到对应的 Character 并自动设置本地输入、摄像机等。

🧠 补充说明：

- PlayerController 只存在于 **本地客户端（控制自己的玩家）**和服务端，不会存在于其他客户端。
- Character 是可复制的，会同步到其他客户端，因此服务端需要确保在 Possess 之前完成设置（如 PlayerState 等）