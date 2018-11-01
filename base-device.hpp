#ifndef _DEVICE_HEADER_INCLUDE_
#define _DEVICE_HEADER_INCLUDE_

#include <nlohmann/json.hpp>

using json = nlohmann::json;

using DevCallbackFn = std::function< void  (int32_t errCode, const std::string &errMesssage, const json & result) >;

class BaseDevice {
public:
    virtual bool connect() = 0;
    virtual void disconnect() = 0;
    virtual bool isConnected() = 0;
    virtual const std::string deviceId() = 0;
    virtual void call_async(const std::string & method, const json & params, DevCallbackFn  cb) = 0;
    virtual ~BaseDevice(){

    };
protected:
    // from client, prepare to send to wallet
    void json_rpc_to_protobuf(const std::string & method,
     const json & params, 
     int32_t &errCode, 
     std::string & errMessage, 
     uint32_t &messageType,
     std::ostream *contentStream );

     // from wallet, prepare to send back to client
     void protobuf_to_json_rpc(const uint32_t messageType,
        std::istream *replyContentStream,
        const std::string & requestMethod,
        int32_t & errCode,
        std::string & errMessage,
        json & result
     );
};


#endif 
