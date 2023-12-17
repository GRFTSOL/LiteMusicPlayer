#include "DspDemo.h"


CDspDemo::CDspDemo() {
    OBJ_REFERENCE_INIT;
    m_pPlayer = nullptr;
}

CDspDemo::~CDspDemo() {

}

ResultCode CDspDemo::init(IMPlayer *pPlayer) {
    m_pPlayer = pPlayer;

    return ERR_OK;
}

ResultCode CDspDemo::quit() {
    assert(m_pPlayer);
    m_pPlayer.release();

    m_pPlayer = nullptr;

    return ERR_OK;
}

short echo_buf[65536], echo_buf2[65536];

void CDspDemo::process(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate) {
    processVoice(pBuf, nBps, nChannels, nSampleRate);

    return;
    /*

        int        nSamples = pBuf->size() / (nBps / 8) / nChannels;
        if (nBps == 16)
        {
            short *buf = (short *)pBuf->data();
    //          for (int i = 0; i < nSamples; i++)
    //          {
    //             int        x = buf[i * 2] / 2;
    //              buf[i * 2] = x;
    //              // if (buf[i * 2 + 1] > 0)
    //              //    buf[i * 2 + 1] /= 2;
    //          }
             int        x, n;
             n = nSamples * nChannels;

             memcpy(echo_buf2,       echo_buf,       n * 2);
             memcpy(echo_buf,        echo_buf+n,     n * 2);
             memcpy(echo_buf+n,    echo_buf+n * 2, n * 2);
             memcpy(echo_buf+n*2,  echo_buf+n*3,   n * 2);
             memcpy(echo_buf+n*3,  buf, n * 2);

             for (x = 0; x < n; x ++)
             {
                     int s = buf[x]/2+echo_buf2[x]/2;
                     buf[x] = (s>32767?32767:s<-32768?-32768:s);
             }
        }*/

}

void CDspDemo::processEcho(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate) {
    // remove left
    int nSamples = pBuf->size() / (nBps / 8) / nChannels;
    if (nBps == 16) {
        short *buf = (short *)pBuf->data();
        //          for (int i = 0; i < nSamples; i++)
        //          {
        //             int        x = buf[i * 2] / 2;
        //              buf[i * 2] = x;
        //              // if (buf[i * 2 + 1] > 0)
        //              //    buf[i * 2 + 1] /= 2;
        //          }
        int x, n;
        n = nSamples * nChannels;

        memcpy(echo_buf2, echo_buf, n * 2);
        memcpy(echo_buf, echo_buf+n, n * 2);
        memcpy(echo_buf+n, echo_buf+n * 2, n * 2);
        memcpy(echo_buf+n*2, echo_buf+n*3, n * 2);
        memcpy(echo_buf+n*3, buf, n * 2);

        for (x = 0; x < n; x ++) {
            int s = buf[x]/2+echo_buf2[x]/2;
            buf[x] = (s>32767?32767:s<-32768?-32768:s);
        }
    }
}

int g_pitch = 100;

void CDspDemo::processPitch(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate) {
    int nSamples = pBuf->size() / (nBps / 8) / nChannels;
    if (nBps == 16) {
        int pitch = g_pitch;
        int rlen = pBuf->size();
        int index = 0, x;
        int n;
        int dindex;
        if (g_pitch == 100) return;
        if (g_pitch > 200) g_pitch=200;
        if (g_pitch < 50) g_pitch=50;
        pitch = 100000 / pitch;
        n = (nSamples * pitch) / 1000;
        dindex=(nSamples<<14)/n;
        if ((int)m_buffPitch.capacity() < rlen) {
            m_buffPitch.reserve(rlen);
        }
        if (nBps == 16 && nChannels == 2) {
            short *buf = (short*)m_buffPitch.data();
            pBuf->reserve(n * nBps / 8 * nChannels + 4);
            pBuf->resize(n * nBps / 8 * nChannels);
            short *bufData = (short *)pBuf->data();

            memcpy(buf, bufData, rlen);
            for (x = 0; x < n; x++) {
                int p = (index >> 14) << 1;
                index+=dindex;
                bufData[0] = buf[p];
                bufData[1] = buf[p+1];
                bufData+=2;
            }
            return;
        } else if (nBps == 16 && nChannels == 1) {
            short *buf = (short*)m_buffPitch.data();
            pBuf->reserve(n * nBps / 8 * nChannels + 4);
            pBuf->resize(n * nBps / 8 * nChannels);
            short *bufData = (short *)pBuf->data();

            memcpy(buf,bufData,rlen);
            for (x = 0; x < n; x ++) {
                int p=(index>>14);
                index+=dindex;
                *bufData++ = buf[p];
            }
            return;
        }
        return;
    }
}

void CDspDemo::processFadeInout(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate) {
    int n = pBuf->size() / (nBps / 8);
    if (nBps == 16) {
        int nFadeDuration = 500; // ms
        int nFadeSamplesCount = nSampleRate * nFadeDuration / 1000;
        static int nDoneSamples = 0;
        static int nDir = 1;

        short *buf = (short *)pBuf->data();
        for (int i = 0; i < n; i++) {
            int s = buf[i] * nDoneSamples / nFadeSamplesCount;
            buf[i] = (s>32767?32767:s<-32768?-32768:s);
            i++;
            s = buf[i] * nDoneSamples / nFadeSamplesCount;
            buf[i] = (s>32767?32767:s<-32768?-32768:s);
            nDoneSamples+=nDir;
            if (nDoneSamples >= nFadeSamplesCount) {
                nDir = -1;
            }
            if (nDoneSamples <= 0) {
                nDir = 1;
            }
            // if (buf[i * 2 + 1] > 0)
            //    buf[i * 2 + 1] /= 2;
        }
        /*         int        x, n;
         n = nSamples * nChannels;

         memcpy(echo_buf2,       echo_buf,       n * 2);
         memcpy(echo_buf,        echo_buf+n,     n * 2);
         memcpy(echo_buf+n,    echo_buf+n * 2, n * 2);
         memcpy(echo_buf+n*2,  echo_buf+n*3,   n * 2);
         memcpy(echo_buf+n*3,  buf, n * 2);

         for (x = 0; x < n; x ++)
         {
                 int s = buf[x]/2+echo_buf2[x]/2;
                 buf[x] = (s>32767?32767:s<-32768?-32768:s);
         }*/
    }
}

void CDspDemo::processVoice(IFBuffer *pBuf, int nBps, int nChannels, int nSampleRate) {
    return;
    //     int        n = pBuf->size() / (nBps / 8);
    //     if (nBps == 16)
    //     {
    //         short *buf = (short *)pBuf->data();
    //           for (int i = 0; i < n; i++)
    //           {
    //             uint32_t    s = buf[i] & buf[i + 1];
    //             buf[i] = buf[i] & ~s;
    //             buf[i+1] = buf[i+1] & ~s;
    //             i++;
    //           }
    // /*         int        x, n;
    //          n = nSamples * nChannels;
    //
    //          memcpy(echo_buf2,       echo_buf,       n * 2);
    //          memcpy(echo_buf,        echo_buf+n,     n * 2);
    //          memcpy(echo_buf+n,    echo_buf+n * 2, n * 2);
    //          memcpy(echo_buf+n*2,  echo_buf+n*3,   n * 2);
    //          memcpy(echo_buf+n*3,  buf, n * 2);
    //
    //          for (x = 0; x < n; x ++)
    //          {
    //                  int s = buf[x]/2+echo_buf2[x]/2;
    //                  buf[x] = (s>32767?32767:s<-32768?-32768:s);
    //          }*/
    //     }
}
