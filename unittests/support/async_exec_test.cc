
#include "srsgnb/srslog/bundled/fmt/ostream.h"
#include "srsgnb/support/async/async_task.h"
#include "srsgnb/support/async/schedule_on.h"
#include "srsgnb/support/task_worker.h"
#include "srsgnb/support/test_utils.h"

using namespace srsgnb;

struct blocking_worker {
  blocking_worker(size_t q_size) : task_queue(q_size) {}
  ~blocking_worker() { request_stop(); }

  void execute(unique_task task) { task_queue.push_blocking(std::move(task)); }

  void request_stop()
  {
    stop_token = true;
    execute([]() {});
  }

  void run()
  {
    while (not stop_token) {
      bool success = false;
      auto job     = task_queue.pop_blocking(&success);
      if (not success) {
        break;
      }
      job();
    }
  }

private:
  dyn_blocking_queue<unique_task> task_queue;
  std::atomic<bool>               stop_token{false};
};

struct blocking_executor : public task_executor {
  blocking_executor(blocking_worker& w) : worker(w) {}

  void execute(unique_task task) override { worker.execute(std::move(task)); }

private:
  blocking_worker& worker;
};

/// In this test, we keep hopping between workers. In each worker we increment an unprotected integer, and print
/// Given that the access to the integer is sequential, we dont need mutexes.
void test_move_exec_context()
{
  blocking_worker      worker0{128};
  task_worker          worker1{"WORKER1", 128}, worker2{"WORKER2", 128};
  blocking_executor    exec0{worker0};
  task_worker_executor exec1{worker1};
  task_worker_executor exec2{worker2};

  int             count = 0;
  async_task<int> t =
      launch_async([exec0, exec1, exec2, &count, &worker0](coro_context<async_task<int> >& ctx) mutable {
        CORO_BEGIN(ctx);
        count++;
        fmt::print("{}: Running in thread: \"{}\"\n", count, thread::get_name());
        CORO_AWAIT(schedule_on(exec1));
        count++;
        fmt::print("{}: Running in thread: \"{}\"\n", count, thread::get_name());
        CORO_AWAIT(schedule_on(exec2));
        count++;
        fmt::print("{}: Running in thread: \"{}\"\n", count, thread::get_name());
        CORO_AWAIT(schedule_on(exec0));
        count++;
        fmt::print("{}: Running in thread: \"{}\"\n", count, thread::get_name());
        worker0.request_stop(); // as we are in worker0, the cancel command is only processed after the return
        CORO_RETURN(count);
      });

  worker0.run();
  TESTASSERT(count == 4);
}

int main()
{
  test_move_exec_context();
}