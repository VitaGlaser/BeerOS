#pragma once

#include "asn_module_config.hpp"

#include "asn/asn-core/types.hpp"

#include "asn/asn-core/logger.hpp"

#include "esp_adc/adc_cali.h"
#include "esp_adc/adc_cali_scheme.h"
#include "esp_adc/adc_continuous.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"
#include "freertos/task.h"
#include <stddef.h>
#include <stdio.h>

namespace AsnPlus::Esp32
{
    class AdcOneShot
    {
    public:
        static constexpr const char TAG[] = "AdcOneShot";

        enum class Channel
        {
            Channel_0 = ADC_CHANNEL_0,
            Channel_1 = ADC_CHANNEL_1,
            Channel_2 = ADC_CHANNEL_2,
            Channel_3 = ADC_CHANNEL_3,
            Channel_4 = ADC_CHANNEL_4,
            Channel_5 = ADC_CHANNEL_5,
            Channel_6 = ADC_CHANNEL_6,
            Channel_7 = ADC_CHANNEL_7,
            Channel_8 = ADC_CHANNEL_8,
            Channel_9 = ADC_CHANNEL_9,
        };

        AdcOneShot( adc_unit_t unit ) : _unit( unit ), _adc_handle( nullptr )
        {
            for ( int i = 0; i < 10; ++i )
            {
                _calibration_channel_handle[ i ] = nullptr;
            }
        }

        ~AdcOneShot()
        {
            if ( _adc_handle )
            {
                adc_oneshot_del_unit( _adc_handle );
            }

            for ( auto & handle : _calibration_channel_handle )
            {
                if ( handle )
                {
                    adc_cali_delete_scheme_curve_fitting( handle );
                }
            }
        }

        void initialize()
        {
            adc_oneshot_unit_init_cfg_t adc_init_config = {
                .unit_id = _unit,
            };

            ESP_ERROR_CHECK( adc_oneshot_new_unit( &adc_init_config, &_adc_handle ) );
        }

        void initialize_channel( Channel channel )
        {
            adc_oneshot_chan_cfg_t config = { .atten = ADC_ATTEN_DB_12, .bitwidth = ADC_BITWIDTH_DEFAULT };

            ESP_ERROR_CHECK(
                adc_oneshot_config_channel( _adc_handle, static_cast< adc_channel_t >( channel ), &config )
            );

            adc_cali_curve_fitting_config_t calibration_config = {
                .unit_id  = _unit,
                .chan     = static_cast< adc_channel_t >( channel ),
                .atten    = ADC_ATTEN_DB_12,
                .bitwidth = ADC_BITWIDTH_DEFAULT,
            };

            adc_cali_create_scheme_curve_fitting(
                &calibration_config, &_calibration_channel_handle[ static_cast< int >( channel ) ]
            );
        }

        u16 readRaw( Channel channel )
        {
            int adc_raw = 0;
            adc_oneshot_read( _adc_handle, static_cast< adc_channel_t >( channel ), &adc_raw );

            return adc_raw;
        }

        i32 readVoltage( Channel channel )
        {
            int voltage = 0;
            int adc_raw = readRaw( channel );

            adc_cali_raw_to_voltage( _calibration_channel_handle[ static_cast< int >( channel ) ], adc_raw, &voltage );

            return voltage;
        }

    private:
        using Log = Logger< ModuleConfig::Esp32::LOG_LEVEL, TAG >;
        adc_unit_t                _unit;
        adc_oneshot_unit_handle_t _adc_handle;
        adc_cali_handle_t         _calibration_channel_handle[ 10 ];
    };

    using AdcChannelIndex = uint8_t;

    class AdcContinuous
    {
        static constexpr int        MAX_CHANNELS    = 10;
        static constexpr int        DMA_BUFFER_SIZE = 4096;
        static constexpr int        ADC_READ_LEN    = 1024;
        static constexpr const char TAG[]           = "AdcContinuous";

        class Channel
        {
        public:
            uint32_t get_sample( uint16_t index ) { return _adc_continuous.get_sample( _channel, index ); }

