#include "base-device.hpp"
#include "keybox-proto-types.h"
#include "keybox-errcodes.h"
#include "messages.pb.h"
#include "base64.h"

void BaseDevice::json_rpc_to_protobuf(const std::string &method,
                                      const json &params,
                                      int32_t &errCode,
                                      std::string &errMessage,
                                      uint32_t &messageType,
                                      std::ostream *contentStream)
{
    errCode = 0;

    if (method == "getPublicKeyFromPath")
    {
        messageType = MsgTypeEccGetPublicKeyRequest;
        EccGetPublicKeyRequest req;
        if (!params.is_string())
        {
            errCode = KEYBOX_ERROR_CLIENT_ISSUE;
            errMessage = "you must specify string param to call this function";
            return;
        }
        std::string path = params;

        req.set_hdpath(path);

        req.SerializeToOstream(contentStream);
    }
    else if (method == "getExtendedPubkeyFromPath")
    {
        messageType = MsgTypeEccGetExtendedPubkeyRequest;
        EccGetExtendedPublicKeyRequest req;
        if (!params.is_string())
        {
            errCode = KEYBOX_ERROR_CLIENT_ISSUE;
            errMessage = "you must specify string param to call this function";
            return;
        }
        std::string path = params;

        req.set_hdpath(path);

        req.SerializeToOstream(contentStream);
    }
    else if (method == "signReq")
    {
        messageType = MsgTypeEccSignRequest;
        EccSignRequest req;
        // params
        //
        if (!params.is_object())
        {
            errCode = KEYBOX_ERROR_CLIENT_ISSUE;
            errMessage = "you must specify  params object to call this function";
            return; // cb(KEYBOX_ERROR_CLIENT_ISSUE, "you must specify object param to call this function", r);
        }
        if (params.find("ver") == params.end() || !params["ver"].is_number())
        {
            errCode = KEYBOX_ERROR_CLIENT_ISSUE;
            errMessage = "you must specify ver in this request";
            return; // cb(KEYBOX_ERROR_CLIENT_ISSUE, "you must specify ver in this request", r);
        }
        int version = params["ver"];
        if (version != 1)
        {
            errCode = KEYBOX_ERROR_SERVER_ISSUE;
            errMessage = "can not support specified version, only support version 1 now ";
            return; // cb(KEYBOX_ERROR_SERVER_ISSUE, "can not support specified version, only support version 1 now ", r);
        }

        if (params.find("path") == params.end() || !params["path"].is_string())
        {
            errCode = KEYBOX_ERROR_CLIENT_ISSUE;
            errMessage = "you must specify path in this request";
            return; //  cb(KEYBOX_ERROR_CLIENT_ISSUE, "you must specify path in this request", r);
        }
        std::string path = params["path"];

        if (params.find("hash") == params.end() ||!params["hash"].is_string())
        {
            errCode = KEYBOX_ERROR_CLIENT_ISSUE;
            errMessage = "you must specify hash in this request";
            return; // cb(KEYBOX_ERROR_CLIENT_ISSUE, "you must specify hash in this request", r);
        }
        std::string hash = params["hash"];
        std::string rawhash;
        if (!base64_decode(hash, rawhash))
        {
            errCode = KEYBOX_ERROR_CLIENT_ISSUE;
            errMessage = "the base64 hash is invalid";
            return; // cb(KEYBOX_ERROR_CLIENT_ISSUE, "the base64 hash is invalid", r);
        }
        if (!params["options"].is_object())
        {
            errCode = KEYBOX_ERROR_CLIENT_ISSUE;
            errMessage = "you must specify options object";
            return; // cb(KEYBOX_ERROR_CLIENT_ISSUE, "you must specify options object", r);
        }

        json origin_options = params["options"];
        EccSignOptions *options = new EccSignOptions;

        if (!origin_options["rfc6979"].is_boolean())
        {
            errCode = KEYBOX_ERROR_CLIENT_ISSUE;
            errMessage = "you must specify option rfc6979";
            return; // cb(KEYBOX_ERROR_CLIENT_ISSUE, "you must specify option rfc6979", r);
        }
        options->set_rfc6979(origin_options["rfc6979"]);

        if (!origin_options["graphene_canonize"].is_boolean())
        {
            errCode = KEYBOX_ERROR_CLIENT_ISSUE;
            errMessage = "you must specify option graphene_canonize";
            return; // cb(KEYBOX_ERROR_CLIENT_ISSUE, "you must specify option graphene_canonize", r);
        }
        options->set_graphene_canonize(origin_options["graphene_canonize"]);

        req.set_hdpath(path);
        req.set_hash(rawhash);
        req.set_algorithm(SECP256K1);
        req.set_allocated_options(options);

        req.SerializeToOstream(contentStream);
    }
    else if (method == "multiplyReq")
    {
        messageType = MsgTypeEccMultiplyRequest;
        EccMultiplyRequest req;
        // params
        //
        if (!params.is_object())
        {
            errCode = KEYBOX_ERROR_CLIENT_ISSUE;
            errMessage = "you must specify  params object to call this function";
            return;
        }
        if (params.find("ver") == params.end() || !params["ver"].is_number())
        {
            errCode = KEYBOX_ERROR_CLIENT_ISSUE;
            errMessage = "you must specify ver in this request";
            return;
        }
        int version = params["ver"];
        if (version != 1)
        {
            errCode = KEYBOX_ERROR_SERVER_ISSUE;
            errMessage = "can not support specified version, only support version 1 now ";
            return;
        }

        if (params.find("path") == params.end() || !params["path"].is_string())
        {
            errCode = KEYBOX_ERROR_CLIENT_ISSUE;
            errMessage = "you must specify path in this request";
            return;
        }
        std::string path = params["path"];

        if (params.find("pubkey") == params.end() || !params["pubkey"].is_string())
        {
            errCode = KEYBOX_ERROR_CLIENT_ISSUE;
            errMessage = "you must specify pubkey(base64 encoded string) in this request";
            return;
        }
        std::string pubkey = params["pubkey"];
        std::string raw_pubkey;
        if (!base64_decode(pubkey, raw_pubkey))
        {
            errCode = KEYBOX_ERROR_CLIENT_ISSUE;
            errMessage = "the base64 pubkey is invalid";
            return;
        }

        req.set_hdpath(path);
        req.set_algorithm(SECP256K1);
        req.set_input_pubkey(raw_pubkey);

        req.SerializeToOstream(contentStream);
    }
    else if (method == "getDeviceInfo")
    {
        messageType = MsgTypeGetModeAndVersionRequst;
        GetModeAndVersionRequest req;
        req.SerializeToOstream(contentStream);
    }
    else if (method == "getWalletIdentifier")
    {
        messageType = MsgTypeGetWalletIdentifierRequest;
        GetWalletIdentifierRequest req;
        req.SerializeToOstream(contentStream);
    }
    else if (method == "bitcoinSignReq")
    {
        messageType = MsgTypeBitcoinSignRequest;
        BitcoinSignRequest req;
        if( params.find("psbt") == params.end() || !params["psbt"].is_string()){
            errCode = KEYBOX_ERROR_CLIENT_ISSUE;
            errMessage = "you must specify psbt(base64 encoded string) in this request";
            return;

        }
        std::string psbt = params["psbt"];
        std::string raw_psbt;
        if(!base64_decode(psbt, raw_psbt)){
            errCode = KEYBOX_ERROR_CLIENT_ISSUE;
            errMessage = "the base64 psbt is invalid";
            return;
        }
        req.set_psbt(raw_psbt);
        if( params.find("testnet") == params.end() || !params["testnet"].is_boolean()){
            errCode = KEYBOX_ERROR_CLIENT_ISSUE;
            errMessage = "you must specify testnet flag in this request";
            return;
        }
        if( params.find("coin") == params.end() || !params["coin"].is_string()){
            errCode = KEYBOX_ERROR_CLIENT_ISSUE;
            errMessage = "you must specify coin in this request(btc, ltc, dash, bch)";
            return;
        }
        std::string coin = params["coin"];
        std::transform(coin.begin(), coin.end(), coin.begin(),
            [](unsigned char c){ return std::tolower(c); });
        if( coin == "btc"){
            req.set_coin(BITCOIN);
        }
        else if( coin == "ltc"){
            req.set_coin(LITECOIN);
        }
        else if( coin == "dash"){
            req.set_coin(DASH);
        }
        else if( coin == "bch"){
            req.set_coin(BITCOINCASH);
        }
        else{
            errCode = KEYBOX_ERROR_CLIENT_ISSUE;
            errMessage = "unknown coin in request";
            return;
        }
        req.set_testnet(params["testnet"]);
        req.SerializeToOstream(contentStream);
    }
    else if (method == "ethereumSignReq")
    {
        messageType = MsgTypeEthereumSignRequest;
        EthereumSignRequest req;
        if( params.find("unsignedTx") == params.end() || !params["unsignedTx"].is_string()){
            errCode = KEYBOX_ERROR_CLIENT_ISSUE;
            errMessage = "you must specify unsignedTx(base64 encoded string) in this request";
            return;

        }
        if( params.find("hdPath") == params.end() ||!params["hdPath"].is_string()){
            errCode = KEYBOX_ERROR_CLIENT_ISSUE;
            errMessage = "you must specify path in this request";
            return;

        }
        std::string unsignedTx = params["unsignedTx"];
        std::string raw_tx;
        if(!base64_decode(unsignedTx, raw_tx)){
            errCode = KEYBOX_ERROR_CLIENT_ISSUE;
            errMessage = "the base64 unsignedTx is invalid";
            return;
        }
        req.set_unsignedtx(raw_tx);
        req.set_hdpath(params["hdPath"]);
        req.SerializeToOstream(contentStream);
    }
    else
    {
        errCode = KEYBOX_ERROR_CLIENT_ISSUE;
        errMessage = "unsupported method";
    }
}

