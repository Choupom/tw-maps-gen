#include <time.h>

#include "benchmark.h"


CBenchmark::CBenchmark()
{
	Restart();
}

void CBenchmark::Restart()
{
	m_Time = 0;
}

void CBenchmark::Pause()
{
	m_Time += clock() - m_ClockStart;
}

void CBenchmark::Unpause()
{
	m_ClockStart = clock();
}

int CBenchmark::GetTime() const
{
	return m_Time*1000/CLOCKS_PER_SEC;
}
