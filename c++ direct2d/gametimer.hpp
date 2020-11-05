/******************************************************************************

   @file    gametimer.hpp
   @author  Rajmund Szymanski
   @date    05.11.2020
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

template<typename Rep, typename Period = std::ratio<1>>
class GameTimer
{
	using Clock  = std::chrono::high_resolution_clock;
	using Target = std::chrono::duration<Rep, Period>;

	Clock::time_point start_;
	Clock::duration   count_;

	bool expired( const Clock::duration _time, const bool _reload)
	{
		if (_time < count_)
			return false;
		if (_reload)
			start_ += count_;
		return true;
	}

public:

	GameTimer()
	{
		start();
	}

	template <typename T>
	GameTimer( const T _period )
	{
		start(_period);
	}

	void restart()
	{
		start_ = Clock::now();
	}

	void start()
	{
		count_ = Clock::duration::max();
		restart();
	}

	template <typename T>
	void start( const T _period )
	{
		count_ = std::chrono::duration_cast<Clock::duration>(Target(_period));
		restart();
	}

	void reset()
	{
		count_ = Clock::duration::zero();
	}

	void stop()
	{
		auto _time = Clock::now() - start_;
		if (_time < count_)
			count_ = _time;
	}

	bool expired( const bool _reload = true )
	{
		auto _time = Clock::now() - start_;
		return expired(_time, _reload);
	}

	template <typename T>
	bool expired( const T _period, const bool _reload = true )
	{
		count_ = std::chrono::duration_cast<Clock::duration>(Target(_period));
		return expired(_reload);
	}

	bool waiting( const bool _reload = true )
	{
		return !expired(_reload);
	}

	template <typename T>
	bool waiting( const T _period, const bool _reload = true )
	{
		count_ = std::chrono::duration_cast<Clock::duration>(Target(_period));
		return waiting(_reload);
	}

	Target::rep from( const bool _reload = false )
	{
		auto _time = Clock::now() - start_;
		return std::chrono::duration_cast<Target>(expired(_time, _reload) ? count_ : _time).count();
	}

	template <typename T>
	Target::rep from( const T _period, const bool _reload = false )
	{
		count_ = std::chrono::duration_cast<Clock::duration>(Target(_period));
		return from(_reload);
	}

	Target::rep until( const bool _reload = false )
	{
		auto _time = Clock::now() - start_;
		return std::chrono::duration_cast<Target>(expired(_time, _reload) ? Clock::duration::zero() : count_ - _time).count();
	}

	template <typename T>
	Target::rep until( const T _period, const bool _reload = false )
	{
		count_ = std::chrono::duration_cast<Clock::duration>(Target(_period));
		return until(_reload);
	}

	Target::rep now()
	{
		return from(false);
	}
};
