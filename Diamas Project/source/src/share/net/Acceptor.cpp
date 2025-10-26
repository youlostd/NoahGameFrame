#include <net/Acceptor.hpp>
#include <SpdLog.hpp>



Acceptor::Acceptor(
                   boost::asio::io_service& ioService)
	: m_ioService(ioService)
	, m_acceptor(ioService)
	, m_socket(ioService)
{
	// ctor
}

void Acceptor::StartAccept()
{
	auto f = [this] (const boost::system::error_code& ec) {
		// Check whether the server was stopped by a signal before this
		// completion handler had a chance to run.
		if (!m_acceptor.is_open())
			return;

		if (!ec) {
			boost::system::error_code ec;
			m_socket.non_blocking(true, ec);

	        Protocol::acceptor::linger opt(false, 0);
	        m_acceptor.set_option(opt, ec);

            m_handler(std::move(m_socket));
	
		}
		else
			SPDLOG_ERROR("Accepting on {0} failed with {1}",
			          m_acceptor.local_endpoint().address().to_string(), ec.message());

		StartAccept();
	};

	m_acceptor.async_accept(m_socket, f);
}

void Acceptor::Close()
{
	m_acceptor.close();
}


