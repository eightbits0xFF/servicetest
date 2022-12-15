// servicetest.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include <boost/asio.hpp>
#include <boost/noncopyable.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/thread.hpp>

template <typename Services>
class basicWritter : private boost::noncopyable{

public:
    typedef Services service_type;
    typedef typename service_type::implement impl_t;

    explicit  basicWritter(boost::asio::execution_context & context, std::string id):
        service_{boost::asio::use_
        service<Services>(context)},
        impl{service_.null()}
    {
        service_.create( impl, id );
    }

    void write_some(std::string text) {
        service_.write(impl,text);
    }

    ~basicWritter() {
        service_.destroy(impl);
    }

private:
    service_type& service_;
    impl_t impl;
};

class writter_service : public boost::asio::execution_context::service {

public:
    typedef writter_service key_type;

    struct writter_impl {

        explicit writter_impl(std::string& idt) : identifier{idt} {

        }
        std::string identifier;
    };
    
    typedef writter_impl* implement;

    writter_service(boost::asio::execution_context & context):
        boost::asio::execution_context::service{ context },
        work_io_context_(),
        work_{boost::asio::make_work_guard(work_io_context_)},
        work_thread_{ new boost::thread(
            boost::bind(&boost::asio::io_context::run, &work_io_context_))}
    {
        

    }


    implement null() const {
        return {};
    }

    ~writter_service() {
        work_.reset();
        if (work_thread_)
            work_thread_->join();
    }

    void create(implement& impl, std::string& id) {
        impl = new writter_impl(id);
    }

    void destroy(implement& impl) {
        delete impl;
        impl = null();
    }

    void shutdown() {

    }

    void write(implement& impl, std::string message) {

        boost::asio::post(work_io_context_, boost::bind(&writter_service::write_impl, this, message + " " + impl->identifier));
    }
private:
    void write_impl(std::string message) {
        std::cout << "Works " << __LINE__ <<" " << message << "\n";
    }
    boost::asio::io_context work_io_context_;

    boost::asio::executor_work_guard<
        boost::asio::io_context::executor_type> work_;

    boost::scoped_ptr<boost::thread> work_thread_;

};

typedef basicWritter<writter_service> writter;

int main()
{
    boost::asio::io_context contxt;
    
    writter wr{ contxt, "FIRST" };

    wr.write_some("What is your name");

    contxt.run();
    
}
