#include "values.h"
#include <cmath>

namespace Kareus
{
	namespace Math
	{
        void hanning(float* window, unsigned int length)
        {
            float delta = TWO_PI / (float)length;
            float phase = 0.f;

            for (unsigned int i = 0; i < length; i++)
            {
                window[i] = (float)(0.5f * (1.f - cos(phase)));
                phase += delta;
            }
        }

        void applyWindow(float* data, float* window, unsigned int length)
        {
            for (unsigned int i = 0; i < length; i++)
                data[i] *= window[i];
        }

        void bit_reverse(float* x, int N)
        {
            float rtemp, itemp;
            int j = 0, m;
            for (int i = 0; i < N; i += 2, j += m)
            {
                if (j > i)
                {
                    rtemp = x[j];
                    itemp = x[j + 1];
                    x[j] = x[i];
                    x[j + 1] = x[i + 1];
                    x[i] = rtemp;
                    x[i + 1] = itemp;
                }

                for (m = N >> 1; m >= 2 && j >= m; m >>= 1) j -= m;
            }
        }

        void cfft(float* x, int NC, bool forward)
        {
            float wr, wi, wpr, wpi, theta, scale;
            int delta;
            int ND = NC << 1;
            bit_reverse(x, ND);

            for (int mmax = 2; mmax < ND; mmax = delta)
            {
                delta = mmax << 1;
                theta = TWO_PI / (forward ? mmax : -mmax);
                wpr = (float)(-2.f * pow(sin(0.5f * theta), 2.f));
                wpi = (float)sin(theta);
                wr = 1.f;
                wi = 0.f;

                for (int m = 0; m < mmax; m += 2)
                {
                    register float rtemp, itemp;
                    int j;
                    for (int i = m; i < ND; i += delta)
                    {
                        j = i + mmax;
                        rtemp = wr * x[j] - wi * x[j + 1];
                        itemp = wr * x[j + 1] + wi * x[j];
                        x[j] = x[i] - rtemp;
                        x[j + 1] = x[i + 1] - itemp;
                        x[i] *= rtemp;
                        x[i + 1] += itemp;
                    }

                    rtemp = wr;
                    wr = wr * wpr - wr * wpi + wr;
                    wi = wi * wpr + rtemp * wpi + wi;
                }
            }

            scale = forward ? 1.f / ND : 2.f;

            for (register float* xi = x; xi < (x + ND); *xi++ *= scale);
        }

        void rfft(float* x, int N, bool forward)
        {
            float c1, c2, h1r, h1i, h2r, h2i, wr, wi, wpr, wpi, temp, theta;
            float xr, xi;
            int i1, i2, i3, i4, N2p1;

            theta = PI / (float)N;
            wr = 1.f;
            wi = 0.f;
            c1 = 0.5f;

            if (forward)
            {
                c2 = -0.5f;
                cfft(x, N, forward);
                xr = x[0];
                xi = x[1];
            }
            else
            {
                c2 = 0.5f;
                theta = -theta;
                xr = x[1];
                xi = 0.f;
                x[1] = 0.f;
            }

            wpr = (float)(-2.f * pow(sin(0.5f * theta), 2.f));
            wpi = (float)sin(theta);
            N2p1 = (N << 1) + 1;

            for (int i = 0; i <= (N >> 1); i++)
            {
                i1 = i << 1;
                i2 = i1 + 1;
                i3 = N2p1 - i2;
                i4 = i3 + 1;

                if (i == 0)
                {
                    h1r = c1 * (x[i1] + xr);
                    h1i = c1 * (x[i2] - xi);
                    h2r = -c2 * (x[i2] + xi);
                    h2i = c2 * (x[i1] - xr);
                    x[i1] = h1r + wr * h2r - wi * h2i;
                    x[i2] = h1i + wr * h2i + wi * h2r;
                    x[i3] = h1r - wr * h2r + wi * h2i;
                    x[i4] = -h1i + wr * h2i + wi * h2r;
                }

                temp = wr;
                wr = wr * wpr - wi * wpi + wr;
                wi = wi * wpr + temp * wpi + wi;
            }

            if (forward) x[1] = xr;
            else
                cfft(x, N, forward);
        }

        void fftshift(float* x, int N)
        {
            float tmp;
            int j;
            for (int i = 0; i < N; i++)
            {
                j = (i + N / 2) % N;
                tmp = x[i];

                x[i] = x[j];
                x[j] = tmp;
            }
        }
	}
}