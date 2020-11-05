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

template<typename _Rep, typename _Period = std::ratio<1>>
class GameTimer
{
	using Clock  = std::chrono::high_resolution_clock;
	using Target = std::chrono::duration<_Rep, _Period>;

	Clock::time_point start_;
	Clock::duration   count_;

	static constexpr Clock::duration COUNTING{ -1 };

public:

	GameTimer()
	{
		start();
	}

	GameTimer( const auto _period )
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

	void start( const Target _duration )
	{
		count_ = std::chrono::duration_cast<Clock::duration>(_duration);
		restart();
	}

	void start( const auto _period )
	{
		start(Target(_period));
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

	bool expired( Clock::duration _time, bool _reload )
	{
		if (_time < count_)
			return false;
		if (_reload)
			start_ += count_;
		return true;
	}

	bool expired( bool _reload = true )
	{
		auto _time = Clock::now() - start_;
		return expired(_time, _reload);
	}

	bool waiting( bool _reload = true )
	{
		return !expired(_reload);
	}

	Target::rep from( bool _reload = false )
	{
		auto _time = Clock::now() - start_;
		if (expired(_time, _reload))
			return std::chrono::duration_cast<Target>(count_).count();
		return std::chrono::duration_cast<Target>(_time).count();
	}

	Target::rep until( bool _reload = false )
	{
		auto _time = Clock::now() - start_;
		if (expired(_time, _reload))
			return static_cast<Target::rep>(0);
		return std::chrono::duration_cast<Target>(count_ - _time).count();
	}

	Target::rep now()
	{
		return from(false);
	}
};
