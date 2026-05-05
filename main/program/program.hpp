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
            instance().processing = new Processing();
            instance().run();
        }

        static Program &instance()
        {
            static Program instance{};
            return instance;
        }

    protected:
        Processing * processing = nullptr;

        void run()
        {
            processing->initialize();
            while (1)
            {
                processing->poll();
            }
        }
    };
}
