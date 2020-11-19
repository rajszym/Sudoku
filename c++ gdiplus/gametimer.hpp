/******************************************************************************

   @file    gametimer.hpp
   @author  Rajmund Szymanski
   @date    17.11.2020
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
	using Source = std::chrono::high_resolution_clock::duration;
	using Target = std::chrono::duration<Rep, Period>;

	Clock::time_point start_;
	Clock::duration   count_;

	bool expired_( const Source _time, const Source _duration, const bool _reload )
	{
		if (_time < _duration)
			return false;
		if (_reload)
			start_ += _duration;
		return true;
	}

	bool expired_( const Source _duration, const bool _reload )
	{
		auto _time = Clock::now() - start_;
		return expired_(_time, _duration, _reload);
	}

	Rep from_( const Source _duration, const bool _reload )
	{
		auto _time = Clock::now() - start_;
		return std::chrono::duration_cast<Target>(expired_(_time, _duration, _reload) ? _duration : _time).count();
	}

	Rep until_( const Source _duration, const bool _reload )
	{
		auto _time = Clock::now() - start_;
		return std::chrono::duration_cast<Target>(expired_(_time, _duration, _reload) ? Source::zero() : _duration - _time).count();
	}

public:

	GameTimer()
	{
		start();
	}

	GameTimer( const Rep _period )
	{
		start(_period);
	}

	void start_if( bool cond )
	{
		if (cond) start();
		else      reset();
	}

	void start()
	{
		count_ = Source::max();
		restart();
	}

	void start( const Rep _period )
	{
		count_ = std::chrono::duration_cast<Source>(Target(_period));
		restart();
	}

	void restart()
	{
		start_ = Clock::now();
	}

	void reset()
	{
		count_ = Source::zero();
	}

	void stop()
	{
		auto _time = Clock::now() - start_;
		if (_time < count_)
			count_ = _time;
	}

	bool expired( const bool _reload = true )
	{
		return expired_(count_, _reload);
	}

	bool expired( const Rep _period, const bool _reload = true )
	{
		auto _duration = std::chrono::duration_cast<Source>(Target(_period));
		return expired_(_duration, _reload);
	}

	bool waiting( const bool _reload = true )
	{
		return !expired_(count_, _reload);
	}

	bool waiting( const Rep _period, const bool _reload = true )
	{
		auto _duration = std::chrono::duration_cast<Source>(Target(_period));
		return !expired_(_duration, _reload);
	}

	Rep from( const bool _reload = false )
	{
		return from_(count_, _reload);
	}

	Rep from( const Rep _period, const bool _reload = false )
	{
		auto _duration = std::chrono::duration_cast<Source>(Target(_period));
		return from_(_duration, _reload);
	}

	Rep until( const bool _reload = false )
	{
		return until_(count_, _reload);
	}

	Rep until( const Rep _period, const bool _reload = false )
	{
		auto _duration = std::chrono::duration_cast<Source>(Target(_period));
		return until_(_duration, _reload);
	}

	Rep now()
	{
		return from_(count_, false);
	}
};
