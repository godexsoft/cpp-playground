#include <boost/asio/bind_cancellation_slot.hpp>
#include <boost/asio/cancellation_signal.hpp>
#include <boost/asio/cancellation_type.hpp>
#include <boost/asio/error.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/steady_timer.hpp>
#include <boost/asio/strand.hpp>
#include <boost/asio/thread_pool.hpp>
#include <boost/asio/use_future.hpp>
#include <boost/signals2/signal.hpp>
#include <gtest/gtest.h>

#include <atomic>
#include <thread>

using namespace testing;

namespace util {
inline constexpr struct PropagatingCompletionHandler {
  void operator()(std::exception_ptr ePtr) {
    if (ePtr)
      std::rethrow_exception(ePtr);
  }
} kPROPAGATE_EXCEPTIONS;
} // namespace util

struct TimerTest : Test {
  boost::asio::thread_pool ctx_{1};
  boost::asio::strand<boost::asio::thread_pool::executor_type> strand_ =
      boost::asio::make_strand(ctx_);

  boost::asio::cancellation_signal cancelSignal_;
  std::atomic_bool running_ = false;
};

TEST_F(TimerTest, DoesNotWork) {
  running_ = true;

  boost::asio::spawn(
      strand_,
      [this](boost::asio::yield_context yield) {
        boost::asio::steady_timer timer(
            boost::asio::get_associated_executor(strand_));
        boost::system::error_code ec;

        while (running_) {
          timer.expires_after(std::chrono::milliseconds{1});
          timer.async_wait(boost::asio::bind_cancellation_slot(
              cancelSignal_.slot(), yield[ec]));

          if (ec == boost::asio::error::operation_aborted)
            break;
        }
      },
      util::kPROPAGATE_EXCEPTIONS);

  std::this_thread::sleep_for(std::chrono::milliseconds{10});

  running_ = false;
  cancelSignal_.emit(boost::asio::cancellation_type::terminal);

  ctx_.join();
}

TEST_F(TimerTest, WorksButSharedPtr) {
  running_ = true;

  auto timer = std::make_shared<boost::asio::steady_timer>(
      boost::asio::get_associated_executor(strand_));
  boost::asio::spawn(
      strand_,
      [this, timer](boost::asio::yield_context yield) {
        boost::system::error_code ec;

        while (running_) {
          timer->expires_after(std::chrono::milliseconds{1});
          timer->async_wait(boost::asio::bind_cancellation_slot(
              cancelSignal_.slot(), yield[ec]));

          if (ec == boost::asio::error::operation_aborted or not running_)
            break;
        }
      },
      util::kPROPAGATE_EXCEPTIONS);

  std::this_thread::sleep_for(std::chrono::milliseconds{10});

  running_ = false;
  cancelSignal_.emit(boost::asio::cancellation_type::total);

  ctx_.join();
}

TEST_F(TimerTest, WorksWithoutCancellation) {
  running_ = true;

  boost::asio::spawn(
      strand_,
      [this](boost::asio::yield_context yield) {
        boost::asio::steady_timer timer(
            boost::asio::get_associated_executor(yield));
        boost::system::error_code ec;

        while (running_) {
          timer.expires_after(std::chrono::milliseconds{1});
          timer.async_wait(yield[ec]);

          if (ec == boost::asio::error::operation_aborted or not running_)
            break;
        }
      },
      util::kPROPAGATE_EXCEPTIONS);

  std::this_thread::sleep_for(std::chrono::milliseconds{10});
  running_ = false;

  ctx_.join();
}
