/*	Multi Threading manager
	more info in README for PreCore Project

	Copyright 2019 Lukas Cone

	Licensed under the Apache License, Version 2.0 (the "License");
	you may not use this file except in compliance with the License.
	You may obtain a copy of the License at

		http://www.apache.org/licenses/LICENSE-2.0

	Unless required by applicable law or agreed to in writing, software
	distributed under the License is distributed on an "AS IS" BASIS,
	WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
	See the License for the specific language governing permissions and
	limitations under the License.
*/

#pragma once
#include <thread>
#include <future>
#include <chrono>

struct __exampleTraits
{
	typedef int return_type; //thread future return type

	return_type RetreiveItem(); //calling working function here
	operator bool(); //compare if queue is finished
	void operator++(int); //increase done queues by 1
	int NumQueues() const;
};

template<class Traits> void RunThreadedQueue(Traits &traits)
{
	const int numThreads = std::thread::hardware_concurrency();
	const int numQueues = traits.NumQueues();

	std::vector<std::future<typename Traits::return_type>> workingThreads(numThreads > numQueues ? numQueues : numThreads);

	for (auto &wt : workingThreads)
	{
		wt = std::async(&Traits::RetreiveItem, traits);
		traits++;
	}

	while (traits)
	{
		for (auto &wt : workingThreads)
		{
			if (!traits)
				break;

			if ((wt.wait_for(std::chrono::milliseconds(2)) == std::future_status::ready))
			{
				wt = std::async(&Traits::RetreiveItem, traits);
				traits++;
			}
		}
	}

	for (auto &wt : workingThreads)
		wt.wait();
}