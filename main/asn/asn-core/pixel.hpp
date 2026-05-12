#pragma once

#include "types.hpp"

namespace AsnPlus
{
    enum class ColorModel : uint8_t
    {
        RGB,
        HSV,
        CMYK
    };

    struct Rgb
    {
        uint8_t red;
        uint8_t green;
        uint8_t blue;

        bool operator==( const Rgb & other ) const
        {
            return red == other.red && green == other.green && blue == other.blue;
        }
    };

    struct Hsv
    {
        uint8_t hue;
        uint8_t saturation;
        uint8_t value;

        static uint16_t hueToDegree( uint8_t h )
        {
            const uint32_t num = static_cast< uint32_t >( h ) * 360u + 127u;
            return static_cast< uint16_t >( ( num / 255u ) % 360u );
        }

        static uint8_t hueFromDegree( uint16_t deg )
        {
            if ( deg > 360u ) deg = 360u;
            const uint32_t num = static_cast< uint32_t >( deg ) * 255u;
            return static_cast< uint8_t >( num / 360u );
        }

        bool operator==( const Hsv & other ) const
        {
            return hue == other.hue && saturation == other.saturation && value == other.value;
        }
    };

    struct Cmyk
    {
        uint8_t cyan;
        uint8_t magenta;
        uint8_t yellow;
        uint8_t key;

        bool operator==( const Cmyk & other ) const
        {
            return cyan == other.cyan && magenta == other.magenta && yellow == other.yellow && key == other.key;
        }
    };

    class Pixel
    {
    public:
        Pixel() : _colorModel( ColorModel::RGB ) {}

        Pixel( const Rgb & rgb ) : _colorModel( ColorModel::RGB ), _rgb( rgb ) { _colorModel = ColorModel::RGB; }

        Pixel( const Hsv & hsv ) : _colorModel( ColorModel::HSV ), _hsv( hsv ) { _colorModel = ColorModel::HSV; }

        void setRgb( const Rgb & rgb )
        {
            _colorModel = ColorModel::RGB;
            _rgb        = rgb;
        }

        void setHsv( const Hsv & hsv )
        {
            _colorModel = ColorModel::HSV;
            _hsv        = hsv;
        }

        ColorModel getColorModel() const { return _colorModel; }

        Rgb getRgb() const
        {
            if ( _colorModel != ColorModel::RGB ) return Rgb {};
            return _rgb;
        }

        Hsv getHsv() const
        {
            if ( _colorModel != ColorModel::HSV ) return Hsv {};
            return _hsv;
        }

    private:
        ColorModel _colorModel = ColorModel::RGB;

        Rgb _rgb {};
        Hsv _hsv {};
    };
}    // namespace AsnPlus
