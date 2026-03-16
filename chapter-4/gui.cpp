#include <deque>
#include <mutex>
#include <future>
#include <thread>
#include <utility>

std::mutex m;
std::deque<std::packaged_task<void()> > tasks;

bool gui_shutdown_message_received();
void get_and_process_gui_message();

void gui_thread() // 1) Thread loops until message has been recieved
{
    while(!gui_shutdown_message_received()) // 2) Flag to check that we havent recieved the shutdown message
    {
        get_and_process_gui_message(); // 3) Polling for GUI messages
        std::packaged_task<void()> task;
        {
            std::lock_guard<std::mutex> lk(m); // 3.5) Scoped as there is a lock_guard incase we need to complete a task
            if(tasks.empty()) continue;  // 4) If theres no tasks, it loops again
            task=std::move(tasks.front()); // 5) Moves task into a variable and pops it from queue()
            tasks.pop_front();
        }
        task(); // 6) Runs the task
    }
}

std::thread gui_bg_thread(gui_thread);
template<typename Func>
std::future<void> post_task_for_gui_thread(Func f)
{
    std::packaged_task<void()> task(f); // 7) new packaged_task is created from supplied function
    std::future<void> res=task.get_future(); // 8) Future is obtained from the task by calling get_future()
    std::lock_guard<std::mutex> lk(m); 
    tasks.push_back(std::move(task)); // 9) Task is put on the queue before the future is returned
    return res;
}