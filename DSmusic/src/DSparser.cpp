#include "DSparser.h"

#include "rapidjson/stringbuffer.h"
#include "rapidjson/writer.h"

#include <typeinfo>
#include <numeric>
#include <sstream>
#include <iostream>
#include <cmath>

namespace DS {
	std::vector<std::string> split_str(const std::string& str, char delimiter) {
		std::vector<std::string> tokens;
		std::stringstream ss(str);
		std::string token;

		while (std::getline(ss, token, delimiter)) {
			tokens.push_back(token);
		}

		return tokens;
	}

	template<typename T>
	std::string toString(const std::vector<T>& input) {
		std::ostringstream oss;
		for (size_t i = 0; i < input.size(); ++i) {
			if constexpr (std::is_same_v<T, bool>) {
				oss << (input[i] ? "1" : "0");  // 将布尔值转换为 1 或 0
			}
			else {
				oss << input[i];
			}
			if (i != input.size() - 1) {
				oss << " ";  // 在元素之间添加空格
			}
		}
		return oss.str();
	}

	parser::parser(
		const std::string& json,
		const std::string& language
	)
		: _offset(0.0f), _language(language)
	{
		if (_dsData.Parse(json.c_str()).HasParseError()) {
			return;
		}
		if (!_dsData.IsArray()) {
			throw DsParserError("不正确的 DS 格式：不是JSON数组");
		}
		_allocator = &_dsData.GetAllocator();
		_hasData = !_dsData.IsNull();
	}

	parser::parser(const std::string& language)
		: _offset(0.0f), _language(language)
	{
		_dsData.Parse("[]");
		_allocator = &_dsData.GetAllocator();
	}

	void parser::load() {
		if (_isLoad) {
			return;
		}
		for (int row = 0; row < getRowCount(); row++) {
			_phNum	.push_back(parseDS<int>("ph_num", row));
			_noteSeq.push_back(parseDS<std::string>("note_seq", row));
			_phSeq	.push_back(parseDS<std::string>("ph_seq", row));
			_noteTime.push_back(parseDS<float>("note_dur", row));
			_noteSlur.push_back(parseDS<int>("note_slur", row));
			_offset	.push_back(parseDS<float>("offset", row).at(0));
			_phNum[row] = makePhNum(_phSeq[row]);
			_phTime.push_back(parseDS<float>("ph_dur", row));

			if (!parseDS<float>("f0_timestep", row).empty()) {
				_f0_ticktime.push_back(parseDS<float>("f0_timestep", row).at(0));
			}
			_f0_seq.push_back(parseDS<float>("f0_seq", row));

			if (!parseDS<float>("f0_timestep", row).empty()) {
				_energy_ticktime.push_back(parseDS<float>("energy_timestep", row).at(0));
			}
			_energy.push_back(parseDS<float>("energy", row));

			if (!parseDS<float>("f0_timestep", row).empty()) {
				_breathiness_ticktime.push_back(parseDS<float>("breathiness_timestep", row).at(0));
			}
			_breathiness.push_back(parseDS<float>("breathiness", row));

			if (!parseDS<float>("f0_timestep", row).empty()) {
				_voicing_ticktime.push_back(parseDS<float>("voicing_timestep", row).at(0));
			}
			_voicing.push_back(parseDS<float>("voicing", row));

			if (!parseDS<float>("f0_timestep", row).empty()) {
				_tension_ticktime.push_back(parseDS<float>("tension_timestep", row).at(0));
			}
			_tension.push_back(parseDS<float>("tension", row));
		}

		_isLoad = true;
	}

