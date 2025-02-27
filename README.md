# `DSmusic` DS乐谱解析器

`DSmusic` 面向 **音乐/语音合成时序数据管理** ，用于解析、操作和序列化 DS 乐谱格式的数据。它封装了音符、音素、音频参数曲线等核心数据，并提供 GPU 优化功能，是连接符号化乐谱与音频合成引擎的桥梁。

---

## 核心功能
- **DS 乐谱解析**：支持从 DS 乐谱或内存数据加载
- **时序参数管理**：直接操作音高、能量、气声等音频合成曲线
- **GPU 优化打包**：通过 `pack()` 提升批量合成效率
- **多行/多句支持**：独立管理不同行（乐句）的偏移时间和参数
- **异常安全**：通过 `DsParserError` 明确数据格式错误

---

## 什么是 DS 乐谱？

##### 本质上是一个`JSON`数组，按行储存乐句。

##### 每个乐句含有这些参数：

###### 乐谱部分

|    字段     |   含义   | 额外说明                                                     |
| :---------: | :------: | ------------------------------------------------------------ |
| `note_seq`  | 音符名称 | 基准序列                                                     |
| `note_dur`  | 音符时长 | 长度对齐`note_seq`                                           |
| `note_slur` | 连音标志 | 长度对齐`note_seq`                                           |
|  `ph_seq `  | 音素名称 | 需要保证是声库支持的音素                                     |
|  `ph_dur`   | 音素时长 | 在有转音时允许不对齐`ph_seq `                                |
|  `ph_num`   | 音节划分 | 元素总和等于除去转音音素后的`ph_seq `长度，用于决定每个音符分配多少个音素 |
|  `offset`   | 偏移时间 | 每句在总时间轴上的起始位置                                   |

###### 曲线部分

|          字段          |     含义     | 额外说明                         |
| :--------------------: | :----------: | :------------------------------- |
|        `f0_seq`        |   音高曲线   | 基频音高，单位为频率             |
|     `f0_ticktime`      | 音高采样时间 | 每个音高元素在时间轴上占用多少秒 |
|        `energy`        |   能量曲线   | 单位为相对分贝，最高`0`          |
| `	energy_ticktime`  | 能量采样时间 | 同音高                           |
|     `breathiness`      |   气声曲线   | 单位为相对分贝，最高`0`          |
| `breathiness_ticktime` | 气声采样时间 | 同音高                           |
|       `voicing `       |   发声曲线   | 单位为相对分贝，最高`0`          |
|   `voicing_ticktime`   | 发声采样时间 | 同音高                           |
|       `tension`        |   张力曲线   | 无单位，区间`[-10,10]`           |
|  `tension_ticktime `   | 张力采样时间 | 同音高                           |



## **主要接口概览**

### 1. 初始化与加载
| 方法/函数                                                    | 说明                                                         |
| ------------------------------------------------------------ | ------------------------------------------------------------ |
| `DS::music* DS::get_music(json, language, ph_map)`           | 工厂函数：从已有的 DS 乐谱中创建对象                         |
| `DS::music* DS::get_music(language, ph_map)`                 | 工厂函数：创建空对象（需后续调用 `set()`）                   |
| `void load()`                                                | 开始解析数据，仅从 DS 乐谱中创建时需要。需线程安全时外部加锁 |
| `bool set(note_seq, note_dur, note_slur, ph_seq, ph_dur, offset, row)` | 从内存加载数据，返回 `false` 表示部分字段被自动修正          |

**示例**：

```cpp
// 从 JSON 创建对象
auto* song = DS::get_music(json_data, "zh", ph_map);
song->load();

// 手动设置数据
std::vector<std::string> notes = {"C4", "D4"};
std::vector<float> durations = {0.5, 0.3};
song->set(notes, durations, {0，0}, {"a", "e"}, {0.2, 0.1}, 0.1, 0);
```

### 2. **数据获取**

| 方法                  | 返回类型         | 说明                                |
| :-------------------- | :--------------- | :---------------------------------- |
| `getPhSeq(row)`       | `vector<string>` | 音素序列（含语言前缀，如 `"zh/a"`） |
| `getNoteSeq(row)`     | `vector<string>` | 音符名称序列                        |
| `getNoteDur(row)`     | `vector<float>`  | 音符时长（秒）                      |
| `getOffset()`         | `vector<float>`  | 所有行的起始偏移时间                |
| `getOffset(row)`      | `float`          | 指定行的起始偏移时间                |
| `getPitch(row)`       | `vector<float>`  | 获取音高曲线                        |
| `getEnergy(row)`      | `vector<float>`  | 获取能量曲线                        |
| `getBreathiness(row)` | `vector<float>`  | 获取气声曲线                        |
| `getVoicing(row)`     | `vector<float>`  | 获取发声曲线                        |
| `getTension(row)`     | `vector<float>`  | 获取张力曲线                        |
| `getTickTime(row)`    | `float`          | 获取指定行曲线部分的采样时间（秒）  |

### 3. 数据写入

| 方法                                | 说明                                        |
| :---------------------------------- | :------------------------------------------ |
| `setPitch(data, offset, row)`       | 设置音高曲线，`offset` 参数用于重置偏移时间 |
| `setEnergy(data, offset, row)`      | 设置能量曲线                                |
| `setBreathiness(data, offset, row)` | 设置气声曲线                                |
| `setVoicing(data, offset, row)`     | 设置发声曲线                                |
| `setTension(data, offset, row)`     | 设置张力曲线                                |

### 4. 序列化与优化

| 方法                          | 说明**                              |
| :---------------------------- | :---------------------------------- |
| `std::string get()`           | 将数据序列化为 DS 乐谱字符串        |
| `pack(time_s, maxInterval_s)` | 按时间窗口打包数据，提升 GPU 利用率 |