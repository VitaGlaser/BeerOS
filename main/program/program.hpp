#pragma once

#include "processing.hpp"

namespace AsnPlus
{
    class Program
    {
    private:
        static constexpr const char *TAG = "Program";

    public:
        static void main()
        {
            instance().run();
        }

        static Program &instance()
        {
            static Program instance{};
            return instance;
        }

    protected:
        Processing processing{};

        void run()
        {
            processing.initialize();
            while (1)
            {
                processing.poll();
            }
        }
    };
}
