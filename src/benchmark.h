#ifndef BENCHMARK_H
#define BENCHMARK_H


class CBenchmark
{
private:
	int m_Time;
	int m_ClockStart;

public:
	CBenchmark();
	void Restart();
	void Pause();
	void Unpause();
	int GetTime() const;
};


#endif
