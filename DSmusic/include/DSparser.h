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
		// ���캯��------------------------------------
		
		// �����е����� DS �ṹ����ȡ
		parser(
			const std::string& json,
			const std::string& language
		);
		// ����һ���յ� DS
		parser(
			const std::string& language
		);

		// ��ʼ��������������Ҫ���ֶβ��洢�ڳ�Ա������
		void load();

		// ������������δ���������� GPU ������
		void pack(float time_s, float maxIntervalS);

		std::vector<std::unique_ptr<music>> split();

		// ���ڴ��м������ݣ�������
		// ���ؼ��سɹ����ı�־
		// - ��������
		// - ����ʱ��
		// - ������־
		// - ��������
		// - ����ʱ��
		// - ������־
		// - ������ʼʱ��
		// - Ҫ�������
		bool set(
			const std::vector<std::string>& note_seq,
			const std::vector<float>& note_dur,
			const std::vector<int>& note_slur,
			const std::vector<std::string>& ph_seq,
			const std::vector<float>& ph_dur,
			float offset = 0,
			int row = 0
		);

		// ���ڴ��м������ݣ����ʸ�
		// - ��������
		// - ����ʱ��
		// - ������־
		// - ������ʼʱ��
		// - Ҫ�������
		bool set(
			const std::vector<std::string>& note_seq,
			const std::vector<float>& note_dur,
			const std::vector<int>& note_slur,
			float offset = 0,
			int row = 0
		);

		// ���ø��
		// - ��������
		// - ����ʱ��
		// - Ҫ�������
		bool set_lyrics(
			const std::vector<std::string>& ph_seq,
			const std::vector<float>& ph_dur,
			int row = 0
		);


		bool set_syllable(
			const std::vector<std::string>& syllable_seq
		);

		// �����л�
		std::string get()const ;

		// �Ƿ�Ϊ��
		bool empty() const { return !_hasData; }

		// ��ȡ ds �ļ��е�������
		int getRowCount() const;

		// ��ȡ��������
		std::vector<std::string> getPhSeq(int row) const;
		std::vector<std::string> getPhSeq_raw(int row) const;

		// ��ȡÿ�����ڵ�������������
		std::vector<int> getPhNum(int row) const { return _phNum.at(row); }

		// ��ȡ��������
		std::vector<std::string> getNoteSeq(int row) const { return _noteSeq.at(row); }

		// ��ȡ����ʱ��
		std::vector<float> getNoteDur(int row) const { return _noteTime.at(row); }

		// ��ȡ������־
		std::vector<int> getNoteSlur(int row) const { return _noteSlur.at(row); }

		// ��ȡĳ�е�ƫ��ʱ��
		float getOffset(int row) const { return _offset.at(row); }

		// ��ȡȫ���е�ƫ��ʱ��
		std::vector<float> getOffset() const { return _offset; }

		// ��ȡ����ʱ��
		float getTickTime(int row = 0) const { return _f0_ticktime.at(row); }

		// ��ȡ����
		std::string getLang() const { return _language; }

		// ������������
		parser& setPitch(std::vector<float> data, float offset, int row);
		// ��ȡ��������
		const std::vector<float>& getPitch(int row) const { return _f0_seq.at(row); }

		// ��������ʱ������
		parser& setPhTime(std::vector<float> data, float offset, int row);
		// ��ȡ����ʱ������
		const std::vector<float>& getPhDur(int row) const { return _phTime.at(row); }

		// ������������
		parser& setEnergy(std::vector<float> data, float offset, int row);
		// ��ȡ��������
		const std::vector<float>& getEnergy(int row) const { 
			if (_energy.empty()) return{};
			return _energy.at(row); 
		}

		// ������������
		parser& setBreathiness(std::vector<float> data, float offset, int row);
		// ��ȡ��������
		const std::vector<float>& getBreathiness(int row) const { 
			if (_breathiness.empty()) return{};
			return _breathiness.at(row); 
		}

		// ���÷�������
		parser& setVoicing(std::vector<float> data, float offset, int row);
		// ��ȡ��������
		const std::vector<float>& getVoicing(int row) const { 
			if (_voicing.empty()) return{};
			return _voicing.at(row); 
		}

		// ������������
		parser& setTension(std::vector<float> data, float offset, int row);
		// ��ȡ��������
		const std::vector<float>& getTension(int row) const { 
			if (_tension.empty()) return{};
			return _tension.at(row); 
		}

	private:
		bool _isLoad = false;	// �Ѽ��ص��ڴ棬�ɵ��� get ϵ�з�����ȡ����
		bool _hasData = false;	// �п������ݣ����� json �����ڵĻ��ڴ��е�
		bool _readyCase = false;// �ʸ��Ѿ���

		rapidjson::Document _dsData;  // ���洫��� ds �ļ�����
		rapidjson::Document::AllocatorType* _allocator = nullptr;

		std::string _language; // ʹ�õ�����

		// �ڲ����ݴ洢
		std::vector<std::vector<std::string>> _phSeq = {};	// ��������
		std::vector<std::vector<int>> _phNum = {};			// ���ڻ���
		std::vector<std::vector<int>> _noteSlur = {};		// ������־
		std::vector<float> _offset = {};					// ƫ��ʱ��

		std::vector<std::vector<std::string>> _noteSeq = {};// ��������
		std::vector<std::vector<float>> _noteTime = {};		// ����ʱ��

		std::vector<std::vector<float>> _phTime = {};		// ����ʱ������

		std::vector<std::vector<float>> _f0_seq = {};		// ��������
		std::vector<float> _f0_ticktime = {};				// ���߲���ʱ��

		std::vector<std::vector<float>> _energy = {};		// ��������
		std::vector<float> _energy_ticktime = {};           // ��������ʱ��

		std::vector<std::vector<float>> _breathiness = {};  // ��������
		std::vector<float> _breathiness_ticktime = {};      // ��������ʱ��

		std::vector<std::vector<float>> _voicing = {};      // ��������
		std::vector<float> _voicing_ticktime = {};          // ��������ʱ��

		std::vector<std::vector<float>> _tension = {};      // ��������
		std::vector<float> _tension_ticktime = {};			// ��������ʱ��

		// ���ߺ���----------------------------------------------------------------------------

		// �� ds �ļ�����ȡ��Ӧ��ֵ
		template <typename T>
		std::vector<T> parseDS(const std::string& key, size_t index);
		// ����Ϊ����
		template<typename T>
		parser& saveNumber(const std::string& key, const T& value, size_t index);
		// ����Ϊ�ַ���
		template<typename T>
		parser& saveString(const std::string& key, const T& value, size_t index);

		// ��������
		std::vector<int> makePhNum(const std::vector<std::string>& ph_seq);

		// TODO ��Щ��Ϊ��ʱ��ת�������ʩ����ת����������֧�ֺ�Ӧ��ɾ��----------
		// Ӧ��ת�����У������µ������б�
		std::vector<std::string> makePhSeq(
			const std::vector<std::string>& ph_seq,
			const std::vector<int>& note_slur
		);
		// ��������
		std::vector<int> makePhNum(const std::vector<int>& ph_num, const std::vector<int>& note_sule);
		void updateJSONData();
		//------------------------------------------------------------------------

		// ����������飺�Ƿ�Ϊstd::vector
		template<typename>
		struct is_vector : std::false_type {};

		template<typename T, typename A>
		struct is_vector<std::vector<T, A>> : std::true_type {};

		// �ݹ�����ת������
		template<typename U>
		static std::string convert_to_string(const U& value) {
			if constexpr (is_vector<U>::value) {
				// ����std::vector
				std::ostringstream oss;
				for (size_t i = 0; i < value.size(); ++i) {
					if (i != 0) oss << " ";
					oss << convert_to_string(value[i]); // �ݹ鴦��Ԫ��
				}
				return oss.str();
			}
			else if constexpr (std::is_same_v<U, std::string>) {
				// �����ַ���
				return value;
			}
			else {
				// ����������ͣ�int64_t��float�ȣ�
				return std::to_string(value);
			}
		}
	};
}