            uint16_t samples_count() { return _adc_continuous.samples_count(); }

            bool is_valid() { return _channel != -1; }

        private:
            Channel( AdcContinuous & adc_continuous, AdcChannelIndex channel ) :
                _adc_continuous( adc_continuous ),
                _channel( channel )
            {
            }

            AdcContinuous & _adc_continuous;
            AdcChannelIndex _channel;

            friend AdcContinuous;
        };

    public:
        AdcContinuous( adc_unit_t unit, adc_atten_t atten, uint32_t sampling_freq ) :
            _unit( unit ),
            _atten( atten ),
            _sampling_frequency( sampling_freq ),
            _channel_count( 0 )
        {
        }

        void initialize()
        {
            adc_continuous_handle_cfg_t adc_config = {
                .max_store_buf_size = DMA_BUFFER_SIZE,
                .conv_frame_size    = 256,
            };

            ESP_ERROR_CHECK( adc_continuous_new_handle( &adc_config, &_adc_handle ) );

            // TODO: calibration
            adc_cali_curve_fitting_config_t calibration_config = {
                .unit_id  = _unit,
                .atten    = ADC_ATTEN_DB_12,
                .bitwidth = ADC_BITWIDTH_DEFAULT,
            };
            adc_cali_create_scheme_curve_fitting( &calibration_config, &_caliHandle );
        }

        AdcChannelIndex add_channel( adc_channel_t channel )
        {
            if ( _channel_count >= MAX_CHANNELS )
            {
                Log::error( "Channel count exceeds maximum" );
                return -1;
            }

            adc_digi_pattern_config_t adc_pattern = {
                .atten     = _atten,
                .channel   = channel,
                .unit      = _unit,
                .bit_width = SOC_ADC_DIGI_MAX_BITWIDTH,
            };
            _adc_pattern[ _channel_count ] = adc_pattern;
            _channel_count++;
            reconfigure();
            return _channel_count - 1;
        }

        void reconfigure()
        {
            adc_continuous_config_t dig_cfg = {
                .pattern_num    = _channel_count,
                .adc_pattern    = _adc_pattern,
                .sample_freq_hz = _sampling_frequency,
                .conv_mode      = ADC_CONV_SINGLE_UNIT_1,
                .format         = ADC_DIGI_OUTPUT_FORMAT_TYPE2,
            };

            ESP_ERROR_CHECK( adc_continuous_config( _adc_handle, &dig_cfg ) );
        }

        void start() { ESP_ERROR_CHECK( adc_continuous_start( _adc_handle ) ); }

        void stop() { ESP_ERROR_CHECK( adc_continuous_stop( _adc_handle ) ); }

        void poll()
        {
            esp_err_t ret;
            ret = adc_continuous_read( _adc_handle, _read_values, ADC_READ_LEN, &_read_values_len, 0 );
            if ( ret == ESP_OK )
            {
                // Log::debug("Read %lu bytes", _read_values_len );
            }
            else
            {
                Log::error( "adc_continuous_read failed: %s", esp_err_to_name( ret ) );
            }
        }

        uint32_t get_sample( AdcChannelIndex channel_index, uint16_t index )
        {
            adc_digi_output_data_t * p = (adc_digi_output_data_t *) &_read_values
                [ ( index * _channel_count + channel_index ) * SOC_ADC_DIGI_RESULT_BYTES ];

            uint32_t data = p->type2.data;
            // return ( data * 3300 ) / 4095;    // TODO: convert via calibration

            int voltage   = 0;
            adc_cali_raw_to_voltage( _caliHandle, data, &voltage );
            return static_cast< uint32_t >( voltage );
        }

        uint16_t total_samples_count() { return _read_values_len / SOC_ADC_DIGI_RESULT_BYTES; }

        uint16_t samples_count() { return _read_values_len / SOC_ADC_DIGI_RESULT_BYTES / _channel_count; }

        uint32_t sampling_frequency() { return _sampling_frequency; }

        uint32_t channel_sampling_frequency() { return _sampling_frequency / _channel_count; }

