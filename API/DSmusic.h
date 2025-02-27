#pragma once
#include <string>
#include <vector>
#include <unordered_map>
#include <stdexcept>

namespace DS {
// �Զ�����쳣��
class DsParserError : public std::runtime_error {
public:
explicit DsParserError(const std::string& msg): std::runtime_error(msg) {}
};

//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
// DS ��������� DS::music
// ֧�ִ��ڴ��м������ݻ�ֱ�ӽ������� DS ����
// ������Ҫ��Щ���������
// note_seq		�������У�ÿ������������
// note_dur		����ʱ�䣺ÿ�������ĳ���ʱ��
// ֧�ִ��ڴ��м������ݻ�ֱ�ӽ������� DS ����
// note_slur	������־������������һ�������Ƿ�������
// note_slur	������־������������һ�������Ƿ�������
// ph_seq		�������У�ÿ�����ص�����
// ph_dur		����ʱ�䣺ÿ�����صĳ���ʱ��
// offset		ƫ��ʱ�䣺��������ʼλ�������ƫ�Ƶ�����
//>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
class music {
public:
	virtual ~music() = default;

	// ��ʼ��������������Ҫ���ֶβ��洢�ڳ�Ա������
	// ���������̼߳��أ����������ⲿ����
	virtual	void load() = 0;

	// ������������δ���������� GPU ������
	// ע�⣺���ܵ���������΢��λ
	// - ������δ�С(��)
	// - ����������С�м��(��)
	virtual void pack(
		float time_s, 
		float maxInterval_s
	) = 0;

	// ���ڴ��м�������
	// ���ؼ����Ƿ���ȫ��ȷ�ı�־
	// ������ĳ�ֶ�δ���뵫����ͨ���Զ��������޸�ʱ������false
	// �޷��޸�ʱ�׳� DsParserError �쳣
	// - ��������
	// - ����ʱ��
	// - ������־
	// - ��������
	// - ����ʱ��
	// - ������־
	// - ������ʼʱ��
	// - Ҫ�������
	virtual bool set(
		const std::vector<std::string>& note_seq,
		const std::vector<float>& note_dur,
		const std::vector<int>& note_slur,
		const std::vector<std::string>& ph_seq,
		const std::vector<float>& ph_dur,
		float offset = 0,
		int row = 0
	) = 0;

	// �����л�
	virtual std::string get()const = 0;

	// �Ƿ�Ϊ��
	virtual bool empty() const = 0;

	// ��ȡ ds �ļ��е�������
	virtual int getRowCount() const = 0;

	// ��ȡ��������
	virtual std::vector<std::string> getPhSeq(int row) const = 0;

	// ��ȡÿ�����ڵ�������������
	virtual std::vector<int> getPhNum(int row) const = 0;

	// ��ȡ��������
	virtual std::vector<std::string> getNoteSeq(int row) const = 0;

	// ��ȡ����ʱ��
	virtual std::vector<float> getNoteDur(int row) const = 0;

	// ��ȡ������־
	virtual std::vector<int> getNoteSlur(int row) const = 0;

	// ��ȡĳ�е�ƫ��ʱ��
	virtual float getOffset(int row) const = 0;

	// ��ȡȫ���е�ƫ��ʱ��
	virtual std::vector<float> getOffset() const = 0;

	// ��ȡ����ʱ��
	virtual float getTickTime(int row = 0) const = 0;

	// ��ȡ����
	virtual std::string getLang() const = 0;

	// ������������
	virtual music& setPitch(std::vector<float> data, float offset, int row) = 0;
	// ��ȡ��������
	virtual const std::vector<float>& getPitch(int row) const = 0;

	// ��������ʱ������
	virtual music& setPhTime(std::vector<float> data, float offset, int row) = 0;
	// ��ȡ����ʱ������
	virtual const std::vector<float>& getPhDur(int row) const = 0;

	// ������������
	virtual music& setEnergy(std::vector<float> data, float offset, int row) = 0;
	// ��ȡ��������
	virtual const std::vector<float>& getEnergy(int row) const = 0;

	// ������������
	virtual music& setBreathiness(std::vector<float> data, float offset, int row) = 0;
	// ��ȡ��������
	virtual const std::vector<float>& getBreathiness(int row) const = 0;

	// ���ñ���������
	virtual music& setVoicing(std::vector<float> data, float offset, int row) = 0;
	// ��ȡ����������
	virtual const std::vector<float>& getVoicing(int row) const = 0;

	// ������������
	virtual music& setTension(std::vector<float> data, float offset, int row) = 0;
	// ��ȡ��������
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