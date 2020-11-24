//Source : https://arduino.stackexchange.com/a/66687

class BlinkLCD {

  private:
    byte m_blinkState;
    unsigned long m_dataToBlink, m_previousMillis;
    char m_text[16 + 1];

  public:
    BlinkLCD(): m_blinkState(0), m_startPosition(0), m_dataLength(0),
      m_dataToBlink(0), m_previousMillis(0), m_blinkRate(500),
      m_text(), m_tekst() {}

    String m_tekst;
    byte m_startPosition, m_dataLength;
    unsigned long m_blinkRate;
    byte m_row;

    void Update() {
      unsigned long m_currentMillis = millis();

      if (m_currentMillis - m_previousMillis >= m_blinkRate) {


        m_blinkState = !m_blinkState;
        m_previousMillis = m_currentMillis;

        lcd.setCursor(m_startPosition, m_row);

        if (m_blinkState) {
          if (m_tekst != NULL) {
            lcd.print(m_tekst);
          }
          else {
            lcd.print(m_dataToBlink);
          }
        }
        else {
          for (byte i = 0; i < m_dataLength; i++) {
            lcd.print(" ");
          }
        }
      }
    }
    void SetBlinkRate(unsigned long blinkRate) {
      m_blinkRate = blinkRate;
    }

    void SetNumber(unsigned long dataToBlink) {
      m_dataToBlink = dataToBlink;
    }

    void SetString(String stringToBlink) {
      m_tekst = stringToBlink;
    }

    void SetLength(byte dataLength) {
      m_dataLength = dataLength;
    }

    // LCD top row, columns 0 - 15
    // LCD Bottom row, 16 - 31
    void SetStartPosition(byte startPosition, byte row) {
      m_startPosition = startPosition;
      m_row = row;
    }

    void SetText(char text[]) {
      strcpy(m_text, text);
    }
};
