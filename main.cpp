#include "json-rpc-interface.hpp"
#include "json-rpc-server-factory.hpp"
#include "rpc-server.hpp"
#include "device-manager.hpp"
#include "local-device.hpp"
#include "libusb.h"
// #include <boost/thread/thread.hpp>

using tcp = boost::asio::ip::tcp;              // from <boost/asio/ip/tcp.hpp>
namespace websocket = boost::beast::websocket; // from <boost/beast/websocket.hpp>
using json = nlohmann::json;

void usb_func()
{
    while (1)
    {
        libusb_handle_events(NULL);
    }
}

int main(int argc, char *argv[])
{

    int ret = libusb_init(NULL);
    if (ret)
    {

        return 1;
    }
    std::thread uthread = std::thread(usb_func);
    rpc_server server;

    auto const address = boost::asio::ip::make_address("127.0.0.1");
    auto const port = static_cast<unsigned short>(23045);
    auto const threads = 2;

    boost::asio::io_context ioc{threads};
    DeviceManager *devManager = DeviceManager::getDeviceManager(&ioc);
    std::string wallet_path = "/tmp/keybox_wallet_server";
    LocalDevice *localDev = new LocalDevice(wallet_path, &ioc);
    devManager->addDevice(localDev);

    std::make_shared<json_rpc_server_factory>(ioc, tcp::endpoint{address, port}, &server)->run();

    std::vector<std::thread> v;
    v.reserve(threads - 1);
    for (auto i = threads - 1; i > 0; --i)
        v.emplace_back(
            [&ioc] {
                ioc.run();
            });
    ioc.run();

    uthread.join();
    libusb_exit(NULL);
    return EXIT_SUCCESS;
}