	void parser::pack(float maxTimeS, float maxIntervalS) {
		if (!_isLoad) {
			load();
		}

		// 临时存储合并后的数据
		std::vector<std::vector<std::string>> new_phSeq;
		std::vector<std::vector<float>> new_phDur;
		std::vector<std::vector<int>> new_phNum;
		std::vector<std::vector<int>> new_noteSlur;
		std::vector<float> new_offset;
		std::vector<std::vector<std::string>> new_noteSeq;
		std::vector<std::vector<float>> new_noteDur;

		size_t i = 0;
		while (i < getRowCount()) {
			// 初始化合并容器
			std::vector<std::string> merged_phSeq;
			std::vector<float> merged_noteDur, merged_phDur;
			std::vector<std::string> merged_noteSeq;
			std::vector<int> merged_noteSlur;

			float merged_total_time = 0.0f;
			float merged_start = getOffset(i);
			size_t merge_count = 0; // 记录合并的行数

			// 尝试合并从 i 开始的多行
			for (size_t j = i; j < getRowCount(); ++j) {
				// 获取当前行的数据
				const auto& current_phSeq = _phSeq[j];
				const auto& current_phDur = getPhDur(j);
				const auto& current_noteDur = getNoteDur(j);
				float current_start = getOffset(j);
				float current_total = std::accumulate(current_noteDur.begin(), current_noteDur.end(), 0.0f);

				// 检查是否是合并首行
				if (j == i) {
					// 首行直接获取
					merged_phSeq = current_phSeq;
					merged_noteDur = current_noteDur;
					merged_phDur = current_phDur;
					merged_noteSeq = getNoteSeq(j);
					merged_noteSlur = getNoteSlur(j);
					merged_total_time = current_total;
				}
				else {
					// 不是首行则进入合并流程

					// 音符时长间隔
					float note_interval = current_start - (merged_start + merged_total_time) + 0.2;
					// 音素时长间隔
					float ph_interval = current_start - (merged_start + std::accumulate(merged_phDur.begin(), merged_phDur.end(), 0.0f) + 0.2);

					// 检查是否可合并（总时间 + 间隔 + 当前行时间 <= maxTimeS）
					if ((merged_total_time + note_interval + current_total > maxTimeS) ||
						(note_interval > maxIntervalS)
						) break; // 不可合并，结束合并

					// 间隔大于 0 且上一行结尾不是休止符
					if (note_interval > 0 && merged_phSeq[merged_phSeq.size() - 1] != "SP") {
						// 直接插入新的休止符
						merged_noteDur.push_back(note_interval);
						merged_noteSeq.push_back("rest");
						merged_phSeq.push_back("SP");
						merged_noteSlur.push_back(0);
						// 音素时长不为空时，同时处理音素时长
						if (!merged_phDur.empty()) {
							merged_phDur.push_back(ph_interval);
						}
					}
					// 上一行结尾是休止符
					else if (merged_phSeq[merged_phSeq.size() - 1] == "SP") {
						// 将时长添加到上一个休止符
						merged_noteDur[merged_noteDur.size() - 1] += note_interval;
						// 音素时长不为空时，同时处理音素时长
						if (!merged_phDur.empty()) {
							merged_phDur[merged_noteSeq.size() - 1] += ph_interval;
						}
					}

					// 合并当前行数据
					auto row_note_dur = getNoteSeq(j);
					auto row_note_slur = getNoteSlur(j);
					merged_phSeq.insert(merged_phSeq.end(), current_phSeq.begin(), current_phSeq.end());
					merged_noteDur.insert(merged_noteDur.end(), current_noteDur.begin(), current_noteDur.end());
					merged_phDur.insert(merged_phDur.end(), current_phDur.begin(), current_phDur.end());
					merged_noteSeq.insert(merged_noteSeq.end(), row_note_dur.begin(), row_note_dur.end());
					merged_noteSlur.insert(merged_noteSlur.end(), row_note_slur.begin(), row_note_slur.end());

					merged_total_time += note_interval + current_total;
				}

				merge_count++;
			}

			// 生成合并后的数据
			if (merge_count > 0) {
				// 计算新的 ph_num
				std::vector<int> merged_phNum = makePhNum(merged_phSeq);

				// 保存合并后的行
				new_phSeq.push_back(merged_phSeq);
				new_phDur.push_back(merged_phDur);
				new_phNum.push_back(merged_phNum);
				new_noteSlur.push_back(merged_noteSlur);
				new_noteDur.push_back(merged_noteDur);
				new_offset.push_back(merged_start);
				new_noteSeq.push_back(merged_noteSeq);

				// 跳过已合并的行
				i += merge_count;
			}
			else {
				// 无法合并，保留当前行
				new_phSeq.push_back(getPhSeq(i));
				new_phDur.push_back(getPhDur(i));
				new_phNum.push_back(getPhNum(i));
				new_noteSlur.push_back(getNoteSlur(i));
				new_noteDur.push_back(getNoteDur(i));
				new_offset.push_back(getOffset(i));
				new_noteSeq.push_back(getNoteSeq(i));
				i++;
			}
		}
		// 更新内部数据
		_phSeq		= std::move(new_phSeq);
		_phTime		= std::move(new_phDur);
		_phNum		= std::move(new_phNum);
		_noteSlur	= std::move(new_noteSlur);
		_noteTime	= std::move(new_noteDur);
		_offset		= std::move(new_offset);
		_noteSeq	= std::move(new_noteSeq);
		// 更新内部 json 对象
		updateJSONData();
	}

