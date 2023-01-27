#pragma once

namespace MckDsp {
    class OnePoleFilter {
        public:
            void prepareToPlay(double sampleRate, int samplesPerBlock);

            double processSample(double in);

            void setLPF(double freq);

            void setHPF(double freq);

            void setBypass(bool bypass);

        private:
            double m_sampleRate { 0 };

            double m_history { 0.0 };
            
            double m_histIn { 0.0 };

            double m_histOut { 0.0 };

            double m_a0 { 1.0 };

            double m_a1 { 0.0 };

            double m_b1 { 0.0 };

            bool m_bypass { false };
    };
}