#pragma once
#include "DSmusic.h"

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace DS {

	static const std::unordered_set<std::string> Vowel = { "a", "o", "e", "ai", "ei","er", "ui", "ao", "ou", "iu", "ie", "an", "en", "in", "ang", "eng", "ing", "ong", "u", "i", "v" ,
															"iao","ian", "iang","iong","ia", "ie", "iu", "van", "vn", "ve" , "ua", "un","uo", "uai", "ui", "uan", "uang" ,
															"E" , "En" , "ir" , "i0" ,"SP" , "AP" };
	bool is_vowel(std::string noteNum);

	class parser : public music {
	public:
		// 构造函数
		parser(
			const std::string& json,
			const std::string& language,
			const std::unordered_map<std::string, std::string>& ph_map
		);
		parser(
			const std::string& language,
			const std::unordered_map<std::string, std::string>& ph_map
		);

		// 初始化：解析所有需要的字段并存储在成员变量中
		void load();

		// 打包：增加批次处理量，提高 GPU 利用率
		void pack(float time_s, float maxIntervalS);

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
		bool set(
			const std::vector<std::string>& note_seq,
			const std::vector<float>& note_dur,
			const std::vector<int>& note_slur,
			const std::vector<std::string>& ph_seq,
			const std::vector<float>& ph_dur,
			float offset = 0,
			int row = 0
		);

		// 反序列化
		std::string get()const ;

		// 是否为空
		bool empty() const { return !_hasData; }

		// 获取 ds 文件中的总行数
		int getRowCount() const;

		// 获取音素序列
		std::vector<std::string> getPhSeq(int row) const { 
			if (_language.empty()) {
				return _phSeq[row];
			}
			std::vector<std::string> out(_phSeq[row].size());
			for (int i = 0;i < _phSeq[row].size();i++) {
				if (_phSeq[row][i] != "SP" && _phSeq[row][i] != "AP") {
					out[i] = getLang() + "/" + _phSeq[row][i];
				}
				else {
					out[i] = _phSeq[row][i];
				}
			}
			return out;
		}

		// 获取每个音节的音素数量序列
		std::vector<int> getPhNum(int row) const { return _phNum.at(row); }

		// 获取音符序列
		std::vector<std::string> getNoteSeq(int row) const { return _noteSeq.at(row); }

		// 获取音符时长
		std::vector<float> getNoteDur(int row) const { return _noteDur.at(row); }

		// 获取滑音标志
		std::vector<int> getNoteSlur(int row) const { return _noteSlur.at(row); }

		// 获取某行的偏移时间
		float getOffset(int row) const { return _offset.at(row); }

		// 获取全部行的偏移时间
		std::vector<float> getOffset() const { return _offset; }

		// 获取采样时间
		float getTickTime(int row = 0) const { return _f0_ticktime.at(row); }

		// 获取语言
		std::string getLang() const { return _language; }

		// 设置音高曲线
		parser& setPitch(std::vector<float> data, float offset, int row);
		// 获取音高曲线
		const std::vector<float>& getPitch(int row) const { return _f0_seq.at(row); }

		// 设置音素时长序列
		parser& setPhTime(std::vector<float> data, float offset, int row);
		// 获取音素时长序列
		const std::vector<float>& getPhDur(int row) const { return _phTime.at(row); }

		// 设置能量曲线
		parser& setEnergy(std::vector<float> data, float offset, int row);
		// 获取能量曲线
		const std::vector<float>& getEnergy(int row) const { return _energy.at(row); }

		// 设置气声曲线
		parser& setBreathiness(std::vector<float> data, float offset, int row);
		// 获取气声曲线
		const std::vector<float>& getBreathiness(int row) const { return _breathiness.at(row); }

		// 设置发声曲线
		parser& setVoicing(std::vector<float> data, float offset, int row);
		// 获取发声曲线
		const std::vector<float>& getVoicing(int row) const { return _voicing.at(row); }

		// 设置张力曲线
		parser& setTension(std::vector<float> data, float offset, int row);
		// 获取张力曲线
		const std::vector<float>& getTension(int row) const { return _tension.at(row); }

	private:
		bool _isLoad = false;	// 已加载到内存，可调用 get 系列方法读取数据
		bool _hasData = false;	// 有可用数据，包括 json 对象内的或内存中的

		rapidjson::Document _dsData;  // 保存传入的 ds 文件数据
		rapidjson::Document::AllocatorType* _allocator = nullptr;

		std::string _language; // 使用的语言
		std::unordered_map<std::string, std::string> _dsPhDic; // 使用的音素字典

		// 内部数据存储
		std::vector<std::vector<std::string>> _phSeq = {};	// 音素序列
		std::vector<std::vector<int>> _phNum = {};			// 音节划分
		std::vector<std::vector<int>> _noteSlur = {};		// 滑音标志
		std::vector<float> _offset = {};					// 偏移时间

		std::vector<std::vector<std::string>> _noteSeq = {};// 音符序列
		std::vector<std::vector<float>> _noteDur = {};		// 音符时长

		std::vector<std::vector<float>> _phTime = {};		// 音素时长序列

		std::vector<std::vector<float>> _f0_seq = {};		// 音高曲线
		std::vector<float> _f0_ticktime = {};				// 音高采样时间

		std::vector<std::vector<float>> _energy = {};		// 能量曲线
		std::vector<float> _energy_ticktime = {};           // 能量采样时间

		std::vector<std::vector<float>> _breathiness = {};  // 气声曲线
		std::vector<float> _breathiness_ticktime = {};      // 气声采样时间

		std::vector<std::vector<float>> _voicing = {};      // 发声曲线
		std::vector<float> _voicing_ticktime = {};          // 发声采样时间

		std::vector<std::vector<float>> _tension = {};      // 张力曲线
		std::vector<float> _tension_ticktime = {};			// 张力采样时间

		// 工具函数----------------------------------------------------------------------------

		// 从 ds 文件中提取相应键值
		template <typename T>
		std::vector<T> parseDS(const std::string& key, size_t index);
		// 保存为数字
		template<typename T>
		parser& saveNumber(const std::string& key, const T& value, size_t index);
		// 保存为字符串
		template<typename T>
		parser& saveString(const std::string& key, const T& value, size_t index);

		// 利用字典切分音素
		std::vector<std::string> getPhonemes(const std::vector<std::string>& phonemeSequence);

		// 划分音节
		std::vector<int> makePhNum(const std::vector<std::string>& ph_seq);

		// TODO 这些作为临时性转音处理措施，在转音机制真正支持后应当删除----------
		// 应用转音序列，生成新的音素列表
		std::vector<std::string> makePhSeq(
			const std::vector<std::string>& ph_seq,
			const std::vector<int>& note_slur
		);
		// 划分音节
		std::vector<int> makePhNum(const std::vector<int>& ph_num, const std::vector<int>& note_sule);
		void updateJSONData();
		//------------------------------------------------------------------------

		// 类型特征检查：是否为std::vector
		template<typename>
		struct is_vector : std::false_type {};

		template<typename T, typename A>
		struct is_vector<std::vector<T, A>> : std::true_type {};

		// 递归类型转换函数
		template<typename U>
		static std::string convert_to_string(const U& value) {
			if constexpr (is_vector<U>::value) {
				// 处理std::vector
				std::ostringstream oss;
				for (size_t i = 0; i < value.size(); ++i) {
					if (i != 0) oss << " ";
					oss << convert_to_string(value[i]); // 递归处理元素
				}
				return oss.str();
			}
			else if constexpr (std::is_same_v<U, std::string>) {
				// 处理字符串
				return value;
			}
			else {
				// 处理基本类型（int64_t、float等）
				return std::to_string(value);
			}
		}
	};
}