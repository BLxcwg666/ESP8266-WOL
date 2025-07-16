## Authorization

在你的请求头添加：
```
Authorization: Bearer {JWT_TOKEN}
```
## Endpoint
### 1. Auth
#### 登录
- `POST /api/login`
- 获取JWT访问令牌

**Body**:
```json
{
  "password": "ADMIN_PASSWORD"
}
```

**成功** (200):
```json
{
  "success": true,
  "token": "eyJhbGciOiJIUzI1NiIsInR5cCI6IkpXVCJ9...",
  "expiresIn": 604800000
}
```

**失败** (401):
```json
{
  "success": false,
  "message": "invalid password"
}
```

**Token 过期** (401):
```json
{
  "error": "token expired"
}
```

**Token 无效** (401):
```json
{
  "error": "invalid token"
}
```

### 2. Health
#### 状态
- `GET /api/health`
- 获取服务器运行状态

**成功** (200):
```json
{
  "status": "ok",
  "connectedDevices": 2,
  "savedDevices": 5,
  "uptime": 3600.123
}
```

### 3. ESP Devices
#### 获取列表
- `GET /api/devices`
- 获取当前连接的ESP8266设备列表
- **需要认证**

**成功** (200):
```json
[
  {
    "id": "device-uuid",
    "ip": "10.11.45.14",
    "connectedAt": "1970-01-01T11:45:14.191Z",
    "authenticated": true,
    "status": "online"
  }
]
```

#### 断开连接
- `POST /api/devices/:id/disconnect`
- 断开指定 ESP 设备的WebSocket连接
- **需要认证**

**成功** (200):
```json
{
  "success": true
}
```

**失败** (404):
```json
{
  "error": "device not found"
}
```

### 4. WOL

#### 发送 WOL 请求
- `POST /api/wol/send`
- 发送 Wake-on-LAN 唤醒命令
- **需要认证**

**Body**:
```json
{
  "mac": "AA:BB:CC:DD:EE:FF",
  "deviceId": "device-uuid"  // 可选，指定通过哪个设备
}
```

**成功** (200):
```json
{
  "success": true,
  "message": "sended wol command"
}
```
或
```json
{
  "success": true,
  "message": "sended wol command to 3 device(s)"
}
```

**失败**:
- 400: `{"error": "invalid mac address"}`
- 404: `{"error": "device not found"}`

### 5. Saved Devices

#### 获取列表
- `GET /api/wol/devices`
- 获取所有保存的设备信息
- **需要认证**

**成功** (200):
```json
[
  {
    "id": "device-uuid",
    "name": "神威·太湖之光",
    "mac": "11-45-14-19-19-81",
    "description": "我怎么知道",
    "createdAt": "1970-01-01T11:45:14.191Z"
  }
]
```

#### 添加
- `POST /api/wol/devices`
- 保存新的设备信息
- **需要认证**

**Body**:
```json
{
  "name": "天河二号",
  "mac": "AA:BB:CC:DD:EE:FF",
  "description": "我怎么知道"
}
```

**成功** (200):
```json
{
  "success": true,
  "device": {
    "id": "new-uuid",
    "name": "天河二号",
    "mac": "11-45-14-19-19-81",
    "description": "我怎么知道",
    "createdAt": "1970-01-01T11:45:14.191Z"
  }
}
```

**失败** (400):
```json
{
  "error": "name and mac address are required"
}
```
或
```json
{
  "error": "invalid mac address"
}
```

#### 更新设备
- `PUT /api/wol/devices/:id`
- 更新已保存的设备信息
- **需要认证**

**Body** (所有字段可选):
```json
{
  "name": "new-name",
  "mac": "new-mac",
  "description": "new-dec"
}
```

**成功** (200):
```json
{
  "success": true,
  "device": {
    "id": "device-uuid",
    "name": "new-name",
    "mac": "new-mac",
    "description": "new-dec",
    "createdAt": "1970-01-01T11:45:14.191Z"
  }
}
```

**失败**:
- 404: `{"error": "device not found"}`
- 400: `{"error": "invalid mac address"}`

#### 删除设备
- `DELETE /api/wol/devices/:id`
- 删除保存的设备信息
- **需要认证**

**成功** (200):
```json
{
  "success": true
}
```

**失败** (404):
```json
{
  "error": "device not found"
}
```