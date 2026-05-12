#pragma once

#include "asn_module_config.hpp"

#include "asn-at-parser/include/at_command.hpp"
#include "asn-at-parser/include/at_interface.hpp"
#include "asn-at-parser/include/at_uart.hpp"
#include "asn-at-parser/include/urc_processor.hpp"

#include "asn/asn-core/array.hpp"
#include "asn/asn-core/logger.hpp"
#include "asn/asn-core/string.hpp"
#include "asn/asn-core/string_util.hpp"
#include "asn/asn-core/types.hpp"
#include "asn/asn-core/vector.hpp"

#include "asn/asn-hal/include/modem/modem.hpp"
#include "asn/asn-hal/include/peripherals/gpio.hpp"

#include "binary_read_at_command.hpp"
#include "exec_at_command.hpp"
#include "prompt_at_command.hpp"
#include "read_at_command.hpp"

namespace AsnPlus
{
    class Eg915 : public IModem
    {
    public:
        enum class RegistrationStatus
        {
            NOTREGISTERED      = 0,
            REGISTEREDHOME     = 1,
            ATTACHINGSEARCHING = 2,
            DENIED             = 3,
            UNKNOWN            = 4,
            REGISTEREDROAMING  = 5
        };

        enum class SslVersion : uint8_t
        {
            SSL_3_0 = 0,
            TLS_1_0 = 1,
            TLS_1_1 = 2,
            TLS_1_2 = 3,
            ALL     = 4
        };

        enum class CipherSuite : uint16_t
        {
            TLS_RSA_WITH_AES_256_CBC_SHA            = 0x0035,
            TLS_RSA_WITH_AES_128_CBC_SHA            = 0x002F,
            TLS_RSA_WITH_RC4_128_SHA                = 0x0005,
            TLS_RSA_WITH_RC4_128_MD5                = 0x0004,
            TLS_RSA_WITH_3DES_EDE_CBC_SHA           = 0x000A,
            TLS_RSA_WITH_AES_256_CBC_SHA256         = 0x003D,
            TLS_ECDH_ECDSA_WITH_RC4_128_SHA         = 0xC002,
            TLS_ECDH_ECDSA_WITH_3DES_EDE_CBC_SHA    = 0xC003,
            TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA     = 0xC004,
            TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA     = 0xC005,
            TLS_ECDHE_ECDSA_WITH_RC4_128_SHA        = 0xC007,
            TLS_ECDHE_ECDSA_WITH_3DES_EDE_CBC_SHA   = 0xC008,
            TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA    = 0xC009,
            TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA    = 0xC00A,
            TLS_ECDHE_RSA_WITH_RC4_128_SHA          = 0xC011,
            TLS_ECDHE_RSA_WITH_3DES_EDE_CBC_SHA     = 0xC012,
            TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA      = 0xC013,
            TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA      = 0xC014,
            TLS_ECDH_RSA_WITH_RC4_128_SHA           = 0xC00C,
            TLS_ECDH_RSA_WITH_3DES_EDE_CBC_SHA      = 0xC00D,
            TLS_ECDH_RSA_WITH_AES_128_CBC_SHA       = 0xC00E,
            TLS_ECDH_RSA_WITH_AES_256_CBC_SHA       = 0xC00F,
            TLS_ECDHE_ECDSA_WITH_AES_128_CBC_SHA256 = 0xC023,
            TLS_ECDHE_ECDSA_WITH_AES_256_CBC_SHA384 = 0xC024,
            TLS_ECDH_ECDSA_WITH_AES_128_CBC_SHA256  = 0xC025,
            TLS_ECDH_ECDSA_WITH_AES_256_CBC_SHA384  = 0xC026,
            TLS_ECDHE_RSA_WITH_AES_128_CBC_SHA256   = 0xC027,
            TLS_ECDHE_RSA_WITH_AES_256_CBC_SHA384   = 0xC028,
            TLS_ECDH_RSA_WITH_AES_128_CBC_SHA256    = 0xC029,
            TLS_ECDH_RSA_WITH_AES_256_CBC_SHA384    = 0xC02A,
            TLS_ECDHE_RSA_WITH_AES_128_GCM_SHA256   = 0xC02F,
            TLS_ECDHE_RSA_WITH_AES_256_GCM_SHA384   = 0xC030,
            SUPPORT_ALL                             = 0xFFFF
        };

        enum class SslSecurityLevel : uint8_t
        {
            NO_AUTHENTICATION      = 0,
            SERVER_AUTHENTICATION  = 1,
            SERVER_AND_CLIENT_AUTH = 2
        };

        using IntermediateLineCallback = ReadAtCommand::IntermediateLineCallback;

        explicit Eg915( AtUart & atUart );

        // MARK: IModem

        bool   initialize() override;
        void   poll();
        bool   configureApn( StringView apn, StringView username, StringView password ) override;
        bool   attach() override;
        Status getStatus() const override;

        // MARK: Network

        bool activatePdpContext();

        // MARK: SSL

        bool configureSsl(
            SslSecurityLevel seclevel,
            SslVersion       version      = SslVersion::ALL,
            CipherSuite      cipher       = CipherSuite::SUPPORT_ALL,
            bool             sessionCache = true
        );

        // MARK: HTTP

        bool configureHttpContext();
        bool httpSetUrl( StringView url, int timeout_s = 80 );
        bool httpGet( int timeout_s, uint16_t & response_code, size_t & response_length );
        bool httpPost(
            ArrayView< const uint8_t > data,
            int                        send_timeout_s,
            int                        recv_timeout_s,
            uint16_t &                 response_code,
            size_t &                   response_length
        );
        bool httpRead( IVector< uint8_t > & dest );
        bool configureHttpSsl();
        bool configureHttpContentType( uint8_t type );

