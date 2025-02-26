#pragma once
#include <string>
#include <vector>
#include <unordered_map>

namespace DS {

class music {
public:
	// 初始化：解析所有需要的字段并存储在成员变量中
	virtual	void load() = 0;

	// 打包：增加批次处理量，提高 GPU 利用率
	virtual void pack(float time_s, float maxIntervalS) = 0;

	// 从内存中加载数据
	// 返回加载成功与否的标志
	// - 音符序列
	// - 音符时长
	// - 连音标志
	// - 音素序列
	// - 音素时长
	// - 连音标志
	// - 本行起始时间
	// - 要保存的行
	virtual bool set(
		const std::vector<std::string>& note_seq,
		const std::vector<float>& note_dur,
		const std::vector<int>& note_slur,
		const std::vector<std::string>& ph_seq,
		const std::vector<float>& ph_dur,
		const std::vector<int>& ph_num,
		float offset = 0,
		int row = 0
	) = 0;

	// 反序列化
	virtual std::string get()const = 0;

	// 是否为空
	virtual bool empty() const = 0;

	// 获取 ds 文件中的总行数
	virtual int getRowCount() const = 0;

	// 获取音素序列
	virtual const std::vector<std::string>& getPhSeq(int row) const = 0;

	// 获取每个音节的音素数量序列
	virtual const std::vector<int>& getPhNum(int row) const = 0;

	// 获取音符序列
	virtual const std::vector<std::string>& getNoteSeq(int row) const = 0;

	// 获取音符时长
	virtual const std::vector<float>& getNoteDur(int row) const = 0;

	// 获取滑音标志
	virtual const std::vector<int>& getNoteSlur(int row) const = 0;

	// 获取某行的偏移时间
	virtual float getOffset(int row) const = 0;

	// 获取全部行的偏移时间
	virtual const std::vector<float>& getOffset() const = 0;

	// 获取采样时间
	virtual float getTickTime(int row = 0) const = 0;

	// 获取语言
	virtual std::string getLang() const = 0;

	// 设置音高曲线
	virtual music& setPitch(std::vector<float> data, float offset, int row) = 0;
	// 获取音高曲线
	virtual const std::vector<float>& getPitch(int row) const = 0;

	// 设置音素时长序列
	virtual music& setPhTime(std::vector<float> data, float offset, int row) = 0;
	// 获取音素时长序列
	virtual const std::vector<float>& getPhDur(int row) const = 0;

	// 设置能量曲线
	virtual music& setEnergy(std::vector<float> data, float offset, int row) = 0;
	// 获取能量曲线
	virtual const std::vector<float>& getEnergy(int row) const = 0;

	// 设置气声曲线
	virtual music& setBreathiness(std::vector<float> data, float offset, int row) = 0;
	// 获取气声曲线
	virtual const std::vector<float>& getBreathiness(int row) const = 0;

	// 设置表现力曲线
	virtual music& setVoicing(std::vector<float> data, float offset, int row) = 0;
	// 获取表现力曲线
	virtual const std::vector<float>& getVoicing(int row) const = 0;

	// 设置张力曲线
	virtual music& setTension(std::vector<float> data, float offset, int row) = 0;
	// 获取张力曲线
	virtual const std::vector<float>& getTension(int row) const = 0;

};

music* get_music(
	const std::string& json,
	const std::string& language,
	const std::unordered_map<std::string, std::string>& ph_map
);

music* get_music(
	const std::string& language,
	const std::unordered_map<std::string, std::string>& ph_map
);

}