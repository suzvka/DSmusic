#pragma once
#include "DSmusic.h"

#include "rapidjson/document.h"
#include "rapidjson/error/en.h"

#include <vector>
#include <string>
#include <unordered_map>
#include <unordered_set>

namespace DS {
	static const std::unordered_set<std::string> Vowel = {
		"a", "ai" ,"ao" ,"an","ang",
		"o", "ou" ,"ong",
		"e", "ei" ,"er" ,"en" ,"eng","E"  ,"En" ,
		"u", "ui" ,"uo" ,"un" ,"ua" ,"uai","ui" ,"uan" ,"uang",
		"i", "iu" ,"ie" ,"in" ,"ing","iao","ian","iang","iong","ia", "ie", "iu", "ir" , "i0" ,
		"v", "van","vn" ,"ve" ,
		"SP", "AP"
	};

	class parser : public music {
	public:
		// 构造函数------------------------------------
		
		// 从已有的完整 DS 结构中提取
		parser(
			const std::string& json,
			const std::string& language
		);
		// 创建一个空的 DS
		parser(
			const std::string& language
		);

		// 初始化：解析所有需要的字段并存储在成员变量中
		void load();

		// 打包：增加批次处理量，提高 GPU 利用率
		void pack(float time_s, float maxIntervalS);

		std::vector<std::string> pack(
			float time_s,
			float maxInterval_s,
			const std::vector<std::string>& word_seq
		);

		std::vector<std::unique_ptr<music>> split();

		// 从内存中加载数据（完整）
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

		// 从内存中加载数据（仅词格）
		// - 音符序列
		// - 音符时长
		// - 连音标志
		// - 本行起始时间
		// - 要保存的行
		bool set(
			const std::vector<std::string>& note_seq,
			const std::vector<float>& note_dur,
			const std::vector<int>& note_slur,
			float offset = 0,
			int row = 0
		);

		// 设置歌词
		// - 音素序列
		// - 音素时长
		// - 要保存的行
		bool set_lyrics(
			const std::vector<std::string>& ph_seq,
			const std::vector<float>& ph_dur,
			int row = 0
		);


		bool set_syllable(
			const std::vector<std::string>& syllable_seq
		);

		// 反序列化
		std::string get()const ;

		// 是否为空
		bool empty() const { return !_hasData; }

		// 获取 ds 文件中的总行数
		int getRowCount() const;

		// 获取音素序列
		std::vector<std::string> getPhSeq(int row) const;
		std::vector<std::string> getPhSeq_raw(int row) const;

		// 获取每个音节的音素数量序列
		std::vector<int> getPhNum(int row) const { return _phNum.at(row); }

		// 获取音符序列
		std::vector<std::string> getNoteSeq(int row) const { return _noteSeq.at(row); }

		// 获取音符时长
		std::vector<float> getNoteTime(int row) const { return _noteTime.at(row); }
		std::vector<float> getNoteDur(int row, float step) const;

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
		const std::vector<float> getPitch(int row) const;
		const std::vector<float> getPitchStep(int row, float step) const;
		const std::vector<float> getMidi(int row) const;
		const std::vector<float> getMidiPh(int row) const;
		const std::vector<float> getMidiStep(int row, float step) const;

		// 设置音素时长序列
		parser& setPhTime(std::vector<float> data, float offset, int row);
		// 获取音素时长序列
		const std::vector<float>& getPhDur(int row) const { return _phTime.at(row); }

		// 设置能量曲线
		parser& setEnergy(std::vector<float> data, float offset, int row);
		// 获取能量曲线
		const std::vector<float>& getEnergy(int row) const { 
			if (_energy.empty()) return{};
			return _energy.at(row); 
		}

		// 设置气声曲线
		parser& setBreathiness(std::vector<float> data, float offset, int row);
		// 获取气声曲线
		const std::vector<float>& getBreathiness(int row) const { 
			if (_breathiness.empty()) return{};
			return _breathiness.at(row); 
		}

		// 设置发声曲线
		parser& setVoicing(std::vector<float> data, float offset, int row);
		// 获取发声曲线
		const std::vector<float>& getVoicing(int row) const { 
			if (_voicing.empty()) return{};
			return _voicing.at(row); 
		}

		// 设置张力曲线
		parser& setTension(std::vector<float> data, float offset, int row);
		// 获取张力曲线
		const std::vector<float>& getTension(int row) const { 
			if (_tension.empty()) return{};
			return _tension.at(row); 
		}

		// 设置口型曲线
		parser& setMouthOpening(std::vector<float> data, float offset, int row);
		// 获取口型曲线
		const std::vector<float>& getMouthOpening(int row) const {
			if (_mouthOpening.empty()) return{};
			return _mouthOpening.at(row);
		}

	private:
		bool _isLoad = false;	// 已加载到内存，可调用 get 系列方法读取数据
		bool _hasData = false;	// 有可用数据，包括 json 对象内的或内存中的
		bool _readyCase = false;// 词格已就绪

		rapidjson::Document _dsData;  // 保存传入的 ds 文件数据
		rapidjson::Document::AllocatorType* _allocator = nullptr;

		std::string _language; // 使用的语言

		// 内部数据存储
		std::vector<std::vector<std::string>> _phSeq = {};	// 音素序列
		std::vector<std::vector<int>> _phNum = {};			// 音节划分
		std::vector<std::vector<int>> _noteSlur = {};		// 滑音标志
		std::vector<float> _offset = {};					// 偏移时间

		std::vector<std::vector<std::string>> _noteSeq = {};// 音符序列
		std::vector<std::vector<float>> _noteTime = {};		// 音符时长

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

		std::vector<std::vector<float>> _mouthOpening = {}; // 口型曲线
		std::vector<float> _mouthOpening_ticktime = {};		// 口型采样时间

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

		// 重采样
		// - data_seq 数据元素序列
		// - element_length_seq 每个数据元素所占长度序列
		// - step 重采样步长
		template<typename T>
		std::vector<T> resampling(
			const std::vector<T>& data_seq,
			const std::vector<float>& element_length_seq,
			float step
		) const;

		std::vector<float> P_F_conversion(
			const std::vector<std::string>& notes
		) const;

		std::vector<float> P_M_conversion(
			const std::vector<std::string>& notes
		) const;

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