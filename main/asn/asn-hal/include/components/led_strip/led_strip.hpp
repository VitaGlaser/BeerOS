#pragma once

#include "asn/asn-core/array.hpp"
#include "asn/asn-core/pixel.hpp"

#include "led_strip_communication.hpp"

namespace AsnPlus
{
    template< uint16_t PIXEL_COUNT >
    class LedStrip
    {
    public:
        LedStrip( LedStripCommunication & communication ) : _communication( communication ) {}

        bool initialize()
        {
            Log::info( "Initializing" );
            bool ret = _communication.initialize();
            Log::info( "Initialized" );
            return ret;
        }

        void poll()
        {
            Log::debug( "Poll" );
            for ( uint16_t i = 0; i < PIXEL_COUNT; ++i )
            {
                switch ( _pixels[ i ].getColorModel() )
                {
                    case ColorModel::RGB:
                        _communication.setPixel( i, _pixels[ i ].getRgb() );
                        break;
                    case ColorModel::HSV:
                        _communication.setPixel( i, _pixels[ i ].getHsv() );
                        break;
                    default:
                        {
                            Log::error( "Unsupported color model" );
                            break;
                        }
                }
            }
            _communication.poll();
        }

        uint16_t size() const { return PIXEL_COUNT; }

        void setPixel( uint32_t index, Rgb color )
        {
            Log::debug( "Set pixel %d to (%d, %d, %d)", index, color.red, color.green, color.blue );
            _pixels[ index ] = Pixel { color };
        }

        void setPixel( uint32_t index, Hsv color )
        {
            Log::debug( "Set pixel %d to (%d, %d, %d)", index, color.hue, color.saturation, color.value );
            _pixels[ index ] = Pixel { color };
        }

        void fill( Rgb color )
        {
            Log::debug( "Fill to (%d, %d, %d)", color.red, color.green, color.blue );
            _pixels.fill( Pixel { color } );
        }

        void fill( Hsv color )
        {
            Log::debug( "Fill to (%d, %d, %d)", color.hue, color.saturation, color.value );
            _pixels.fill( Pixel { color } );
        }

        void clear()
        {
            Log::debug( "Clear" );
            _pixels.fill( Pixel {} );
        }

    private:
        static constexpr const char TAG[] = "LedStrip";
        using Log                         = Logger< ModuleConfig::Hal::LOG_LEVEL, TAG >;

        LedStripCommunication & _communication;

        Array< Pixel, PIXEL_COUNT > _pixels {};
    };
}    // namespace AsnPlus
