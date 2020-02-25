#include <iostream>
#include <boost/asio.hpp>
#include <boost/bind.hpp>
#include <boost/date_time/posix_time/posix_time.hpp>




void print(const boost::system::error_code& /*e*/, boost::asio::steady_timer* t, int* count)
{
    std::cout << "Current count:" << *count << std::endl;

    if (*count < 5)
    {
        ++(*count);
        t->expires_at(t->expiry() + boost::asio::chrono::seconds(1));
        t->async_wait(boost::bind(print,
            boost::asio::placeholders::error, t, count));
    }
}

int main()
{
    boost::asio::io_service io;

    int count = 0;
    boost::asio::steady_timer t(io, boost::asio::chrono::seconds(1));
    std::cout << "Running!.."<< std::endl;

    t.async_wait(boost::bind(print,
            boost::asio::placeholders::error, &t, &count));
    io.run();

    std::cout << "Final count is :" << count << std::endl;

    return 0;
}