void BaseDevice::protobuf_to_json_rpc(const uint32_t messageType,
                                      std::istream *replyContentStream,
                                      const std::string &requestMethod,
                                      int32_t &errCode,
                                      std::string &errMessage,
                                      json &result)
{
    // deserialze now
    errCode = 0;
    errMessage = "OK";
    if (requestMethod == "getPublicKeyFromPath")
    {

        if (messageType == MsgTypeEccGetPublicKeyReply)
        {
            EccGetPublicKeyReply rep;
            if (rep.ParseFromIstream(replyContentStream))
            {
                result["ver"] = 1; // rep.hdpath();
                result["type"] = "ecc-pubkey";
                result["curve"] = "secp256k1";
                result["data"] = base64_encode((uint8_t *)rep.pubkey().data(), rep.pubkey().size());
                // return cb(0, "getOK", r);
            }
            else
            {
                errCode = KEYBOX_ERROR_SERVER_ISSUE;
                errMessage = "parse data from wallet error";
            }
        }
        else
        {
            errCode = KEYBOX_ERROR_SERVER_ISSUE;
            errMessage = "unexpected reply from wallet";
        }
    }
    else if (requestMethod == "getExtendedPubkeyFromPath")
    {

        if (messageType == MsgTypeEccGetExtendedPubkeyReply)
        {
            EccGetExtendedPublicKeyReply rep;
            if (rep.ParseFromIstream(replyContentStream))
            {
                result["ver"] = 1; // rep.hdpath();
                result["type"] = "bip32-extended-pubkey";
                result["curve"] = "secp256k1";
                result["pubkey"] = base64_encode((uint8_t *)rep.pubkey().data(), rep.pubkey().size());
                result["chaincode"] = base64_encode((uint8_t*)rep.chaincode().data(), rep.chaincode().size());
                // return cb(0, "getOK", r);
            }
            else
            {
                errCode = KEYBOX_ERROR_SERVER_ISSUE;
                errMessage = "parse data from wallet error";
            }
        }
        else
        {
            errCode = KEYBOX_ERROR_SERVER_ISSUE;
            errMessage = "unexpected reply from wallet";
        }
    }
    else if (requestMethod == "getWalletIdentifier"){
        if(messageType == MsgTypeGetWalletIdentifierReply)
        {
            GetWalletIdentifierReply rep;
            if (rep.ParseFromIstream(replyContentStream))
            {
                result["bip32MasterKeyId"] = base64_encode((uint8_t*)rep.bip32masterkeyid().data(), rep.bip32masterkeyid().size());
            }
            else{
                errCode = KEYBOX_ERROR_SERVER_ISSUE;
                errMessage = "parse data from wallet error";
            }
        }
        else{
            errCode = KEYBOX_ERROR_SERVER_ISSUE;
            errMessage = "unexpected reply from wallet";
        }
    }
    else if (requestMethod == "signReq")
    {
        if (messageType == MsgTypeEccSignResult)
        {
            EccSignResult reply;
            if (reply.ParseFromIstream(replyContentStream))
            {
                result["ver"] = 1;
                result["curve"] = "secp256k1";
                result["type"] = "ecc-signature";
                result["input-hash"] = base64_encode((uint8_t *)reply.hash().data(), reply.hash().size());
                result["pubkey"] = base64_encode((uint8_t *)reply.pubkey().data(), reply.pubkey().size());
                result["data"] = json::object();
                result["data"]["R"] = base64_encode((uint8_t *)reply.r().data(), reply.r().size());
                result["data"]["S"] = base64_encode((uint8_t *)reply.s().data(), reply.s().size());
                result["data"]["recover_param"] = reply.recover_param();
                //return cb(0, "signOK", r);
            }
            else
            {
                errCode = KEYBOX_ERROR_SERVER_ISSUE;
                errMessage = "parse data from wallet error";
            }
        }
        else
        {
            errCode = KEYBOX_ERROR_SERVER_ISSUE;
            errMessage = "unexpected reply from wallet";
        }
    }
    else if (requestMethod == "bitcoinSignReq")
    {
        if (messageType == MsgTypeBitcoinSignResult)
        {
            BitcoinSignResult reply;
            if (reply.ParseFromIstream(replyContentStream))
            {
                result["ver"] = 1;
                result["type"] = "psbt";
                result["psbt"] = base64_encode((uint8_t *)reply.psbt().data(), reply.psbt().size());
                //return cb(0, "signOK", r);
            }
            else
            {
                errCode = KEYBOX_ERROR_SERVER_ISSUE;
                errMessage = "parse data from wallet error";
            }
        }
        else
        {
            errCode = KEYBOX_ERROR_SERVER_ISSUE;
            errMessage = "unexpected reply from wallet";
        }
    }
    else if (requestMethod == "ethereumSignReq")
    {
        if (messageType == MsgTypeEthereumSignResult)
        {
            EthereumSignResult reply;
            if (reply.ParseFromIstream(replyContentStream))
            {
                result["ver"] = 1;
                result["type"] = "signedTransaction";
                result["signedTx"] = base64_encode((uint8_t *)reply.signedtx().data(), reply.signedtx().size());
                //return cb(0, "signOK", r);
            }
            else
            {
                errCode = KEYBOX_ERROR_SERVER_ISSUE;
                errMessage = "parse data from wallet error";
            }
        }
        else
        {
            errCode = KEYBOX_ERROR_SERVER_ISSUE;
            errMessage = "unexpected reply from wallet";
        }
    }
    else if (requestMethod == "multiplyReq")
    {
        if (messageType == MsgTypeEccMultiplyReply)
        {
            EccMultiplyReply reply;
            if (reply.ParseFromIstream(replyContentStream))
            {
                result["ver"] = 1;
                result["curve"] = "secp256k1";
                result["type"] = "ecc-multiply-result";
                result["input-pubkey"] = base64_encode((uint8_t *)reply.input_pubkey().data(), reply.input_pubkey().size());
                result["dev-pubkey"] = base64_encode((uint8_t *)reply.dev_pubkey().data(), reply.dev_pubkey().size());
                result["data"] = base64_encode((uint8_t *)reply.result().data(), reply.result().size());
                //return cb(0, "multiplyOK", r);
            }
            else
            {
                errCode = KEYBOX_ERROR_SERVER_ISSUE;
                errMessage = "parse data from wallet error";
            }
        }
        else
        {
            errCode = KEYBOX_ERROR_SERVER_ISSUE;
            errMessage = "unexpected reply from wallet";
        }
    }
    else if (requestMethod == "getDeviceInfo")
    {
        // messageType = MsgTypeGetModeAndVersionRequst;
        if (messageType == MsgTypeGetModeAndVersionReply )
        {

            GetModeAndVersionReply reply;
            if (reply.ParseFromIstream(replyContentStream))
            {
                result["ver"] = 1;
                result["type"] = "device-info";
                result["device-serial-no"] = reply.deviceserialno();
                result["firmware-version"] = reply.firmwareversion();
                result["mode"] = reply.mode();
                result["mode_name"] = DeviceMode_Name(reply.mode());
                result["isLocked"] = LockState_Name(reply.islocked());
                //return cb(0, "multiplyOK", r);
            }
            else
            {
                errCode = KEYBOX_ERROR_SERVER_ISSUE;
                errMessage = "parse data from wallet error";
            }
        }
        else
        {
            errCode = KEYBOX_ERROR_SERVER_ISSUE;
            errMessage = "unexpected reply from wallet";
        }
    }
}