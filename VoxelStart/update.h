#include <future>
#include <vector>

template <typename T>
bool is_ready(const std::future<T>& target)
{
	return target.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
}


template <typename ResultType>
struct updater
{
	bool new_result()
	{
		
		// loop from the back to find the most recently scheduled task that is completed
		for (int i = this->m_tasks.size() - 1; i >= 0; i--)
		{
			if (is_ready(this->m_tasks[i]))
			{
				return true;
			}
		}
		return false;
	}
	ResultType get_result()
	{
		// loop from the back to find the most recently scheduled task that is completed
		for (int i = this->m_tasks.size() - 1; i >= 0; i--)
		{
			if (is_ready(this->m_tasks[i]))
			{
				// get result
				auto ret = this->m_tasks[i].get();

				// clean up previous threads
				this->m_tasks.erase(this->m_tasks.begin(), this->m_tasks.begin() + i + 1);

				// return result
				return ret;
			}
		}
		return ResultType();
	}
	template <typename Fn, typename... Args>
	void schedule(Fn f, Args&&... args)
	{
		this->m_tasks.emplace_back(std::async(std::launch::async, f, args...));
	}

	int remaining()
	{

		return this->m_tasks.size();
	}
private:
	std::vector<std::future<ResultType>> m_tasks;
};
