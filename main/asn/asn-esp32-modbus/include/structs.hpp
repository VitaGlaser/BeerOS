#pragma once

#include <cstdint>

namespace AsnPlus::Modbus
{
    enum class Commands : uint8_t
    {
        READ_COILS = 0x01,
        READ_DISCRETE_INPUTS,
        READ_HOLDING_REGISTERS,
        READ_INPUT_REGISTERS,
        WRITE_SINGLE_COIL,
        WRITE_SINGLE_HOLDING_REGISTER,
        WRITE_MULTIPLE_COILS = 0x0F,
        WRITE_MULTIPLE_HOLDING_REGISTERS
    };

    enum class RegisterType : uint8_t
    {
        COIL,
        DISCRETE_INPUT,
        INPUT_REGISTER,
        HOLDING_REGISTER,
    };

    class Register
    {
    public:
        Register( RegisterType type, uint16_t address, uint16_t & data) :
            _data( data ),
            _type( type ),
            _address( address )
        {
        }

        RegisterType getType() { return _type; }

        uint16_t getAddress() { return _address; }

        uint16_t & ref() { return _data; }

        uint16_t get() { return _data; }

        void set( uint16_t data ) { _data = data; }

        void setBit( uint16_t bit, bool value )
        {
            if ( value )
            {
                _data |= ( 1 << bit );
            }
            else
            {
                _data &= ~( 1 << bit );
            }
        }

        bool getBit( uint16_t bit ) { return ( _data & ( 1 << bit ) ) != 0; }

    protected:
        uint16_t & _data;

    private:
        RegisterType _type;
        uint16_t     _address;
    };

    class HoldingRegister : public Register
    {
    public:
        HoldingRegister( uint16_t address, uint16_t & data ) :
            Register( RegisterType::HOLDING_REGISTER, address, data )
        {
        }
    };

    class InputRegister : public Register
    {
    public:
        InputRegister( uint16_t address, uint16_t & data ) : Register( RegisterType::INPUT_REGISTER, address, data )
        {
        }
    };

}    // namespace AsnPlus::Modbus