	std::vector<std::unique_ptr<music>> parser::split() {
		load();
		std::vector<std::unique_ptr<music>> out;

		const auto& rows_data = _dsData.GetArray();
		for (const auto& element : rows_data) {
			rapidjson::StringBuffer buffer;
			rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
			element.Accept(writer);

			if (buffer.GetSize() == 0) {
				continue; // 序列化失败处理
			}

			std::string row_ds = "[" + std::string(buffer.GetString()) + "]";
			out.emplace_back(get_music(row_ds, _language));
		}

		return out;
	}

	// Todo: 根据音符序列自动修复
	bool parser::set(
		const std::vector<std::string>& note_seq,
		const std::vector<float>& note_dur,
		const std::vector<int>& note_slur,
		const std::vector<std::string>& ph_seq,
		const std::vector<float>& ph_dur,
		float offset,
		int row
	) {
		// 先检查数据是否合法
		if (note_seq.empty())	throw DsParserError("音符序列为空");
		if (note_dur.empty())	throw DsParserError("音符时长为空");
		if (note_slur.empty())	throw DsParserError("连音标志为空");
		if (ph_seq.empty())		throw DsParserError("音素序列为空");
		if (ph_dur.empty())		throw DsParserError("音素时长为空");
		if (offset < 0)			throw DsParserError("起始时间小于 0");

		if (note_seq.size() != note_dur.size()) throw DsParserError("音符序列与音符时长未对齐");
		if (note_seq.size() != note_slur.size()) throw DsParserError("音符序列与连音标志未对齐");

		// 然后保存
		if (row >= _noteSeq.size())	_noteSeq.insert(_noteSeq.end(), row - _noteSeq.size() + 1, {});
		if (row >= _noteTime.size())	_noteTime.insert(_noteTime.end(), row - _noteTime.size() + 1, {});
		if (row >= _noteSlur.size()) _noteSlur.insert(_noteSlur.end(), row - _noteSlur.size() + 1, {});
		if (row >= _phSeq.size())	_phSeq.insert(_phSeq.end(), row - _phSeq.size() + 1, {});
		if (row >= _phTime.size())	_phTime.insert(_phTime.end(), row - _phTime.size() + 1, {});
		if (row >= _phNum.size())	_phNum.insert(_phNum.end(), row - _phNum.size() + 1, {});
		if (row >= _offset.size())	_offset.insert(_offset.end(), row - _offset.size() + 1, 0.0f);

		_noteSeq[row] = note_seq;		saveString("note_seq", note_seq, row);
		_noteTime[row] = note_dur;		saveString("note_dur", note_dur, row);
		_noteSlur[row] = note_slur;		saveString("note_slur", note_slur, row);
		_phSeq[row] = ph_seq;			saveString("ph_seq", ph_seq, row);
		_phTime[row] = ph_dur;			saveString("ph_dur", ph_dur, row);
		_phNum[row] = makePhNum(ph_seq);saveString("ph_num", ph_seq, row);
		_offset[row] = offset;			saveNumber("offset", offset, row);

		_hasData = true;
		_isLoad = true;
		_readyCase = true;

		return true;
	}

