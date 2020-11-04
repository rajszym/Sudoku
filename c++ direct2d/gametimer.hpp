/******************************************************************************

   @file    gametimer.hpp
   @author  Rajmund Szymanski
   @date    04.11.2020
   @brief   GameTimer class

*******************************************************************************

   Copyright (c) 2018 - 2020 Rajmund Szymanski. All rights reserved.

   Permission is hereby granted, free of charge, to any person obtaining a copy
   of this software and associated documentation files (the "Software"), to
   deal in the Software without restriction, including without limitation the
   rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
   sell copies of the Software, and to permit persons to whom the Software is
   furnished to do so, subject to the following conditions:

   The above copyright notice and this permission notice shall be included
   in all copies or substantial portions of the Software.

   THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
   OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
   FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
   THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
   LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
   FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
   IN THE SOFTWARE.

******************************************************************************/

#pragma once

#include <chrono>

class GameTimer
{
	using Clock  = std::chrono::high_resolution_clock;

	Clock::time_point start_;
	Clock::duration   count_;

	static constexpr Clock::duration COUNTING{ -1 };

public:

	GameTimer()
	{
		start();
	}

	template<typename Duration>
	GameTimer( const int _d )
	{
		start(_d);
	}

	template<typename Duration>
	GameTimer( const Duration _d )
	{
		start(_d);
	}

	void start()
	{
		start(COUNTING);
	}

	template<typename Duration>
	void start( const int _d )
	{
		start(Duration(_d));
	}

	template<typename Duration>
	void start( const Duration _d )
	{
		start_ = Clock::now();
		count_ = std::chrono::duration_cast<Clock::duration>(_d);
	}

	void stop()
	{
		if (count_ == COUNTING)
			count_ = Clock::now() - start_;
	}

	void reset()
	{
		count_ = Clock::duration::zero();
	}

	template<typename Duration = std::chrono::seconds>
	Duration::rep counter()
	{
		auto time_ = count_ == COUNTING ? Clock::now() - start_ : count_;
		return std::chrono::duration_cast<Duration>(time_).count();
	}

	bool expired()
	{
		auto time_ = Clock::now() - start_;
		if (time_ < count_)
			return false;

		start_ += count_;
		return true;
	}

	bool waiting()
	{
		return !expired();
	}

	template<typename Duration = std::chrono::milliseconds>
	Duration::rep from()
	{
		auto time_ = Clock::now() - start_;
		return std::chrono::duration_cast<Duration>(std::min(time_, count_)).count();
	}

	template<typename Duration = std::chrono::milliseconds>
	Duration::rep until()
	{
		auto time_ = start_ + count_ - Clock::now();
		return std::chrono::duration_cast<Duration>(std::max(time_, Clock::duration::zero())).count();
	}
};