        // MARK: SSL Socket

        bool sslOpen( StringView host, uint16_t port );
        bool sslClose();
        bool sslSend( ArrayView< const uint8_t > data );
        bool sslReceive( IVector< uint8_t > & dest, size_t max_len, size_t & received );
        bool waitForSslDataOrClose( uint32_t timeout_ms );
        bool isSslDataAvailable() const;
        bool isSslClosed() const;
        void clearSslDataFlag();

        // MARK: Accessors

        bool               isReady() const;
        bool               isSimCardReady() const;
        bool               isFullFunctionality() const;
        RegistrationStatus getRegistrationStatus() const;
        AtInterface &      at();
        bool               setPowerKeyGpio( IGpio & gpio );

    private:
        // =====================================================================
        // MARK: Constants
        // =====================================================================

        static constexpr const char TAG[]               = "Eg915";
        using Log                                       = Logger< ModuleConfig::Eg915::LOG_LEVEL, TAG >;

        static constexpr uint32_t COMMAND_TIMEOUT_MS    = 5000;
        static constexpr uint32_t CREG_POLL_INTERVAL_MS = 5000;

        static constexpr uint8_t SSL_CTX_ID             = 1;
        static constexpr uint8_t SSL_OPEN_CTX_ID        = 1;
        static constexpr uint8_t SSL_CLIENT_ID          = 0;
        static constexpr uint8_t PDP_CTX_ID             = 1;

        // =====================================================================
        // MARK: Data members
        // =====================================================================

        IGpio * _powerKey                               = nullptr;

        AtUart &       _atUart;
        AtInterface    _atInterface;
        UrcProcessor & _urcProcessor;

        Vector< uint8_t, ModuleConfig::Eg915::DATA_BUFFER_SIZE > _dataBuffer;

        ATCommand           _atCommand;
        ATCommand           _cmeeCommand;
        ExecAtCommand       _qigetErrorCommand;
        ReadAtCommand       _cpinCommand;
        ATCommand           _cfunCommand;
        ATCommand           _qsclkCommand;
        ATCommand           _cpsmsCommand;
        ReadAtCommand       _cregCommand;
        ATCommand           _qicsgpCommand;
        ATCommand           _qiactCommand;
        ReadAtCommand       _qiactStatusCommand;
        ATCommand           _qsslcfgCommand;
        ATCommand           _qhttpcfgCommand;
        ATCommand           _httpGetCommand;
        PromptAtCommand     _httpSetUrlCommand;
        PromptAtCommand     _httpPostCommand;
        BinaryReadAtCommand _httpReadCommand;
        ATCommand           _qsslopenCommand;
        ReadAtCommand       _qsslstateCommand;
        ATCommand           _qsslcloseCommand;
        PromptAtCommand     _qsslsendCommand;
        BinaryReadAtCommand _qsslrecvCommand;

        bool               _deviceReady    = false;
        bool               _cpinReady      = false;
        bool               _pdpActive      = false;
        uint32_t           _lastCregPollMs = 0;
        int                _cfunStatus     = 0;
        RegistrationStatus _cregStatus     = RegistrationStatus::NOTREGISTERED;
        Status             _status         = Status::UNINITIALIZED;

        bool     _httpResponseReady        = false;
        bool     _httpResponseOk           = false;
        uint16_t _httpResponseCode         = 0;
        size_t   _httpResponseLength       = 0;

        bool _qiactContextActive           = false;

        bool _sslOpenReady                 = false;
        bool _sslOpenOk                    = false;
        bool _sslDataAvailable             = false;
        bool _sslClosed                    = false;

        // =====================================================================
        // MARK: Helpers
        // =====================================================================

        bool        _parseHttpReadHeader( ArrayView< uint8_t > header, size_t & dataLength, size_t & remaining );
        static bool _parseSslRecvHeader( ArrayView< uint8_t > header, size_t & dataLength, size_t & remaining );
        bool        _sendAndWait( ATCommand & command, uint32_t timeoutMs = COMMAND_TIMEOUT_MS );
        void        _logTcpIpError();
        bool        _waitRegistration( uint32_t timeoutMs );
        bool        _waitFlag( bool & flag, uint32_t timeoutMs, uint32_t checkInterval = 10 );

        // =====================================================================
        // MARK: URC handlers
        // =====================================================================

        void _httpGetUrcHandler( StringView urc );
        void _httpPostUrcHandler( StringView urc );
        void _httpReadUrcHandler( StringView urc );
        void _parseHttpResponseUrc( StringView urc );
        void _qigetErrorIntermediateHandler( ArrayView< uint8_t > line );
        void _rdyHandler( StringView urc );
        void _cpinRdyHandler( StringView urc );
        void _cpinIntermediateHandler( ArrayView< uint8_t > line );
        void _qiactIntermediateHandler( ArrayView< uint8_t > line );
        void _cregIntermediateHandler( ArrayView< uint8_t > line );
        void _cfunHandler( StringView urc );
        void _cregHandler( StringView urc );
        void _ipUrcHandler( StringView urc );
        void _qsslopenUrcHandler( StringView urc );
        void _qsslurcHandler( StringView urc );

        // =====================================================================
        // MARK: Status
        // =====================================================================

        static const char * _statusToString( Status status );
        void                _checkStatus();
        static Status       _registrationStatusToModemStatus( RegistrationStatus status );
        Status              _determineStatus();
    };
}    // namespace AsnPlus
