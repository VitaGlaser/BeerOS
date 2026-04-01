#pragma once

#include "components.hpp"

namespace AsnPlus
{
    class Processing : public Components
    {
    private:
        static constexpr const char *TAG = "Processing";

    public:
        Processing() : Components() {}

        void initialize()
        {
            Components::initialize();
        }

        void poll()
        {
            Components::poll();
        }
    };
}