	bool parser::set(
		const std::vector<std::string>& note_seq, 
		const std::vector<float>& note_dur, 
		const std::vector<int>& note_slur, 
		float offset, 
		int row
	){
		// 先检查数据是否合法
		if (note_seq.empty())	throw DsParserError("音符序列为空");
		if (note_dur.empty())	throw DsParserError("音符时长为空");
		if (note_slur.empty())	throw DsParserError("连音标志为空");
		if (offset < 0)			throw DsParserError("起始时间小于 0");

		// 然后保存
		if (row >= _noteSeq.size())	_noteSeq.insert(_noteSeq.end(), row - _noteSeq.size() + 1, {});
		if (row >= _noteTime.size())	_noteTime.insert(_noteTime.end(), row - _noteTime.size() + 1, {});
		if (row >= _noteSlur.size()) _noteSlur.insert(_noteSlur.end(), row - _noteSlur.size() + 1, {});
		if (row >= _phSeq.size())	_phSeq.insert(_phSeq.end(), row - _phSeq.size() + 1, {});
		if (row >= _phTime.size())	_phTime.insert(_phTime.end(), row - _phTime.size() + 1, {});
		if (row >= _offset.size())	_offset.insert(_offset.end(), row - _offset.size() + 1, 0.0f);

		_noteSeq[row] = note_seq;		saveString("note_seq", note_seq, row);
		_noteTime[row] = note_dur;		saveString("note_dur", note_dur, row);
		_noteSlur[row] = note_slur;		saveString("note_slur", note_slur, row);
		_phSeq[row] = std::vector<std::string>(note_seq.size(), "SP");
		_phTime[row] = note_dur;		saveString("ph_dur", note_dur, row);
		_offset[row] = offset;			saveNumber("offset", offset, row);

		// 此时还不是有效 DS
		_readyCase = true;

		return true;
	}

	bool parser::set_lyrics(const std::vector<std::string>& ph_seq, const std::vector<float>& ph_dur, int row){
		if(!_readyCase)			throw DsParserError("没有词格");

		// 先检查数据是否合法
		if (ph_seq.empty())		throw DsParserError("音素序列为空");
		if (ph_dur.empty())		throw DsParserError("音素时长为空");

		// 然后保存
		if (row >= _phSeq.size())	_phSeq.insert(_phSeq.end(), row - _phSeq.size() + 1, {});
		if (row >= _phTime.size())	_phTime.insert(_phTime.end(), row - _phTime.size() + 1, {});
		if (row >= _phNum.size())	_phNum.insert(_phNum.end(), row - _phNum.size() + 1, {});

		_phSeq[row] = ph_seq;			saveString("ph_seq", ph_seq, row);
		_phTime[row] = ph_dur;			saveString("ph_dur", ph_dur, row);
		_phNum[row] = makePhNum(ph_seq);saveString("ph_num", ph_seq, row);

		_hasData = true;
		_isLoad = true;

		return true;
	}

	bool parser::set_syllable(const std::vector<std::string>& syllable_seq){



		return false;
	}

	std::string parser::get()const {
		rapidjson::StringBuffer buffer;
		rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
		_dsData.Accept(writer);
		return buffer.GetString();
	}

	int parser::getRowCount() const {
		return _dsData.GetArray().Size();
	}