    private:
        using Log = Logger< ModuleConfig::Esp32::LOG_LEVEL, TAG >;

        adc_unit_t                _unit;
        adc_atten_t               _atten;
        uint32_t                  _sampling_frequency;
        adc_digi_pattern_config_t _adc_pattern[ MAX_CHANNELS ];
        uint8_t                   _channel_count;

        adc_continuous_handle_t _adc_handle;
        adc_cali_handle_t       _caliHandle;

        uint8_t  _read_values[ ADC_READ_LEN ];
        uint32_t _read_values_len;
    };

    // class AdcContinuous
    // {
    // public:
    //     static constexpr const char * TAG = "ADC_CONTINUOUS";
    //     TaskHandle_t                  task_handle;

    //     struct ChannelConfig
    //     {
    //         adc_channel_t channel;
    //         adc_atten_t   atten;
    //     };

    //     AdcContinuous(
    //         adc_unit_t unit,
    //         size_t     max_channels,
    //         u32        conv_frame_size,
    //         u32        max_store_buf_size,
    //         u32        sample_freq_hz
    //     ) :
    //         _unit( unit ),
    //         _adc_handle( nullptr ),
    //         _caliHandle( nullptr ),
    //         _max_channels( max_channels ),
    //         _conv_frame_size( conv_frame_size ),
    //         _max_store_buf_size( max_store_buf_size ),
    //         _sample_freq_hz( sample_freq_hz )
    //     {
    //         task_handle          = nullptr;
    //         _channel_count       = 0;
    //         _calibration_enabled = false;
    //     }

    //     ~AdcContinuous()
    //     {
    //         if ( _adc_handle )
    //         {
    //             adc_continuous_stop( _adc_handle );
    //             adc_continuous_deinit( _adc_handle );
    //         }

    //         if ( _caliHandle )
    //         {
    //             adc_cali_delete_scheme_curve_fitting( _caliHandle );
    //         }
    //     }

    //     void initialize()
    //     {
    //         adc_continuous_handle_cfg_t adc_config = {
    //             .max_store_buf_size = _max_store_buf_size,
    //             .conv_frame_size    = _conv_frame_size,
    //         };

    //         ESP_ERROR_CHECK( adc_continuous_new_handle( &adc_config, &_adc_handle ) );

    //         adc_continuous_config_t dig_cfg = {
    //             .sample_freq_hz = _sample_freq_hz,
    //             .conv_mode      = ADC_CONV_SINGLE_UNIT_1,
    //             .format         = ADC_DIGI_OUTPUT_FORMAT_TYPE2,
    //         };

    //         adc_digi_pattern_config_t adc_pattern[ _max_channels ];
    //         dig_cfg.pattern_num = _channel_count;

    //         for ( size_t i = 0; i < _channel_count; ++i )
    //         {
    //             adc_pattern[ i ].atten     = _channels[ i ].atten;
    //             adc_pattern[ i ].channel   = _channels[ i ].channel;
    //             adc_pattern[ i ].unit      = _unit;
    //             adc_pattern[ i ].bit_width = SOC_ADC_DIGI_MAX_BITWIDTH;

    //             Log::debug("Channel %d: atten=%d, unit=%d", _channels[ i ].channel, _channels[ i ].atten, _unit
    //             );
    //         }

    //         dig_cfg.adc_pattern = adc_pattern;
    //         ESP_ERROR_CHECK( adc_continuous_config( _adc_handle, &dig_cfg ) );

    //         _initializeCalibration();
    //     }

    //     void addChannel( adc_channel_t channel, adc_atten_t atten )
    //     {
    //         if ( _channel_count >= _max_channels )
    //         {
    //             Log::error("Cannot add more channels. Max channels reached." );
    //             return;
    //         }

    //         _channels[ _channel_count++ ] = { channel, atten };
    //     }

    //     void start()
    //     {
    //         adc_continuous_evt_cbs_t cbs = {
    //             .on_conv_done = AdcContinuous::_conversionDoneCallback,
    //         };

    //         ESP_ERROR_CHECK( adc_continuous_register_event_callbacks( _adc_handle, &cbs, this ) );
    //         ESP_ERROR_CHECK( adc_continuous_start( _adc_handle ) );
    //     }

