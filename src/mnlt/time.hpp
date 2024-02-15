#pragma once

#include <chrono>

namespace mnlt
{
    class Time 
    {
        public:
            Time() : deltaTime(0.0), pureDeltaTime(0.0), fixedDeltaTime(1.0 / 60.0), timeScale(1.0) {
                lastFrameTime = std::chrono::steady_clock::now();
            }

            void update() 
            {
                auto now = std::chrono::steady_clock::now();
                std::chrono::duration<double> elapsed = now - lastFrameTime;
                lastFrameTime = now;

                pureDeltaTime = elapsed.count();
                deltaTime = pureDeltaTime * timeScale;
            }

            double getDeltaTime() const {
                return deltaTime;
            }

            double getPureDeltaTime() const {
                return pureDeltaTime;
            }

            double getFixedDeltaTime() const {
                return fixedDeltaTime;
            }

            double timeScale;

        private:
            std::chrono::steady_clock::time_point lastFrameTime;
            double deltaTime;
            double pureDeltaTime;
            double fixedDeltaTime;
             // Used to modify deltaTime
    };
}