	std::vector<std::string> parser::getPhSeq(int row) const{
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

	std::vector<std::string> parser::getPhSeq_raw(int row) const{
		return _phSeq[row];
	}

	parser& parser::setPitch(std::vector<float> data, float offset, int row) {
		if (row >= _f0_seq.size())	_f0_seq.insert(_f0_seq.end(), row - _f0_seq.size() + 1, {});
		if (row >= _offset.size())	_offset.insert(_offset.end(), row - _noteSeq.size() + 1, 0);
		_f0_seq[row] = data;
		_offset[row] = offset;
		saveString("f0_seq", data, row);
		return *this;
	}

	parser& parser::setPhTime(std::vector<float> data, float offset, int row) {
		if (row >= _phTime.size())	_phTime.insert(_phTime.end(), row - _phTime.size() + 1, {});
		if (row >= _offset.size())	_offset.insert(_offset.end(), row - _noteSeq.size() + 1, 0);
		_phTime[row] = data;
		_offset[row] = offset;
		saveString("ph_dur", data, row);
		return *this;
	}

	parser& parser::setEnergy(std::vector<float> data, float offset, int row) {
		if (row >= _energy.size())	_energy.insert(_energy.end(), row - _energy.size() + 1, {});
		if (row >= _offset.size())	_offset.insert(_offset.end(), row - _noteSeq.size() + 1, 0);
		_energy[row] = data;
		_offset[row] = offset;
		saveString("energy", data, row);
		return *this;
	}

	parser& parser::setBreathiness(std::vector<float> data, float offset, int row) {
		if (row >= _breathiness.size())	_breathiness.insert(_breathiness.end(), row - _breathiness.size() + 1, {});
		if (row >= _offset.size())	_offset.insert(_offset.end(), row - _noteSeq.size() + 1, 0);
		_breathiness[row] = data;
		_offset[row] = offset;
		saveString("breathiness", data, row);
		return *this;
	}

	parser& parser::setVoicing(std::vector<float> data, float offset, int row) {
		if (row >= _voicing.size())	_voicing.insert(_voicing.end(), row - _voicing.size() + 1, {});
		if (row >= _offset.size())	_offset.insert(_offset.end(), row - _noteSeq.size() + 1, 0);
		_voicing[row] = data;
		_offset[row] = offset;
		saveString("voicing", data, row);
		return *this;
	}

	parser& parser::setTension(std::vector<float> data, float offset, int row) {
		if (row >= _tension.size())	_tension.insert(_tension.end(), row - _tension.size() + 1, {});
		if (row >= _offset.size())	_offset.insert(_offset.end(), row - _noteSeq.size() + 1, 0);
		_tension[row] = data;
		_offset[row] = offset;
		saveString("tension", data, row);
		return *this;
	}

	parser& parser::setMouthOpening(std::vector<float> data, float offset, int row){
		if (row >= _mouthOpening.size())	_mouthOpening.insert(_mouthOpening.end(), row - _mouthOpening.size() + 1, {});
		if (row >= _offset.size())	_offset.insert(_offset.end(), row - _noteSeq.size() + 1, 0);
		_mouthOpening[row] = data;
		_offset[row] = offset;
		saveString("mouth_opening", data, row);
		return *this;
	}

	std::vector<std::string> parser::makePhSeq(
		const std::vector<std::string>& ph_seq,
		const std::vector<int>& note_slur
	) {
		auto ph_num = makePhNum(ph_seq);
		std::vector<std::string> out;
		for (int i = 0, j = 0, k = 0; i < note_slur.size(); ++i) {
			if (note_slur[i] == 1) {
				// 重复第一个（且唯一）音素
				out.push_back(ph_seq[j]);
			}
			else {
				// 普通音节处理
				for (int e = 0; e < ph_num[k];e++) {
					out.push_back(ph_seq[j++]);
				}
				++k;
			}
		}
		return out;
	}

	std::vector<int> parser::makePhNum(const std::vector<std::string>& ph_seq) {
		std::vector<int> ph_num;
		int num = 1;
		// 遍历每一个音素
		for (size_t i = 1; i < ph_seq.size(); ++i) {
			// 提取音素名称
			const std::string& ph = ph_seq[i];

			// 判断是否为元音（使用处理后的 ph）
			if (is_vowel(ph)) {
				ph_num.push_back(num);
				num = 0; // 重置计数器
			}
			num++;
		}
		// 最后一个音素，将计数器输出
		if (num > 0) {
			ph_num.push_back(num);
		}

		return ph_num;
	}

	std::vector<int> parser::makePhNum(const std::vector<int>& ph_num, const std::vector<int>& note_sulr) {
		std::vector<int> new_ph_num = ph_num;
		for (int i = 0;i < note_sulr.size();i++) {
			if (note_sulr[i] == 1) {
				new_ph_num[i]++;
			}
		}
		return new_ph_num;
	}

	bool is_vowel(std::string noteNum) {
		return (Vowel.find(noteNum) != Vowel.end());
	}

	template<typename T>
	std::vector<T> parser::parseDS(const std::string& key, size_t index) {
		// 获取指定索引的对象
		if (index >= _dsData.Size() || !_dsData[index].IsObject()) {
			return {};
		}
		const rapidjson::Value& obj = _dsData[index];
		// 查找指定的键
		if (!obj.HasMember(key.c_str())) {
			return {};
		}
		const rapidjson::Value& value = obj[key.c_str()];

		// 如果值是数字类型，将其转为字符串
		std::string valueStr;
		if (value.IsNumber()) {
			valueStr = std::to_string(value.GetDouble());  // 使用 GetDouble() 来处理整数或浮点数
		}
		else if (value.IsString()) {
			valueStr = value.GetString();  // 如果已经是字符串类型，直接使用
		}
		else {
			return {};  // 如果既不是数字也不是字符串，返回空结果
		}

		// 将值按空格分割成字符串
		std::vector<std::string> splitValues = DS::split_str(valueStr, ' ');

		// 将分割后的字符串转换为目标类型并返回
		std::vector<T> result;
		for (const auto& item : splitValues) {
			std::istringstream iss(item);
			T convertedValue;
			iss >> convertedValue;
			if (iss.fail()) {
				continue; // 只会输出可被正确解释的数据
			}
			result.push_back(convertedValue);
		}
		return result;
	}

	template<typename T>
	parser& parser::saveNumber(const std::string& key, const T& value, size_t index) {
		rapidjson::Value& arr = _dsData.GetArray();
		rapidjson::Value& obj = arr[index];

		rapidjson::Value json_key(key.c_str(), *_allocator);

		if constexpr (std::is_arithmetic_v<T>) {
			// 存储为数字
			rapidjson::Value json_val(value);
			obj.AddMember(json_key, json_val, *_allocator);
		}
		else {
			// 如果传入的是非数字类型
			static_assert(false, "Cannot store non-numeric value as a number.");
		}

		return *this;
	}

	template<typename T>
	parser& parser::saveString(const std::string& key, const T& value, size_t index) {
		rapidjson::Value& arr = _dsData.GetArray();

		// 确保数组足够大以容纳index
		if (index >= arr.Size()) {
			size_t currentSize = arr.Size();
			for (size_t i = currentSize; i <= index; ++i) {
				rapidjson::Value newObj(rapidjson::kObjectType); // 创建新对象
				arr.PushBack(newObj, *_allocator); // 添加到数组
			}
		}

		rapidjson::Value& obj = arr[index];
		rapidjson::Value json_key(key.c_str(), *_allocator);

		if constexpr (std::is_arithmetic_v<T>) {
			std::ostringstream oss;
			oss << value;
			rapidjson::Value json_val(oss.str().c_str(), *_allocator);
			obj.AddMember(json_key, json_val, *_allocator);
		}
		else if constexpr (std::is_same_v<T, std::vector<int>> || std::is_same_v<T, std::vector<float>> || std::is_same_v<T, std::vector<std::string>>) {
			std::ostringstream oss;
			bool first = true;
			for (const auto& num : value) {
				if (!first) oss << " ";
				oss << num;
				first = false;
			}
			rapidjson::Value json_val(oss.str().c_str(), *_allocator);
			obj.AddMember(json_key, json_val, *_allocator);
		}
		else {
			static_assert(false, "Unsupported type for string storage.");
		}

		return *this;
	}

	template<typename T>
	std::string vectorToString(const std::vector<T>& vec) {
		std::ostringstream oss;
		for (size_t i = 0; i < vec.size(); ++i) {
			if (i != 0) oss << " ";
			oss << vec[i];
		}
		return oss.str();
	}

	void parser::updateJSONData() {
		// 清空原有 JSON 数据
		_dsData.SetArray();
		rapidjson::Document::AllocatorType& allocator = _dsData.GetAllocator();

		// 遍历每一行数据
		for (size_t row = 0; row < _phSeq.size(); ++row) {
			rapidjson::Value rowObj(rapidjson::kObjectType);

			// 填充必填字段
			// 1. ph_seq
			rowObj.AddMember(
				"ph_seq",
				rapidjson::Value(vectorToString(_phSeq[row]).c_str(), allocator).Move(),
				allocator
			);

			// 2. ph_num
			rowObj.AddMember(
				"ph_num",
				rapidjson::Value(vectorToString(_phNum[row]).c_str(), allocator).Move(),
				allocator
			);

			// 3. note_dur
			rowObj.AddMember(
				"note_dur",
				rapidjson::Value(vectorToString(_noteTime[row]).c_str(), allocator).Move(),
				allocator
			);

			// 4. note_slur
			rowObj.AddMember(
				"note_slur",
				rapidjson::Value(vectorToString(_noteSlur[row]).c_str(), allocator).Move(),
				allocator
			);

			// 5. offset
			rowObj.AddMember("offset", _offset[row], allocator);

			// 6. note_seq
			rowObj.AddMember(
				"note_seq",
				rapidjson::Value(vectorToString(_noteSeq[row]).c_str(), allocator).Move(),
				allocator
			);

			// 6. ph_dur
			rowObj.AddMember(
				"ph_dur",
				rapidjson::Value(vectorToString(_phTime[row]).c_str(), allocator).Move(),
				allocator
			);

			// 7. tickTime（假设对应字段是 "f0_timestep"）
			if (row < _f0_ticktime.size()) {
				rowObj.AddMember("f0_timestep", _f0_ticktime[row], allocator);
			}


			// 将当前行对象添加到 JSON 数组
			_dsData.PushBack(rowObj.Move(), allocator);
		}
	}
}