    //     void stop() { ESP_ERROR_CHECK( adc_continuous_stop( _adc_handle ) ); }

    //     size_t readVoltage( int * values, size_t max_results, u16 reference_voltage, adc_channel_t specific_channel )
    //     {
    //         u8     result[ _conv_frame_size ];
    //         u32    ret_num = 0;
    //         size_t count   = 0;

    //         while ( count < max_results )
    //         {
    //             esp_err_t ret = adc_continuous_read( _adc_handle, result, _conv_frame_size, &ret_num, 100 );

    //             if ( ret == ESP_OK )
    //             {
    //                 for ( u32 i = 0; i < ret_num; i += SOC_ADC_DIGI_RESULT_BYTES )
    //                 {
    //                     adc_digi_output_data_t * data = (adc_digi_output_data_t *) &result[ i ];

    //                     if ( data->type2.channel < SOC_ADC_CHANNEL_NUM( _unit ) && count < max_results )
    //                     {
    //                         if ( data->type2.channel == specific_channel )
    //                         {
    //                             int raw = data->type2.data;

    //                             if ( _calibration_enabled )
    //                             {
    //                                 int voltage = 0;
    //                                 if ( adc_cali_raw_to_voltage( _caliHandle, raw, &voltage ) == ESP_OK )
    //                                 {
    //                                     values[ count ] = voltage;
    //                                 }
    //                                 else
    //                                 {
    //                                     ESP_LOGW( TAG, "Failed to calibrate raw data: %d", raw );
    //                                     values[ count ] = -1;    // Označení chybného měření
    //                                 }
    //                             }
    //                             else
    //                             {
    //                                 values[ count ] = raw * reference_voltage / 4095;    // Výpočet bez kalibrace
    //                             }
    //                             count++;
    //                         }
    //                     }
    //                 }
    //             }
    //             else if ( ret == ESP_ERR_TIMEOUT )
    //             {
    //                 break;
    //             }
    //             else
    //             {
    //                 ESP_LOGW( "ADC", "Error reading ADC continuous: %s", esp_err_to_name( ret ) );
    //             }
    //         }

    //         return count;
    //     }

    // private:
    //     void _initializeCalibration()
    //     {
    //         adc_cali_curve_fitting_config_t cali_config = {
    //             .unit_id  = _unit,
    //             .atten    = _channels[ 0 ].atten,    // Použití útlumu z prvního kanálu
    //             .bitwidth = ADC_BITWIDTH_DEFAULT,
    //         };

    //         if ( adc_cali_create_scheme_curve_fitting( &cali_config, &_caliHandle ) == ESP_OK )
    //         {
    //             _calibration_enabled = true;
    //             Log::debug("ADC calibration enabled" );
    //         }
    //         else
    //         {
    //             _calibration_enabled = false;
    //             ESP_LOGW( TAG, "ADC calibration not available, using raw data" );
    //         }
    //     }

    //     static bool IRAM_ATTR _conversionDoneCallback(
    //         adc_continuous_handle_t           handle,
    //         const adc_continuous_evt_data_t * edata,
    //         void *                            user_data
    //     )
    //     {
    //         auto *     instance  = static_cast< AdcContinuous * >( user_data );
    //         BaseType_t mustYield = pdFALSE;

    //         if ( instance && instance->task_handle )
    //         {
    //             vTaskNotifyGiveFromISR( instance->task_handle, &mustYield );
    //         }

    //         return ( mustYield == pdTRUE );
    //     }

    //     adc_unit_t              _unit;
    //     adc_continuous_handle_t _adc_handle;
    //     adc_cali_handle_t       _caliHandle;

    //     const size_t  _max_channels;
    //     size_t        _channel_count;
    //     ChannelConfig _channels[ 10 ];

    //     u32 _conv_frame_size;
    //     u32 _max_store_buf_size;
    //     u32 _sample_freq_hz;

    //     bool _calibration_enabled;
    // };
}    // namespace AsnPlus::Esp32
