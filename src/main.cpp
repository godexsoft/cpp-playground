#include <boost/asio/error.hpp>
#include <boost/asio/io_context.hpp>
#include <boost/asio/spawn.hpp>
#include <boost/asio/steady_timer.hpp>

#include <chrono>
#include <memory>
#include <semaphore>
#include <thread>

int main() {
    boost::asio::io_context ioContext;

    std::thread runnerThread([&]() { ioContext.run(); });
    (void)boost::asio::spawn(ioContext, [&](boost::asio::yield_context yield) {
        boost::asio::steady_timer timer(ioContext);
        timer.expires_after(std::chrono::seconds(10));
        boost::system::error_code ec;
        timer.async_wait(yield[ec]);
    });

    std::this_thread::sleep_for(std::chrono::milliseconds(2000));
    ioContext.stop();
    runnerThread.join();
}
