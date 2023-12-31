#include <Receiver.h>


namespace Radio
{
    bool __g_preamble = false;
    bool __g_received = false;
    unsigned long __g_received_millis = 0;

    void Receiver::tickerCounter(Receiver *receiver)
    {
       receiver->tickCounter += 1;
       receiver->preambleDetected = __g_preamble;
       receiver->crcReceivedOk = __g_received;
       receiver->crcReceivedMillis = __g_received_millis;

       if (receiver->preambleDetected)
       {
            receiver->preCounter += 1;
            if ((receiver->preCounter * SM_GRANULARITY_US) >= SM_PREAMBLE_RECOVERY_TIMEOUT_US) // Avoid hanging on a too long preamble detect 
            {
                receiver->f_lock = false;
                receiver->preCounter = 0;
                Radio::clearFlags();
                return;
            }
            receiver->f_lock = true;
            receiver->tickCounter = 0;
       }
       else
       {
            if ((receiver->tickCounter * SM_GRANULARITY_US) >= RESET_AFTER_LAST_MSG_US)
                receiver->f_lock = false;
       }
    }

    void Receiver::manageStates(Receiver *receiver)
    {
        if (receiver->crcReceivedOk)
        {
            receiver->getPacket();
        }
    }

    void Receiver::Init()
    {
        Radio::setCarrier(Radio::Carrier::Deviation, 19200);
        Radio::setCarrier(Radio::Carrier::Bitrate, 38400);
        Radio::setCarrier(Radio::Carrier::Bandwidth, 250);
        Radio::setCarrier(Radio::Carrier::Modulation, Radio::Modulation::FSK);

        Radio::initRx();
    }

    bool Receiver::getPacket()
    {
        iohc.buffer_length = 0;
        iohc.frequency = scan_freqs[currentFreq];
        iohc.millis = crcReceivedMillis;
        iohc.rssi = (float)(readByte(REG_RSSIVALUE)/2)*-1;
        
        while (!Radio::dataAvail())
            ;
        do
        {
            digitalWrite(RX_LED, digitalRead(RX_LED)^1);
            iohc.packet.buffer[iohc.buffer_length] = Radio::readByte(REG_FIFO);
//            Serial.printf("%2.2x", iohc.packet.buffer[iohc.buffer_length]);
            iohc.buffer_length += 1;
        } while (Radio::dataAvail());
        Radio::clearFlags();
        if (packetCB)
            packetCB(&iohc);
        digitalWrite(RX_LED, digitalRead(RX_LED)^1);
        return true;
    }

    void Receiver::Start()
    {
        TickTimer.attach_us(SM_GRANULARITY_US, tickerCounter, this);
        StateMachine.attach_us(SM_CHECK_US, manageStates, this);
        Receiver::Scan(true);

        // Attach interrupts to Preamble detected and end of packet sent/received
        attachInterrupt(RADIO_PREAMBLE_DETECTED, i_preamble, CHANGE);
        attachInterrupt(RADIO_PACKET_AVAIL, i_payload, CHANGE);

        Radio::clearBuffer();
        Radio::clearFlags();
        Radio::setRx();
    }

    void Receiver::Stop()
    {
        Receiver::Scan(false);
        Radio::setStandby();

        detachInterrupt(RADIO_PACKET_AVAIL);
        detachInterrupt(RADIO_PREAMBLE_DETECTED);

        StateMachine.detach();
        TickTimer.detach();
    }

    void Receiver::Scan(bool enabled)
    {
        if (fHopping == enabled) return;
        if (!enabled || !scanTimeUs)
        {
            if (fHopping)
            {
                FreqScanner.detach();
                fHopping = false;
            }
            else
            {
                Radio::setCarrier(Radio::Carrier::Frequency, scan_freqs[0]);
                fHopping = false;
                return;
            }
        }

        fHopping = true;
        FreqScanner.attach_us(scanTimeUs, [&](void)->void{
            if (f_lock) return;
            digitalWrite(SCAN_LED, 0);
            currentFreq += 1;
            if (currentFreq >= num_freqs)
                currentFreq = 0;
            Radio::setCarrier(Radio::Carrier::Frequency, scan_freqs[currentFreq]);
            digitalWrite(SCAN_LED, 1);
        });
    }

    void IRAM_ATTR Receiver::i_preamble() {
        __g_preamble = digitalRead(RADIO_PREAMBLE_DETECTED) ? true:false;
    }

    void IRAM_ATTR Receiver::i_payload() {
            __g_received = digitalRead(RADIO_PACKET_AVAIL) ? true:false;
            if (__g_received)
                __g_received_millis = millis();
    }
}