/******************************************************************************

   @file    gametimer.hpp
   @author  Rajmund Szymanski
   @date    01.11.2020
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

template<typename T = std::chrono::milliseconds>
class GameTimer
{
	std::chrono::time_point<std::chrono::system_clock> start_;
	T count_;

	static constexpr T COUNTING = T(-1);

public:

	GameTimer( const int _period = -1 )
	{
		start(_period);
	}

	template<typename D = T>
	GameTimer( const D _duration )
	{
		start(_duration);
	}

	void start( const int _period = -1 )
	{
		start(T(_period));
	}

	template<typename D = T>
	void start( const D _duration )
	{
		start_ = std::chrono::system_clock::now();
		count_ = std::chrono::duration_cast<T>(_duration);
	}

	void stop()
	{
		if (count_ == COUNTING)
			count_ = std::chrono::duration_cast<T>(std::chrono::system_clock::now() - start_);
	}

	void reset()
	{
		count_ = T::zero();
	}

	int counter()
	{
		T time_ = std::chrono::duration_cast<T>(std::chrono::system_clock::now() - start_);
		return (count_ == COUNTING ? time_ : count_).count();
	}

	bool expired()
	{
		T time_ = std::chrono::duration_cast<T>(std::chrono::system_clock::now() - start_);
		if (count_ > time_)
			return false;

		start_ += count_;
		return true;
	}

	bool waiting()
	{
		return !expired();
	}

	int remaining()
	{
		T time_ = std::chrono::duration_cast<T>(std::chrono::system_clock::now() - start_);
		return count_ > time_ ? (count_ - time_).count() : 0;
	}
};
